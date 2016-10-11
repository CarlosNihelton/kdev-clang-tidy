/*
 * This file is part of KDevelop
 *
 * Copyright 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "perprojectconfigpage.h"
#include <KSharedConfig>
#include <interfaces/iproject.h>
#include <QMessageBox>
#include <algorithm>

namespace ClangTidy
{

PerProjectConfigPage::PerProjectConfigPage(KDevelop::IPlugin* plugin, KDevelop::IProject* project, const QStringList& checks, QWidget* parent)
    : KDevelop::ConfigPage(plugin, new PerProjectSettings, parent),
      ui(new Ui::PerProjectConfig()), m_underlineAvailChecks(checks)
{
    configSkeleton()->setSharedConfig(project->projectConfiguration());
    configSkeleton()->load();
    m_projectSettings = dynamic_cast<PerProjectSettings*>(configSkeleton());
    ui->setupUi(this);

    m_availableChecksModel = new QStringListModel();
    m_availableChecksModel->setStringList(m_underlineAvailChecks);
    ui->checkListView->setModel(m_availableChecksModel);
    m_selectedItemModel = new QItemSelectionModel(m_availableChecksModel);
    ui->checkListView->setSelectionModel(m_selectedItemModel);

//     m_config = project->projectConfiguration()->group(configSkeleton()->currentGroup());
    loadSelectedChecksFromConfig();
    updateSelectedChecksView();

    connect(m_selectedItemModel, &QItemSelectionModel::selectionChanged, this, &PerProjectConfigPage::changed);
}

QIcon PerProjectConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("dialog-ok"));
}

QString PerProjectConfigPage::name() const
{
    return i18n("clang-tidy");
}

void PerProjectConfigPage::loadSelectedChecksFromConfig(){

    m_selectedChecks.clear();
    QString fromConfig(m_projectSettings->enabledChecks());

    //In case it's passed patterns ending with *, as supported by clang-tidy, we need to know exactly what checks to 
    // exhibit as selected in the UI.
    if(fromConfig.contains('*')){
        //Copy any element of the collection of available clang-tidy checks if its name starts with any of the 
        //patterns suggested in the configuration.
        const QStringList splitted(fromConfig.remove(QChar('*')).split(','));
        std::copy_if(m_underlineAvailChecks.begin(), m_underlineAvailChecks.end(), 
                     std::back_inserter(m_selectedChecks),

                    [&splitted](const QString& check){
                        return std::any_of(splitted.begin(), splitted.end(),

                                            [&check](const QString& selected){
                                                return check.startsWith(selected);
                                            });
                     });
    } else {
        m_selectedChecks << fromConfig.split(',');
    }

    m_selectedChecks.removeDuplicates();
}

void PerProjectConfigPage::updateSelectedChecksView(){
    for (int i = 0; i < m_availableChecksModel->rowCount(); ++i) {
        QModelIndex index = m_availableChecksModel->index(i, 0);
        if (index.isValid()) {
            if (m_selectedChecks.contains((index.data().toString()))) {
                m_selectedItemModel->select(index, QItemSelectionModel::Select);
            } else {
                m_selectedItemModel->select(index, QItemSelectionModel::Deselect);
            }
        }
    }
}

void PerProjectConfigPage::joinChecks(){
    m_selectedChecks.clear();
    auto selectedList(m_selectedItemModel->selectedIndexes());
    for(auto const& index : selectedList){
        m_selectedChecks << index.data(0).toString();
    }
    m_selectedChecks.removeDuplicates();
    if (m_selectedChecks.at(0).isEmpty()) {
        m_selectedChecks.removeFirst();
    }

    ui->kcfg_EnabledChecks->setText(m_selectedChecks.join(','));
}

void PerProjectConfigPage::apply()
{
    joinChecks();

    emit this->changed();

    KDevelop::ConfigPage::apply();
    emit selectedChecksChanged(m_selectedChecks);

}

void PerProjectConfigPage::defaults(){
    KDevelop::ConfigPage::defaults();
    bool restore = m_projectSettings->useDefaults(true);
    loadSelectedChecksFromConfig();
    updateSelectedChecksView();
    m_projectSettings->useDefaults(restore);
    emit this->changed();

}

void PerProjectConfigPage::reset(){
    KDevelop::ConfigPage::reset();
    loadSelectedChecksFromConfig();
    updateSelectedChecksView();
}

} // namespace ClangTidy

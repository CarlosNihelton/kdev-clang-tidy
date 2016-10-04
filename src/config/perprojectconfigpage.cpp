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

#include "config/perprojectconfigpage.h"

#include <KSharedConfig>
#include <interfaces/iproject.h>

namespace ClangTidy
{

PerProjectConfigPage::PerProjectConfigPage(KDevelop::IProject* project, QWidget* parent)
    : KDevelop::ConfigPage(nullptr, nullptr, parent)
    , ui(new Ui::PerProjectConfig())
{
    ui->setupUi(this);

    m_availableChecksModel = new QStringListModel();
    ui->checkListView->setModel(m_availableChecksModel);

    m_selectedItemModel = new QItemSelectionModel(m_availableChecksModel);
    ui->checkListView->setSelectionModel(m_selectedItemModel);

    m_config = project->projectConfiguration()->group("ClangTidy");
}

QIcon PerProjectConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("emblem-checked"));
}

KDevelop::ConfigPage::ConfigPageType PerProjectConfigPage::configPageType() const
{
    return ConfigPage::AnalyzerConfigPage;
}

QString PerProjectConfigPage::name() const
{
    return i18n("clang-tidy");
}

void PerProjectConfigPage::setList(const QStringList& list)
{

    m_underlineAvailChecks = list;
    m_availableChecksModel->setStringList(m_underlineAvailChecks);

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

void PerProjectConfigPage::apply()
{
    // TODO (coliveira): discover a way to set the project folders where user header files
    // might exist into this option. Right now it only works with manual entry.
    m_config.writeEntry(ConfigGroup::HeaderFilter, ui->headerFilterText->text());
    m_config.writeEntry(ConfigGroup::AdditionalParameters, ui->clangTidyParameters->text());
    m_config.enableEntry(ConfigGroup::CheckSystemHeaders, ui->sysHeadersCheckBox->isChecked());
    m_config.enableEntry(ConfigGroup::UseConfigFile, !ui->overrideConfigFileCheckBox->isChecked());
    m_config.enableEntry(ConfigGroup::DumpConfig, ui->dumpCheckBox->isChecked());

    for (int i = 0; i < m_availableChecksModel->rowCount(); ++i) {
        QModelIndex index = m_availableChecksModel->index(i, 0);
        if (index.isValid()) {
            bool isSelected = m_selectedItemModel->isSelected(index);
            auto check = index.data().toString();
            if (isSelected) {
                m_selectedChecks << check;
            } else {
                m_selectedChecks.removeAll(check);
            }
        }
    }
    m_selectedChecks.removeDuplicates();
    if (m_selectedChecks.at(0).isEmpty()) {
        m_selectedChecks.removeFirst();
    }
    m_config.writeEntry(ConfigGroup::EnabledChecks, m_selectedChecks.join(','));
    emit selectedChecksChanged(m_selectedChecks);
}

void PerProjectConfigPage::defaults()
{
    bool wasBlocked = signalsBlocked();
    blockSignals(true);

    m_config.writeEntry(ConfigGroup::ExecutablePath, CLANG_TIDY_PATH);

    // TODO: discover a way to set the project folders where user header files
    // might exist into this option. Right now it only works with manual entry.
    m_config.writeEntry(ConfigGroup::HeaderFilter, "");
    ui->headerFilterText->setText("");

    m_config.writeEntry(ConfigGroup::AdditionalParameters, "");
    ui->clangTidyParameters->setText(QString(""));

    m_config.writeEntry(ConfigGroup::CheckSystemHeaders, "");
    ui->sysHeadersCheckBox->setChecked(false);

    m_config.enableEntry(ConfigGroup::UseConfigFile, false);
    ui->overrideConfigFileCheckBox->setChecked(true);
    ui->checkListGroupBox->setEnabled(true);

    m_config.enableEntry(ConfigGroup::DumpConfig, true);
    ui->dumpCheckBox->setChecked(true);
    ui->checkListGroupBox->setEnabled(true);

    m_config.enableEntry(ConfigGroup::ExportFixes, true);
    //     ui->autoFixCheckBox->setChecked(true);

    for (int i = 0; i < m_availableChecksModel->rowCount(); ++i) {
        QModelIndex index = m_availableChecksModel->index(i, 0);
        if (index.isValid()) {
            auto check = index.data().toString();
            bool enable = check.contains("cert") || check.contains("-core.") || check.contains("-cplusplus")
                || check.contains("-deadcode") || check.contains("-security") || check.contains("cppcoreguide");
            m_selectedItemModel->select(index, enable ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
            if (enable) {
                m_selectedChecks << check;
            } else {
                m_selectedChecks.removeAll(check);
            }
        }
    }
    m_selectedChecks.removeDuplicates();
    m_config.writeEntry(ConfigGroup::EnabledChecks, m_selectedChecks.join(','));
    blockSignals(wasBlocked);
}

void PerProjectConfigPage::reset()
{
    if (!m_config.isValid()) {
        return;
    }
    ui->headerFilterText->setText(m_config.readEntry(ConfigGroup::HeaderFilter).remove("--header-filter="));
    ui->clangTidyParameters->setText(m_config.readEntry(ConfigGroup::AdditionalParameters));
    ui->sysHeadersCheckBox->setChecked(!m_config.readEntry(ConfigGroup::CheckSystemHeaders).isEmpty());
    ui->overrideConfigFileCheckBox->setChecked(m_config.readEntry(ConfigGroup::UseConfigFile).isEmpty());
    ui->checkListGroupBox->setEnabled(m_config.readEntry(ConfigGroup::UseConfigFile).isEmpty());
    ui->dumpCheckBox->setChecked(!m_config.readEntry(ConfigGroup::DumpConfig).isEmpty());
}

} // namespace ClangTidy

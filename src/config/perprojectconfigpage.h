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

#ifndef CLANGTIDY_PERPROJECTCONFIGPAGE_H_
#define CLANGTIDY_PERPROJECTCONFIGPAGE_H_

#include "perprojectsettings.h"
#include "ui_perprojectconfig.h"

#include <QItemSelectionModel>
#include <QObject>
#include <QStringListModel>
#include <interfaces/configpage.h>

class QIcon;
class QStringListModel;

namespace KDevelop
{
class IProject;
}

namespace ClangTidy
{
class Plugin;

namespace Ui
{
    class PerProjectConfig;
}
/**
 * \class
 * \brief Implements the clang-tidy's configuration project for the current
 * project.
 */
class PerProjectConfigPage : public KDevelop::ConfigPage
{
    Q_OBJECT

public:
    PerProjectConfigPage(KDevelop::IPlugin* plugin, KDevelop::IProject* project, const QStringList& checks, QWidget* parent);
    ~PerProjectConfigPage() override = default;

    QString name() const override;
    QIcon icon() const override;

signals:
    void selectedChecksChanged(const QStringList& selectedChecks);

public slots:
    void apply() override;
    void defaults() override;
    void reset() override;
protected:
    void joinChecks();
    void loadSelectedChecksFromConfig();
    void updateSelectedChecksView();
private:
    QScopedPointer<Ui::PerProjectConfig> ui;
    PerProjectSettings* m_projectSettings;
    QStringList m_selectedChecks;
    QStringList m_underlineAvailChecks;
    QStringListModel* m_availableChecksModel;
    QItemSelectionModel* m_selectedItemModel;
};
}

#endif /* CLANGTIDY_PERPROJECTCONFIGPAGE_H_ */

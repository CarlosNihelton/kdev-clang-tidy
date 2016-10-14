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

#include "parameters.h"
#include "globalsettings.h"
#include "perprojectsettings.h"
#include <interfaces/idocument.h>
#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>
#include <QFileInfo>

namespace ClangTidy
{

Parameters::Parameters(KDevelop::IProject* project, KDevelop::IDocument* document)
    : m_project(project)
    , m_doc(document)
{
    PerProjectSettings projectSettings;
    if (m_project != nullptr) {
        projectSettings.setSharedConfig(m_project->projectConfiguration());
        projectSettings.load();

        m_projectRootPath = m_project->path();
        m_projectBuildPath = m_project->buildSystemManager()->buildDirectory(m_project->projectItem());
    }

    m_executablePath = KDevelop::Path(GlobalSettings::executablePath()).toLocalFile();
    m_filePath = (m_doc == nullptr ? m_projectRootPath.toLocalFile() : m_doc->url().toLocalFile());
    m_analiseTempDtors = projectSettings.analiseTempDtors();
    m_enabledChecks = projectSettings.enabledChecks();
    m_useConfigFile = projectSettings.useConfigFile();
    m_dumpConfig = projectSettings.dumpConfigToFile();
    m_enableChecksProfile = projectSettings.enableChecksProfile();
    m_exportFixes = projectSettings.exportFixes();
    m_extraArgs = projectSettings.extraArgs();
    m_extraArgsBefore = projectSettings.extraArgsBefore();
    m_autoFix = projectSettings.autoFix();
    m_autoFixError = projectSettings.autoFixError();
    m_matchHeaderOfTU = projectSettings.matchHeaderOfTU();
    QFileInfo fileInfo(m_filePath);
    m_headerPattern = fileInfo.baseName();
    m_headerFilter = projectSettings.headerFilter();
    m_lineFilter = projectSettings.lineFilter();
    m_listChecks = projectSettings.listChecks();
    m_checkSystemHeaders = projectSettings.checkSystemHeaders();

    m_commandLine = composeCommandLine();
}

QStringList Parameters::composeCommandLine() const
{
    QStringList cli;

    if (m_useConfigFile) {
        cli << QStringLiteral("--config=");
    }

    if (!m_enabledChecks.isEmpty()) {
        cli << QString("--checks=%1").arg(m_enabledChecks);
    }

    if (m_listChecks) {
        cli << QStringLiteral("--list-checks");
        return cli;
    }

    if (m_dumpConfig) {
        cli << QStringLiteral("--dump-config");
        return cli;
    }

    if (m_analiseTempDtors) {
        cli << QString("--analyze-temporary-dtors");
    }

    if (m_exportFixes) {
        cli << QString("--export-fixes=%1.yaml").arg(m_filePath);
    }

    if (m_autoFix) {
        cli << QStringLiteral("--fix");
    }

    if (m_autoFixError) {
        cli << QStringLiteral("--fix-errors");
    }

    if (m_enableChecksProfile) {
        cli << QStringLiteral("--enable-check-profile");
    }

    if (m_checkSystemHeaders) {
        cli << QStringLiteral("--system-headers");
    }

    if (!m_extraArgs.isEmpty()) {
        cli << QString("--extra-args=%1").arg(m_extraArgs);
    }

    if (!m_extraArgsBefore.isEmpty()) {
        cli << QString("--extra-args-before=%1").arg(m_extraArgsBefore);
    }

    if(m_matchHeaderOfTU){
        cli << QString("--header-filter=%1").arg(m_headerPattern);
    } else {
        if (!m_headerFilter.isEmpty()) {
            cli << QString("--header-filter=%1").arg(m_headerFilter);
        }
    }

    if (!m_lineFilter.isEmpty()) {
        cli << QString("--line-filter=%1").arg(m_lineFilter);
    }

    cli << QString("--p=%1").arg(m_projectBuildPath.toLocalFile()) << m_filePath;
    return cli;
}

} // namespace ClangTidy

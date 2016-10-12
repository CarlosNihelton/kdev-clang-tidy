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
    analiseTempDtors = projectSettings.analiseTempDtors();
    enabledChecks = projectSettings.enabledChecks();
    useConfigFile = projectSettings.useConfigFile();
    dumpConfig = projectSettings.dumpConfigToFile();
    enableChecksProfile = projectSettings.enableChecksProfile();
    exportFixes = projectSettings.exportFixes();
    extraArgs = projectSettings.extraArgs();
    extraArgsBefore = projectSettings.extraArgsBefore();
    autoFix = projectSettings.autoFix();
    autoFixError = projectSettings.autoFixError();
    headerFilter = projectSettings.headerFilter();
    lineFilter = projectSettings.lineFilter();
    listChecks = projectSettings.listChecks();
    checkSystemHeaders = projectSettings.checkSystemHeaders();

    m_commandLine = composeCommandLine();
}

QStringList Parameters::composeCommandLine() const
{
    QStringList cli;

    if (useConfigFile) {
        cli << QStringLiteral("--config=");
    }

    if (!enabledChecks.isEmpty()) {
        cli << QString("--checks=%1").arg(enabledChecks);
    }

    if (listChecks) {
        cli << QStringLiteral("--list-checks");
        return cli;
    }

    if (dumpConfig) {
        cli << QStringLiteral("--dump-config");
        return cli;
    }

    if (analiseTempDtors) {
        cli << QString("--analyze-temporary-dtors");
    }

    if (exportFixes) {
        cli << QString("--export-fixes=%1.yaml").arg(m_filePath);
    }

    if (autoFix) {
        cli << QStringLiteral("--fix");
    }

    if (autoFixError) {
        cli << QStringLiteral("--fix-errors");
    }

    if (enableChecksProfile) {
        cli << QStringLiteral("--enable-check-profile");
    }

    if (checkSystemHeaders) {
        cli << QStringLiteral("--system-headers");
    }

    if (!extraArgs.isEmpty()) {
        cli << QString("--extra-args=%1").arg(extraArgs);
    }

    if (!extraArgsBefore.isEmpty()) {
        cli << QString("--extra-args-before=%1").arg(extraArgsBefore);
    }

    if (!headerFilter.isEmpty()) {
        cli << QString("--header-filter=%1").arg(headerFilter);
    }

    if (!lineFilter.isEmpty()) {
        cli << QString("--line-filter=%1").arg(lineFilter);
    }

    cli << QString("--p=%1").arg(m_projectBuildPath.toLocalFile()) << m_filePath;
    return cli;
}

void Parameters::setDumpConfig(bool value)
{
    PerProjectSettings settings;
    if (m_project != nullptr) {
        settings.setSharedConfig(m_project->projectConfiguration());
        settings.load();
    }
    dumpConfig = value;
    settings.setDumpConfigToFile(value);
    m_commandLine = composeCommandLine();
}

} // namespace ClangTidy

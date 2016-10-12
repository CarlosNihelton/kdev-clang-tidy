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

#ifndef CLANGTIDY_PARAMETERS_H
#define CLANGTIDY_PARAMETERS_H

#define CLANG_TIDY_PATH "/usr/bin/clang-tidy"

#include <QString>
#include <util/path.h>

namespace KDevelop
{
class IProject;
class IDocument;
class Path;
}

namespace ClangTidy
{

class PerProjectSettings;

class Parameters
{
private:
    KDevelop::IProject* m_project;
    KDevelop::IDocument* m_doc;
    KDevelop::Path m_projectRootPath;
    KDevelop::Path m_projectBuildPath;
    QStringList m_commandLine;

    QStringList composeCommandLine() const;

    // global settings
    QString m_executablePath;

    // project settings.
    QString m_filePath;
    QString additionalParameters;
    bool analiseTempDtors;
    QString enabledChecks;
    bool useConfigFile;
    bool dumpConfig;
    bool enableChecksProfile;
    bool exportFixes;
    QString extraArgs;
    QString extraArgsBefore;
    bool autoFix;
    bool autoFixError;
    QString headerFilter;
    QString lineFilter;
    bool listChecks;
    bool checkSystemHeaders;

public:
    Parameters(KDevelop::IProject* project = nullptr, KDevelop::IDocument* document = nullptr);
    const QStringList& commandLine() const { return m_commandLine; }
    const QString& filePath() const { return m_filePath; }
    const QString& executablePath() const { return m_executablePath; }
    bool mustDumpToConfigFile() const { return dumpConfig; }
    void setDumpConfig(bool value);
    QString projectRootDir() const { return m_projectRootPath.toLocalFile(); }
};
}

#endif // CLANGTIDY_PARAMETERS_H

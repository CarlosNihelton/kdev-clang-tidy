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

#include "job.h"

#include "parsers/clangtidyparser.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <KShell>
#include <QApplication>
#include <QFile>
#include <QRegularExpression>

namespace ClangTidy
{

Job::Job(const Parameters& params)
    : KDevelop::OutputExecuteJob(nullptr)
    , m_parameters(params)
{
    setToolTitle("ClangTidy");
    setJobName(i18n("clang-tidy output"));

    setCapabilities(KJob::Killable);
    setStandardToolView(KDevelop::IOutputView::TestView);
    setBehaviours(KDevelop::IOutputView::AutoScroll);

    setProperties(KDevelop::OutputExecuteJob::JobProperty::DisplayStdout);
    setProperties(KDevelop::OutputExecuteJob::JobProperty::DisplayStderr);
    setProperties(KDevelop::OutputExecuteJob::JobProperty::PostProcessOutput);

    *this << params.executablePath() << params.commandLine();

    qCDebug(KDEV_CLANGTIDY) << "checking path" << params.filePath();
}

Job::~Job()
{
    doKill();
}

void Job::processStdoutLines(const QStringList& lines)
{
    if (!m_parameters.mustDumpToConfigFile()) {
        m_standardOutput << lines;
    } else {
        QFile file(m_parameters.projectRootDir() + "/.clang-tidy");
        file.open(QIODevice::WriteOnly);
        QTextStream os(&file);
        os << lines.join('\n');
    }
}

void Job::processStderrLines(const QStringList& lines)
{
    for (const QString& line : lines) {
        m_standardOutput << line;
    }
}

void Job::postProcessStdout(const QStringList& lines)
{
    processStdoutLines(lines);

    KDevelop::OutputExecuteJob::postProcessStdout(lines);
}

void Job::postProcessStderr(const QStringList& lines)
{
    processStderrLines(lines);

    KDevelop::OutputExecuteJob::postProcessStderr(lines);
}

void Job::start()
{
    m_standardOutput.clear();
    qCDebug(KDEV_CLANGTIDY) << "executing:" << commandLine().join(' ');
    KDevelop::OutputExecuteJob::start();
}

QVector<KDevelop::IProblem::Ptr> Job::problems() const
{
    return m_problems;
}

void Job::childProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(KDEV_CLANGTIDY) << "Process Finished, exitCode" << exitCode << "process exit status" << exitStatus;

    if (exitCode != 0) {
        qCDebug(KDEV_CLANGTIDY) << "clangTidy failed, standard output: ";
        qCDebug(KDEV_CLANGTIDY) << m_standardOutput.join('\n');
    } else {
        ClangTidyParser parser;
        parser.addData(m_standardOutput);
        parser.parse();
        m_problems = parser.problems();
    }

    KDevelop::OutputExecuteJob::childProcessExited(exitCode, exitStatus);
}
} // namespace ClangTidy

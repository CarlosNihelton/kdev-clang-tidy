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

#include "plugin.h"

#include <QAction>
#include <QMessageBox>

#include <kactioncollection.h>
#include <klocalizedstring.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kprocess.h>

#include <execute/iexecuteplugin.h>

#include <KXMLGUIFactory>
#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/launchconfigurationtype.h>
#include <language/interfaces/editorcontext.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectconfigpage.h>
#include <project/projectmodel.h>
#include <shell/problemmodelset.h>
#include <util/executecompositejob.h>

#include "config/clangtidypreferences.h"
#include "config/perprojectconfigpage.h"
#include "job.h"

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(ClangTidyFactory, "res/kdevclangtidy.json", registerPlugin<ClangTidy::Plugin>();)
namespace ClangTidy
{
Plugin::Plugin(QObject* parent, const QVariantList& /*unused*/)
    : IPlugin("kdevclangtidy", parent),
    m_model(new KDevelop::ProblemModel(parent))
{
    qCDebug(KDEV_CLANGTIDY) << "setting clangTidy rc file";
    setXMLFile("kdevclangtidy.rc");

    m_model->setFeatures(
        KDevelop::ProblemModel::SeverityFilter |
        KDevelop::ProblemModel::Grouping |
        KDevelop::ProblemModel::CanByPassScopeFilter);

    QAction* actionCheckFile;
    actionCheckFile = actionCollection()->addAction("clangTidy_file", this, SLOT(runClangTidyFile()));
    actionCheckFile->setStatusTip(i18n("Launches ClangTidy for current file"));
    actionCheckFile->setText(i18n("clang-tidy"));

    IExecutePlugin* iface = KDevelop::ICore::self()
                                ->pluginController()
                                ->pluginForExtension("org.kdevelop.IExecutePlugin")
                                ->extension<IExecutePlugin>();
    Q_ASSERT(iface);

    ProblemModelSet* pms = core()->languageController()->problemModelSet();
    pms->addModel(QStringLiteral("ClangTidy"), m_model.data());

    m_config = KSharedConfig::openConfig()->group("ClangTidy");
    auto clangTidyPath = m_config.readEntry(ConfigGroup::ExecutablePath);

    if (clangTidyPath.isEmpty()) {
        clangTidyPath = QString(CLANG_TIDY_PATH);
    }

    collectAllAvailableChecks(clangTidyPath);

    QStringList defaults(m_config.readEntry(ConfigGroup::EnabledChecks).remove(QChar('*')).split(','));
    for (const auto& check : m_allChecks) {
        for(const auto& enabled : defaults){
            if(check.contains(enabled)){
                m_activeChecks << check;
            } else {
                m_activeChecks.removeAll(check);
            }
        }
    }
    m_activeChecks.removeDuplicates();
}

void Plugin::unload()
{
    ProblemModelSet* pms = core()->languageController()->problemModelSet();
    pms->removeModel(QStringLiteral("ClangTidy"));
}

void Plugin::collectAllAvailableChecks(QString clangTidyPath)
{
    m_allChecks.clear();
    KProcess tidy;
    tidy << clangTidyPath << QLatin1String("-checks=*") << QLatin1String("--list-checks");
    tidy.setOutputChannelMode(KProcess::OnlyStdoutChannel);
    tidy.start();

    if (!tidy.waitForStarted()) {
        qCDebug(KDEV_CLANGTIDY) << "Unable to execute clang-tidy.";
        return;
    }

    tidy.closeWriteChannel();
    if (!tidy.waitForFinished()) {
        qCDebug(KDEV_CLANGTIDY) << "Failed during clang-tidy execution.";
        return;
    }

    QTextStream ios(&tidy);
    QString each;
    while (ios.readLineInto(&each)) {
        m_allChecks.append(each.trimmed());
    }
    if (m_allChecks.size() > 3) {
        m_allChecks.removeAt(m_allChecks.length() - 1);
        m_allChecks.removeAt(0);
    }
    m_allChecks.removeDuplicates();
}

void Plugin::runClangTidy(bool allFiles)
{
    KDevelop::IDocument* doc = core()->documentController()->activeDocument();
    if (doc == nullptr) {
        QMessageBox::critical(nullptr, i18n("Error starting clang-tidy"),
                              i18n("No suitable active file, unable to deduce project."));
        return;
    }

    KDevelop::IProject* project = core()->projectController()->findProjectForUrl(doc->url());
    if (project == nullptr) {
        QMessageBox::critical(nullptr, i18n("Error starting clang-tidy"), i18n("Active file isn't in a project"));
        return;
    }

    m_config = project->projectConfiguration()->group("ClangTidy");
    if (!m_config.isValid()) {
        QMessageBox::critical(nullptr, i18n("Error starting ClangTidy"),
                              i18n("Can't load parameters. They must be set in the "
                                   "project settings."));
        return;
    }

    auto clangTidyPath = m_config.readEntry(ConfigGroup::ExecutablePath);
    auto buildSystem = project->buildSystemManager();

    Job::Parameters params;

    params.projectRootDir = project->path().toLocalFile();

    if (clangTidyPath.isEmpty()) {
        params.executablePath = QStringLiteral(CLANG_TIDY_PATH);
    } else {
        params.executablePath = clangTidyPath;
    }

    if (allFiles) {
        params.filePath = project->path().toUrl().toLocalFile();
    } else {
        params.filePath = doc->url().toLocalFile();
    }
    params.buildDir = buildSystem->buildDirectory(project->projectItem()).toLocalFile();
    params.additionalParameters = m_config.readEntry(ConfigGroup::AdditionalParameters);
    params.analiseTempDtors = m_config.readEntry(ConfigGroup::AnaliseTempDtors);
    params.enabledChecks = m_activeChecks.join(',');
    params.useConfigFile = m_config.readEntry(ConfigGroup::UseConfigFile);
    params.dumpConfig = m_config.readEntry(ConfigGroup::DumpConfig);
    params.enableChecksProfile = m_config.readEntry(ConfigGroup::EnableChecksProfile);
    params.exportFixes = m_config.readEntry(ConfigGroup::ExportFixes);
    params.extraArgs = m_config.readEntry(ConfigGroup::ExtraArgs);
    params.extraArgsBefore = m_config.readEntry(ConfigGroup::ExtraArgsBefore);
    params.autoFix = m_config.readEntry(ConfigGroup::AutoFix);
    params.headerFilter = m_config.readEntry(ConfigGroup::HeaderFilter);
    params.lineFilter = m_config.readEntry(ConfigGroup::LineFilter);
    params.listChecks = m_config.readEntry(ConfigGroup::ListChecks);
    params.checkSystemHeaders = m_config.readEntry(ConfigGroup::CheckSystemHeaders);

    if (!params.dumpConfig.isEmpty()) {
        auto job = new ClangTidy::Job(params, this);
        core()->runController()->registerJob(job);
        params.dumpConfig = QString();
    }
    auto job2 = new ClangTidy::Job(params, this);
    connect(job2, &Job::finished, this, &Plugin::result);
    core()->runController()->registerJob(job2);
}

void Plugin::runClangTidyFile()
{
    bool allFiles = false;
    runClangTidy(allFiles);
}

void Plugin::runClangTidyAll()
{
    bool allFiles = true;
    runClangTidy(allFiles);
}

void Plugin::loadOutput()
{
}

void Plugin::result(KJob* job)
{
    Job* aj = dynamic_cast<Job*>(job);
    if (aj == nullptr) {
        return;
    }

    if (aj->status() == KDevelop::OutputExecuteJob::JobStatus::JobSucceeded) {
        m_model->setProblems(aj->problems());
        core()->languageController()->problemModelSet()->showModel(i18nd("kdevproblemreporter", "ClangTidy"));
    }
}

KDevelop::ContextMenuExtension Plugin::contextMenuExtension(KDevelop::Context* context)
{
    KDevelop::IDocument* doc = core()->documentController()->activeDocument();
    KDevelop::ContextMenuExtension extension = KDevelop::IPlugin::contextMenuExtension(context);

    if (context->type() == KDevelop::Context::EditorContext) {
        auto mime = doc->mimeType().name();
        if (mime == QLatin1String("text/x-c++src") || mime == QLatin1String("text/x-csrc")) {
            QAction* action = new QAction(QIcon::fromTheme("dialog-ok"), i18n("clang-tidy"), this);
            connect(action, &QAction::triggered, this, &Plugin::runClangTidyFile);
            extension.addAction(KDevelop::ContextMenuExtension::AnalyzeGroup, action);
        }
    }
    return extension;
}

KDevelop::ConfigPage* Plugin::perProjectConfigPage(int number, const ProjectConfigOptions& options, QWidget* parent)
{
    if (number != 0) {
        return nullptr;
    }
    auto config = new PerProjectConfigPage(this, options.project, m_allChecks, parent);
    connect(config, &PerProjectConfigPage::selectedChecksChanged, this, &Plugin::setSelectedChecks);
    return config;
}

KDevelop::ConfigPage* Plugin::configPage(int number, QWidget* parent)
{
    if (number != 0) {
        return nullptr;
    }
    return new ClangTidyPreferences(this, parent);
}
} // namespace ClangTidy

#include "plugin.moc"

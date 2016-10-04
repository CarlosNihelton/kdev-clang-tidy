/*************************************************************************************
 *  Copyright (C) 2016 by Carlos Nihelton <carlosnsoliveira@gmail.com>               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *                                                                                   *
 *  Now let's read a beautiful song:                                                 *

                夢のつづき
                追いかけていたはずなのに

                曲がりくねった
                細い道　人につまずく

                あの頃みたいにって
                戻りたい訳じゃないの

                無くしてきた空を
                探してる

                わかってくれますように

                犠牲になったような
                悲しい顔はやめてよ

                罪の最後は涙じゃないよ
                ずっと苦しく背負ってくんだ

                出口見えない感情迷路に
                誰を待ってるの?

                白いノートに綴ったように
                もっと素直に吐き出したいよ

                何から
                逃れたいんだ

                …現実ってやつ?

                叶えるために
                生きてるんだって

                忘れちゃいそうな
                夜の真ん中

                無難になんて
                やってられないから

                …帰る場所もないの

                この想いを　消してしまうには
                まだ人生長いでしょ?(I'm on the way)

                懐かしくなる
                こんな痛みも歓迎じゃん

                謝らなくちゃいけないよね
                ah　ごめんね

                うまく言えなくて
                心配かけたままだったね

                あの日かかえた全部
                あしたかかえる全部

                順番つけたりは
                しないから

                わかってくれますように

                そっと目を閉じたんだ
                見たくないものまで
                見えんだもん

                いらないウワサにちょっと
                初めて聞く発言どっち?

                2回会ったら友達だって??
                ウソはやめてね

                赤いハートが苛立つように
                身体ん中燃えているんだ

                ホントは
                期待してんの

                …現実ってやつ?

                叶えるために
                生きてるんだって

                叫びたくなるよ
                聞こえていますか?

                無難になんて
                やってられないから

                …帰る場所もないの

                優しさには　いつも感謝してる
                だから強くなりたい(I'm on the way)

                進むために
                敵も味方も歓迎じゃん

                どうやって次のドア
                開けるんだっけ?考えてる?

                もう引き返せない
                物語　始まってるんだ

                目を覚ませ　目を覚ませ

                この想いを　消してしまうには
                まだ人生長いでしょ?

                やり残してるコト
                やり直してみたいから

                もう一度ゆこう

                叶えるために
                生きてるんだって

                叫びたくなるよ
                聞こえていますか?

                無難になんて
                やってられないから

                …帰る場所もないの

                優しさには　いつも感謝してる
                だから強くなりたい(I'm on the way)

                懐かしくなる
                こんな痛みも歓迎じゃん

 *************************************************************************************/

#include <unistd.h>

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

#include "./config/clangtidypreferences.h"
#include "./config/perprojectconfigpage.h"
#include "debug.h"
#include "job.h"
#include "plugin.h"

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(ClangTidyFactory, "res/kdevclangTidy.json", registerPlugin<ClangTidy::Plugin>();)
namespace ClangTidy
{
Plugin::Plugin(QObject* parent, const QVariantList& /*unused*/)
    : IPlugin("kdevclangtidy", parent)
    , m_model(new KDevelop::ProblemModel(parent))
{
    qCDebug(KDEV_CLANGTIDY) << "setting clangTidy rc file";
    setXMLFile("kdevclangTidy.rc");

    QAction* act_checkfile;
    act_checkfile = actionCollection()->addAction("clangTidy_file", this, SLOT(runClangTidyFile()));
    act_checkfile->setStatusTip(i18n("Launches ClangTidy for current file"));
    act_checkfile->setText(i18n("clang-tidy"));

    /*     TODO: Uncomment this only when discover a safe way to run clang-tidy on the whole project.
    //     QAction* act_check_all_files;
    //     act_check_all_files = actionCollection()->addAction ( "clangTidy_all", this, SLOT ( runClangTidyAll() ) );
    //     act_check_all_files->setStatusTip ( i18n ( "Launches clangTidy for all translation "
    //                                         "units of current project" ) );
    //     act_check_all_files->setText ( i18n ( "clang-tidy (all)" ) );
    */

    IExecutePlugin* iface = KDevelop::ICore::self()
                                ->pluginController()
                                ->pluginForExtension("org.kdevelop.IExecutePlugin")
                                ->extension<IExecutePlugin>();
    Q_ASSERT(iface);

    ProblemModelSet* pms = core()->languageController()->problemModelSet();
    pms->addModel(QStringLiteral("ClangTidy"), m_model.data());

    m_config = KSharedConfig::openConfig()->group("ClangTidy");
    auto clangTidyPath = m_config.readEntry(ConfigGroup::ExecutablePath);

    // TODO(cnihelton): auto detect clang-tidy executable instead of hard-coding it.
    if (clangTidyPath.isEmpty()) {
        clangTidyPath = QString("/usr/bin/clang-tidy");
    }

    collectAllAvailableChecks(clangTidyPath);

    m_config.writeEntry(ConfigGroup::AdditionalParameters, "");
    for (auto check : m_allChecks) {
        bool enable = check.contains("cert") || check.contains("-core.") || check.contains("-cplusplus")
            || check.contains("-deadcode") || check.contains("-security") || check.contains("cppcoreguide");
        if (enable) {
            m_activeChecks << check;
        } else {
            m_activeChecks.removeAll(check);
        }
    }
    m_activeChecks.removeDuplicates();
    m_config.writeEntry(ConfigGroup::EnabledChecks, m_activeChecks.join(','));
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
    if (!doc) {
        QMessageBox::critical(nullptr, i18n("Error starting clang-tidy"),
                              i18n("No suitable active file, unable to deduce project."));
        return;
    }

    KDevelop::IProject* project = core()->projectController()->findProjectForUrl(doc->url());
    if (!project) {
        QMessageBox::critical(nullptr, i18n("Error starting clang-tidy"), i18n("Active file isn't in a project"));
        return;
    }

    m_config = project->projectConfiguration()->group("ClangTidy");
    if (!m_config.isValid()) {
        QMessageBox::critical(nullptr, i18n("Error starting ClangTidy"),
                              i18n("Can't load parameters. They must be set in the project settings."));
        return;
    }

    auto clangTidyPath = m_config.readEntry(ConfigGroup::ExecutablePath);
    auto buildSystem = project->buildSystemManager();

    Job::Parameters params;

    params.projectRootDir = project->path().toLocalFile();

    // TODO: auto detect clang-tidy executable instead of hard-coding it.
    if (clangTidyPath.isEmpty()) {
        params.executablePath = QStringLiteral("/usr/bin/clang-tidy");
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
        Job* job = new ClangTidy::Job(params, this);
        core()->runController()->registerJob(job);
        params.dumpConfig = QString();
    }
    Job* job2 = new ClangTidy::Job(params, this);
    connect(job2, SIGNAL(finished(KJob*)), this, SLOT(result(KJob*)));
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
    if (!aj) {
        return;
    }

    if (aj->status() == KDevelop::OutputExecuteJob::JobStatus::JobSucceeded) {
        m_model->setProblems(aj->problems());

        core()->uiController()->findToolView(i18nd("kdevproblemreporter", "Problems"), 0,
                                             KDevelop::IUiController::FindFlags::Raise);
    }
}

KDevelop::ContextMenuExtension Plugin::contextMenuExtension(KDevelop::Context* context)
{
    KDevelop::IDocument* doc = core()->documentController()->activeDocument();
    KDevelop::ContextMenuExtension extension = KDevelop::IPlugin::contextMenuExtension(context);

    if (context->type() == KDevelop::Context::EditorContext) {

        auto mime = doc->mimeType().name();
        if (mime == QLatin1String("text/x-c++src") || mime == QLatin1String("text/x-csrc")) {
            QAction* action
                = new QAction(QIcon::fromTheme("document-new"), i18n("Check current unit with clang-tidy"), this);
            connect(action, SIGNAL(triggered(bool)), this, SLOT(runClangTidyFile()));
            extension.addAction(KDevelop::ContextMenuExtension::ExtensionGroup, action);
        }
    }
    return extension;
}

KDevelop::ConfigPage* Plugin::perProjectConfigPage(int number, const ProjectConfigOptions& options, QWidget* parent)
{
    if (number != 0) {
        return nullptr;
    } else {
        auto config = new PerProjectConfigPage(options.project, parent);
        config->setActiveChecksReceptorList(&m_activeChecks);
        config->setList(m_allChecks);
        return config;
    }
}

KDevelop::ConfigPage* Plugin::configPage(int number, QWidget* parent)
{
    if (number != 0) {
        return nullptr;
    } else {
        return new ClangTidyPreferences(this, parent);
    }
}
}

#include "plugin.moc"

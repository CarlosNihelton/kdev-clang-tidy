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

#include "config/clangtidypreferences.h"
#include "clangtidyconfig.h"
#include "config/configgroup.h"
#include "ui_clangtidysettings.h"

#include <QVBoxLayout>

using KDevelop::IPlugin;
using ClangTidy::ConfigGroup;
using KDevelop::ConfigPage;

ClangTidyPreferences::ClangTidyPreferences(IPlugin* plugin, QWidget* parent)
    : ConfigPage(plugin, ClangtidySettings::self(), parent)
{
    auto layout = new QVBoxLayout(this);
    auto widget = new QWidget(this);
    ui = new Ui::ClangTidySettings();
    ui->setupUi(widget);
    layout->addWidget(widget);
}

ClangTidyPreferences::~ClangTidyPreferences()
{
    delete ui;
}

ConfigPage::ConfigPageType ClangTidyPreferences::configPageType() const
{
    return ConfigPage::AnalyzerConfigPage;
}

QString ClangTidyPreferences::name() const
{
    return i18n("clang-tidy");
}

QString ClangTidyPreferences::fullName() const
{
    return i18n("Configure clang-tidy settings");
}

QIcon ClangTidyPreferences::icon() const
{
    return QIcon::fromTheme(QStringLiteral("dialog-ok"));
}

void ClangTidyPreferences::apply()
{
    ConfigGroup projConf = KSharedConfig::openConfig()->group("ClangTidy");
    projConf.writeEntry(ConfigGroup::ExecutablePath, ui->kcfgClangTidyPath->text());
}

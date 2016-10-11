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

#ifndef CLANGTIDY_CONFIG_H
#define CLANGTIDY_CONFIG_H

#include <kconfig.h>
#include <kconfiggroup.h>
#include <tuple>

#define CLANG_TIDY_PATH "/usr/bin/clang-tidy"

namespace ClangTidy
{
/**
 * \class
 * \brief Specializes KConfigGroup for clang-tidy, using a type system to avoid
 * configuration control by passing
 * strings around.
 */
class ConfigGroup
{
    using Option = std::pair<QString, QString>;
    
    using OptionArray = std::array<const Option*, 18>;

private:
    KConfigGroup m_group;

public:
    ConfigGroup() = default;
    ConfigGroup(const KConfigGroup& cg) { m_group = cg; }
    ~ConfigGroup() = default;
    ConfigGroup& operator=(const KConfigGroup& cg)
    {
        this->m_group = cg;
        return *this;
    }
    bool isValid() { return m_group.isValid(); }
    /// This technique allows us to avoid passing strings to KConfigGroup member
    /// functions, thus allowing compile time
    /// detection of wrong parameters.
    static const Option ExecutablePath;
    static const Option FilePath;
    static const Option BuildPath;
    static const Option AdditionalParameters;
    static const Option AnaliseTempDtors;
    static const Option EnabledChecks;
    static const Option UseConfigFile;
    static const Option DumpConfig;
    static const Option EnableChecksProfile;
    static const Option ExportFixes;
    static const Option ExtraArgs;
    static const Option ExtraArgsBefore;
    static const Option AutoFix;
    static const Option AutoFixError;
    static const Option HeaderFilter;
    static const Option LineFilter;
    static const Option ListChecks;
    static const Option CheckSystemHeaders;
    static const OptionArray AllOptions;

    QString readEntry(const Option& key) const { 
        QString read(m_group.readEntry(key.first));
        if(read==QStringLiteral("true")){
            return key.second;
        } else if(read == QStringLiteral("false") || read.isEmpty()){
            return QString();
        } else {
            return key.second.arg(read); 
        }
    }
};
}

#endif // CLANGTIDY_CONFIG_H

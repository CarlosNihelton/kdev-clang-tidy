/*
 * ************************************************************************************
 *    Copyright (C) 2016 by Carlos Nihelton <carlosnsoliveira@gmail.com>               *
 *                                                                                     *
 *    This program is free software; you can redistribute it and/or                    *
 *    modify it under the terms of the GNU General Public License                      *
 *    as published by the Free Software Foundation; either version 2                   *
 *    of the License, or (at your option) any later version.                           *
 *                                                                                     *
 *    This program is distributed in the hope that it will be useful,                  *
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *    GNU General Public License for more details.                                     *
 *                                                                                     *
 *    You should have received a copy of the GNU General Public License                *
 *    along with this program; if not, write to the Free Software                      *
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *  ************************************************************************************
 */

#ifndef PROBLEM_H
#define PROBLEM_H

#include "debug.h"
#include <interfaces/iassistant.h>
#include <language/duchain/problem.h>

namespace ClangTidy
{

struct Fixit {
    size_t offset, length; // read from YAML.
    size_t line, column; // calculated by parser. Might drop eventually.
    QString replacementText; // read from YAML.
    KDevelop::DocumentRange range; // created from line and colum.
    QString description;
    QString currentText; // TODO: reason about if it's needed.

    bool operator==(const Fixit& other)
    {
        return (offset == other.offset && length == other.length && line == other.line && column == other.column
                && replacementText == other.replacementText && description == other.description
                && currentText == other.currentText);
    }
};

using Fixits = QVector<Fixit>;

class Problem : public KDevelop::Problem
{
public:
    using Ptr = QExplicitlySharedDataPointer<Problem>;
    using ConstPtr = QExplicitlySharedDataPointer<const Problem>;
    
    KDevelop::IAssistant::Ptr solutionAssistant() const override;
    Fixits replacements() const;
    void setReplacements(const Fixits& repls);

private:
    Fixits m_fixes;
};

class FixitAssistant : public KDevelop::IAssistant
{
    Q_OBJECT
public:
    FixitAssistant(const Fixits& fixits);
    FixitAssistant(QString title, const Fixits& fixits);
    QString title() const override;
    void createActions() override;
    Fixits replacements() const;

private:
    QString m_title;
    Fixits m_fixes;
};

class FixitAction : public KDevelop::IAssistantAction
{
    Q_OBJECT
public:
    FixitAction(const Fixit& fixit);
    QString description() const override;

public Q_SLOTS:
    void execute() override;

private:
    Fixit m_fix;
};

} // namespace ClangTidy
#endif // PROBLEM_H

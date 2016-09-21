/***************************************************************************************
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
 ***************************************************************************************/

#include "problem.h"
#include <KLocalizedString>
#include <interfaces/icore.h>
#include <language/codegen/documentchangeset.h>
#include <language/duchain/duchainlock.h>

namespace ClangTidy
{

KDevelop::IAssistant::Ptr Problem::solutionAssistant() const
{
    if (m_fixes.isEmpty())
        return {};
    return KDevelop::IAssistant::Ptr(new FixitAssistant(m_fixes));
}

Fixits Problem::replacements() const
{
    return m_fixes;
}

void Problem::setReplacements(const Fixits& repls)
{
    m_fixes = repls;
}

FixitAssistant::FixitAssistant(const Fixits& fixits)
    : m_title{ tr("clang-tidy Suggestions") }
    , m_fixes{ fixits }
{
}

FixitAssistant::FixitAssistant(QString title, const Fixits& fixits)
    : m_title{ title }
    , m_fixes{ fixits }
{
}

QString FixitAssistant::title() const
{
    return m_title;
}

void FixitAssistant::createActions()
{
    KDevelop::IAssistant::createActions();
    for (const auto& fixit : m_fixes) {
        addAction(KDevelop::IAssistantAction::Ptr(new FixitAction(fixit)));
    }
}

Fixits FixitAssistant::replacements() const
{
    return m_fixes;
}

FixitAction::FixitAction(const ClangTidy::Fixit& fixit)
    : m_fix(fixit)
{
}

QString FixitAction::description() const
{
    if (!m_fix.description.isEmpty())
        return m_fix.description;

    const auto& range = m_fix.range;
    if (range.start() == range.end()) {
        return i18n("Insert \"%1\" at line: %2, column: %3", m_fix.replacementText, range.start().line() + 1,
                    range.start().column() + 1);
    }

    if (range.start().line() == range.end().line()) {
        if (m_fix.currentText.isEmpty()) {
            return i18n("Replace text at line:%1, column: %2 with: \"%3\"", range.start().line() + 1,
                        range.start().column() + 1, m_fix.replacementText);
        } else {
            return i18n("Replace \"%1\" with \"%2\"", m_fix.currentText, m_fix.replacementText);
        }
    }

    return i18n("Replace multiple lines starting at line: %1, column: %2 with: \"%3\"", range.start().line() + 1,
                range.start().column() + 1, m_fix.replacementText);
}

void FixitAction::execute()
{
    KDevelop::DocumentChangeSet changes;
    {
        KDevelop::DUChainReadLocker lock;
        KDevelop::DocumentChange change(m_fix.range.document, m_fix.range, m_fix.currentText, m_fix.replacementText);
        change.m_ignoreOldText = !m_fix.currentText.isEmpty();
        changes.addChange(change);
    }

    changes.setReplacementPolicy(KDevelop::DocumentChangeSet::WarnOnFailedChange);
    changes.applyAllChanges();
    emit executed(this);
}

} // namespace ClangTidy

#include "moc_problem.cpp"

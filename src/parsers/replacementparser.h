/**************************************************************************************
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

#ifndef CLANGTIDY_REPLACEMENT_H
#define CLANGTIDY_REPLACEMENT_H

#include <QFile>
#include <QRegularExpression>
#include <QVector>
#include <boost/utility/string_ref.hpp>
#include <language/editor/documentrange.h>

#include "debug.h"
#include "problem.h"

namespace ClangTidy
{

class ReplacementParser
{
private:
    size_t currentLine;
    size_t currentColumn;
    size_t currentOffset;
    size_t cReplacements;

    QString m_yamlname;
    QString m_sourceName;
    KDevelop::IndexedString i_source;
    QString m_yamlContent;
    std::string m_sourceCode;
    boost::string_ref m_sourceView;
    static const QRegularExpression regex, check;
    Fixits all_replacements;

protected:
    Fixit nextNode(const QRegularExpressionMatch& smatch);
    KDevelop::DocumentRange composeNextNodeRange(size_t offset, size_t length);

public:
    ReplacementParser() = default;
    explicit ReplacementParser(const QString& source_file);
    void setReplacementsFileName(const QString& source_file);
    void parse();
    Fixits allReplacements() { return all_replacements; }
};
}

#endif // CLANGTIDY_REPLACEMENT_H

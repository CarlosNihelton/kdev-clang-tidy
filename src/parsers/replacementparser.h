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

#ifndef CLANGTIDY_REPLACEMENT_H
#define CLANGTIDY_REPLACEMENT_H

#include <QFile>
#include <QRegularExpression>
#include <QVector>
#include <boost/utility/string_ref.hpp>
#include <language/editor/documentrange.h>

#include "problem.h"

namespace ClangTidy
{

/**
 * \struct
 * \brief contains basic elements for one replacement in source code.
 *
 */
struct Replacement {
    size_t offset, length; ///< read from YAML.
    QString replacementText; ///< read from YAML.
    KDevelop::DocumentRange range; ///< created from line and colum.
};

using Replacements = QVector<Replacement>;

/**
 * Implements the parser for the YAML file generated by clang-tidy with the recommended corrections.
 */
class ReplacementParser
{
private:
    size_t currentLine; ///< current line on source code while parsing.
    size_t currentColumn; ///< current column on source code while parsing.
    size_t currentOffset; ///< current offset in bytes since the beginning of the source code while parsing.
    size_t cReplacements; ///< current count of replacements parsed.

    QString m_yamlname;
    QString m_sourceName;
    KDevelop::IndexedString i_source;
    QString m_yamlContent;
    std::string m_sourceCode;
    boost::string_ref m_sourceView;
    static const QRegularExpression regex, check;
    Replacements all_replacements;

protected:
    /**
    * \function
    * \brief generates the next replacement from the regex capture list.
    * \param smatch: the captured match.
    * \return Replacement
    */
    Replacement nextNode(const QRegularExpressionMatch& smatch);

    /**
    * \function
    * \brief compose a range in KTextEditor from the offset and length components of the Replacement being processed.
    * \return KDevelop::DocumentRange
    * \warning the range can be invalid in case offset and length overcome the substring length.
    */
    KDevelop::DocumentRange composeNextNodeRange(size_t offset, size_t length);

public:
    ReplacementParser() = default;
    explicit ReplacementParser(const QString& source_file);
    void setReplacementsFileName(const QString& source_file);
    void parse();
    size_t count() const { return cReplacements; }
    Replacements allReplacements() { return all_replacements; }
};
}

#endif // CLANGTIDY_REPLACEMENT_H

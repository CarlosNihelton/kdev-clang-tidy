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

#include "replacementparser.h"
#include "qCDebug/debug.h"
// See <https://github.com/CarlosNihelton/kdev-clang-tidy/issues/1>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <tuple>

#ifdef BOOST_NO_EXCEPTIONS
// Because we are using boost we need to provide an implementation of this
// function, because KDE disables exceptions on
// boost libraries.
namespace boost
{
void throw_exception(std::exception const& e)
{
    qCDebug(KDEV_CLANGTIDY) << e.what();
}
} // namespace boost
#endif

namespace
{

/**
* @function
* @brief For a given string_ref representing the source code, it counts the number of rows and the column where the
* string_ref ends.
* @param substring : a piece of the source code.
* @return std::pair<size_t,size_t>: <b>first</b>: count of lines and <b>second</b>: column at the end.
*/
inline std::pair<size_t, size_t> countOfRowAndColumnToTheEndOfSubstr(boost::string_ref substring)
{
    size_t line = 0, col = 0;
    for (const auto elem : substring) {
        if (elem == char('\n')) {
            ++line;
            col = 0;
        } else {
            ++col;
        }
    }

    return std::make_pair(line, col);
}

} // namespace

namespace ClangTidy
{

const QRegularExpression ReplacementParser::check{ QStringLiteral(
    "---\\s+MainSourceFile:\\s+.+\\s+Replacements:(\\s+.+)+\\s\\.\\.\\.") };

const QRegularExpression ReplacementParser::regex{ (
    QStringLiteral("\\s+\\s+-\\s+FilePath:\\s+(.+\\.cpp)\\s+Offset:\\s+(\\d+)\\s+Length:\\s+"
                   "(\\d+)\\s+ ReplacementText:\\s(.+)")) };

ReplacementParser::ReplacementParser(const QString& yaml_file, const QString& source_file)
    : currentLine{ 0 }
    , currentColumn{ 0 }
    , currentOffset{ 0 }
    , cReplacements{ 0 }
    , m_yamlname{ yaml_file }
    , m_sourceFile{ source_file }
{
    if (m_yamlname.endsWith(".yaml")) {
        QFile yaml(m_yamlname);
        yaml.open(QIODevice::ReadOnly);
        m_yamlContent = yaml.readAll();

        auto checkMatch = check.match(m_yamlContent);
        if (!checkMatch.hasMatch()) {
            m_yamlname.clear();
            m_yamlContent.clear();
        }
    }

    // TODO (CarlosNihelton): Discover a way to get that from KDevelop.
    if (m_sourceFile.endsWith(".cpp")) {
        i_source = IndexedString(m_sourceFile);
        // See <https://github.com/CarlosNihelton/kdev-clang-tidy/issues/1>
        std::ifstream cpp;
        cpp.open(m_sourceFile.toUtf8());
        std::copy(std::istreambuf_iterator<char>(cpp), std::istreambuf_iterator<char>(),
                  std::back_insert_iterator<std::string>(m_sourceCode));
        m_sourceView = boost::string_ref(m_sourceCode);
    }
}

void ReplacementParser::parse()
{
    if (m_yamlContent.isEmpty()) {
        return; // Nothing to parse.
    }

    for (auto iMatch = regex.globalMatch(m_yamlContent); iMatch.hasNext(); ++cReplacements) {
        auto smatch = iMatch.next();
        auto rep = nextNode(smatch);
        all_replacements.push_back(rep);
    }
}

Replacement ReplacementParser::nextNode(const QRegularExpressionMatch& smatch)
{
    Replacement repl;

    if (smatch.captured(1) != m_sourceFile) {
        return repl; // Parsing output from only one file.
    }
    int off{ smatch.captured(2).toInt() };
    int len{ smatch.captured(3).toInt() };
    repl.range = composeNextNodeRange(off, len);
    if (repl.range.isValid()) {
        repl.offset = off;
        repl.length = len;
        repl.replacementText = smatch.captured(4);
        if (repl.replacementText.startsWith('\'') && repl.replacementText.endsWith('\'')) {
            repl.replacementText.remove(0, 1);
            repl.replacementText.remove(repl.replacementText.length() - 1, 1);
        }
    } else {
        repl.offset = 0;
        repl.length = 0;
        repl.replacementText = QStringLiteral("");
    }

    return repl;
}

KDevelop::DocumentRange ReplacementParser::composeNextNodeRange(size_t offset, size_t length)
{
    qDebug() << "count: " << cReplacements << "\toffset: " << offset << "\tlength: " << length << '\n';
    KDevelop::DocumentRange range{ KDevelop::IndexedString(), KTextEditor::Range::invalid() };
    /// See https://github.com/CarlosNihelton/kdev-clang-tidy/issues/2.
    if (offset < 1 || offset + length >= m_sourceView.length()) {
        return range;
    }

    range.document = i_source;
    auto sourceView = m_sourceView.substr(currentOffset, offset - currentOffset);

    auto pos = ::countOfRowAndColumnToTheEndOfSubstr(sourceView);

    if (pos.first == 0) {
        currentColumn += pos.second;
    } else {
        currentColumn = pos.second;
    }
    currentLine += pos.first;
    currentOffset = offset;

    if (length == 0) {
        range.setBothColumns(currentColumn);
        range.setBothLines(currentLine);
        return range;
    }

    sourceView = m_sourceView.substr(offset, length);
    pos = ::countOfRowAndColumnToTheEndOfSubstr(sourceView);

    KTextEditor::Cursor start(currentLine, currentColumn);

    size_t endCol;
    if (pos.first == 0) {
        endCol = currentColumn + pos.second;
    } else {
        endCol = pos.second;
    }

    KTextEditor::Cursor end(currentLine + pos.first, endCol);

    range.setRange(start, end);

    return range;
}
} // namespace ClangTidy

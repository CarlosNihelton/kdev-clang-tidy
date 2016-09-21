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

#ifndef PROBLEMSOLUTIONPARSER_H
#define PROBLEMSOLUTIONPARSER_H

#include "clangtidyparser.h"
#include "replacementparser.h"
#include <interfaces/iproblem.h>
namespace ClangTidy
{
class ProblemSolutionParser
{
    using Problems = QVector<KDevelop::IProblem::Ptr>;

public:
    ProblemSolutionParser() = default;
    ProblemSolutionParser(const QStringList& jobStdOut, const QString& yamlFileName);
    ~ProblemSolutionParser() = default;
    void addStdOutData(const QStringList& data);
    void setReplacementsFileName(const QString& yamlFileName);
    void parse();
    Problems problemsAndSolutions(){return m_problemsAndSolutions;}

private:
    QStringList m_standardOutput; // from clang-tidy's job.
    QString m_replacementsFile; // replacements yaml file.
    ClangtidyParser m_problemParser;
    ReplacementParser m_solutionParser;
    Problems m_problemsAndSolutions;
};
}
#endif // PROBLEMSOLUTIONPARSER_H

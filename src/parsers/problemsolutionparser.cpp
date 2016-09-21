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

#include "problem.h"
#include "problemsolutionparser.h"

namespace ClangTidy
{
ProblemSolutionParser::ProblemSolutionParser(const QStringList& jobStdOut, const QString& yamlFileName)
    : m_standardOutput{ jobStdOut }
    , m_replacementsFile{ yamlFileName }
    , m_solutionParser(yamlFileName)
{
    m_problemParser.addData(m_standardOutput);
    
}

void ProblemSolutionParser::addStdOutData(const QStringList& data)
{
    m_standardOutput = data;
    m_problemParser.addData(m_standardOutput);
}

void ProblemSolutionParser::setReplacementsFileName(const QString& yamlFileName)
{
    m_replacementsFile = yamlFileName;
    m_solutionParser.setReplacementsFileName(m_replacementsFile);
}

void ProblemSolutionParser::parse()
{
    if (!m_problemsAndSolutions.isEmpty())
        return;

    if (m_standardOutput.isEmpty())
        return;

    m_problemParser.parse();
    auto allProblems = m_problemParser.problems();

    if (allProblems.isEmpty())
        return;
        
    if(m_replacementsFile.isEmpty()){
        for(auto p : allProblems){
            m_problemsAndSolutions.push_back(p);
        }
        return;
    }
    
    m_solutionParser.parse();
    
    auto allFixits = m_solutionParser.allReplacements();
    
    if (allFixits.isEmpty()){
            for(auto p : allProblems){
            m_problemsAndSolutions.push_back(p);
        }
        return;
    }
    
    int problemsIterator = 0;
    int fixesIterator = 0;
    for (; problemsIterator < allProblems.length() - 1; ++problemsIterator) {
        size_t line = allProblems[problemsIterator]->finalLocation().start().line();
        size_t nextLine = allProblems[problemsIterator + 1]->finalLocation().start().line();

        if (allFixits[fixesIterator].line == line) {
            Fixits fixits;
            for (; fixesIterator < allFixits.length(); ++fixesIterator) {
                if (allFixits[fixesIterator].line >= line && allFixits[fixesIterator].line < nextLine) {
                    fixits.push_back(allFixits[fixesIterator]);
                }
            }
            allProblems[problemsIterator]->setReplacements(fixits);
        }
    }
    size_t line = allProblems[problemsIterator]->finalLocation().start().line();
    if (allFixits[fixesIterator].line == line) {
        Fixits fixits;
        for (; fixesIterator < allFixits.length(); ++fixesIterator) {
            if (allFixits[fixesIterator].line >= line) {
                fixits.push_back(allFixits[fixesIterator]);
            }
        }
       allProblems[problemsIterator]->setReplacements(fixits);
    }
    
    for(auto p : allProblems){
        m_problemsAndSolutions.push_back(p);
    }
}
    
} //namespace ClangTidy

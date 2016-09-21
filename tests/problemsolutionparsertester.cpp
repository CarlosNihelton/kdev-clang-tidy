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

#include "problemsolutionparsertester.h"

#include <QtTest>
#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include "parsers/problemsolutionparser.h"

using namespace KDevelop;
void
ProblemSolutionParserTester::initTestCase ()
{
  // Called before the first testfunction is executed
    AutoTestShell::init({ "kdevclangtidy" });
    TestCore::initialize(Core::NoUi);
}

void
ProblemSolutionParserTester::cleanupTestCase ()
{
  // Called after the last testfunction was executed
    TestCore::shutdown();  
}

void ProblemSolutionParserTester::doTest()
{
    ClangTidy::ProblemSolutionParser parser;
    
    // prepare QStringList from file to emulate clang-tidy stdout.
    QFile output_example_file("data/output_example_2");
    output_example_file.open(QIODevice::ReadOnly);
    QTextStream ios(&output_example_file);
    QStringList clangtidy_example_output;
    QString line;
    while (ios.readLineInto(&line)) {
        clangtidy_example_output << line;
    }

    QVERIFY(!clangtidy_example_output.isEmpty());

    parser.addStdOutData(clangtidy_example_output);
    parser.setReplacementsFileName("./data/plugin.cpp.yaml");
    
    parser.parse();
    auto v = parser.problemsAndSolutions();
    
    QVERIFY(!v.isEmpty());
    QCOMPARE(v.length(), 16);

    QCOMPARE(v[6]->finalLocation().document.str(), QStringLiteral("/plugin.cpp"));
    QCOMPARE(v[6]->finalLocation().start().line() + 1, 155); // as would appear in editor.
    QCOMPARE(v[6]->finalLocation().start().column()+1, 10);
    QCOMPARE(dynamic_cast<const ClangTidy::Problem*>(v[6].constData())->replacements().count(), 1);
    
    
//     QCOMPARE(v[0].offset, size_t(6263));
//     QCOMPARE(v[0].length, size_t(1));
//     QCOMPARE(v[0].replacementText, QString());
//     
// 
//     QCOMPARE(v[1].range.document.str(), QStringLiteral("data/plugin.cpp"));
//     QCOMPARE(v[1].offset, size_t(6267));
//     QCOMPARE(v[1].length, size_t(0));
//     QCOMPARE(v[1].replacementText, QString(" == nullptr"));
//     QCOMPARE(v[1].range.start().line() + 1, 155);
//     QCOMPARE(v[1].range.start().column() + 1, 13);
// 
//     QCOMPARE(v[2].range.document.str(), QStringLiteral("data/plugin.cpp"));
//     QCOMPARE(v[2].offset, size_t(6561));
//     QCOMPARE(v[2].length, size_t(1));
//     QCOMPARE(v[2].replacementText, QString());
//     QCOMPARE(v[2].range.start().line() + 1, 162);
//     QCOMPARE(v[2].range.start().column() + 1, 9);
// 
//     QCOMPARE(v[3].range.document.str(), QStringLiteral("data/plugin.cpp"));
//     QCOMPARE(v[3].offset, size_t(6569));
//     QCOMPARE(v[3].length, size_t(0));
//     QCOMPARE(v[3].replacementText, QStringLiteral(" == nullptr"));
//     QCOMPARE(v[3].range.start().line() + 1, 162);
//     QCOMPARE(v[3].range.start().column() + 1, 17);
// 
//     QCOMPARE(v[4].range.document.str(), QStringLiteral("data/plugin.cpp"));
//     QCOMPARE(v[4].offset, size_t(7233));
//     QCOMPARE(v[4].length, size_t(69));
//     QCOMPARE(v[4].replacementText, QStringLiteral("// TODO(cnihelton): auto detect clang-tidy executable"
//                                                   " instead of hard-coding it."));
//     QCOMPARE(v[4].range.start().line() + 1, 181);
//     QCOMPARE(v[4].range.start().column() + 1, 5);
}


QTEST_GUILESS_MAIN(ProblemSolutionParserTester);

// #include "problemsolutionparsertester.moc"

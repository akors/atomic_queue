// testutils.hpp

/*
 *   nuke-ms - Nuclear Messaging System
 *   Copyright (C) 2011  Alexander Korsunsky
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TESTUTILS_HPP
#define TESTUTILS_HPP

#include <iostream>
#include <cstring>


class TestModule
{
    static const char* const linestr; /**< Just a string with 80 dashes */
    const char* const testname; /**< Name of the test currently being run */

    /**< State of the current test module.
     * 0 if nothing failed, 1 if anything failed and -1 if a test could not be
     * performed due to an error.
    */
    int teststate;


    /** Constructor.
     * This is a singleton class, so no public constructor is
     * provided.
     *
     * Prints the "welcome message"
    */
    TestModule(const char* const _testname)
        : testname(_testname), teststate(0)
    {
        std::size_t testname_length  = std::strlen(testname);

        if (13 + testname_length < 80)
            std::cout<<&linestr[40+(13 + testname_length)/2+(13 + testname_length)%2]<<
                "  Testing "<<testname<<".  "<<
                &linestr[40+(13 + testname_length)/2]<<
                std::endl;
        else
            std::cout<<linestr<<"Testing "<<testname<<".\n"<<linestr<<std::endl;
    }

    /** No Copy construction allowed */
    TestModule(const TestModule& other);

public:
    ~TestModule()
    {
        std::size_t testname_length  = std::strlen(testname);

        if (17 + testname_length < 80)
            std::cout<<&linestr[40+(17 + testname_length)/2+(17 + testname_length)%2]<<
                "  "<<testname<<" test "<<(teststate ? "FAILED!  " : "passed.  ")<<
                &linestr[40+(17 + testname_length)/2]<<
                std::endl;
        else
            std::cout<<linestr<<
                "  "<<testname<<" test "<<
                (teststate ? "FAILED!  " : "passed.  ")
                <<'\n'<<linestr<<std::endl;

    }

    /** Static const reference to an instance of this class */
    static TestModule& test_module;

    static TestModule& instance(const char* const _testname)
    {
        /** Private instance of this class */
        static TestModule _instance(_testname);

        return _instance;
    }

    void failed_assert(
        const char* const expr,
        int line,
        const char* const filename
    )
    {
        if (!teststate)
            teststate = 1;

        std::cout<<"\n* Test assertion \""<< expr <<"\" failed.\n  Line "<<
            line<<" in file "<<filename<<"\n\n";
    }

    int conclude_test()
    {
        return teststate;
    }
};


#define DECLARE_TEST(testname) \
    TestModule& TestModule::test_module =TestModule::instance(testname);\
    const char* const TestModule::linestr = \
"--------------------------------------------------------------------------------";

#define TEST_ASSERT(expr) if (!(expr)) \
    TestModule::test_module.failed_assert( #expr, __LINE__, __FILE__)

#define CONCLUDE_TEST() TestModule::instance(NULL).conclude_test();




#endif // ifndef TESTUTILS_HPP

//  File: main.cpp
//
//  This file is part of the Ember Motor Controller firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//
//  Authors:
//  Jason Lefley
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

int main(int argc, char* argv[])
{
    // Get the top level suite from the registry
    CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

    // Adds the test to the list of test to run
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(suite);

    // Change the default outputter to a compiler error format outputter
    runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(), std::cerr));

    // Run the tests.
    bool wasSucessful = runner.run();

    // Return error code 1 if the one of test failed.
    return wasSucessful ? 0 : 1;
}

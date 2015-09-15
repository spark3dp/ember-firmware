//  File:   ScreenUT.cpp
//  Tests Screen-related methods that don't require actual display
//
//  This file is part of the Ember firmware.
//
//  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Richard Greene
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <http://www.gnu.org/licenses/>.

#include <stdlib.h>
#include <iostream>
#include <Screen.h>

int mainReturnValue = EXIT_SUCCESS;

#define	REP_LINE1      Left, 0, 0, 0, 0, "hey check it out: %s"
#define	LINE2          Left, 0, 0, 0, 0, "wassup?"
#define	REP_LINE3      Left, 0, 0, 0, 0, "with %s, anyway?"
#define	LINE4          Left, 0, 0, 0, 0, "nothing to see here"
#define	REP_LINE5      Left, 0, 0, 0, 0, "%s = value" 
#define	LINE6          Left, 0, 0, 0, 0, "bye bye" 

class TestReplaceableLine : public ReplaceableLine
{
public:
    TestReplaceableLine(Alignment align, unsigned char x, unsigned char y, 
                        unsigned char size, int color, const char* text) :
    ReplaceableLine(align, x, y, size, color, text)                         
    {
        
    };
    
    bool OrigTextIs(const char* text)
    {
        return std::string(text).compare(_text) == 0;
    };
    
    bool ReplacedTextIs(const char* text)
    {
        return std::string(text).compare(_replacedText) == 0;
    };
    
};

void test1() {
    std::cout << "ScreenUT test 1" << std::endl;
    
    TestReplaceableLine* repLine1 = new TestReplaceableLine(REP_LINE1);
    TestReplaceableLine* repLine3 = new TestReplaceableLine(REP_LINE3);
    TestReplaceableLine* repLine5 = new TestReplaceableLine(REP_LINE5);
    
    ScreenText* testText = new ScreenText;
    testText->Add(repLine1); 
    testText->Add(new ScreenLine(LINE2));
    testText->Add(repLine3);
    testText->Add(new ScreenLine(LINE4));
    testText->Add(repLine5);
    testText->Add(new ScreenLine(LINE6));
    
    // test getting the replaceable lines from ScreenText
    ReplaceableLine* t = testText->GetReplaceable(0);
    if (NULL != t)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=0th replaceable line not NULL" << std::endl; 
        mainReturnValue = EXIT_FAILURE;
    }
    t = testText->GetReplaceable(1);
    if (repLine1 != t)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=incorrect first replaceable line" << std::endl;        
        mainReturnValue = EXIT_FAILURE;
    }
    t = testText->GetReplaceable(2);
    if (repLine3 != t)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=incorrect second replaceable line" << std::endl;   
        mainReturnValue = EXIT_FAILURE;
    }
    t = testText->GetReplaceable(3);
    if (repLine5 != t)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=incorrect third replaceable line" << std::endl;  
        mainReturnValue = EXIT_FAILURE;
    }    
    t = testText->GetReplaceable(4);
    if (NULL != t)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=fourth replaceable line not NULL" << std::endl;  
        mainReturnValue = EXIT_FAILURE;
    }
    
    // test text replacement
    repLine1->ReplaceWith("testing123");
    if (!repLine1->OrigTextIs("hey check it out: %s"))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=original text corrupted in first replaceable line" << std::endl;
        mainReturnValue = EXIT_FAILURE;
    }
    if (!repLine1->ReplacedTextIs("hey check it out: testing123"))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=unexpected text in first replaceable line" << std::endl;   
        mainReturnValue = EXIT_FAILURE;
    }        
    
    repLine3->ReplaceWith("another test string");
    if (!repLine3->OrigTextIs("with %s, anyway?"))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=original text corrupted in second replaceable line" << std::endl;  
        mainReturnValue = EXIT_FAILURE;
    }
    if (!repLine3->ReplacedTextIs("with another test string, anyway?"))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=unexpected text in second replaceable line" << std::endl;  
        mainReturnValue = EXIT_FAILURE;
    }        
    
    repLine5->ReplaceWith("  how about this   ");
    if (!repLine5->OrigTextIs("%s = value"))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=original text corrupted in third replaceable line" << std::endl;  
        mainReturnValue = EXIT_FAILURE;
    }
    if (!repLine5->ReplacedTextIs("  how about this    = value"))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=unexpected text in third replaceable line" << std::endl;  
        mainReturnValue = EXIT_FAILURE;
    }        
    
    // test case where no replaceable lines in the ScreenText
    delete testText;
    testText = new ScreenText;
    testText->Add(new ScreenLine(LINE2));
    testText->Add(new ScreenLine(LINE4));
    testText->Add(new ScreenLine(LINE6));

    t = testText->GetReplaceable(1);
    if (NULL != t)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=no replaceable lines, but one found anyway" << std::endl;  
        mainReturnValue = EXIT_FAILURE;
    }
    
    // test case where no format string in the "replaceable" text    
    repLine1 = new TestReplaceableLine(LINE6);
    repLine1->ReplaceWith("should be no change");
    if (!repLine1->OrigTextIs("bye bye"))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=original text corrupted when no format string" << std::endl; 
        mainReturnValue = EXIT_FAILURE;
    }
    if (!repLine1->ReplacedTextIs("bye bye"))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=text replaced when no format string" << std::endl;     
        mainReturnValue = EXIT_FAILURE;
    } 
    delete testText;
    delete repLine1;   
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% ScreenUT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (ScreenUT)" << std::endl;
    test1();
    std::cout << "%TEST_FINISHED% time=0 test1 (ScreenUT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (mainReturnValue);
}


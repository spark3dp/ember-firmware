/* 
 * File:   ScreenUT.cpp
 * Author: Richard Greene
 * 
 * Tests Screen-related methods that don't require actual display.
 *
 * Created on Aug 6, 2014, 1:23:59 PM
 */

#include <stdlib.h>
#include <iostream>
#include <Screen.h>

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
    if(NULL != t)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=0th replaceable line not NULL" << std::endl;        
    }
    t = testText->GetReplaceable(1);
    if(repLine1 != t)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=incorrect first replaceable line" << std::endl;        
    }
    t = testText->GetReplaceable(2);
    if(repLine3 != t)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=incorrect second replaceable line" << std::endl;        
    }
    t = testText->GetReplaceable(3);
    if(repLine5 != t)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=incorrect third replaceable line" << std::endl;        
    }    
    t = testText->GetReplaceable(4);
    if(NULL != t)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=fourth replaceable line not NULL" << std::endl;        
    }
    
    // test text replacement
    repLine1->Replace(NULL, std::string("testing123"));
    if(!repLine1->OrigTextIs("hey check it out: %s"))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=original text corrupted in first replaceable line" << std::endl;                
    }
    if(!repLine1->ReplacedTextIs("hey check it out: testing123"))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=unexpected text in first replaceable line" << std::endl;                
    }        
    
    repLine3->Replace(NULL, std::string("another test string"));
    if(!repLine3->OrigTextIs("with %s, anyway?"))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=original text corrupted in second replaceable line" << std::endl;                
    }
    if(!repLine3->ReplacedTextIs("with another test string, anyway?"))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=unexpected text in second replaceable line" << std::endl;                
    }        
    
    repLine5->Replace(NULL, std::string("  how about this   "));
    if(!repLine5->OrigTextIs("%s = value"))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=original text corrupted in third replaceable line" << std::endl;                
    }
    if(!repLine5->ReplacedTextIs("  how about this    = value"))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=unexpected text in third replaceable line" << std::endl;                
    }        
    
    // test case where no replaceable lines in the ScreenText
    delete testText;
    testText = new ScreenText;
    testText->Add(new ScreenLine(LINE2));
    testText->Add(new ScreenLine(LINE4));
    testText->Add(new ScreenLine(LINE6));

    t = testText->GetReplaceable(1);
    if(NULL != t)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=no replaceable lines, but one found anyway" << std::endl;        
    }
    
    // test case where no format string in the "replaceable" text    
    repLine1 = new TestReplaceableLine(LINE6);
    repLine1->Replace(NULL, std::string("should be no change"));
    if(!repLine1->OrigTextIs("bye bye" ))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=original text corrupted when no format string" << std::endl;                
    }
    if(!repLine1->ReplacedTextIs("bye bye" ))
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (ScreenUT) message=text replaced when no format string" << std::endl;                
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

    return (EXIT_SUCCESS);
}


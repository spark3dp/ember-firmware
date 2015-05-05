/*
 * Command.h
 * Author: Jason Lefley
 * Date  : 2015-04-27
 */

#ifndef COMMAND_H
#define COMMAND_H

#include <stdint.h>

class Command
{
    public:
        Command();
        ~Command();

        void SetRegister(unsigned char value);
        unsigned char Register() const;

        void SetAction(unsigned char value);
        unsigned char Action() const;

        void SetParameterBytes(const unsigned char* buffer);
        int32_t Parameter() const;

    private:
        Command(const Command&);
        
    private:
        unsigned char mRegister;
        unsigned char mAction;
        int32_t mParameter;
};

#endif /* COMMAND_H */

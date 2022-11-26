//
//  main.cpp
//  atomic
//
//  Created by GUSTAVO CAMPOS on 29/08/2021.
//

#include "atomicx.hpp"

#include <stdio.h>
#include <unistd.h>

#include <string.h>
#include <stdint.h>
#include <setjmp.h>

#include <stdlib.h>

#include <iostream>

void* ref;

class WaitThread : public atomicx::thread
{
private:
    size_t stack [128];

public:
    WaitThread () : thread (stack)
    {

    }

    ~WaitThread ()
    {
        std::cout << " WaitThread : ID:" << ((size_t) this) << ", being destructed." << std::endl;
    }

    void run ()
    {
        size_t nMessage = 0;

        std::cout << __func__ << ", Starting waiting..." << std::endl << std::flush;

        while (true)
        {
            std::cout << __func__ << ", WAIT for a  message... " << std::endl << std::flush;

            Wait (ref, 1, nMessage);

            std::cout << __func__ << ", received a message from: " << std::hex << nMessage << std::dec << std::endl << std::flush;

        }

    }
};

class Test : public atomicx::thread
{
    private:
        size_t stack [128];

    public:
        Test () : thread(stack)
        {

        }

        ~Test ()
        {
            std::cout << "ID:" << ((size_t) this) << ", being destructed." << std::endl;
        }

        void run ()
        {
            std::cout << __func__ << ": Thread initiating: " << std::hex << this << std::dec << std::endl;

            int nValue = 0;

            while (yield(0))
            {
                std::cout << __func__ << ": Value: [" << nValue++ << "], ID:" << std::hex << (this) << std::dec << ", StackSize: " << GetStackSize () << " bytes" << ((char) 13) << std::flush;

                Notify (ref, 1, (size_t) this);
            }
        }
};

int main ()
{

    WaitThread wait1;
     //WaitThread wait2;

    Test test1;
    // Test test2;
    // Test test3;
    // Test test4;
    // Test test5;
    // Test test6;
    // Test test7;
    // Test test8;

    for (auto& th : atomicx::kernel)
    {
        std::cout << __func__ << ": listing thread: " << th().GetName () << std::hex << ", ID: " << ((size_t) &(th())) << std::dec << std::endl;
    }

    atomicx::kernel.start ();

    return 0;
}
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

    const char* GetName ()
    {
        return "WaitThread";
    }

    void run ()
    {
        size_t nMessage = 0;

        std::cout << __func__ << ", Starting waiting..." << std::endl << std::flush;

        while (true)
        {
            //std::cout << __func__ << ", WAIT for a  message... " << std::endl << std::flush;

            Wait (ref, 1, nMessage);

            //std::cout << __func__ << ", received a message from: " << std::hex << nMessage << std::dec << std::endl << std::flush;

        }

    }

    void StackOverflowHandler ()
    {
        
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

        const char* GetName ()
        {
            return "Test";
        }

        void run ()
        {
            std::cout << __func__ << ": Thread initiating: " << std::hex << this << std::dec << std::endl;

            size_t nNotified = 0;
            int nValue = 0;

            while (yield(0))
            {
                // Also force context change
                nNotified = NotifyAll (ref, 1, (size_t) this, 2,1);

                std::cout << __func__ << ": Value: [" << nValue++ << "], Notified: [" << (ssize_t) nNotified << "]. ID:" << std::hex << (this) << std::dec << ", StackSize: " << GetStackSize () << "/" << GetMaxStackSize () << ((char) 27) << "[K" << ((char) 13) << std::flush;
            }
        }

        void StackOverflowHandler ()
        {

        }
};

int main ()
{

    Test test1;
    Test test2;
    Test test3;
    Test test4;
    Test test5;
    Test test6;
    Test test7;
    // Test test8;

    WaitThread wait1;
    WaitThread wait2;
    WaitThread wait3;

    for (auto& th : atomicx::kernel)
    {
        std::cout << __func__ << ": listing thread: " << th().GetName () << std::hex << ", ID: " << ((size_t) &(th())) << std::dec << std::endl;
    }

    atomicx::kernel.start ();

    return 0;
}
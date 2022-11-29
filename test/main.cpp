//
//  main.cpp
//  atomic
//
//  Created by GUSTAVO CAMPOS on 29/08/2021.
//

#include "atomicx.hpp"

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include <string.h>
#include <stdint.h>

#include <stdlib.h>
#include <iostream>

void* ref;

/*
 * Define the default ticket granularity
 * to milliseconds or round tick if -DFAKE_TICKER
 * is provided on compilation
 */
atomicx_time atomicx::Kernel::GetTick (void)
{
    struct timeval tp;
    gettimeofday (&tp, NULL);

    return (atomicx_time)tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

/*
 * Sleep for few Ticks, since the default ticket granularity
 * is set to Milliseconds (if -DFAKE_TICKET provide will it will
 * be context switch countings), the thread will sleep for
 * the amount of time needed till next thread start.
 */
void atomicx::Kernel::SleepTick(atomicx_time nSleep)
{
    usleep ((useconds_t)nSleep * 1000);
}

class WaitCounter : public atomicx::thread
{
private:
    size_t stack [128];

public:
    static size_t nCounter;

    WaitCounter () : thread (10, stack)
    {

    }

    ~WaitCounter ()
    {
        std::cout << " WaitCounter : ID:" << ((size_t) this) << ", being destructed." << std::endl;
    }

    const char* GetName ()
    {
        return "WaitCounter";
    }

    void run ()
    {
        size_t nType;
        size_t nMessage = 0;

        std::cout << this << GetName () << __func__ << ", Starting waiting..." << std::endl << std::flush;

        while (true)
        {
            //std::cout << __func__ << ", WAIT for a  message... " << std::endl << std::flush;

            if (WaitAll (ref, nType, nMessage, 1000) == false)
            {
                std::cout << this << "<<<TIMEOUT>>>." << __func__ << ": Wait timeout detected." << std::endl;
            }
            else
            {
                nCounter++;
            }
        }
    }

    void StackOverflowHandler ()
    {

    }

};

size_t WaitCounter::nCounter=0; 

class WaitThread : public atomicx::thread
{
private:
    size_t stack [128];

public:
    WaitThread () : thread (10, stack)
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

            if (Wait (ref, 1, nMessage, 100) == false)
            {
                std::cout << this << "<<<TIMEOUT>>>." << __func__ << ": Wait timeout detected." << std::endl;
            }
            else
            {
                TRACE (TRACE, atomicx::kernel.GetTick () << ":" << GetName () << "." << __func__ << ", received a message from: " << std::hex << nMessage << std::dec << std::endl << std::flush);
            }
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
        Test () : thread(300, stack)
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

            while (true)
            {
                // Also force context change
                if ((nNotified = NotifyAll (ref, 1, (size_t) this, 1,1000)) == -1)
                {
                    std::cout << this << "<<<TIMEOUT>>>." << __func__ << ": Wait timeout detected." << std::endl;
                }
                
                std::cout << __func__ << "<" << atomicx::kernel.GetTick () << "> WaitCounter: " << WaitCounter::nCounter <<", Value: [" << nValue++ << "], Notified: [" << (ssize_t) nNotified << "]. ID:" << std::hex << (this) << std::dec << ", StackSize: " << GetStackSize () << "/" << GetMaxStackSize () << ((char) 27) << "[K" << std::endl << ((char) 13) << std::flush;
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

     WaitThread wait1;

    //Test test4;
    Test test5;

    WaitThread wait2;
    WaitThread wait3;

    // // Test test6;
    Test test7;
    Test test8;

    WaitThread wait4;
    WaitThread wait5;
    WaitThread wait6;
    WaitThread wait7;

    WaitCounter counter1;
    
    for (auto& th : atomicx::kernel)
    {
        std::cout << __func__ << ": listing thread: " << th().GetName () << std::hex << ", ID: " << ((size_t) &(th())) << std::dec << std::endl;
    }

    atomicx::kernel.start ();

    return 0;
}
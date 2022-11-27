//
//  atomic.cpp
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

//#include <iostream> 

#define _THREAD(thr) ((thread*)thr)

namespace atomicx
{    

    /*
     * Tiemout functions 
    */

#if 0
    Timeout::Timeout () : m_timeoutValue (0)
    {
        Set (0);
    }

    Timeout::Timeout (atomicx_time nTimeoutValue) : m_timeoutValue (0)
    {
        Set (nTimeoutValue);
    }

    void Timeout::Set(atomicx_time nTimeoutValue)
    {
        m_timeoutValue = nTimeoutValue ? nTimeoutValue + Atomicx_GetTick () : 0;
    }

    bool Timeout::IsTimedout()
    {
        return (m_timeoutValue == 0 || Atomicx_GetTick () < m_timeoutValue) ? false : true;
    }

    atomicx_time Timeout::GetRemaining()
    {
        auto nNow = Atomicx_GetTick ();

        return (nNow < m_timeoutValue) ? m_timeoutValue - nNow : 0;
    }

    atomicx_time Timeout::GetDurationSince(atomicx_time startTime)
    {
        return startTime - GetRemaining ();
    }
#endif

    /*
    * Kernel functions 
    */

    Kernel& Kernel::GetInstance()
    {
        static Kernel kernel;

        return kernel; 
    }

    Kernel::Kernel ()
    {

    }

    inline void Kernel::SetNextThread (void)
    {
        do
        {
            m_pCurrent = m_pCurrent == nullptr ? (thread*) m_first : (thread*) m_pCurrent->next;

            if (m_pCurrent != nullptr) switch (m_pCurrent->m_status)
            {
                // if waiting or syncWait (see syncNotify) dont select
                case status::wait:
                case status::syncWait:
                    continue;
                    ;;

                // if sleeping automatically set to running
                case status::sleeping:
                    m_pCurrent->m_status = status::running;
                case status::starting:
                case status::running:
                default:
                    return;
            }

        } while (true);
    }

    void Kernel::start(void)
    {
        nRunning = true;

        m_pCurrent = (thread*) m_first;

        do
        {
            SetNextThread ();

            if (setjmp (m_context) == 0)
            {
                if (m_pCurrent->m_status == status::starting)
                {
                    volatile uint8_t __var = 0;
                    m_pStackStart =  &__var;

                    m_pCurrent->run ();
                }
                else
                {
                    longjmp (m_pCurrent->m_context, 1);
                }
            }

            
        } while (m_pCurrent != nullptr && nRunning);
    }

    /*
    * thread functions
    */

    bool thread::yield(atomicx_time aTime, status type)
    {
        (void) aTime;
    
        if (! kernel.nRunning) return 1;
                    
        kernel.m_pCurrent->m_status = type;

        volatile uint8_t __var = 0;
        kernel.m_pCurrent->m_pStackEnd = (uint8_t*) &__var;

        kernel.m_pCurrent->m_nStackSize = (kernel.m_pStackStart - kernel.m_pCurrent->m_pStackEnd);

        // Adding a 4 size_t's as padding to allow safe execution within the
        // thread context changing procedures 
        if (kernel.m_pCurrent->m_nMaxStackSize >= kernel.m_pCurrent->m_nStackSize)
        {
            memcpy ((void*) kernel.m_pCurrent->m_pStack, (const void*)  kernel.m_pCurrent->m_pStackEnd, kernel.m_pCurrent->m_nStackSize);

            if (setjmp (kernel.m_pCurrent->m_context) == 0)
            {
                longjmp (kernel.m_context, 1);
            }
            
            //m_pCurrent->m_nStackSize = (m_pStackStart - m_pCurrent->m_pStackEnd) + 4;
            if (memcpy ((void*) kernel.m_pCurrent->m_pStackEnd, (const void*) kernel.m_pCurrent->m_pStack, kernel.m_pCurrent->m_nStackSize) != kernel.m_pCurrent->m_pStackEnd)
            {
                exit (-1);
            }
        }
        else
        {
            StackOverflowHandler ();
            exit (-2);
        }

        kernel.m_pCurrent->m_status = status::running;

        return true;
    }

    thread::~thread ()
    {
        kernel.Detach (*this);
    }

    const char* thread::GetName()
    {
        return "thread";
    }

    unsigned int thread::GetStatus ()
    {
        return (unsigned int) m_status;
    }

    size_t thread::GetStackSize ()
    {
        return m_nStackSize;
    }

    size_t thread::GetMaxStackSize ()
    {
        return m_nMaxStackSize;
    }

}

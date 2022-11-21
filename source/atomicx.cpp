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

#define _THREAD(thr) ((thread*)thr)

namespace atomicx
{    
    /*
    * Kernel functions 
    */

    void Kernel::SetNextThread (void)
    {
        do
        {
            m_pCurrent = m_pCurrent == nullptr ? (thread*) m_first : (thread*) m_pCurrent->next;
        } while (m_pCurrent == nullptr);

        if (m_pCurrent->m_status == status::sleeping)
        { 
            m_pCurrent->m_status = status::running;
        }
    }

    void Kernel::start(void)
    {
        volatile uint8_t nStart;
        m_pStackStart = &nStart;

        nRunning = true;

        m_pCurrent = (thread*) m_first;

        while (m_pCurrent != nullptr && nRunning)
        {
            if (setjmp (m_pCurrent->m_joinContext) == 0)
            {
                if (m_pCurrent->m_status == status::starting)
                {
                    m_pCurrent->run ();
                }
                else
                {
                    longjmp (m_pCurrent->m_context, 1);
                }
            }

            SetNextThread ();
        } 
    }

    /*
    * thread functions
    */

    thread::~thread ()
    {
        m_kContext.Detach (*this);
    }

    const char* thread::GetName()
    {
        return "thread";
    }

    unsigned int thread::GetStatus ()
    {
        return (unsigned int) m_status;
    }

    void thread::yield(atomicx_time aTime, status type)
    {
        (void) aTime;
    
        if (m_kContext.nRunning)
        {
            volatile uint8_t nStop = 0xAA;
            m_pStackStop = &nStop;
            
            if (type == status::running)
            {
                m_status = status::sleeping;
            }

            // Adding a 4 size_t's as padding to allow safe execution within the
            // thread context changing procedures 
            if (m_nMaxStackSize >= (m_nStackSize = (m_kContext.m_pStackStart - m_pStackStop)))
            {
                memcpy ((void*) m_pStack, (const void*) m_pStackStop, m_nStackSize);

                if (setjmp (m_context) == 0)
                {
                    //longjmp (m_joinContext, 1);
                    longjmp (m_joinContext, 1);
                }

                memcpy ((void*) m_kContext.m_pCurrent->m_pStackStop, (const void*) m_kContext.m_pCurrent->m_pStack, m_kContext.m_pCurrent->m_nStackSize);
            }
        }
    }
}

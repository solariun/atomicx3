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

    inline void Kernel::SetNextThread (void)
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
        volatile uint8_t __var = 0;
        m_pStackStart =  &__var;

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

    bool Kernel::yield(atomicx_time aTime, status type)
    {
        (void) aTime;
    
        if (nRunning)
        {            
            if (type == status::running)
            {
                m_pCurrent->m_status = status::sleeping;
            }

            m_pCurrent->m_pStackEnd = (uint8_t*) &aTime;

            m_pCurrent->m_nStackSize = (m_pStackStart - m_pCurrent->m_pStackEnd);

            // Adding a 4 size_t's as padding to allow safe execution within the
            // thread context changing procedures 
            if (m_pCurrent->m_nMaxStackSize >= m_pCurrent->m_nStackSize)
            {
                memcpy ((void*) m_pCurrent->m_pStack, (const void*)  m_pCurrent->m_pStackEnd, m_pCurrent->m_nStackSize);

                if (setjmp (m_pCurrent->m_context) == 0)
                {
                    longjmp (m_pCurrent->m_joinContext, 1);
                }
                else
                {
                    m_pCurrent->m_nStackSize = m_pStackStart - m_pCurrent->m_pStackEnd;
                    if (memcpy ((void*) m_pCurrent->m_pStackEnd, (const void*) m_pCurrent->m_pStack, m_pCurrent->m_nStackSize) != m_pCurrent->m_pStackEnd)
                    {
                        exit (-1);
                    }
                }
            }
            else
            {
                exit (-2);
            }

            m_pCurrent->m_status = status::running;

            return true;
        }

        return false;
    }


    /*
    * thread functions
    */

    thread::~thread ()
    {
        m_Kernel.Detach (*this);
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

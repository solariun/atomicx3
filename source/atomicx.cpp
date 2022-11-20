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

#include <iostream>

namespace atomicx
{    
    /*
    * Kernel functions 
    */

    void Kernel::SetNextThread (void)
    {
        m_pCurrent = m_pCurrent == nullptr ? (thread*) m_first : (thread*) m_pCurrent->next;
    }

    void Kernel::start(void)
    {
        uint8_t nStart;
        m_pStackStart = &nStart;

        nRunning = true;

        m_pCurrent = (thread*) m_first;

        while (m_pCurrent != nullptr && nRunning)
        {
            std::cout << "Thread ID:" << ((uint8_t) m_pCurrent->GetStatus ()) << std::endl; 

            if (m_pCurrent->m_status == status::starting)
            {
                if (setjmp (m_context) == 0)
                {
                    m_pCurrent->run ();
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

    status thread::GetStatus ()
    {
        return m_status;
    }
}

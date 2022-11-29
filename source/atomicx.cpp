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

    /*
    * timeout methods implementations
    */

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
        m_timeoutValue = nTimeoutValue ? nTimeoutValue + kernel.GetTick () : 0;
    }

    bool Timeout::IsTimedout()
    {
        return (m_timeoutValue == 0 || kernel.GetTick () < m_timeoutValue) ? false : true;
    }

    atomicx_time Timeout::GetRemaining()
    {
        auto nNow = kernel.GetTick ();

        return (nNow < m_timeoutValue) ? m_timeoutValue - nNow : 0;
    }

    atomicx_time Timeout::GetDurationSince(atomicx_time startTime)
    {
        return startTime - GetRemaining ();
    }

    /*
    * Kernel methods implementations
    */

    Kernel& Kernel::GetInstance()
    {
        static Kernel kernel;

        return kernel;
    }

    Kernel::Kernel ()
    {

    }

    thread* Kernel::GetNextThread (void)
    {
        thread* thCandidate = nullptr;
        thread* iterator = m_pCurrent;

        atomicx_time nNow;

        TRACE (DEBUG, "Num Threads: " << m_nNodeCounter);

        for (size_t nCounter=0; nCounter < m_nNodeCounter; nCounter++)
        {
            nNow = GetTick ();

            iterator = iterator == nullptr ? (thread*) m_first : iterator->next == nullptr ? m_first : (thread*) iterator->next;

            if (iterator != nullptr) TRACE (DEBUG, "LOOP st: " << (int) iterator->m_status <<", next: " << iterator->m_tmNextEvent << ": " << iterator->GetName ());

            if (iterator != nullptr) switch (iterator->m_status)
            {
                // if waiting or syncWait (see syncNotify) dont select
                case status::wait:
                case status::syncWait:
                    if (iterator->m_tmNextEvent == 0) continue;

                case status::sleeping:
                    // Select the sooner next event time
                    if (thCandidate == nullptr || thCandidate->m_tmNextEvent > iterator->m_tmNextEvent)
                        thCandidate = iterator;

                    break;

                case status::running:
                    // Should never be a running here, error, stop
                    thCandidate = nullptr;

                case status::starting:
                case status::now:
                default:
                    thCandidate = iterator;
                    thCandidate->m_tmNextEvent = nNow;

                    nCounter = m_nNodeCounter;
                    continue;
            }

            // if already expired execute it
            if (thCandidate && thCandidate->m_tmNextEvent <= nNow) break;
        }

        nNow = GetTick ();

        TRACE (DEBUG,
            "RETURN: thCandidate: [" << thCandidate << "], m_pCurrent: " \
            << iterator << "." << (iterator ? iterator->GetName () : "NULL" ) \
            << ", Status: " << (iterator ? (int) iterator->m_status : (int)0 ) \
            << ", Next Event: " <<  (iterator ? iterator->m_tmNextEvent : 0) << ", Now: " << nNow
        );

        if (thCandidate)
        {
            if (thCandidate->m_tmNextEvent > nNow)
            {
                TRACE (DEBUG, "Sleeping " << (thCandidate->m_tmNextEvent - nNow) << " ticks");
                SleepTick (thCandidate->m_tmNextEvent - nNow);
            }
            else
            {
                thCandidate->m_tmLateBy = nNow - thCandidate->m_tmNextEvent;
                TRACE (DEBUG, "LATE by " << (thCandidate->m_tmLateBy) << " ticks");
            }
        }

        return thCandidate;
    }

    void Kernel::start(void)
    {
        nRunning = true;

        m_pCurrent = (thread*) m_first;

        while (nRunning && (m_pCurrent = GetNextThread ()))
        {
            if (m_pCurrent && setjmp (m_context) == 0)
            {
                if (m_pCurrent->m_status == status::starting)
                {
                    volatile uint8_t __var = 0;
                    m_pStackStart =  &__var;

                    m_pCurrent->run ();
                }
                else
                {
                    TRACE (DEBUG, "1 JUMP st: " << (int) m_pCurrent->m_status <<", next: " << m_pCurrent->m_tmNextEvent << ": " << m_pCurrent->GetName ());

                    switch (m_pCurrent->m_status)
                    {
                    case status::wait:
                    case status::syncWait:
                        m_pCurrent->m_status = status::timeout;
                        break;

                    default:
                        m_pCurrent->m_status = status::running;
                    }

                    TRACE (DEBUG, "2 JUMP st: " << (int) m_pCurrent->m_status <<", next: " << m_pCurrent->m_tmNextEvent << ": " << m_pCurrent->GetName ());

                    longjmp (m_pCurrent->m_context, 1);
                }
            }


        }
    }

    thread& Kernel::operator()()
    {
        return *m_pCurrent;
    }

    /*
    * thread methods implementations
    */

    bool thread::yield(atomicx_time aTime, status type)
    {
        (void) aTime;

        if (! kernel.nRunning) return 1;

        // Get final util stack point
        volatile uint8_t __var = 0;
        kernel.m_pCurrent->m_pStackEnd = (uint8_t*) &__var;


        TRACE (DEBUG, "SWITCH type: " << (int) type << ", waitFor: " << aTime << ", Current: " << kernel.m_pCurrent << ". nextEvent: " << kernel.m_pCurrent->m_tmNextEvent << ", Status: " << (int) kernel.m_pCurrent->m_status);

        //Prepare status
        // if yield (x>0), the default status is status::ctxSwitch
        // .  X > 0, it will work as a sleep, otherwise use
        // .  the given default nice value
        //
        switch (type)
        {

        case status::ctxSwitch:
            kernel.m_pCurrent->m_tmNextEvent =  kernel.GetTick () + (aTime ? aTime : kernel.m_pCurrent->m_nNice);
            type = status::sleeping;
            break;

        case status::wait:
        case status::syncWait:
            // if wait for == 0, wait indefinitely
            if (aTime == 0)
            {
                kernel.m_pCurrent->m_tmNextEvent = 0;
                break;
            }

        case status::sleeping:
            kernel.m_pCurrent->m_tmNextEvent =  kernel.GetTick () + aTime;
            break;

        case status::now:
            kernel.m_pCurrent->m_tmNextEvent =  kernel.GetTick ();
            break;

        default:
            kernel.m_pCurrent->m_tmNextEvent = (atomicx_time) kernel.GetTick ();
        }

        TRACE (DEBUG, "SWITCH final: " << kernel.m_pCurrent << ". nextEvent: " << kernel.m_pCurrent->m_tmNextEvent << ", Selected status: " << (int) type);

        kernel.m_pCurrent->m_status = type;

        // Calculate used stack size
        kernel.m_pCurrent->m_nStackSize = (kernel.m_pStackStart - kernel.m_pCurrent->m_pStackEnd);

        // Adding a 4 size_t's as padding to allow safe execution within the
        // thread context changing procedures
        if (kernel.m_pCurrent->m_nMaxStackSize >= kernel.m_pCurrent->m_nStackSize)
        {
            // Save full stack context
            memcpy ((void*) kernel.m_pCurrent->m_pStack, (const void*)  kernel.m_pCurrent->m_pStackEnd, kernel.m_pCurrent->m_nStackSize);

            if (setjmp (kernel.m_pCurrent->m_context) == 0)
            {
                longjmp (kernel.m_context, 1);
            }

            // Restore full stack context
            if (memcpy ((void*) kernel.m_pCurrent->m_pStackEnd, (const void*) kernel.m_pCurrent->m_pStack, kernel.m_pCurrent->m_nStackSize) != kernel.m_pCurrent->m_pStackEnd)
            {
                exit (-1);
            }
        }
        else
        {
            StackOverflowHandler ();
            exit (0);
        }

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

    status thread::GetStatus ()
    {
        return m_status;
    }

    size_t thread::GetStackSize ()
    {
        return m_nStackSize;
    }

    size_t thread::GetMaxStackSize ()
    {
        return m_nMaxStackSize;
    }

    void thread::SetNice (atomicx_time nNice)
    {
        m_nNice = nNice;
    }

    /*
    * ------------------------------------------------------------
    * Mutex methods implementations
    * ------------------------------------------------------------
    */

    bool SmartLock::Lock(atomicx_time ttimeout)
    {
        if (m_control.nValue != 0) return false;

        // If not perform the task
        thread& k = kernel ();
        Timeout timeout(ttimeout);
        size_t nMessage = 0;

        TRACE (DEBUG, " -->>> TRYING TO EXCLUSIVE LOCK");

        // Get exclusive lock
        while (m_mutex.bExclusiveLock) if  (! k.Wait (m_mutex.bExclusiveLock, 1, nMessage, timeout.GetRemaining())) return false;

        m_mutex.bExclusiveLock = true;
        m_control.data.bLocked = true;

        // Wait all shared locks to be done
        while (m_mutex.nSharedLockCount) if (! k.Wait(m_mutex.nSharedLockCount,2, nMessage, timeout.GetRemaining())) return false;

        TRACE (DEBUG, " -->>> ACQUIRED EXCLUSIVE LOCK");

        return true;
    }

    void SmartLock::Unlock()
    {
        thread& k = kernel ();
        size_t nMessage = 0;

        if (m_mutex.bExclusiveLock == true)
        {
            m_mutex.bExclusiveLock = false;
            if (m_control.data.bLocked == true) m_control.data.bLocked = false;

            TRACE (DEBUG, " -->>> UNLOCKED");

            // Notify Other locks procedures
            k.NotifyAll (m_mutex.nSharedLockCount, 2, nMessage);
            k.Notify (m_mutex.bExclusiveLock, 1, nMessage);
        }
    }

    bool SmartLock::SharedLock(atomicx_time ttimeout)
    {
        if (m_control.nValue != 0) return false;

        thread& k = kernel ();
        Timeout timeout(ttimeout);
        size_t nMessage = 0;

        TRACE (DEBUG, " -->>> TRYING TO SHARED LOCK");

        // Wait for exclusive mutex
        while (m_mutex.bExclusiveLock) if (! k.Wait(m_mutex.bExclusiveLock, 1, nMessage, timeout.GetRemaining())) return false;

        m_mutex.nSharedLockCount++;
        m_control.data.bShared = true;

        // Notify Other locks procedures
        k.Notify (m_mutex.nSharedLockCount, 2, nMessage);

        TRACE (DEBUG, " -->>> ACQUIRED SHARED LOCK");

        return true;
    }

    void SmartLock::SharedUnlock()
    {
        size_t nMessage = 0;
        thread& k = kernel ();

        if (m_mutex.nSharedLockCount)
        {
            m_mutex.nSharedLockCount--;
            if (m_control.data.bShared == true) m_control.data.bShared = false;

            TRACE (DEBUG, " -->>> SHARED UNLOCKED. actives: " << m_mutex.nSharedLockCount);

            k.Notify(m_mutex.nSharedLockCount, 2, nMessage);
        }
    }

    size_t SmartLock::IsShared()
    {
        TRACE (DEBUG, " -->>> IS SHARED: " << m_mutex.nSharedLockCount);

        // If not perform original task
        return m_mutex.nSharedLockCount;
    }

    bool SmartLock::IsLocked()
    {
        TRACE (DEBUG, " -->>> IS LOCKED: " << m_mutex.bExclusiveLock);

        // If not perform original task
        return m_mutex.bExclusiveLock;
    }

    SmartLock::SmartLock (mutex& mutex) : m_mutex (mutex)
    {
    }

    SmartLock::~SmartLock()
    {
        TRACE(DEBUG, "<<<< DESTRUCTION >>>>");

        if (IsLocked () && m_control.data.bLocked == true)
        {
            Unlock ();
        }

        if (IsShared () && m_control.data.bShared == true)
        {
            SharedUnlock ();
        }
    }

}

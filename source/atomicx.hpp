//
//  atomic.hpp
//  atomic
//
//  Created by GUSTAVO CAMPOS on 29/08/2021.
//

#ifndef atomic_hpp
#define atomic_hpp

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

/* Official version */
#define ATOMICX_VERSION "1.3.0"
#define ATOMIC_VERSION_LABEL "AtomicX v" ATOMICX_VERSION " built at " __TIMESTAMP__

typedef uint32_t atomicx_time;

#ifdef PC
#include <iostream> 
#endif 

namespace atomicx
{
    #define GetStackPoint() ({volatile uint8_t ___var = 0; &___var;})
    #define STRINGIFY_2(a) #a
    #define STRINGIFY(a) STRINGIFY_2(a)


#if 0
    /*
    * ---------------------------------------------------------------------
    * timeout Implementation
    * ---------------------------------------------------------------------
    */

    atomicx::Timeout::Timeout () : m_timeoutValue (0)
    {
        Set (0);
    }

    atomicx::Timeout::Timeout (atomicx_time nTimeoutValue) : m_timeoutValue (0)
    {
        Set (nTimeoutValue);
    }

    void atomicx::Timeout::Set(atomicx_time nTimeoutValue)
    {
        m_timeoutValue = nTimeoutValue ? nTimeoutValue + Atomicx_GetTick () : 0;
    }

    bool atomicx::Timeout::IsTimedout()
    {
        return (m_timeoutValue == 0 || Atomicx_GetTick () < m_timeoutValue) ? false : true;
    }

    atomicx_time atomicx::Timeout::GetRemaining()
    {
        auto nNow = Atomicx_GetTick ();

        return (nNow < m_timeoutValue) ? m_timeoutValue - nNow : 0;
    }

    atomicx_time atomicx::Timeout::GetDurationSince(atomicx_time startTime)
    {
        return startTime - GetRemaining ();
    }
#endif 

    /*
    * ---------------------------------------------------------------------
    * Iterator Implementation
    * ---------------------------------------------------------------------
    */

    /**
     * @brief Timeout Check object
     */

    class Timeout
    {
        public:

            /**
             * @brief Default construct a new Timeout object
             *
             * @note    To decrease the amount of memory, Timeout does not save
             *          the start time.
             *          Special use case: if nTimeoutValue == 0, IsTimedout is always false.
             */
            Timeout ();

            /**
             * @brief Construct a new Timeout object
             *
             * @param nTimeoutValue  Timeout value to be calculated
             *
             * @note    To decrease the amount of memory, Timeout does not save
             *          the start time.
             *          Special use case: if nTimeoutValue == 0, IsTimedout is always false.
             */
            Timeout (atomicx_time nTimeoutValue);

            /**
             * @brief Set a timeout from now
             *
             * @param nTimeoutValue timeout in atomicx_time
             */
            void Set(atomicx_time nTimeoutValue);

            /**
             * @brief Check wether it has timeout
             *
             * @return true if it timeout otherwise 0
             */
            bool IsTimedout();

            /**
             * @brief Get the remaining time till timeout
             *
             * @return atomicx_time Remaining time till timeout, otherwise 0;
             */
            atomicx_time GetRemaining();

            /**
             * @brief Get the Time Since specific point in time
             *
             * @param startTime     The specific point in time
             *
             * @return atomicx_time How long since the point in time
             *
             * @note    To decrease the amount of memory, Timeout does not save
             *          the start time.
             */
            atomicx_time GetDurationSince(atomicx_time startTime);

        private:
            atomicx_time m_timeoutValue = 0;
    };

    /*
    * ---------------------------------------------------------------------
    * Iterator Implementation
    * ---------------------------------------------------------------------
    */

    /**
    * @brief General purpose Iterator facility
    * 
    * @tparam T 
    */
    template <typename T> class Iterator
    {
    public:
        Iterator() = delete;

        /**
         * @brief Type based constructor
         *
         * @param ptr Type pointer to iterate
         */
        Iterator(T*  ptr);

        /*
        * Access operator
        */
        T& operator*();

        T* operator->();

        /*
        * Movement operator
        */
        Iterator<T>& operator++();

        /*
        * Binary operators
        */
        friend bool operator== (const Iterator& a, const Iterator& b)
        { 
            return a.m_ptr == b.m_ptr;
        }


        friend bool operator!= (const Iterator& a, const Iterator& b)
        {
            return a.m_ptr != b.m_ptr;
        }


    private:
        T* m_ptr;
    };

    template <typename T> Iterator<T>::Iterator(T*  ptr) : m_ptr(ptr)
    {}

    /*
    * Access operator
    */
    template <typename T> T& Iterator<T>::operator*()
    {
        return *m_ptr;
    }

    template <typename T> T* Iterator<T>::operator->()
    {
        return &m_ptr;
    }

    /*
    * Movement operator
    */
    template <typename T> Iterator<T>& Iterator<T>::operator++()
    {
        if (m_ptr != nullptr)
        {
            m_ptr = (T*) m_ptr->operator++ ();
        }
        
        return *this;
    }

    /*
    * ---------------------------------------------------------------------
    * Implementations for Item 
    * ---------------------------------------------------------------------
    */

    /**
    * @brief Link Item to turn any object attachable to LinkList
    * 
    * @param T    Type to be managed
    */
    template<class I> class Item
    {
        public:

            /**
             * @brief Easy way to have access to the item object
             * 
             * @return T&   Reference to the item object
             */
            I& operator()();

            Item<I>* operator++ (void);

            Item<I>* getNext ();

        private:
            friend class Kernel;
            template<class U> friend class DynamicList;
            template <typename U> friend class Iterator;

            Item<I>* next = nullptr;
            Item<I>* prev = nullptr;           
    };

    template<class I> I& Item<I>::operator()()
    {
        return (I&)*this;
    }

    template<class I> Item<I>* Item<I>::operator++ (void)
    {
        return next;
    }

    template<class I> Item<I>* Item<I>::getNext ()
    {
        return next;
    }

    /*
    * ---------------------------------------------------------------------
    * Implementations for DynamicList 
    * ---------------------------------------------------------------------
    */

    /**
    * @brief DynamicList of attachable object by extending Item
    * 
    * @tparam T    Type of the object
    * 
    * @note    Note that this does not create memory, it uses next/prev added to the object 
    *          by extending the object to Item and manages it life cycle, is automatically
    *          inserted and deleted on instantiation and object destruction.
    */
    template<class T> class DynamicList
    {
    protected:
        template <typename U> friend class Iterator;

        T* m_first=nullptr;
        T* m_last=nullptr;

    public:
        
         /**
         * @brief Attach a Item enabled obj to the list
         * 
         * @param listItem  The object
         * 
         * @return true if it was successful attached
         */
        bool AttachBack(T& listItem);

        /**
         * @brief Detach a specific Item from the managed list
         * 
         * @param listItem  Item enabled object to be deleted
         * 
         * @return true     if successful detached
         */
        bool Detach(T& listItem);

        T* getFirst ();

        /**
         * @brief thread::Iterator helper for signaling beginning
         * 
         * @return Iterator<Item<T>>  the first item
         */
        Iterator<T> begin();

        T* GetLast ();

        /**
         * @brief thread::Iterator helper for signaling ending
         * 
         * @return Iterator<Item<T>>  the final of the list
         */
        Iterator<T> end();
    };

    template<class T> bool DynamicList<T>::AttachBack(T& listItem)
    {
        if (m_first == nullptr)
        {
            m_first = &listItem;
            m_last = m_first;
        }
        else
        {
            listItem.prev = m_last;
            m_last->next = &listItem;
            m_last = &listItem;
        }

        return true;
    }

    template<class T> bool DynamicList<T>::Detach(T& listItem)
    {
        if (listItem.next == nullptr && listItem.prev == nullptr)
        {
            m_first = nullptr;
            listItem.prev = nullptr;
        }
        else if (listItem.prev == nullptr)
        {
            listItem.next->prev = nullptr;
            m_first = (T*)listItem.next;
        }
        else if (listItem.next == nullptr)
        {
            listItem.prev->next = nullptr;
            m_last = (T*)listItem.prev;
        }
        else
        {
            listItem.prev->next = listItem.next;
            listItem.next->prev = listItem.prev;
        }

        return true;
    }

    template<class T> T* DynamicList<T>::getFirst ()
    {
        return m_first;
    }

    template<class T> Iterator<T> DynamicList<T>::begin()
    {
        return Iterator<T>(getFirst());
    }

    template<class T> T* DynamicList<T>::GetLast ()
    {
        return m_last;
    }

    template<class T> Iterator<T> DynamicList<T>::end()
    {
        return Iterator<T>(nullptr);
    }

    class thread; // to be used by kernel 

    /*
    * ---------------------------------------------------------------------
    *   Notification interface
    * ---------------------------------------------------------------------
    */

    // Payload containing
    // the message: the size_t message transported (capable of send pointers)
    // the type, that describes the message, allowing layers of information within the same 
    // reference notification 
    typedef struct
    {
        size_t nType;
        size_t nMessage;
    } Payload;
    

    typedef uint8_t NotifyChannel;

    #define ATOMICX_NOTIFY_MAX_CUSTOM_CHANNEL 235

    // 20 reserved kernel notification channels are stated here
    // Those notification channels are used internally to create
    // private and specialized lanes for notifications inside
    // the kernel.
    //
    // All other synchronizations will be ported using it, 
    // allowing autonomous and detached powerful tools and 
    // extra libs.
    enum NotifyType : NotifyChannel
    {
        NOTIFY_KERNEL = ATOMICX_NOTIFY_MAX_CUSTOM_CHANNEL,
        NOTIFY_WAIT
    };

    
    /*
    * ---------------------------------------------------------------------
    * Kernel implementation
    * ---------------------------------------------------------------------
    */

    enum class status : uint8_t
    {
        none =0,
        starting=1,
        wait=10,
        syncWait,
        sleeping,
        halted,
        paused,
        locked=100,
        running=200,
        now
    };

    class Kernel : public DynamicList<thread>
    {
    private:
        friend class thread;

        bool nRunning = false;

        thread* m_pCurrent = nullptr;

        volatile uint8_t* m_pStackStart = nullptr;
        jmp_buf m_context;

        // no delete used for back compatibility with old cpp versions
        Kernel ();

    protected:
        void SetNextThread (void);

    public:

        static Kernel& GetInstance();

        void start(void);
    };


    static Kernel& kernel = Kernel::GetInstance();
    /*
    * ---------------------------------------------------------------------
    * thread implementation
    * ---------------------------------------------------------------------
    */

    class thread : public Item<thread>
    {
    private:
        // Give full access to Kernel, so main look
        // can only use the most necessary function calls
        // avoid corrupting the stack space on context change
        friend class Kernel;
        
        // Initial state for the thread
        status m_status = status::starting;

        // Thread context register buffer
        jmp_buf m_context;
        
        // Total size of the virtual stack
        size_t m_nMaxStackSize; 
        // Actual used virtual stack
        size_t m_nStackSize = 0;

        // Pointer for the virtual stack
        volatile size_t* m_pStack;
        // The last point in the processing stack
        volatile uint8_t* m_pStackEnd = nullptr;

        // Wait and notify implementation

        // Channel of the notification
        // it exist to create specialized notifications
        // and also custom that would not collide and 
        // to be safe. 
        // Up to 236 custom channels # 0 - 235
        // 20 channels are reserved to # 236 - 255 enum class KernelChannel
        uint8_t m_nNotifyChannel;

        // Notification Reference, can be any variable in the system,
        // for consistency, only real variables can be used, no naked 
        // values will be allowed, this wat any variable can be used as 
        // a notifier. 
        void* m_pRefPointer;

        // Payload containing
        // the message: the size_t message transported (capable of send pointers)
        // the type, that describes the message, allowing layers of information within the same 
        // reference notification
        Payload m_payload; 

        struct
        {
            bool bWaitAnyType : 1;
        } m_flags;

    protected:

        template<typename T> size_t PrvSafeNotify(T& ref, size_t& nType, size_t& nMessage, status targetStatus = status::wait, bool bNotifyAll = true, NotifyChannel nChannel = NOTIFY_WAIT);

        bool yield(atomicx_time aTime, status type = status::sleeping);

        template <typename T> bool SetWait (T& ref, size_t nType, size_t nMessage, bool bWaitAllTypes, NotifyChannel nChannel);

        // Sync Notify infra-structure running over status::SyncWait
        // Every time a wait is issue a notification is issued over SyncWait

        template <typename T> inline bool NotifyWait (T& ref, size_t& nType, size_t &nMessage, NotifyChannel nChannel);

        template <typename T> bool LookForWait (T& ref, size_t nType, size_t &nMessage, size_t nAtLeast, atomicx_time nWaitFor, NotifyChannel nChannel);
        
    public:

        template<size_t N>thread (size_t (&stack)[N]);

        virtual ~thread ();

        virtual const char* GetName();

        virtual void run(void) = 0;

        virtual void StackOverflowHandler () = 0;

        unsigned int GetStatus ();

        size_t GetStackSize ();

        size_t GetMaxStackSize ();

        // --------------------------------------
        // Synchronization foundation functions
        // --------------------------------------

        // Waiting for notifications
        template <typename T> bool Wait (T& ref, size_t nType, size_t &nMessage, NotifyChannel nChannel = NOTIFY_WAIT);

        template <typename T> bool WaitAll (T& ref, size_t &nType, size_t &nMessage, NotifyChannel nChannel = NOTIFY_WAIT);


        // Notifying 
        template<typename T> size_t SafeNotify(T& ref, size_t& nType, size_t& nMessage, bool bNotifyOne = true, NotifyChannel nChannel = NOTIFY_WAIT);

        /**
         * @brief   Notify one active 'wait' call for the same reference and type, also capable of 
         *          waiting for 'wait' calls (Sync notification) when nWaitFor is set
         *          ATTENTION: Force context change.
         * 
         * @tparam T            Reference Value, used as the "synchronization" variable, can be any variable (only)
         * @param nType         The Type of the notification (user defined)
         * @param nMessage      The Message bound to the type (user defined)
         * @param nWaitFor      (sync notification) How long (in ticks) to 'wait' for wait calls
         * @param nChannel      the channel where it is running, if not defined it will use the default kernel NOTIFY_WAIT
         * 
         * @return size_t       How many 'wait' calls get notified
         * 
         * @note                Once nWaitFor is set, the process will wait until either one 'wait' call or nWaitFor
         *                      timesout.
         *                      ATTENTION:  if nAtLeast and nWaitFor are not set, the function will NOT WAIT FOR 'WAIT' CALLS, 
         *                                  instead it will only used whatever 'wait' calls are active.
         * 
         * @todo                TODO: Add timeout once time is ported to the kernel
         */
        template<typename T> size_t Notify(T& ref, size_t nType, size_t nMessage, atomicx_time nWaitFor=0, NotifyChannel nChannel = NOTIFY_WAIT);
        
        /**
         * @brief   Notify all active 'wait' calls for the same reference and type, also capable of 
         *          waiting for # number of 'wait' calls (Sync notification) when nAtLeast and 
         *          nWaitFor are set
         *          ATTENTION: Force context change.
         * 
         * @tparam T            Reference Value, used as the "synchronization" variable, can be any variable (only)
         * @param nType         The Type of the notification (user defined)
         * @param nMessage      The Message bound to the type (user defined)
         * @param nAtLeast      (sync notification) wait for at least nAtLeast 'wait' calls
         * @param nWaitFor      (sync notification) How long (in ticks) to 'wait' for wait calls
         * @param nChannel      the channel where it is running, if not defined it will use the default kernel NOTIFY_WAIT
         * 
         * @return size_t       How many 'wait' calls get notified
         * 
         * @note                Once nAtLeast and nWaitFor are set, the process will wait until either nAtLeast or nWaitFor
         *                      timesout.
         *                      ATTENTION:  if nAtLeast and nWaitFor are not set, the function will NOT WAIT FOR 'WAIT' CALLS, 
         *                                  instead it will only used whatever 'wait' calls are active.
         * 
         * @todo                TODO: Add timeout once time is ported to the kernel
         */
        template<typename T> size_t NotifyAll(T& ref, size_t nType, size_t nMessage, size_t nAtLeast=0, atomicx_time nWaitFor=0, NotifyChannel nChannel = NOTIFY_WAIT);
    };


    /*
    * ---------------------------------------------------------------------
    * Class functions implementation
    * ---------------------------------------------------------------------
    */

    /*
    * ---------------------------------------------------------------------
    * Kernel
    * ---------------------------------------------------------------------
    */


    /*
    * ---------------------------------------------------------------------
    * thread
    * ---------------------------------------------------------------------
    */


    template<size_t N> thread::thread (size_t (&stack)[N]) : 
        m_status(status::starting),
        m_context{},
        m_nMaxStackSize(N*sizeof (size_t)), 
        m_pStack ((size_t*) &stack)
    {
        kernel.AttachBack (*this);
    }

    
    // *********************
    // WAIT / Notify bases
    // *********************
    
    //private

    template<typename T> size_t thread::PrvSafeNotify(T& ref, size_t& nType, size_t& nMessage, status targetStatus, bool bNotifyOne, NotifyChannel nChannel)
    {
        size_t nNotified = 0;

        for (auto& th : kernel)
        {
            // highest level, if it is in the correct status and notification channel
            if (th.m_status == targetStatus && th.m_nNotifyChannel == nChannel)
            {
                // if ref pointer matches
                if (th.m_pRefPointer == &ref)
                {
                    // decide to notify based on
                    // if set to notify all waits regardless of type
                    if (th.m_flags.bWaitAnyType || nType == th.m_payload.nType)
                    {
                        // Return now 
                        th.m_status = status::now;

                        // Populate retuning data

                        th.m_payload.nType = nType;
                        th.m_payload.nMessage = nMessage;

                        // disable reference
                        th.m_pRefPointer = nullptr;
                        th.m_flags.bWaitAnyType = false;

                        nNotified++;

                        if (bNotifyOne) break;
                    }
                }
            }
        }
        
        return nNotified;
    }


    template <typename T> bool thread::SetWait (T& ref, size_t nType, size_t nMessage, bool bWaitAllTypes, NotifyChannel nChannel)
    {
        m_nNotifyChannel = nChannel;

        m_pRefPointer = (void*) &ref;

        m_payload.nMessage = nMessage;

        m_payload.nType = nType;

        m_flags.bWaitAnyType = bWaitAllTypes;

        return true;
    }


    //public

    template <typename T> bool thread::Wait (T& ref, size_t nType, size_t& nMessage, NotifyChannel nChannel)
    {
        NotifyWait (ref, nType, nMessage, nChannel);

        //PrvSafeNotify (ref, nType, nMessage, status::syncWait, false, nChannel);

        if (! SetWait (ref, nType, nMessage, false, nChannel)) return false;

        return yield (0, status::wait);
    }

    template <typename T> bool thread::WaitAll (T& ref, size_t& nType, size_t& nMessage, NotifyChannel nChannel)
    {
        NotifyWait (ref, nType, nMessage, nChannel);

        if (! SetWait (ref, nType, nMessage, true, nChannel)) return false;

        return yield (0, status::wait);
    }

    template<typename T> size_t thread::SafeNotify(T& ref, size_t& nType, size_t& nMessage, bool bNotifyOne, NotifyChannel nChannel)
    {
        return PrvSafeNotify (ref, nType, nMessage, status::wait, bNotifyOne, nChannel);
    }

    template<typename T> size_t thread::Notify(T& ref, size_t nType, size_t nMessage, atomicx_time nWaitFor, NotifyChannel nChannel)
    {
        if (nWaitFor) if(LookForWait (ref, nType, nMessage, 1, nWaitFor, nChannel) == false) return -1;
    
        size_t nReturn = SafeNotify (ref, nType, nMessage, true, nChannel);

        yield (0, status::now);

        return nReturn;
    }

    template<typename T> size_t thread::NotifyAll(T& ref, size_t nType, size_t nMessage, size_t nAtLeast, atomicx_time nWaitFor, NotifyChannel nChannel)
    {
        if (nWaitFor && nAtLeast)  if(LookForWait (ref, nType, nMessage, nAtLeast, nWaitFor, nChannel) == false) return -1;

        size_t nReturn = SafeNotify (ref, nType, nMessage, false, nChannel);

        yield (0, status::now);

        return nReturn;
    }

    template <typename T> bool thread::NotifyWait (T& ref, size_t& nType, size_t &nMessage, NotifyChannel nChannel)
    {
        size_t nReturn = PrvSafeNotify (ref, nType, nMessage, status::syncWait, false, nChannel);

        return nReturn ? true : false;
    }

     template <typename T> bool thread::LookForWait (T& ref, size_t nType, size_t &nMessage,size_t nAtLeast, atomicx_time nWaitFor, NotifyChannel nChannel)
     {
        size_t nCounter = 0;

        do
        {
            nCounter = 0;

            for (auto& th : kernel)
            {
                if (th.m_status == status::wait && th.m_nNotifyChannel == nChannel)
                {
                    if (th.m_pRefPointer && th.m_payload.nType == nType)
                    {
                        if ((--nCounter) >= nAtLeast)
                        {
                            return true;
                        }
                    }
                }
            }

            // Wait for one more start waiting, as a result, 
            // other contexts have been executed so the actual nCounter may have 
            // changed, counter again, if nCounter >= nCounter return.
            // TODO Soon time is ported, add timeout and error handler here
            if (SetWait (ref, nType, nMessage, false, nChannel) == false) return false;
            yield (nWaitFor, status::syncWait);

        } while (nCounter < nAtLeast);

        return true;
     }
}

#endif
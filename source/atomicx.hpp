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

#include <iostream>

/* Official version */
#define ATOMICX_VERSION "1.3.0"
#define ATOMIC_VERSION_LABEL "AtomicX v" ATOMICX_VERSION " built at " __TIMESTAMP__

typedef uint32_t atomicx_time;

namespace atomicx
{

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
        T* m_ptr = nullptr;
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
            m_ptr = m_ptr->operator++ ();
        }
        
        return *this;
    }


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

        Item<T>* m_current=nullptr;
        Item<T>* m_first=nullptr;
        Item<T>* m_last=nullptr;

    public:
        
         /**
         * @brief Attach a Item enabled object to the list
         * 
         * @param listItem  The object
         * 
         * @return true if it was successful attached
         */
        bool AttachBack(Item<T>& listItem)
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

        /**
         * @brief Detach a specific Item from the managed list
         * 
         * @param listItem  Item enabled object to be deleted
         * 
         * @return true     if successful detached
         */
        bool Detach(Item<T>& listItem)
        {
            if (listItem.next == nullptr && listItem.prev == nullptr)
            {
                m_first = nullptr;
                listItem.prev = nullptr;
                m_current = nullptr;
            }
            else if (listItem.prev == nullptr)
            {
                listItem.next->prev = nullptr;
                m_first = listItem.next;
                m_current = m_first;
            }
            else if (listItem.next == nullptr)
            {
                listItem.prev->next = nullptr;
                m_last = listItem.prev;
                m_current = m_last;
            }
            else
            {
                listItem.prev->next = listItem.next;
                listItem.next->prev = listItem.prev;
                m_current = listItem.next->prev;
            }

            return true;
        }

        Item<T>* getFirst ()
        {
            return m_first;
        }

        /**
         * @brief thread::Iterator helper for signaling beginning
         * 
         * @return Iterator<Item<T>>  the first item
         */
        Iterator<Item<T>> begin()
        {
            return Iterator<Item<T>>(getFirst());
        }

        Item<T>* GetLast ()
        {
            return m_last;
        }

        /**
         * @brief thread::Iterator helper for signaling ending
         * 
         * @return Iterator<Item<T>>  the final of the list
         */
        Iterator<Item<T>> end()
        {
            return Iterator<Item<T>>(nullptr);
        }

    };

    class thread;

    enum class status : uint8_t
    {
        none,
        starting,
        running,
        halted,
        paused,
        locked,
        wait
    };

    class Kernel : public DynamicList<thread>
    {
    private:
        friend class thread;

        bool nRunning = false;

        thread* m_pCurrent = nullptr;

        volatile uint8_t* m_pStackStart = nullptr;
        jmp_buf m_context;

    protected:
        void SetNextThread (void);

    public:

        void start(void);

    };

    class thread : public Item<thread>
    {
    private:
        // Give full access to Kernel, so main look
        // can only use the most necessary function calls
        // avoid corrupting the stack space on context change
        friend class Kernel;

        Kernel& m_kContext;

        status m_status = status::starting;

        jmp_buf m_context;

        size_t m_nStackSize = 0;

        volatile size_t& m_pStack;
        volatile uint8_t* m_pStackStop = nullptr;

    protected:

    public:
        template<size_t N>thread (Kernel& kContext, size_t (&stack)[N]);

        virtual ~thread ();

        virtual const char* GetName();

        virtual void run(void) = 0;

        status GetStatus ();
    };

    /*
    * thread functions
    */

    template<size_t N> thread::thread (Kernel& kContext, size_t (&stack)[N]) : 
        m_kContext(kContext),
        m_status(status::starting),
        m_context{},
        m_nStackSize(N), 
        m_pStack (stack[0])
    {
        m_kContext.AttachBack (*this);
    }

}

#endif
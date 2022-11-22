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

namespace atomicx
{
    #define GetStackPoint() ({volatile uint8_t ___var = 0; &___var;})

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

        T* m_current=nullptr;
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
            m_current = nullptr;
        }
        else if (listItem.prev == nullptr)
        {
            listItem.next->prev = nullptr;
            m_first = (T*)listItem.next;
            m_current = m_first;
        }
        else if (listItem.next == nullptr)
        {
            listItem.prev->next = nullptr;
            m_last = (T*)listItem.prev;
            m_current = m_last;
        }
        else
        {
            listItem.prev->next = listItem.next;
            listItem.next->prev = listItem.prev;
            m_current = (T*)listItem.next->prev;
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


    /*
    * ---------------------------------------------------------------------
    * Kernel implementation
    * ---------------------------------------------------------------------
    */
    class thread;

    enum class status : uint8_t
    {
        none =0,
        starting=1,
        wait=10, 
        sleeping,
        halted,
        paused,
        locked=100,
        running=200
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

        bool yield(atomicx_time aTime, status type=status::running);

        void start(void);

    };

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
        // Start context to return to the kernel
        jmp_buf m_joinContext;
        // Total size of the virtual stack
        size_t m_nMaxStackSize; 
        // Actual used virtual stack
        size_t m_nStackSize = 0;

        // Pointer for the virtual stack
        volatile size_t* m_pStack;
        // The last point in the processing stack
        volatile uint8_t* m_pStackEnd = nullptr;

    protected:
        //Kernel context binding
        Kernel& m_Kernel;
        
    public:
        template<size_t N>thread (Kernel& kContext, size_t (&stack)[N]);

        virtual ~thread ();

        virtual const char* GetName();

        virtual void run(void) = 0;

        unsigned int GetStatus ();

        size_t GetStackSize ();

        size_t GetMaxStackSize ();
    };

    template<size_t N> thread::thread (Kernel& kContext, size_t (&stack)[N]) : 
        m_status(status::starting),
        m_context{},
        m_nMaxStackSize(N*sizeof (size_t)), 
        m_pStack ((size_t*) &stack),
        m_Kernel(kContext)
    {
        m_Kernel.AttachBack (*this);
    }

}

#endif

#include "atomicx.hpp"

void* ref;

atomicx::mutex mt;

atomicx_time atomicx::Kernel::GetTick(void)
{
    return millis();
}

void atomicx::Kernel::SleepTick(atomicx_time nSleep)
{
    delay(nSleep);
}


class WaitCounter : public atomicx::thread
{
private:
    size_t stack [45];

public:
    static size_t nCounter;

    WaitCounter () : thread (10, stack)
    {

    }

    ~WaitCounter ()
    { }

    const char* GetName ()
    {
        return "WaitCounter";
    }

    void run ()
    {
        size_t nType;
        size_t nMessage = 0;

        while (true)
        {
            if (WaitAll (ref, nType, nMessage, 1000))
            {
                atomicx::SmartLock local(mt);
                local.Lock (); Now ();
                nCounter++;
            }
        }
    }

    void StackOverflowHandler ()
    {
        Serial.println ((size_t) this);
        Serial.print (F(": WaitCounter::StackOverflow, stack size:"));
        Serial.print (GetStackSize ());
        Serial.print ("/");
        Serial.println (GetMaxStackSize ());
        Serial.flush ();
        delay (5000);
    }
};

size_t WaitCounter::nCounter=0; 

class WaitThread : public atomicx::thread
{
private:
    size_t stack [20];

public:
    WaitThread () : thread (10, stack)
    {

    }

    ~WaitThread ()
    {
        Serial.println ((size_t) this);
        Serial.print (F(": Destructing waitThread ID:"));
        Serial.flush ();
    }

    void run ()
    {
        size_t nMessage = 0;

        Serial.println ((size_t) this);
        Serial.print (F(": Initiating waitThread ID:"));
        Serial.flush ();        

        while (yield ())
        {
            Wait (ref, 1, nMessage, 1000);
        }
    }

    void StackOverflowHandler ()
    {
        Serial.println ((size_t) this);
        Serial.print (F(":test::StackOverflow, stack size:"));
        Serial.print (GetStackSize ());
        Serial.print ("/");
        Serial.println (GetMaxStackSize ());
        Serial.flush ();
        delay (5000);
    }

};

class test : public atomicx::thread
{
private:
    size_t stack [50];

public:
    test () : thread(10, stack)
    {

    }

    ~test ()
    {
        Serial.print (F("ID:"));
        Serial.print ((size_t) this);
        Serial.println (F(", Beind destructed."));
        Serial.flush ();
    }

    void run ()
    {
        Serial.print (F("run"));
        Serial.print (F(": Thread initiating: "));
        Serial.println ((size_t) this);
        Serial.flush ();

        int nValue = 0;
        size_t nNotified = 0;
        bool bTimeout;

        atomicx_time start = 0;

        while (yield ())
        {
            start = atomicx::kernel.GetTick ();
            nNotified = NotifyAll (ref, 1, (size_t) this, 1000, 3);

            start = atomicx::kernel.GetTick () - start;

            bTimeout = GetStatus () == atomicx::status::timeout;

            atomicx::SmartLock local(mt);
            local.SharedLock (); 
            Now ();
            
            Serial.print (F("test::run"));

            Serial.print (F(": taken: ["));
            Serial.print (start);

            Serial.print (F("] Timeour: ["));
            Serial.print (bTimeout);

            Serial.print (F("] Status: ["));
            Serial.print ((int) GetStatus ());

            Serial.print (F("] Value: ["));
            Serial.print (nValue++);
            
            Serial.print (F("] Notified: ["));
            Serial.print (nNotified);

            Serial.print (F(" / "));
            Serial.print (WaitCounter::nCounter);

            Serial.print (F("] ID: ["));
            Serial.print ((size_t) this);
            
            Serial.print (F("] StackSize: "));
            Serial.print (GetStackSize ());
            Serial.print (F("/"));
            Serial.print (GetMaxStackSize ());

            // Terminal scape control tv100/xterm to
            // clear the rest of the line.
            Serial.print ((char) 27) ;
            Serial.print (F("[0K"));

            Serial.println ();
            //Serial.println ((char) 13);
            Serial.flush ();

            if (bTimeout) exit (-1);
        }
    }

    void StackOverflowHandler ()
    {
        Serial.print ((size_t) this);
        Serial.print (F(": test::StackOverflow, stack size:"));
        Serial.print (GetStackSize ());
        Serial.print ("/");
        Serial.println (GetMaxStackSize ());
        Serial.flush ();

        delay (5000);
    }

};

void setup()
{
    test test1;
    test test2;

    WaitThread wait1;

    test test4;

    WaitThread wait2;

    test test5; 

    WaitThread wait3;

    WaitCounter wcount1;

    Serial.begin (115200);

    Serial.println (F(""));
    Serial.println (F("Starting atomicx 3 demo."));
    delay(100);

    for (auto& th : atomicx::kernel)
    {
        Serial.print (__func__);
        Serial.print (": Listing thread: ");
        Serial.print (th.GetName ());
        Serial.print (", ID:");
        Serial.println (((size_t) &(th())));
        Serial.flush ();
    }

    Serial.println ("-------------------------------------");

    delay (1000);

    atomicx::kernel.start();

}

void loop() {
}

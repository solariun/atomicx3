
#include "atomicx.hpp"

void* ref;

atomicx_time atomicx::Kernel::GetTick(void)
{
    return millis();
}

void atomicx::Kernel::SleepTick(atomicx_time nSleep)
{
    delay(nSleep);
}

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
        Serial.print (": Destructing waitThread ID:");
        Serial.flush ();
    }

    void run ()
    {
        size_t nMessage = 0;

        Serial.println ((size_t) this);
        Serial.print (": Initiating waitThread ID:");
        Serial.flush ();        

        while (Sleep (500))
        {
            Wait (ref, 1, nMessage);
        }
    }

    void StackOverflowHandler ()
    {
        Serial.print ("test::StackOverflow, stack size:");
        Serial.print (GetStackSize ());
        Serial.print ("/");
        Serial.println (GetMaxStackSize ());
    }

};

class test : public atomicx::thread
{
private:
    size_t stack [30];

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
        (void) __COUNTER__;

        Serial.print (__FUNCTION__);
        Serial.print (F(": Thread initiating: "));
        Serial.println ((size_t) this);
        Serial.flush ();

        Sleep(100);

        int nValue = 0;
        size_t nNotified = 0;

        while (true)
        {
            nNotified = NotifyAll (ref, 1, (size_t) this, 2, 10);

            Serial.print (__FUNCTION__);

            Serial.print (F(": Time: ["));
            Serial.print (atomicx::kernel.GetTick ());

            Serial.print (F("]: Value: ["));
            Serial.print (nValue++);
            
            Serial.print (F("], Notified: ["));
            Serial.print (nNotified);

            Serial.print (F("], ID:"));
            Serial.print ((size_t) this);
            
            Serial.print (F(", StackSize: "));
            Serial.print (GetStackSize ());
            Serial.print (F("/"));
            Serial.print (GetMaxStackSize ());

            // Terminal scape control tv100/xterm to
            // clear the rest of the line.
            Serial.print ((char) 27) ;
            Serial.print ("[0K");

            Serial.println ();
            //Serial.println ((char) 13);
            Serial.flush ();
            
            yield ();
        }
    }

    void StackOverflowHandler ()
    {
        Serial.print ("test::StackOverflow, stack size:");
        Serial.print (GetStackSize ());
        Serial.print ("/");
        Serial.println (GetMaxStackSize ());

        delay (5000);
    }

};

void setup()
{
    test test1;
    test test2;
    test test3;
    WaitThread wait1;

    test test4;

    WaitThread wait2;

    test test5; 
    test test6;
    WaitThread wait3;

    test test7;

    WaitThread wait4;

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


#include "atomicx.hpp"

class test : public atomicx::thread
{
    private:
        size_t stack [20];

    public:
        test () : thread(stack)
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

            delay(100);

            int nValue = 0;

            while (true)
            {
                Serial.print (__FUNCTION__);

                Serial.print (": Value: [");
                Serial.print (nValue++);
                
                Serial.print ("], ID:");
                Serial.print ((size_t) this);
                
                Serial.print (F(", StackSize: "));
                Serial.print (GetStackSize ());
                
                Serial.print ((char) 13);
                Serial.flush ();

                yield (0);
            }
        }
};

void setup()
{
    test test1;
    test test2;
    test test3;
    test test4;
    test test5; 
    test test6;
    test test7;
    test test8;
    test test9;
    test test10;
    test test11;
    test test12;

    (void) __COUNTER__;
    (void) __COUNTER__;
    (void) __COUNTER__;

    Serial.begin (115200);

    Serial.println (F(""));
    Serial.println (F("Starting atomicx 3 demo."));
    delay(100);

    for (auto& th : atomicx::kernel)
    {
        Serial.print ((char) 27);
        Serial.print ("0P");
        Serial.flush ();
        Serial.print (__func__);
        Serial.print (": Listing thread: ");
        Serial.print (th.GetName ());
        Serial.print (", ID:");
        Serial.println (((size_t) &(th())));
        Serial.print (", Counter: ");
        Serial.print (__COUNTER__);
        Serial.flush ();
    }

    Serial.println ("-------------------------------------");

    delay (1000);

    atomicx::kernel.start();

}

void loop() {
}

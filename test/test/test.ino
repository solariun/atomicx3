
#include "atomicx.hpp"

atomicx::Kernel kernel;

class test : public atomicx::thread
{
    private:
        size_t stack [20];

    public:
        test () : thread(kernel, stack)
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
            Serial.print (__FUNCTION__);
            Serial.print (F(": Thread initiating: "));
            Serial.println ((size_t) this);
            Serial.flush ();

            delay(500);

            int nValue = 0;

            while (true)
            {
                Serial.print (__FUNCTION__);
                Serial.print (": Value: [");
                Serial.print (nValue++);
                Serial.print ("], ID:");
                Serial.print ((size_t) this);
                Serial.print (F(", StackSize: "));
                Serial.println (GetStackSize ());
                Serial.flush ();

                kernel.yield (0);
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

    Serial.begin (115200);

    Serial.println ("Starting atomicx 3 demo.");
    delay(100);

    kernel.start();

}

void loop() {
}

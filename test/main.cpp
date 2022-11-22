//
//  main.cpp
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

atomicx::Kernel kernel;

class test : public atomicx::thread
{
    private:
        size_t stack [128];

    public:
        test () : thread(kernel, stack)
        {

        }

        ~test ()
        {
            std::cout << "ID:" << ((size_t) this) << ", being destructed." << std::endl;
        }

        void run ()
        {
            std::cout << __func__ << ": Thread initiating: " << std::hex << this << std::dec << std::endl;

            int nValue = 0;

            while (true)
            {
                std::cout << __func__ << ": Value: [" << nValue++ << "], ID:" << std::hex << (this) << std::dec << ", StackSize: " << GetStackSize () << " bytes" << ((char) 13) << std::flush;

                //::usleep (20000);

                kernel.yield(0);
            }
        }
};

int main ()
{
    test test1;
    test test2;
    test test3;
    test test4;
    test test5;
    test test6;
    test test7;
    test test8;

    for (auto& th : kernel)
    {
        std::cout << __func__ << ": listing thread: " << th().GetName () << ", ID: " << ((size_t) &(th()))<< std::endl;
    }

    kernel.start ();

    return 0;
}
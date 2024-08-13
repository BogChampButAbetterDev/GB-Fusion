#include "CPU.h"

#undef main // this is just a cheap way to fix unresolved symbols. it tells the compiler that I don't want to use SDL_main

int main()
{
    try {

        CPU cpu;

        if (cpu.LoadROM("../ROMs/02.gb"))
        {
            std::cout << "ROM loaded" << std::endl;
        }
        else
        {
            std::cout << "ROM could not be loaded because of that" << std::endl;
            exit(1);
        }

        std::cout << std::hex << std::setw(4) << std::setfill('0') << cpu.memory[0x100] << std::endl;

        while (true)
        {
            cpu.Update();
            cpu.Cycle();

            if (!cpu.running) {
                break;
            }
        }

        cpu.check_test();
    }

    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}

#include "cpu.h"

#include <iostream>

namespace svm
{
    Registers::Registers()
        : a(0), b(0), c(0), flags(0), ip(0), sp(0) { }

    CPU::CPU(Memory &memory, PIC &pic)
        : registers(),
          _memory(memory),
          _pic(pic) { }

    CPU::~CPU() { }

    void CPU::Step()
    {
        int ip =
            registers.ip;

        int instruction =
            _memory.ram[ip];
        int data =
            _memory.ram[ip + 1];

        if (instruction ==
                CPU::MOVA_OPCODE) {
            registers.a = data;
            registers.ip += 2;
        } else if (instruction ==
                       CPU::MOVB_OPCODE) {
            registers.b = data;
            registers.ip += 2;
        } else if (instruction ==
                       CPU::MOVC_OPCODE) {
            registers.c = data;
            registers.ip += 2;
        } else if (instruction ==
                       CPU::JMP_OPCODE) {
            registers.ip += data;
        } else if (instruction ==
                       CPU::INT_OPCODE) {
            switch (data)
            {
                case 1:
                    _pic.isr_3();
                    break;
                    //case 2:
                    //  _pic.isr_5(); // `isr_4` is reserved for page fault
                    //                // exceptions
                    //  break;
                    // ...
            }
            registers.ip += 2;
        /*
         *  TODO:
         *
         *      } else if (instruction ==
         *                     CPU::LDA_OPCODE) {
         *          1. Get a page index and physical address offset from the MMU
         *               for a virtual address in 'data'
         *          2. Get the page from the current page table in the MMU with
         *               the acquired index
         *          If the page is invalid
         *              1. Save the value of register 'a' on a stack
         *              2. Place the index of this page into register 'a'
         *              3. Call the page fault handler in PIC (isr_4)
         *              4. Restore register 'a'
         *          or if the page is valid
         *              1. Calculate the physical address with the value in the
         *                   page entry and the physical address offset
         *              2. Read or write (for ST...) from/to the physical memory
         *                   with the calculated address
         *              3. Increment the instruction pointer
         *
         *      } else if (instruction ==
         *                     CPU::LDB_OPCODE) {
         *          ...
         *
         *      } else if (...
         *          ...
         *
         *      } else if (instruction ==
         *                     CPU::STA_OPCODE) {
         *          ...
         *
         */
        } else {
            std::cerr << "CPU: invalid opcode data. Skipping..."
                      << std::endl;
            registers.ip += 2;
        }
    }
}

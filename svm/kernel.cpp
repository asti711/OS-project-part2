#include "kernel.h"

#include <iostream>
#include <algorithm>

namespace svm
{
    Kernel::Kernel(
                Scheduler scheduler,
                std::vector<Memory::ram_type> executables_paths
            )
        : board(),
          processes(),
          priorities(),
          scheduler(scheduler),
          _last_issued_process_id(0),
          _last_ram_position(0),
          _cycles_passed_after_preemption(0),
          _current_process_index(0)
    {

        // Memory Management

        /*
         *   TODO:
         *
         *     Initialize data structures for methods `AllocateMemory` and
         *       `FreeMemory`
         */

        // Process page faults (find empty frames)
        board.pic.isr_4 = [&]() {
            std::cout << "Kernel: page fault." << std::endl;

            /*
             *  TODO:
             *
             *    Get the faulting page index from the register 'a'
             *
             *    Try to acquire a new frame from the MMU by calling
             *      `AcquireFrame`
             *    Check if the frame is valid
             *        If valid
             *            Write the frame to the current faulting page in the
             *              MMU page table (at index from register 'a')
             *            or else if invalid
             *              Notify the process or stop the board (out of
             *              physical memory)
             */
        };

        // Process Management

        std::for_each(
            executables_paths.begin(),
            executables_paths.end(),
            [&](Memory::ram_type &executable) {
                CreateProcess(executable);
            }
        );

        /*
         *  TODO:
         *
         *    Switch to the first process on the CPU
         *    Switch the page table in the MMU to the table of the current
         *      process
         *    Set a proper state for the first process
         */

        /*
         *  TODO:
         *
         *    Each scheduler should get the page table switching step
         *    Each exit call handler in `isr_3` should start using `FreeMemory`
         *      to release data in RAM or virtual memory
         */

        if (scheduler == FirstComeFirstServed) {
            board.pic.isr_0 = [&]() {
                // ToDo: Process the timer interrupt for the FCFS
            };

            board.pic.isr_3 = [&]() {
                // ToDo: Process the first software interrupt for the FCFS

                // Unload the current process
            };
        } else if (scheduler == ShortestJob) {
            board.pic.isr_0 = [&]() {
                // ToDo: Process the timer interrupt for the Shortest
                //  Job First scheduler
            };

            board.pic.isr_3 = [&]() {
                // ToDo: Process the first software interrupt for the Shortest
                //  Job scheduler

                // Unload the current process
            };
        } else if (scheduler == RoundRobin) {
            board.pic.isr_0 = [&]() {
                // ToDo: Process the timer interrupt for the Round Robin
                //  scheduler
            };

            board.pic.isr_3 = [&]() {
                // ToDo: Process the first software interrupt for the
                //  Round Robin scheduler

                // Unload the current process
            };
        } else if (scheduler == Priority) {
            board.pic.isr_0 = [&]() {
                // ToDo: Process the timer interrupt for the Priority Queue
                //  scheduler
            };

            board.pic.isr_3 = [&]() {
                // ToDo: Process the first software interrupt for the Priority
                //  Queue scheduler

                // Unload the current process
            };
        }

        board.Start();
    }

    Kernel::~Kernel() { }

    void Kernel::CreateProcess(Memory::ram_type &executable)
    {
        Memory::ram_size_type
            new_memory_position = -1; // TODO:
                                      //   allocate memory for the process
                                      //   with `AllocateMemory`
        if (new_memory_position == -1) {
            std::cerr << "Kernel: failed to allocate memory."
                      << std::endl;
        } else {
            // Assume that the executable image size can not be greater than
            //   a page size
            std::copy(
                executable.begin(),
                executable.end(),
                board.memory.ram.begin() + new_memory_position
            );

            Process process(
                _last_issued_process_id++,
                new_memory_position,
                new_memory_position + executable.size()
            );

            // Old sequential allocation
            //
            //   std::copy(
            //       executable.begin(),
            //       executable.end(),
            //       board.memory.ram.begin() + _last_ram_position
            //   );
            //
            //   Process process(
            //       _last_issued_process_id++,
            //       _last_ram_position,
            //       _last_ram_position + executable.size()
            //   );
            //
            //   _last_ram_position +=
            //       executable.size();

            // ToDo: add the new process to an appropriate data structure
            processes.push_back(process);

            // ToDo: process the data structure (e.g., sort if necessary)

            // ---
        }
    }

    Memory::ram_size_type Kernel::AllocateMemory(
                                      Memory::ram_size_type units
                                  )
    {
        /*
         *  TODO:
         *
         *    Task 1: allocate physical memory by using a free list with the
         *      next fit approach.
         *
         *    You can adapt the algorithm from the book The C Programming
         *      Language (Second Edition) by Brian W. Kernighan and Dennis M.
         *      Ritchie (8.7 Example - A Storage Allocator).
         *
         *    Task 2: adapt the algorithm to work with your virtual memory
         *      subsystem.
         */

        return -1;
    }

    void Kernel::FreeMemory(
                     Memory::ram_size_type physical_address
                 )
    {
        /*
         *  TODO:
         *
         *    Task 1: free physical memory
         *
         *    You can adapt the algorithm from the book The C Programming
         *      Language (Second Edition) by Brian W. Kernighan and Dennis M.
         *      Ritchie (8.7 Example - A Storage Allocator).
         *
         *    Task 2: adapt the algorithm to work with your virtual memory
         *      subsystem
         */
    }
}

#ifndef KERNEL_H
#define KERNEL_H

#include <deque>
#include <queue>
#include <string>

#include "board.h"
#include "process.h"

namespace svm
{
    // OS Kernel
    class Kernel
    {
        public:
            enum Scheduler
            {
                Undefined,
                FirstComeFirstServed,
                ShortestJob,
                RoundRobin,
                Priority
            };

            typedef std::deque<Process> process_list_type;
            typedef std::priority_queue<Process> process_priorities_type;

            Board board;

            process_list_type processes;
            process_priorities_type priorities;

            Scheduler scheduler;

            // Kernel page table (should be used to allocate processes)
            Memory::page_table_type *page_table;

            // Kernel boot process (setup ISRs, create processes, etc.)
            Kernel(
                Scheduler scheduler,
                std::vector<Memory::ram_type> executables_paths
            );

            virtual ~Kernel();

            // Creates a new PCB, places the executable image into memory
            void CreateProcess(Memory::ram_type &executable);

            // Allocates `units` of memory for kernel data. Returns an
            // address on success or -1 on failure
            Memory::ram_size_type AllocateMemory(
                                      Memory::ram_size_type units
                                  );
            // Deallocates memory previously allocated at the specified address
            void FreeMemory(
                     Memory::ram_size_type physical_address
                 );

            // TODO:
            //
            //   For a virtual memory system, change memory allocation functions
            //   to the following
            //
            //     Memory::vmem_size_type AllocateMemory(
            //                                Memory::ram_size_type units
            //                            );
            //     void FreeMemory(
            //              Memory::vmem_size_type virtual_address
            //          );

        private:
            static const unsigned int _MAX_CYCLES_BEFORE_PREEMPTION = 100;

            Process::process_id_type _last_issued_process_id;
            Memory::ram_type::size_type _last_ram_position;

            unsigned int _cycles_passed_after_preemption;
            process_list_type::size_type _current_process_index;
    };
}

#endif

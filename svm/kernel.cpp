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
	freep = NULL;

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
	     Memory::page_table_size_type faultingPageIndex = board.cpu.registers.a;
	     Memory::page_entry_type newFrame = board.memory.AcquireFrame();
	     if (newFrame != Memory::INVALID_PAGE) {
		board.memory.page_table[faultingPageIndex] = newFrame;
	     }
	     else {
		board.Stop();
	     }
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

	board.cpu.Step();
	processes[0].state = Process::States::Running;
	board.memory.page_table = processes[0].page_table;

        if (scheduler == FirstComeFirstServed) {
            board.pic.isr_0 = [&]() {
                // ToDo: Process the timer interrupt for the FCFS
            };

            board.pic.isr_3 = [&]() {
                // ToDo: Process the first software interrupt for the FCFS

                // Unload the current process
		std::cout << "Number of Processes left = " << processes.size() << std::endl;

		FreeMemory(processes.front.memory_start_position);
		processes.pop_front();
		if (processes.empty()) {
			board.Stop();
		}
		else {
			board.cpu.registers = processes[0].registers;
			processes[0].state = Process::States::Running;
		}
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
		std::cout << "Number of Processes left = " << processes.size() << std::endl;

		FreeMemory(processes.front.memory_start_position);
		processes.pop_front();
		if (processes.empty()) {
			board.Stop();
		}
		else {
			board.cpu.registers = processes[0].registers;
			processes[0].state = Process::States::Running;
		}
            };
        } else if (scheduler == RoundRobin) {
            board.pic.isr_0 = [&]() {
                // ToDo: Process the timer interrupt for the Round Robin
                //  scheduler
		std::cout << "States of processes: ";
		for (int i = 0; i < processes.size(); ++i) {
			std::cout << processes[i].state << " ";
		}
		std::cout << std::endl;
		_cycles_passed_after_preemption++;
		if (_cycles_passed_after_preemption > _MAX_CYCLES_BEFORE_PREEMPTION) {
			_cycles_passed_after_preemption = 0;

			processes[_current_process_index].registers = board.cpu.registers;
			processes[_current_process_index].state = Process::States::Ready;
			_current_process_index++;
			if (_current_process_index >= processes.size()) {
				_current_process_index = 0;
			}
			board.cpu.registers = processes[_current_process_index].registers;
			processes[_current_process_index].state = Process::States::Running;
		}
            };

            board.pic.isr_3 = [&]() {
                // ToDo: Process the first software interrupt for the
                //  Round Robin scheduler

                // Unload the current process
		std::cout << "Number of Processes left = " << processes.size() << std::endl;

		FreeMemory(processes[_current_process_index].memory_start_position);
		processes.erase(processes.begin() + _current_process_index);
		if (processes.empty()) {
			board.Stop();
		}
		else {
			if (_current_process_index >= processes.size()) {
				_current_process_index = 0;
			}
			board.cpu.registers = processes[_current_process_index].registers;
			processes[_current_process_index].state = Process::States::Running;
		}
            };
        } else if (scheduler == Priority) {
            board.pic.isr_0 = [&]() {
                // ToDo: Process the timer interrupt for the Priority Queue
                //  scheduler
		std::cout << "Priority of the current process = " << priorities.top().priority << "\t Cycles = " << priorities.top()._DYNAMIC_MAX_CYCLES_BEFORE_PREEMPTION << std::endl;

		_cycles_passed_after_preemption++;
		if (_cycles_passed_after_preemption > priorities.top()._DYNAMIC_MAX_CYCLES_BEFORE_PREEMPTION) {
			_cycles_passed_after_preemption = 0;
			Process temp = priorities.top();
			temp.registers = board.cpu.registers;
			temp.state = Process::States::Ready;
			priorities.pop();
			if (temp.priority > 0) {
				temp.priority--;
				temp.updateCycles();
			}
			priorities.push(temp);
			if (priorities.empty()) {
				board.Stop();
			}
			else {
				Process t = priorities.top();
				board.cpu.registers = t.registers;
				t.state = Process::States::Running;
			}
		}
            };

            board.pic.isr_3 = [&]() {
                // ToDo: Process the first software interrupt for the Priority
                //  Queue scheduler

                // Unload the current process
		std::cout << "Number of Processes left = " << processes.size() << std::endl;

		if (board.cpu.registers.a == 1) {
			FreeMemory(priorities.top.memory_start_position);
			priorities.pop();
			if (priorities.empty()) {
				board.Stop();
			}
			else {
				Process t = priorities.top();
				board.cpu.registers = t.registers;
				priorities.pop();
				t.state = Process::States::Running;
				priorities.push(t);
			}
		}
		else if (board.cpu.registers.a == 2) {
			Process t = priorities.top();
			FreeMemory(priorities.top.memory_start_position);
			priorities.pop();
			t.priority = board.cpu.registers.b;
			priorities.push(t);
		}
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
	new_memory_position = AllocateMemory(new_memory_position);

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
            //processes.push_back(process);

            // ToDo: process the data structure (e.g., sort if necessary)

            if (scheduler == Priority) {
		if (board.cpu.registers.a = 2) {
			process.priority = board.cpu.registers.b;
		}
		board.cpu.registers.a = 1;
		priorities.push(process);
	     }
	     else if (scheduler == ShortestJob) {
		processes.push_back(process);
		std::sort(processes.begin(), processes.end(), [](const Process &a, const Process &b) {
			return a.sequential_instruction_count > b.sequential_instruction_count;
		});
	     }
	     else {
	 	processes.push_back(process);
	     }
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

        //return -1;
	header *p, *prevp;
	unsigned nunits = (units + sizeof(long) - 1) / sizeof(long) + 1;
	if ((prevp = freep) == NULL) {
		base.ptr = &base;
		base.size = 0;
	}
	for (p = prevp->ptr; ; prevp = p, p = p->ptr) {
		if (p->size >= nunits) {
			if (p->size == nunits)
				prevp->ptr = p->ptr;
			else {
				p->size -= nunits;
				p += p->size;
				p->size = nunits;
			}
			freep = prevp;
			return *(p + 1);
		}
		if (p == freep) {
			return -1;
		}
	}
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
	header *bp, *p;
	for (p = freep; !(bp > p && bp < p->ptr); p = p->ptr) {
		if (p >= p->ptr && (bp > p || bp < p->ptr))
			break; /* freed block at start or end of arena */
	}
	if (bp + bp->size == p-.ptr) { /* join to upper nbr */
		bp->size += p->ptr->s.size;
		bp->ptr = p->ptr->s.ptr;
	}
	else
		bp->ptr = p->ptr;
	if (p + p->size == bp) { /* join to lower nbr */
		p->size += bp->size;
		p->ptr = bp->ptr;
	}
	else {
		p->ptr = bp;
	}
	freep = p;
    }
}

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
	freep = 0;

	board.memory.ram[0] = freep;
	board.memory.ram[1] = board.memory.ram.size() - 2;

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
	     Memory::page_entry_type pageIndex = board.cpu.registers.a;
	     Memory::page_entry_type newFrame = board.memory.AcquireFrame();
	     if (newFrame != Memory::INVALID_PAGE) {
		(*(board.memory.page_table))[pageIndex] = newFrame;
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

	if(!processes.empty() && scheduler != Priority ) {

		std::cout << "The process " << _current_process_index <<", has been set for the execution." << std::endl;

		board.cpu.registers = processes[_current_process_index].registers;
		board.memory.page_table = processes[_current_process_index].page_table;

		processes[_current_process_index].state = Process::States::Running;

	}
	else if (!priorities.empty() && scheduler == Priority) {

		std::cout << "The process " << priorities[_current_process_index].id << ", has been set for the execution." << std::endl;

		board.cpu.registers = priorities[_current_process_index].registers;
		board.memory.page_table = priorities[_current_process_index].page_table;

		priorities[_current_process_index].state = Process::States::Running;

	}

        if (scheduler == FirstComeFirstServed) {
            board.pic.isr_0 = [&]() {
                // ToDo: Process the timer interrupt for the FCFS
            };

            board.pic.isr_3 = [&]() {
                // ToDo: Process the first software interrupt for the FCFS

                std::cout << "Number of Processes left = " << processes.size() << std::endl;

		FreeMemory(processes[_current_process_index].memory_start_position);
		processes.erase(processes.begin() + _current_process_index);
		if (processes.empty()) {
			board.Stop();
		}
		else {
			board.cpu.registers = processes[_current_process_index].registers;
			board.memory.page_table = processes[_current_process_index].page_table;
			processes[_current_process_index].state = Process::States::Running;
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

		FreeMemory(processes[_current_process_index].memory_start_position);
		processes.erase(processes.begin() + _current_process_index);
		if (processes.empty()) {
			board.Stop();
		}
		else {
			board.cpu.registers = processes[_current_process_index].registers;
			board.memory.page_table = processes[_current_process_index].page_table;
			processes[_current_process_index].state = Process::States::Running;
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
			process_list_type::size_type next_process_index = (_current_process_index + 1)% processes.size();

			processes[_current_process_index].registers = board.cpu.registers;
			processes[_current_process_index].state = Process::States::Ready;

			_current_process_index = next_process_index;

			board.cpu.registers = processes[_current_process_index].registers;
			board.memory.page_table = processes[_current_process_index].page_table;
			processes[_current_process_index].state = Process::States::Running;
			_cycles_passed_after_preemption = 0;
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
			board.memory.page_table = processes[_current_process_index].page_table;
			processes[_current_process_index].state = Process::States::Running;
		}
            };
        } else if (scheduler == Priority) {
            board.pic.isr_0 = [&]() {
                // ToDo: Process the timer interrupt for the Priority Queue
                //  scheduler
		std::cout << "Priority of the current process = " << priorities[0].priority << "\t Cycles = " << _MAX_CYCLES_BEFORE_PREEMPTION << std::endl;

		for (unsigned int i = 1; i < priorities.size(); ++i){
			priorities[i].priority++;
		}

		process_list_type::size_type next_process_index = (_current_process_index + 1)% priorities.size();

		if (priorities[_current_process_index].priority < priorities[next_process_index].priority) {
			priorities[_current_process_index].registers = board.cpu.registers;
			priorities[_current_process_index].state = Process::States::Ready;

			for(unsigned int i = 1; i < priorities.size(); ++i){
				int j = i - 1;
				Process temp = priorities[i];
				while(j >=0 && priorities[j].priority < temp.priority){
					priorities[j+1] = priorities[j];
					j--;
				}
				priorities[j+1] = temp;
			}
			board.cpu.registers = priorities[_current_process_index].registers;
			board.memory.page_table = priorities[_current_process_index].page_table;
			priorities[_current_process_index].state = Process::States::Running;
			_cycles_passed_after_preemption = 0;
		}
            };

            board.pic.isr_3 = [&]() {
                // ToDo: Process the first software interrupt for the Priority
                //  Queue scheduler

                // Unload the current process
		std::cout << "Number of Processes left = " << processes.size() << std::endl;

		FreeMemory(priorities.front().memory_start_position);
		priorities.pop_front();

		if (priorities.empty()) {
			board.Stop();
		}
		else {
			if (_current_process_index >= priorities.size()) {
				_current_process_index = 0;
			}
			board.cpu.registers = priorities[_current_process_index].registers;
			board.memory.page_table = priorities[_current_process_index].page_table;
			priorities[_current_process_index].state = Process::States::Running;
		}
            };
        }

        board.Start();
    }

    Kernel::~Kernel() { }

    void Kernel::CreateProcess(Memory::ram_type &executable)
    {
        Memory::ram_size_type new_memory_position = AllocateMemory(executable.size());

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
	    srand (time(NULL));
	    process.priority = rand() % (processes.size() + 1) + 1;
            // ToDo: process the data structure (e.g., sort if necessary)

            if (scheduler == Priority) {
		priorities.push_back(process);
		std::sort (priorities.begin(), priorities.end());
	     }
	     else {
	 	processes.push_back(process);
	     }

	     if (scheduler == ShortestJob) {
		for(int i = 1; i < processes.size(); ++i){
			int j = i - 1;
			Process temp = processes[i];
			while(j >= 0 && processes[j].sequential_instruction_count > temp.sequential_instruction_count){
				processes[j+1] = processes[j];
				j--;
			}
			processes[j+1] = temp;
		}
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

        Memory::ram_size_type p, prevp = freep;

	for (p = board.memory.ram[prevp]; ; prevp = p, p = board.memory.ram[p]) {
		if(board.memory.ram[p+1] >= units){
			if(board.memory.ram[p+1] == units) {
				board.memory.ram[prevp] = board.memory.ram[p];
			}
			else {
				board.memory.ram[p + 1] -= units + 2;
				p += board.memory.ram[p + 1];
				board.memory.ram[p+1] = units;
			}

			freep = prevp;
			return  p+2;
		}
		if(p == freep)
			return -1;
	}
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
	Memory::ram_size_type p = freep;
	Memory::ram_size_type bp = physical_address - 2;

	for(; !(bp > p && bp < board.memory.ram[p]); p = board.memory.ram[p]) {
		if(p >= board.memory.ram[p] && (bp > p || bp < board.memory.ram[p])) {
			break;
		}
	}
	if(bp + board.memory.ram[bp+1] == board.memory.ram[p]) {
		board.memory.ram[bp+1] += board.memory.ram[board.memory.ram[p+1]];
		board.memory.ram[bp] = board.memory.ram[board.memory.ram[p]];
	}
	else {
		board.memory.ram[bp] = board.memory.ram[p];
	}
	if(p + board.memory.ram[p+1] == bp) {
		board.memory.ram[p+1] += board.memory.ram[bp+1];
		board.memory.ram[p] = board.memory.ram[bp];
	}
	else {
		board.memory.ram[p] = bp;
	}
	freep = p;
    }
}

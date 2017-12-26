#include "memory.h"

namespace svm
{
    Memory::Memory()
        : ram(DEFAULT_RAM_SIZE)
    {
        // TODO: initialize data structures for the frame allocator
	for (page_entry_type i = PAGE_SIZE; i < DEFAULT_RAM_SIZE; i += PAGE_SIZE) {
		freePhysicalFrames.push(i);
	}
    }

    Memory::~Memory() { }

    Memory::page_table_type* Memory::CreateEmptyPageTable()
    {
        /*
            TODO:

              Return a new page table (for kernel or processes)
              Each entry should be invalid
        */

        //return nullptr;
	Memory::ram_size_type numberOfPages = DEFAULT_RAM_SIZE/PAGE_SIZE;
	Memory::page_table_type emptyPageTable(numberOfPages);
	for(Memory::ram_size_type i = 0; i < numberOfPages; ++i){
		emptyPageTable[i] = INVALID_PAGE;
	}
	return &emptyPageTable;
    }

    Memory::page_index_offset_pair_type
        Memory::GetPageIndexAndOffsetForVirtualAddress(
                 vmem_size_type virtual_address
             )
    {
        page_index_offset_pair_type result =
            std::make_pair((page_table_size_type) -1, (ram_size_type) -1);

        /*
            TODO:

             Calculate the page index from the virtual address
             Calculate the offset in the physical memory from the virtual
             address
        */
	result.first = virtual_address / PAGE_SIZE;
	result.second = virtual_address % PAGE_SIZE;

        return result;
    }

    Memory::page_entry_type Memory::AcquireFrame()
    {
        // TODO: find a new free frame (you can use a bitmap or stack)
	if(!freePhysicalFrames.empty()) {
		page_entry_type result = freePhysicalFrames.top();
		freePhysicalFrames.pop();
		return result;
	}

        return INVALID_PAGE;
    }

    void Memory::ReleaseFrame(page_entry_type page)
    {
        // TODO: free the physical frame (you can use a bitmap or stack)
	freePhysicalFrames.push(page);
    }
}

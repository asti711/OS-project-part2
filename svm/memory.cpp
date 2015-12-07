#include "memory.h"

namespace svm
{
    Memory::Memory()
        : ram(DEFAULT_RAM_SIZE)
    {
        // TODO: initialize data structures for the frame allocator
    }

    Memory::~Memory() { }

    Memory::page_table_type* Memory::CreateEmptyPageTable()
    {
        /*
            TODO:

              Return a new page table (for kernel or processes)
              Each entry should be invalid
        */

        return nullptr;
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

        return result;
    }

    Memory::page_entry_type Memory::AcquireFrame()
    {
        // TODO: find a new free frame (you can use a bitmap or stack)

        return INVALID_PAGE;
    }

    void Memory::ReleaseFrame(page_entry_type page)
    {
        // TODO: free the physical frame (you can use a bitmap or stack)
    }
}

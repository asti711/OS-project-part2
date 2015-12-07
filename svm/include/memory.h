#ifndef MEMORY_H
#define MEMORY_H

#include <vector>
#include <stack>
#include <utility>

namespace svm
{
    // Physical Memory
    class Memory
    {
        public:
            typedef std::vector<int> ram_type;
            typedef ram_type::size_type ram_size_type;

            typedef ram_size_type vmem_size_type;
            typedef vmem_size_type page_entry_type;

            typedef std::vector<page_entry_type>  page_table_type;
            typedef page_table_type::size_type page_table_size_type;

            typedef std::pair<page_table_size_type, ram_size_type>
                page_index_offset_pair_type;

            static const ram_size_type DEFAULT_RAM_SIZE = 0x10000; // 64 KB
            static const ram_size_type PAGE_SIZE        = 0x80;    // 128 B
            static const ram_size_type INVALID_PAGE     = 0;

            ram_type ram; // physical memory as a fixed size array
            page_table_type* page_table; // current process's page table used to
                                         //   translate virtual addresses to
                                         //   physical

            Memory();
            virtual ~Memory();

            // Creates an empty page table for a process or the kernel
            static page_table_type* CreateEmptyPageTable();
            // Translates a virtual address into an index of a page table and an
            // offset in the physical address space
            page_index_offset_pair_type
                GetPageIndexAndOffsetForVirtualAddress(
                    vmem_size_type virtual_address
                );

            // Tries to find an empty physical frame
            page_entry_type AcquireFrame();
            // Releases a frame into a pool of free frames
            void ReleaseFrame(page_entry_type page);

        private:
            // TODO: select a data structure for your frame allocator
    };
}

#endif

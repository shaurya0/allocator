#include <cstdint>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <array>
#include <list>
#include <assert.h>
#include <limits>


template<class T, size_t POOL_SIZE, size_t ALIGNMENT=std::alignment_of<uintptr_t>()>
class static_memory_pool
{
private:
    struct free_block_header
    {
        size_t size;
        free_block_header *next;
    };

    free_block_header *_free_list;
    static constexpr size_t HEADER_SIZE = sizeof( free_block_header );
    static constexpr size_t ALIGNMENT_MASK = ALIGNMENT - 1;
    static constexpr size_t ALIGNED_HEADER_SIZE = (HEADER_SIZE + ALIGNMENT_MASK) & ~ALIGNMENT_MASK;
    alignas(ALIGNMENT) uint8_t _buffer[POOL_SIZE];

    const uintptr_t BUFFER_START = (uintptr_t)_buffer;
    const uintptr_t BUFFER_END = (uintptr_t)(&_buffer[POOL_SIZE]);
    static constexpr size_t ALLOCATED_FLAG = ~(std::numeric_limits<size_t>::max() >> 1);

    static_memory_pool()
    {
        assert( ((size_t)_buffer & ALIGNMENT_MASK) == 0 );
        _free_list = reinterpret_cast<free_block_header*>(_buffer);
        _free_list->size = POOL_SIZE - ALIGNED_HEADER_SIZE;
        _free_list->next = nullptr;
    }

public:

    void print_status() const
    {

    }

    const uint8_t* buffer() const {return _buffer;}

    static static_memory_pool<T, POOL_SIZE, ALIGNMENT> &get_instance()
    {
        static static_memory_pool<T, POOL_SIZE, ALIGNMENT> instance;
        return instance;
    }


    void *allocate(size_t requested_size)
    {
        void *result = nullptr;
        if( requested_size > POOL_SIZE )
        {
            std::cerr << "requested size " << requested_size << ", larger than POOL_SIZE " << POOL_SIZE << "\n";
            return result;
        }

        if( requested_size == 0 )
        {
            return result;
        }

        requested_size += ALIGNED_HEADER_SIZE;
        requested_size += ALIGNMENT_MASK & ~ALIGNMENT_MASK;

        free_block_header *prev = nullptr;
        free_block_header *it = _free_list;

        while( it != nullptr )
        {
            if( it->size >= requested_size )
            {
                //move pointer to after the header
                result = reinterpret_cast<uint8_t*>(it) + ALIGNED_HEADER_SIZE;


                if( it->next == nullptr )
                {
                    // TODO: check that next block is within bounds of buffer
                    // TODO: determine size of the next free block
                    free_block_header *next = reinterpret_cast<free_block_header *>(
						reinterpret_cast<uint8_t*>(it) + requested_size);

                    next->next = nullptr;
                    it->next = next;
                }

                // Set the upper most bit to indicate that it is allocated
                it->size = requested_size | ALLOCATED_FLAG;

                if(prev != nullptr)
                    prev->next = it->next;
                it->next = nullptr;
                break;
            }

            prev = it;
            it = it->next;
        }


		free_block_header *xx = reinterpret_cast<free_block_header *>(
			reinterpret_cast<uint8_t*>(result) - ALIGNED_HEADER_SIZE);

        return result;
    }

    void deallocate( void *p )
    {
        const uintptr_t addr = (uintptr_t)p;
        if(addr < (BUFFER_START + (uintptr_t)ALIGNED_HEADER_SIZE) || addr > (BUFFER_END - (uintptr_t)ALIGNED_HEADER_SIZE))
        {
            std::cout << "Address : " << addr << ", outside of static buffer range\n";
            return;
        }

        free_block_header *hdr = reinterpret_cast<free_block_header *>( addr - ALIGNED_HEADER_SIZE );

        if( !(hdr->size & ALLOCATED_FLAG) )
        {
            std::cout << "Pointer is not allocated\n";
            return;
        }

        hdr->size &= ~ALLOCATED_FLAG;
        hdr->next = _free_list;
        _free_list = hdr;

        //coalescing
    }
};

constexpr size_t POOL_SIZE = 1024;
using static_memory_pool_t = static_memory_pool<uint8_t, POOL_SIZE>;

void* operator new(std::size_t sz)
{
    auto &pool_instance = static_memory_pool_t::get_instance();
    return pool_instance.allocate(sz);
}

void operator delete  ( void* ptr )
{
    auto &pool_instance = static_memory_pool_t::get_instance();
    return pool_instance.deallocate(ptr);
}


int main(int argc, char const *argv[])
{
    int *x = new int(34);
	delete x;
    std::cout << *x << std::endl;
    return 0;
}

#include "static_memory_pool.h"

using namespace ss;

// constexpr size_t POOL_SIZE = 1<<16;
// using static_memory_pool_t = static_memory_pool<uint8_t, POOL_SIZE>;

// void* operator new(std::size_t sz)
// {
// 	return static_memory_pool_t::get_instance().allocate(sz);
// }

// void operator delete (void* ptr)
// {
// 	return static_memory_pool_t::get_instance().deallocate(ptr);
// }

// void* operator new (std::size_t size, const std::nothrow_t& nothrow_value) noexcept
// {
// 	return static_memory_pool_t::get_instance().allocate(size);
// }


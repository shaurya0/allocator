#include "static_memory_pool.h"
#include <cstdint>
using namespace ss;

#ifndef POOL_SIZE
	constexpr size_t POOL_SIZE = 1<<20;
#endif

//#define GLOBAL_NEW_OVERRIDE
#ifdef GLOBAL_NEW_OVERRIDE
using static_memory_pool_t = static_memory_pool<POOL_SIZE>;

 void* operator new(std::size_t sz) noexcept
 {
 	return static_memory_pool_t::get_instance().allocate(sz);
 }

 void operator delete (void* ptr) noexcept
 {
 	return static_memory_pool_t::get_instance().deallocate(ptr);
 }

 void* operator new (std::size_t size, const std::nothrow_t& nothrow_value) noexcept
 {
 	return static_memory_pool_t::get_instance().allocate(size);
 }

 void *operator new[](std::size_t s) throw(std::bad_alloc)
 {
	 return static_memory_pool_t::get_instance().allocate(s, true);
 }

void operator delete[](void *p) noexcept
 {
	 return static_memory_pool_t::get_instance().deallocate(p);
 }
#endif
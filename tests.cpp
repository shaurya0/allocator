#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#define UNIT_TEST
#include "static_memory_pool.h"
#include <memory>

constexpr size_t POOL_SIZE = 1<<10;
using namespace ss;

using static_memory_pool_t = static_memory_pool<POOL_SIZE>;



TEST_CASE("allocate 4 bytes", "[allocate]")
{
    auto &instance = static_memory_pool_t::get_instance();
    int32_t *x = (int32_t*)instance.allocate(sizeof(int32_t));
    *x = 999;
    REQUIRE(*x == 999);
	REQUIRE(instance.allocated() == sizeof(int32_t));

    instance.deallocate((void*)x);
	REQUIRE(instance.deallocated() == sizeof(int32_t));
}






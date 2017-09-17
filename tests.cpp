#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "static_memory_pool.h"
#include <memory>
#include <cstdint>

constexpr size_t POOL_SIZE = 1<<10;
using namespace ss;

using static_memory_pool_t = static_memory_pool<POOL_SIZE>;


struct something
{
	float x;
	double y;
	int32_t z;
    std::string s;
	std::vector<int> v;
};

TEST_CASE("allocate 4 bytes", "[allocate]")
{
	auto &instance = static_memory_pool_t::get_instance();
	int32_t *x = (int32_t*)instance.allocate(sizeof(int32_t));
	*x = 999;
	const auto it = instance.free_list();

	REQUIRE(*x == 999);
	REQUIRE(instance.allocated() == sizeof(int32_t));
	REQUIRE(it->is_allocated() == true);
	REQUIRE(it->get_size() == sizeof(int32_t));

	instance.deallocate((void*)x);
	REQUIRE(instance.deallocated() == sizeof(int32_t));
	instance.reset();
}


TEST_CASE("allocate struct", "[allocate]")
{
	auto &instance = static_memory_pool_t::get_instance();

	something *st = (something *)instance.allocate(sizeof(something));
	st->x = 3.0f;
	st->y = 42.1f;
	st->z = 918;
	st->s = std::string("hello");
	st->v = { 1,2,3,4,5,6 };

	const auto it = instance.free_list();

	REQUIRE(instance.allocated() == sizeof(something));
	REQUIRE(it->is_allocated() == true);
	REQUIRE(it->get_size() == sizeof(something));

	REQUIRE(st->x == 3.0f);
	REQUIRE(st->y == 42.1f);
	REQUIRE(st->z == 918);
	REQUIRE(st->s == "hello");
	REQUIRE(st->v == std::vector<int>({ 1,2,3,4,5,6 }));


	instance.deallocate((void*)st);
	REQUIRE(instance.deallocated() == sizeof(something));
	instance.reset();
}


TEST_CASE("allocate multiple structs, deallocate in reverse order", "[allocate]")
{
	auto &instance = static_memory_pool_t::get_instance();
	instance.reset();

	const size_t N = 8;

	std::vector<something*> somethings;
	for (unsigned i = 0; i < N; ++i)
	{
		something *st = (something *)instance.allocate(sizeof(something));

		st->x = 3.0f;
		st->y = 42.1f;
		st->z = 918;
		st->s = std::string("hello");
		st->v = { 1,2,3,4,5,6 };

		somethings.push_back(st);
	}

	static_memory_pool_t::free_block_header *it = instance.free_list();

	REQUIRE(instance.allocated() == N * sizeof(something));
	while (it != nullptr && it->is_allocated())
	{
		REQUIRE(it->is_allocated() == true);
		REQUIRE(it->get_size() == sizeof(something));
		something *st = (something*)(reinterpret_cast<uint8_t*>(it) + sizeof(*it));

		REQUIRE(st->x == 3.0f);
		REQUIRE(st->y == 42.1f);
		REQUIRE(st->z == 918);
		REQUIRE(st->s == "hello");
		REQUIRE(st->v == std::vector<int>({ 1,2,3,4,5,6 }));

		it = it->get_next();
	}

	for (auto rit = somethings.rbegin(); rit != somethings.rend(); ++rit)
	{
		something *st = *rit;
		instance.deallocate((void*)st);
	}

	 REQUIRE(instance.deallocated() == N*sizeof(something));

	 // check for coalescing
	 it = instance.free_list();
	 REQUIRE(it->get_size() == POOL_SIZE-static_memory_pool_t::ALIGNED_HEADER_SIZE);
	 REQUIRE(it->get_next() == nullptr);
	 REQUIRE(it->get_prev() == nullptr);
	 instance.reset();
}



TEST_CASE("allocate multiple structs, deallocate in original order", "[allocate]")
{
	auto &instance = static_memory_pool_t::get_instance();
	instance.reset();

	const size_t N = 8;

	std::vector<something*> somethings;
	for (unsigned i = 0; i < N; ++i)
	{
		something *st = (something *)instance.allocate(sizeof(something));

		st->x = 3.0f;
		st->y = 42.1f;
		st->z = 918;
		st->s = std::string("hello");
		st->v = { 1,2,3,4,5,6 };

		somethings.push_back(st);
	}

	static_memory_pool_t::free_block_header *it = instance.free_list();

	REQUIRE(instance.allocated() == N * sizeof(something));
	while (it != nullptr && it->is_allocated())
	{
		REQUIRE(it->is_allocated() == true);
		REQUIRE(it->get_size() == sizeof(something));
		something *st = (something*)(reinterpret_cast<uint8_t*>(it) + sizeof(*it));

		REQUIRE(st->x == 3.0f);
		REQUIRE(st->y == 42.1f);
		REQUIRE(st->z == 918);
		REQUIRE(st->s == "hello");
		REQUIRE(st->v == std::vector<int>({ 1,2,3,4,5,6 }));

		it = it->get_next();
	}

	for (auto it = somethings.begin(); it != somethings.end(); ++it)
	{
		something *st = *it;
		instance.deallocate(st);
	}

	REQUIRE(instance.deallocated() == N * sizeof(something));

	// check for coalescing
	it = instance.free_list();
	REQUIRE(it->get_size() == POOL_SIZE - static_memory_pool_t::ALIGNED_HEADER_SIZE);
	REQUIRE(it->get_next() == nullptr);
	REQUIRE(it->get_prev() == nullptr);
	instance.reset();
}


TEST_CASE("allocate multiple structs, deallocate half", "[allocate]")
{
    auto &instance = static_memory_pool_t::get_instance();
    instance.reset();

    const size_t N = 8;

    std::vector<something*> somethings;
    for (unsigned i = 0; i < N; ++i)
    {
        something *st = (something *)instance.allocate(sizeof(something));

        st->x = 3.0f;
        st->y = 42.1f;
        st->z = 918;
        st->s = std::string("hello");
        st->v = { 1,2,3,4,5,6 };

        somethings.push_back(st);
    }

    static_memory_pool_t::free_block_header *it = instance.free_list();

    REQUIRE(instance.allocated() == N * sizeof(something));
    while (it != nullptr && it->is_allocated())
    {
        REQUIRE(it->is_allocated() == true);
        REQUIRE(it->get_size() == sizeof(something));
        something *st = (something*)(reinterpret_cast<uint8_t*>(it) + sizeof(*it));

        REQUIRE(st->x == 3.0f);
        REQUIRE(st->y == 42.1f);
        REQUIRE(st->z == 918);
        REQUIRE(st->s == "hello");
        REQUIRE(st->v == std::vector<int>({ 1,2,3,4,5,6 }));

        it = it->get_next();
    }


    for (int i = 0; i < N/2; ++i)
    {
        something *st = somethings[i];
        instance.deallocate(st);
    }

    REQUIRE(instance.deallocated() == (N/2) * sizeof(something));

    // check for coalescing
    it = instance.free_list();
    REQUIRE(it->get_size() == 3*static_memory_pool_t::ALIGNED_HEADER_SIZE + 4*sizeof(something));
    REQUIRE(it->get_prev() == nullptr);
    instance.reset();
}



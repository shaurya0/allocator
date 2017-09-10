#pragma once

#include <cstdint>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <array>
#include <list>
#include <assert.h>
#include <limits>
#include <memory>

namespace ss
{
	template<size_t POOL_SIZE, size_t ALIGNMENT = std::alignment_of<uintptr_t>()>
	class static_memory_pool
	{
	public:
		struct free_block_header
		{
		private:
			size_t _size;
			free_block_header *_next;
			free_block_header *_prev;

		public:
			const bool is_allocated() const
			{
				return (_size & ALLOCATED_FLAG) > 0;
			}

			void set_size( size_t new_size, bool allocated=false)
			{
				if( allocated )
				{
					// Set the upper most bit to indicate that it is allocated
					_size = (new_size | ALLOCATED_FLAG);
				}
				else
				{
					_size = new_size;
				}
			}

			const size_t get_size() const noexcept
			{
				return static_cast<size_t>(_size & ~ALLOCATED_FLAG);
			}

			const free_block_header *get_next() const noexcept
			{
				return _next;
			}

			free_block_header *get_next() noexcept
			{
				return _next;
			}

			void set_next(free_block_header *next)
			{
				_next = next;
			}

			const free_block_header *get_prev() const noexcept
			{
				return _prev;
			}

			free_block_header *get_prev() noexcept
			{
				return _prev;
			}

			void set_prev(free_block_header *prev)
			{
				_prev = prev;
			}


		};

		static constexpr size_t HEADER_SIZE = sizeof(free_block_header);
		static constexpr size_t ALIGNMENT_MASK = ALIGNMENT - 1;
		static constexpr size_t ALIGNED_HEADER_SIZE = (HEADER_SIZE + ALIGNMENT_MASK) & ~ALIGNMENT_MASK;
		alignas(ALIGNMENT)uint8_t _buffer[POOL_SIZE];
		static constexpr size_t ALLOCATED_FLAG = ~(std::numeric_limits<size_t>::max() >> 1);

	private:

		free_block_header *_free_list;

		size_t _allocated;
		size_t _deallocated;


		const uintptr_t BUFFER_START = reinterpret_cast<uintptr_t>(_buffer);
		const uintptr_t BUFFER_END = reinterpret_cast<uintptr_t>(&_buffer[POOL_SIZE]);

		static_memory_pool()
		{
			assert(((size_t)_buffer & ALIGNMENT_MASK) == 0);
			reset();
		}

	public:

		void reset()
		{
			_allocated = 0;
			_deallocated = 0;
			_free_list = reinterpret_cast<free_block_header*>(_buffer);
			_free_list->set_size( POOL_SIZE );
			_free_list->set_next( nullptr );
			_free_list->set_prev( nullptr );
		}

		static static_memory_pool<POOL_SIZE, ALIGNMENT> &get_instance() noexcept
		{
			static static_memory_pool<POOL_SIZE, ALIGNMENT> instance;
			return instance;
		}

		bool is_inside_pool(uintptr_t addr) const noexcept
		{
			return (addr >= (BUFFER_START + ALIGNED_HEADER_SIZE) && addr < (BUFFER_END - ALIGNED_HEADER_SIZE));
		}

		void *allocate(size_t requested_size) noexcept
		{
			if (requested_size == 0)
				return nullptr;

			if (requested_size > POOL_SIZE)
			{
				std::cerr << "requested size " << requested_size << ", larger than POOL_SIZE " << POOL_SIZE << "\n";
				return nullptr;
			}

			size_t requested_size_with_header = requested_size + ALIGNED_HEADER_SIZE;
			requested_size_with_header += ALIGNMENT_MASK & ~ALIGNMENT_MASK;

			free_block_header *prev = nullptr;
			free_block_header *it = _free_list;

			void *result = nullptr;

			while (it != nullptr)
			{
				if (false == it->is_allocated() && it->get_size() >= requested_size)
				{
					//move data pointer to after the header
					result = reinterpret_cast<uint8_t*>(it) + ALIGNED_HEADER_SIZE;

					if (it->get_next() == nullptr)
					{
						free_block_header *next = reinterpret_cast<free_block_header*>(
							reinterpret_cast<uint8_t*>(it) + requested_size_with_header);


						if ((BUFFER_END - ALIGNED_HEADER_SIZE) < reinterpret_cast<uintptr_t>(next))
						{
							// out of memory
							return nullptr;
						}

						_allocated += requested_size;

						const size_t remaining_size = (it->get_size() - requested_size);

						next->set_size(remaining_size);
						next->set_next(nullptr);
						next->set_prev(it);
						it->set_next(next);
					}

					it->set_size(requested_size, true);

					if (prev != nullptr)
						prev->set_next( it );

					break;
				}

				prev = it;
				it = it->get_next();
			}

			return result;
		}


		void deallocate(void *p)
		{
			const uintptr_t addr = reinterpret_cast<uintptr_t>(p);
			if ( false == is_inside_pool(addr) )
			{
				throw std::runtime_error("Tried to deallocate pointer outside of static buffer range\n");
			}

			free_block_header *hdr = reinterpret_cast<free_block_header *>(
				reinterpret_cast<uint8_t*>(addr) - ALIGNED_HEADER_SIZE);

			if (false == hdr->is_allocated())
			{
				throw std::runtime_error("Tried to deallocate unallocated pointer");
			}

			_deallocated += hdr->get_size();

			// coalesce
			free_block_header *next_block = hdr->get_next();
			if (false == next_block->is_allocated())
			{
				const size_t next_size = next_block->get_size();
				hdr->set_size(hdr->get_size() + next_size);

				next_block->set_size(0);

				hdr->set_next(next_block->get_next());
			}
		}

		#ifdef UNIT_TEST
		free_block_header *free_list () const noexcept
		{
			return _free_list;
		}

		const size_t allocated() const noexcept { return _allocated; }
		const size_t deallocated() const noexcept { return _deallocated; }

		#endif
	};
}

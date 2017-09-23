#pragma once

#include <chrono>

namespace ss
{
	template<typename CLOCK_TYPE>
	class timer
	{
		using clock_t = CLOCK_TYPE;
		std::chrono::time_point<CLOCK_TYPE> _tick;
		std::chrono::time_point<CLOCK_TYPE> _tock;

	public:
		void tick()
		{
			_tick = clock_t::now();
		};

		void tock()
		{
			_tock = clock_t::now();
		}

		template<typename DURATION_TYPE>
		uint64_t duration()
		{
			return std::chrono::duration_cast<DURATION_TYPE>(_tock - _tick).count();

		}
	};
}
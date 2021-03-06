#include <core/Win32_System.h>

namespace k2d
{
	unsigned long long GetPerfFrequency()
	{
		LARGE_INTEGER ret;
		QueryPerformanceFrequency(&ret);
		return ret.QuadPart;
	}

	unsigned long long GetPerfCounter()
	{
		LARGE_INTEGER num_ticks;
		QueryPerformanceCounter(&num_ticks);
		return num_ticks.QuadPart;
	}

	int SleepMilliseconds(unsigned int _ms)
	{
		Sleep((DWORD)_ms);
		return 0;
	}


} // End of namespace k2d
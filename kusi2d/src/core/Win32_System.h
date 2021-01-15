#pragma once

#include <winsock2.h>
#include <Windows.h>

namespace k2d
{
	unsigned long long GetPerfFrequency();

	unsigned long long GetPerfCounter();

	int SleepMilliseconds(unsigned int _ms);


} // End of namespace k2d
#include <iostream>
#include <core/Conquest.h>
#include <core/ConquestLocal.h>

#pragma comment(lib, "Ws2_32.lib")


int main(int argc, char** argv)
{
	std::string window_title = "Conquest AI Training";
	int window_width = 1600;
	int window_height = 900;
	float fps_target = 600000.0f;
	bool v_sync = false;

	ConquestLocal conqlocal(window_title, window_width, window_height, fps_target, v_sync);

	return 0;
}
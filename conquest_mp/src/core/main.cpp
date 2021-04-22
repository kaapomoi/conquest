#include <iostream>
#include <core/Conquest.h>
#include <core/ConquestMultiplayer.h>

#pragma comment(lib, "Ws2_32.lib")


int main(int argc, char** argv)
{
	std::string window_title = "Conquest Multiplayer";
	int window_width = 1600;
	int window_height = 900;
	float fps_target = 600000.0f;
	bool v_sync = false;

	ConquestMultiplayer conqmulti(window_title, window_width, window_height, fps_target, v_sync);

	return 0;
}
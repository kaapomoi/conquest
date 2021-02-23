#include <util/util.h>

namespace k2d
{
	/// Prints error to console and exits the program
	void KUSI_ERROR(std::string _error_string) {
		std::cout << _error_string << "\n";
		std::cout << "Enter any key to quit...";
		int tmp;
		std::cin >> tmp;
		SDL_Quit();
		exit(-1);
	}

	/// printf
	void KUSI_DEBUG(const char* _fmt, ...)
	{
		va_list args;
		va_start(args, _fmt);
		vprintf(_fmt, args);
		va_end(args);
	}
	
	/// Reads a file into a char buffer
	bool ReadFileToBuffer(std::string _file, std::vector<unsigned char>& _buffer)
	{
		std::ifstream file(_file, std::ios::binary);
		if (file.fail())
		{
			perror(_file.c_str());
			return false;
		}

		// Seek to the end of the file
		file.seekg(0, std::ios::end);

		// Get the file size
		int fileSize = file.tellg();
		file.seekg(0, std::ios::beg);
		fileSize -= file.tellg();

		_buffer.resize(fileSize);
		file.read((char*)&(_buffer[0]), fileSize);
		file.close();

		return true;
	}

	float sqrt(int x, int y)
	{
		x = abs(x);
		y = abs(y);

		return floats[y][x];
	}

	void error_callback_for_glfw(int error, const char* desc)
	{
		fprintf(stderr, "Error: %s\n", desc);
	}

	/// Converts given degrees to radians
	float DegToRad(float _deg)
	{
		return _deg * 3.14159265359f / 180;
	}

	double clamp(double in, double min, double max)
	{
		if (in <= min)
		{
			return min;
		}
		if (in >= max)
		{
			return max;
		}
		return in;
	}

	float clamp(float in, float min, float max)
	{
		if (in <= min)
		{
			return min;
		}
		if (in >= max)
		{
			return max;
		}
		return in;
	}

	int clamp(int in, int min, int max)
	{
		if (in <= min)
		{
			return min;
		}
		if (in >= max)
		{
			return max;
		}
		return in;
	}
}
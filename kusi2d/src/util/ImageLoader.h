#pragma once
#include <graphics/GLTexture.h>
#include <util/util.h>
#include <util/picoPNG.h>
#include <vector>
#include <string>

namespace k2d
{
	class ImageLoader
	{
	public:
		static GLTexture LoadPNG(std::string _filePath, bool _interpolation);
		static GLTexture* LoadTexturePTR(std::string _filePath, bool _interpolation);
	};

}
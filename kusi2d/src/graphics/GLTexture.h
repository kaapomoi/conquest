#pragma once
#include <GL/glew.h>

namespace k2d
{
	typedef unsigned int GLUint;
	struct GLTexture
	{
		GLUint  id;
		int		width;
		int		height;

		GLTexture()
		{
			id = 0;
			width = 0;
			height = 0;
		};
	};


} // End of namespace: k2d
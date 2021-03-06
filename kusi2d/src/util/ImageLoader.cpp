#include <util/ImageLoader.h>

namespace k2d
{
	/*
	 *	Loads a png with the specified path, if no texture is found,
     *  loads a default_texture from the Textures folder
	 */
	GLTexture ImageLoader::LoadPNG(std::string _filePath, bool _interpolation)
	{
		//Init texture to 0
		GLTexture texture = {};

		std::vector<unsigned char> in;
		std::vector<unsigned char> out;

		unsigned long width, height;

		if (ReadFileToBuffer(_filePath, in) == false)
		{
			KUSI_ERROR("Failed to load PNG file to buffer " + _filePath);
		}

		int errorCode = DecodePNG(out, width, height, &(in[0]), in.size());
		if (errorCode != 0)
		{
			KUSI_ERROR("decodePNG failed with error: " + std::to_string(errorCode));
		}

		glGenTextures(1, &(texture.id));

		glBindTexture(GL_TEXTURE_2D, texture.id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(out[0]));

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// GL_NEAREST FOR NO INTERPOLATION ON PIXEL UPSCALING
		if (_interpolation)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}

		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);

		texture.width = width;
		texture.height = height;


		return texture;
	}

	GLTexture* ImageLoader::LoadTexturePTR(std::string _filePath, bool _interpolation)
	{
		//Init texture to 0
		GLTexture* texture = new GLTexture();

		std::vector<unsigned char> in;
		std::vector<unsigned char> out;

		unsigned long width, height;

		if (ReadFileToBuffer(_filePath, in) == false)
		{
			KUSI_ERROR("Failed to load PNG file to buffer " + _filePath);
		}

		int errorCode = DecodePNG(out, width, height, &(in[0]), in.size());
		if (errorCode != 0)
		{
			KUSI_ERROR("decodePNG failed with error: " + std::to_string(errorCode));
		}

		glGenTextures(1, (&texture->id));

		glBindTexture(GL_TEXTURE_2D, texture->id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(out[0]));

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// GL_NEAREST FOR NO INTERPOLATION ON PIXEL UPSCALING
		if (_interpolation)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		}

		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);

		texture->width = width;
		texture->height = height;


		return texture;
	}

} // End of k2d namespace
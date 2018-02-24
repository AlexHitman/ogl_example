#include "imgtools.h"

#include <stdio.h>

#include <fstream>

RawImage::RawImage()
{}

RawImage::RawImage(const std::vector<char> & data, size_t width, size_t height)
	: m_data(data)
	, m_width(width)
	, m_height(height)
{}

size_t RawImage::GetWidth() const
{
	return m_width;
}

size_t RawImage::GetHeight() const
{
	return m_height;
}

const char *RawImage::GetData() const
{
	return m_data.data();
}

RawImage RawImage::LoadFromBMP(const std::string & path)
{
	std::ifstream file(path.c_str());

	if (!file)
		throw std::runtime_error("Can't open file " + path);

	char header[54];
	file.read(header, 54);
	if (!file)
		throw std::runtime_error("Can't read header from " + path);

	if (header[0]!='B' || header[1]!='M')
		throw std::runtime_error("Incorrect header in " + path);

	RawImage image;

	size_t dataPos    = *(int*)&(header[0x0A]);
	size_t imageSize  = *(int*)&(header[0x22]);
	image.m_width     = *(int*)&(header[0x12]);
	image.m_height   = *(int*)&(header[0x16]);

	if (imageSize == 0)
		imageSize = image.m_width * image.m_height * 3;
	if (dataPos == 0)
		dataPos = 54;

	image.m_data.resize(imageSize);
	file.read(image.m_data.data(), imageSize);
	if (!file)
		throw std::runtime_error("Can't read data from " + path);

	return image;
}

void RawImage::SaveToBMP(std::string const & format, std::string const & path)
{
	std::string const cmd = "ffmpeg -pix_fmt " + format + " -s " + std::to_string(m_width) + "x" + std::to_string(m_height) + " -f rawvideo -i pipe:0 -y " + path;
	FILE * pipe = popen(cmd.c_str(), "w");
	if (!pipe)
		throw std::runtime_error("Can't open file for writng: " + path);

	fwrite((void const *)m_data.data(), 1, m_data.size(), pipe);

	pclose(pipe);
}




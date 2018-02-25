#include "imgtools.h"

#include <stdio.h>

#include <fstream>

RawImage::RawImage()
{}

RawImage::RawImage(const std::vector<char> & data, std::string const & pixFmt, size_t width, size_t height)
	: m_data(data)
	, m_pixFmt(pixFmt)
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

std::string RawImage::GetPixFmt() const
{
	return m_pixFmt;
}

RawImage RawImage::LoadFromFile(const std::string & path, size_t width, size_t height)
{
	RawImage image;
	image.m_width = width;
	image.m_height = height;
	image.m_data.resize(3 * width * height);
	image.m_pixFmt = "rgb24";

	std::string const cmd = "ffmpeg -i " + path + " -s " + std::to_string(width) + "x" + std::to_string(height) +
			" -pix_fmt " + image.m_pixFmt + " -f rawvideo -";
	FILE * pipe = popen(cmd.c_str(), "r");
	if (!pipe)
		throw std::runtime_error("Can't open file for reading: " + path);

	size_t retBytesCnt = 0;
	size_t sumBytesCnt = 0;
	while ((retBytesCnt = fread(&image.m_data[sumBytesCnt], 1, BUFSIZ, pipe)) > 0)
		sumBytesCnt += retBytesCnt;

	pclose(pipe);

	return image;

}

void RawImage::SaveToFile(std::string const & path) const
{
	std::string const cmd = "ffmpeg -pix_fmt " + m_pixFmt + " -s " + std::to_string(m_width) + "x" + std::to_string(m_height) +
			" -f rawvideo -i pipe:0 -y " + path;
	FILE * pipe = popen(cmd.c_str(), "w");
	if (!pipe)
		throw std::runtime_error("Can't open file for writng: " + path);

	fwrite((void const *)m_data.data(), 1, m_data.size(), pipe);

	pclose(pipe);
}




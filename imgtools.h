#pragma once

#include <string>
#include <vector>

class RawImage
{
public:
	RawImage(std::vector<char> const & data, size_t width, size_t height);

	size_t GetWidth() const;
	size_t GetHeight() const;
	char const * GetData() const;

	static RawImage LoadFromBMP(std::string const & path);
	void SaveToBMP(std::string const & format, std::string const & path);

private:
	RawImage();

private:
	std::vector<char> m_data;
	size_t m_width;
	size_t m_height;
};

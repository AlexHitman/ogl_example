#pragma once

#include <string>
#include <vector>

class RawImage
{
public:
	size_t GetWidth() const;
	size_t GetHeight() const;
	char const * GetData() const;

	static RawImage LoadFromBMP(std::string const & path);
	void SaveToBMP(std::string const & format, std::string const & path);

private:
	std::vector<char> m_data;
	size_t m_width;
	size_t m_height;
};

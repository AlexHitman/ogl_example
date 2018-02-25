#pragma once

#include <string>
#include <vector>

class RawImage
{
public:
	RawImage(std::vector<char> const & data, std::string const & pixFmt, size_t width, size_t height);

	size_t GetWidth() const;
	size_t GetHeight() const;
	char const * GetData() const;
	std::string GetPixFmt() const;

	static RawImage LoadFromFile(std::string const & path, size_t width, size_t height);
	void SaveToFile(std::string const & path) const;

private:
	RawImage();

private:
	std::vector<char> m_data;
	std::string m_pixFmt;
	size_t m_width;
	size_t m_height;
};

#include "HexFont.hpp"
#include <fstream>

//===========================================

#define HEX2QUAD(byte) ((byte) > '9'? 10 + (byte) - 'A': (byte) - '0')
#define HEX2BYTE(byte0, byte1) (HEX2QUAD(byte0) << 4) | HEX2QUAD(byte1)

//===========================================

namespace OCIF
{
	
//=========================================== Glyph

HexFont::Glyph::Glyph(const uint8_t* data, size_t size)
{
	setData(data, size);
}

HexFont::Glyph::Glyph(const Glyph& copy)
{
	setData(copy.m_data, copy.m_size);
}

HexFont::Glyph::Glyph(Glyph&& move) noexcept
{
	std::swap(m_data, move.m_data);
	std::swap(m_size, move.m_size);
}

HexFont::Glyph::~Glyph()
{
	if (m_data)
		delete[](m_data);

	m_data = nullptr;
	m_size = 0;
}

void HexFont::Glyph::setData(const uint8_t* data, size_t size)
{
	uint8_t* tmp = new uint8_t[size];
	std::memcpy(tmp, data, size);
	std::swap(m_data, tmp);

	if (tmp)
		delete[](tmp);

	m_size = size;
}

bool HexFont::Glyph::dot(unsigned x, unsigned y) const
{
	// Each byte representing a row of 8 or 16 pixels
	return ((m_data[(y * getWidth() + x) / 8] >> (7 - (x % 8)))) & 1;
}

void HexFont::Glyph::rasterize(sf::Image& image, unsigned x, unsigned y, sf::Color background, sf::Color foreground) const
{
	const auto& image_size = image.getSize();
	for (size_t dot_x = 0; dot_x < getWidth() && x + dot_x < image_size.x; dot_x++)
		for (size_t dot_y = 0; dot_y < getHeight() && y + dot_y < image_size.y; dot_y++)
			image.setPixel(x + dot_x, y + dot_y, dot(dot_x, dot_y)? foreground: background);
}

size_t HexFont::Glyph::getSize() const
{
	return m_size;
}

const uint8_t* HexFont::Glyph::getData() const
{
	return m_data;
}

//=========================================== Font

void HexFont::loadFromStream(std::istream& stream)
{
	std::string line;
	std::vector<uint8_t> data;
	while (stream.peek() != EOF && std::getline(stream, line))
	{
		auto pos = line.find(':');
		if (pos == std::string::npos)
		{
			std::cerr << "Delimiter not found" << std::endl;
			continue;
		}

		uint32_t code = std::stoul(line.substr(0, pos), nullptr, 16);

		data.clear();
		for (size_t i = pos+1; i < line.length(); i += 2)
			data.push_back(HEX2BYTE(line[i], line[i+1]));

		m_data.insert({code, Glyph(data.data(), data.size())});
	}
}

void HexFont::loadFromFile(const std::filesystem::path& path)
{
	std::ifstream stream;
	stream.exceptions(stream.exceptions() | std::ios::failbit);
	stream.open(path, std::ios::binary);

	loadFromStream(stream);
}

const HexFont::Glyph& HexFont::getGlyph(uint32_t ch)
{
	return m_data.at(ch);
}

const HexFont::Glyph& HexFont::operator[](uint32_t ch)
{
	return getGlyph(ch);
}

//===========================================

}; // namespace OCIF

//===========================================
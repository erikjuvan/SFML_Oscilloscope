#pragma once

#include <vector>
#include <SFML/Graphics.hpp>

#include "fft.h"

class Display {
	sf::RenderWindow& window_;
	int x_, y_, w_, h_;
	sf::VertexArray axis_;

	int nCh_;
	int nPoints_;
	std::vector<sf::VertexArray>	points_;
	std::vector<int>				idxPoints_;
	std::vector<bool>				channelOn_;

	sf::Font font_;
	std::vector<sf::Text> text_;
	bool usingText_;

	const uint32_t colors_[10] {0xFF0000FF, 0x00FF00FF, 0x0000FFFF, 0xFFFF00FF, 0x00FFFFFF, 0xFF00FFFF, 0xFF8000FF, 0xC0C0C0FF, 0x800000FF, 0x808000FF};

public:
	Display(sf::RenderWindow& window, int x, int y, int w, int h, int nCh, int nPoints, bool usingText);

	void				draw();
	void				setTextPosition(const int channel, const sf::Vector2f& pos);
	const sf::Vector2f& getTextPosition(int channel) const;
	void				setTextString(int channel, const std::string& str);
	int					textPositionToChannel(const sf::Vector2f& pos);
	inline sf::Vector2f	transform(double x, double y);
	void				fillPoints(const int channel, const std::vector<double>& data, int len);
	void				toggleChannelVisibility(const int channel);
};

class Time {
	Display display_;

	int nCh_;
	int nData_;
	int idxData_;
	std::vector<std::vector<double>> data_;

public:
	Time(sf::RenderWindow& window, int x, int y, int w, int h, int nCh, int nData, int nPoints);

	void toggleChannel(const int channel);
	void draw();
	void run(const uint8_t* rawData, int size);
};

class Freq {
	Display display_;
	FFT fft_;

	int nCh_;
	int nData_;
	int idxData_;	
	std::vector<std::vector<double>> data_;
	std::vector<double> freq_;

public:
	Freq(sf::RenderWindow& window, int x, int y, int w, int h, int nCh, int nData, int nPoints, int sampleFreq, uint8_t* buffer);

	int toggleChannel(const sf::Vector2f& pos);
	void draw();
	void run(const uint8_t* rawData, int size);
};

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>

#include <SFML/Graphics.hpp>

class Display {

	sf::RenderWindow& window_;
	int x_, y_, w_, h_;
	sf::VertexArray axis_;

	int nCh_;
	int nPoints_;
	int idxPoints_;
	std::vector<sf::VertexArray> points_;

	sf::Font font_;
	std::vector<sf::Text> text_;

	const uint32_t colors_[10] {0xFF0000FF, 0x00FF00FF, 0x0000FFFF, 0xFFFF00FF, 0x00FFFFFF, 0xFF00FFFF, 0xFF8000FF, 0xC0C0C0FF, 0x800000FF, 0x808000FF};

public:

	Display(sf::RenderWindow& window, int x, int y, int w, int h, int nCh, int nPoints) :
		window_(window), x_(x), y_(y), w_(w), h_(h), axis_(sf::PrimitiveType::LinesStrip, 3),
		nCh_(nCh), nPoints_(nPoints), idxPoints_(0), text_(nCh)  {		

		if (!font_.loadFromFile("../res/arial.ttf")) std::cerr << "Couldn't load font arial.ttf\n";

		axis_[0].position.x = x_; axis_[0].position.y = y_ - h_;
		axis_[1].position.x = x_; axis_[1].position.y = y_;
		axis_[2].position.x = x_ + w_; axis_[2].position.y = y_;

		for (int i = 0; i < nCh_; ++i) {
			// Points
			points_.emplace_back(sf::PrimitiveType::LinesStrip, nPoints_);
			for (int j = 0; j < nPoints_; ++j) {				
				points_[i][j].color = sf::Color(colors_[i]);
				points_[i][j].position = sf::Vector2f(j * w_ / nPoints_ + x, y - 1);
			}

			// Text
			text_[i].setFont(font_);
			text_[i].setCharacterSize(18); // in pixels, not points!
			text_[i].setFillColor(sf::Color(colors_[i]));
		}
	}

	void draw() {
		window_.draw(axis_);
		for (int i = 0; i < nCh_; ++i) {
			window_.draw(points_[i]);
			window_.draw(text_[i]);
		}
	}

	void setTextPosition(int channel, int x, int y) {
		if (channel <= nCh_) 
			text_[channel].setPosition(x, y);
	}

	void setTextString(int channel, const std::string& str) {
		if (channel <= nCh_) 
			text_[channel].setString(str);
	}

	inline sf::Vector2f Transform(double x, double y) {
		const double MaxY = 255.0;
		return sf::Vector2f(x_ + x * w_ / nPoints_, -1 + y_ - h_ * y / MaxY);
	}

	void fillPoints(const std::vector<std::vector<double>>& organizedData, int sizePerChannel) {
		for (int i = 0; i < sizePerChannel; ++i) {
			if (idxPoints_ >= nPoints_) idxPoints_ = 0;
			for (int chi = 0; chi < nCh_; ++chi) {
				points_[chi][idxPoints_].position = Transform(idxPoints_, organizedData[chi][i]);
			}
			idxPoints_++;
		}
	}
};

class Time {
	Display display_;

	int nCh_;
	int nData_;
	int idxData_;
	std::vector<std::vector<double>> data_;

public:
	Time(sf::RenderWindow& window, int x, int y, int w, int h, int nCh, int nData, int nPoints) :
		display_(window, x, y, w, h, nCh, nPoints), nCh_(nCh), nData_(nData), idxData_(0), data_(nCh, std::vector<double>(nData)) { }

	void draw() {
		display_.draw();
	}

	void run(const uint8_t* rawData, int size) {
		for (int i = 0; i < size; ++i) {
			if (idxData_ >= nData_) idxData_ = 0;
			for (int chi = 0; chi < nCh_; ++chi) {
				data_[chi][idxData_] = static_cast<double>(*(rawData++));
			}
			idxData_++;
		}

		display_.fillPoints(data_, nData_);
	}
};

class Freq {
	Display display_;
	FFT fft_;

	int nCh_;
	int nData_;
	int idxData_;	
	std::vector<std::vector<double>> data_;
	std::vector<std::vector<double>> dataOut_;
	std::vector<double> freq_;

public:
	Freq(sf::RenderWindow& window, int x, int y, int w, int h, int nCh, int nData, int nPoints, int usPerSample) :
		display_(window, x, y, w, h, nCh, nPoints),
		fft_(nCh, nData, usPerSample),
		nCh_(nCh), nData_(nData), idxData_(0), data_(nCh, std::vector<double>(nData)), dataOut_ (nCh, std::vector<double>(nPoints)),
		freq_(nCh) { 
		
		for (int chi = 0; chi < nCh; ++chi)
			display_.setTextPosition(chi, x + 35 + chi * (w / 15), y - h - 20);	// Some magical position values
	}

	void draw() {
		display_.draw();
	}

	void run(const uint8_t* rawData, int size) {
		for (int i = 0; i < size; ++i) {
			if (idxData_ >= nData_) {
				idxData_ = 0;
				fft_.run(data_, dataOut_, freq_);
				for (int chi = 0; chi < nCh_; ++chi) {
					char cbuf[10];
					std::sprintf(cbuf, "%.2f", freq_[chi]);
					display_.setTextString(chi, cbuf);
				}					
				display_.fillPoints(dataOut_, dataOut_[0].size());
			}
			for (int chi = 0; chi < nCh_; ++chi) {
				data_[chi][idxData_] = static_cast<double>(*(rawData++));
			}
			idxData_++;
			
		}
	}

};

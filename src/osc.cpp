#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>

#include <SFML/Graphics.hpp>

#include "osc.h"


///////////////////////////////////////////////////////////
///////////////////////// DISPLAY /////////////////////////
///////////////////////////////////////////////////////////

Display::Display(sf::RenderWindow& window, int x, int y, int w, int h, int nCh, int nPoints, bool usingText) :
	window_(window), x_(x), y_(y), w_(w), h_(h), axis_(sf::PrimitiveType::LinesStrip, 3),
	nCh_(nCh), nPoints_(nPoints), idxPoints_(nCh, 0), text_(nCh), channelOn_(nCh, true), usingText_(usingText) {

	// Load font file
	if (!font_.loadFromFile("../res/arial.ttf")) std::cerr << "Couldn't load font arial.ttf\n";

	// Set axis lines
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

void Display::draw() {
	window_.draw(axis_);
	for (int i = 0; i < nCh_; ++i) {
		if (channelOn_[i]) {
			window_.draw(points_[i]);
			if (usingText_) window_.draw(text_[i]);
		} else if (usingText_) {
			setTextString(i, "OFF");
			window_.draw(text_[i]);
		}
	}
}

void Display::setTextPosition(const int channel, const sf::Vector2f& pos) {
	if (0 <= channel && channel < nCh_)
		text_[channel].setPosition(pos);
}

const sf::Vector2f& Display::getTextPosition(int channel) const {
	if (0 <= channel && channel < nCh_)
		return text_[channel].getPosition();
	else
		return sf::Vector2f(-1, -1);
}

int Display::textPositionToChannel(const sf::Vector2f& pos) {
	for (int i = 0; i < nCh_; i++) {
		if (text_[i].getGlobalBounds().contains(pos))
			return i;
	}
	return -1;
}

void Display::setTextString(const int channel, const std::string& str) {
	if (0 <= channel && channel < nCh_)
		text_[channel].setString(str);
}

inline sf::Vector2f Display::transform(const double x, const double y) {
	const double MaxY = 255.0;
	return sf::Vector2f(x_ + x * w_ / nPoints_, -1 + y_ - h_ * y / MaxY);
}

void Display::fillPoints(const int channel, const std::vector<double>& data, int len) {
	for (int i = 0; i < len; ++i) {
		if (idxPoints_[channel] >= nPoints_) idxPoints_[channel] = 0;
		points_[channel][idxPoints_[channel]].position = transform(idxPoints_[channel], data[i]);
		++idxPoints_[channel];
	}
}

void Display::toggleChannelVisibility(const int channel) {
	if (0 <= channel && channel < nCh_)
		channelOn_[channel] = !channelOn_[channel];
}


////////////////////////////////////////////////////////
///////////////////////// TIME /////////////////////////
////////////////////////////////////////////////////////


Time::Time(sf::RenderWindow& window, int x, int y, int w, int h, int nCh, int nData, int nPoints) :
		display_(window, x, y, w, h, nCh, nPoints, false), nCh_(nCh), nData_(nData), idxData_(0), data_(nCh, std::vector<double>(nData)) { 
}

void Time::toggleChannel(const int channel) {
	display_.toggleChannelVisibility(channel);
}

void Time::draw() {
	display_.draw();
}

void Time::run(const uint8_t* rawData, int size) {
	for (int i = 0; i < size; ++i) {
		if (idxData_ >= nData_) idxData_ = 0;
		for (int chi = 0; chi < nCh_; ++chi) {
			data_[chi][idxData_] = static_cast<double>(*(rawData++));
		}
		idxData_++;
	}

	for (int chi = 0; chi < nCh_; ++chi) {
		display_.fillPoints(chi, data_[chi], nData_);
	}	
}

////////////////////////////////////////////////////////
///////////////////////// FREQ /////////////////////////
////////////////////////////////////////////////////////

Freq::Freq(sf::RenderWindow& window, int x, int y, int w, int h, int nCh, int nData, int nPoints, int sampleFreq, uint8_t* buffer) :
	display_(window, x, y, w, h, nCh, nPoints, true),
	fft_(nCh, sampleFreq, nData, buffer, nPoints),
	nCh_(nCh), nData_(nData), idxData_(0), data_(nCh, std::vector<double>(nData)), freq_(nCh) {	

	fft_.setOptimizationLevel(40);
	for (int chi = 0; chi < nCh; ++chi)
		display_.setTextPosition(chi, sf::Vector2f(x + 35 + chi * (w / 15), y - h - 20));	// Some magical position values
}

int Freq::toggleChannel(const sf::Vector2f& pos) {
	int channel = display_.textPositionToChannel(pos);
	display_.toggleChannelVisibility(channel);
	return channel;
}

void Freq::draw() {
	display_.draw();
}

void Freq::run(const uint8_t* rawData, int size) {
	for (int i = 0; i < size; ++i) {
		if (idxData_ >= nData_) {
			idxData_ = 0;
			fft_.run();
			for (int chi = 0; chi < nCh_; ++chi) {
				char cbuf[10];
				std::sprintf(cbuf, "%.2f", fft_.getFreq(chi));
				display_.setTextString(chi, cbuf);
				auto retData = fft_.getData(chi);
				display_.fillPoints(chi, retData, retData.size());
			}			
		}
		for (int chi = 0; chi < nCh_; ++chi) {
			data_[chi][idxData_] = static_cast<double>(*(rawData++));
		}
		idxData_++;

	}
}
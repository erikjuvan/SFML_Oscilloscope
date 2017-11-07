#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>

#include <SFML/Graphics.hpp>

class Oscilloscope {

	class Display {

		class Drawable {
		protected:
			sf::RenderWindow& window_;
		public:
			Drawable(sf::RenderWindow& window) : window_(window) {}
			virtual void Draw() const = 0;
		};

		class Canvas : public Drawable {
		public:
			int x_, y_;
			int width_, height_;
			sf::VertexArray axes_;

			Canvas(sf::RenderWindow& window, const sf::Color& axisColor) : Drawable(window), x_(0), y_(0), width_(0), height_(0), axes_(sf::PrimitiveType::LinesStrip, 3) {
				axes_[0].color = axes_[1].color = axes_[2].color = axisColor;
			}

			void SetDimensions(int x, int y, int width, int height) {
				x_ = x; y_ = y; width_ = width; height_ = height;
				axes_[0].position = sf::Vector2f(x, y + height);
				axes_[1].position = sf::Vector2f(x, y);
				axes_[2].position = sf::Vector2f(x + width, y);
			}

			void Draw() const {
				window_.draw(axes_);
			}
		};

		class Data : public Drawable {

			struct Channel {
				sf::VertexArray buffer_;

				Channel(int size) : buffer_(sf::PrimitiveType::LinesStrip, size) {}
			};

			int nChannels_;
			int channelSize_;
			std::vector<Channel> channel_;
			int elementi;

		public:
			Data(sf::RenderWindow& window, int nChannels, int channelSize) : Drawable(window), elementi(0), nChannels_(nChannels), channelSize_(channelSize) {
				for (int i = 0; i < nChannels; i++)
					channel_.emplace_back(channelSize);
			}

			void Draw() const {
				for (auto& c : channel_) {
					window_.draw(c.buffer_);
				}
			}

			int IdxToX(const int idx, const int width, const int MaxIdx) const {
				return std::round((double)idx * ((double)width / (double)MaxIdx));
			}

			void AdaptTo(const Canvas& cv, int chi, int first, int last, int* data) {
				if (first < last && last < channelSize_) {
					for (int i = 0; first < last; first++, i++) {
						channel_[chi].buffer_[first].position = sf::Vector2f(IdxToX(first, cv.width_, channelSize_), cv.height_ - data[i] * ((double)cv.height_ / 255.0));
					}
				}
			}

			void Run(const uint8_t* buffer, int chBufSize, const Canvas& cv) {

				for (int ci = 0; ci < nChannels_; ++ci) {
					// Setup data
					for (int i = 0; i < chBufSize; ++i) {
						channel_[ci].buffer_[i].position.x = IdxToX(first, cv.width_, channelSize_);
						channel_[ci].buffer_[i].position.y = cv.height_ - static_cast<double>(*(buffer + i * nChannels_ + ci)) * ((double)cv.height_ / 255.0);
					}
				}
			}
			
		};

		Canvas	canvas_;
		Data	data_;
		int		nChannels_;

	public:

		Display(sf::RenderWindow& window, int oscWidth, int oscHeight, int nChannels) : nChannels_(nChannels),
			canvas_(window, sf::Color::White),
			data_(window, nChannels) {

		}

		void Run(const uint8_t* buffer, int chBufSize) {

			data_.Run(buffer, chBufSize, canvas_);

		}

		void Draw() {
			canvas_.Draw();
			data_.Draw();
		}

	};

	sf::RenderWindow window_;
	Display time_;
	Display freq_;

public:

	Oscilloscope(int width, int height, std::string name, int nCh) :
		window_(sf::VideoMode(width, height), name),
		time_(window_, width, height, nCh),
		freq_(window_, width, height, nCh) {

	}	

	void Run(const uint8_t* buffer) {
		if (window_.isOpen()) {
			time_.Run(buffer);
			freq_.Run(fft.Compute(buffer));
			// Process events
			sf::Event event;
			while (window_.pollEvent(event)) {
				// Close window : exit
				if (event.type == sf::Event::Closed)
					window_.close();
			}

			// Clear screen
			window_.clear();
			time_.Draw();
			freq_.Draw();
			window_.display();
		}
	}
};

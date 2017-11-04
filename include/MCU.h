#pragma once
#include <serial/serial.h>
#include <string>
#include <iostream>

class MCU {
private:
	serial::Serial serial_;
	int numOfChannels_;
	int usPerSample_;
	int chBufSize_;

public:

	MCU(const std::string& port, int numOfChannels, int chBufSize, int usPerSample) :
		serial_(port), numOfChannels_(numOfChannels), chBufSize_(chBufSize), usPerSample_(usPerSample_) {

			serial_.flush();
			serial_.write(std::string("go"));
			serial_.write(std::to_string(numOfChannels));
			serial_.write(std::to_string(usPerSample));
	}

	~MCU() {
		if (serial_.isOpen()) {
			serial_.close();
		}
	}

	size_t ReadChunk(uint8_t* data) {
		return serial_.read(data, numOfChannels_ * chBufSize_);
	}

	int GetNumOfChannels() { return numOfChannels_; }
	int GetUsPerSample() { return usPerSample_; }
	size_t GetRxBufferLen() { return serial_.available(); }
	bool IsSerialOpen() { return serial_.isOpen(); }

};

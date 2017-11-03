#pragma once
#include <SerialLibrary.h>
#include <string>
#include <iostream>

class MCU {
private:
	SerialLibrary::Serial serial_;
	int numOfChannels_;
	int usPerSample_;
	int chBufSize_;

public:
	MCU() : numOfChannels_(0), usPerSample_(0) {}

	bool Init(const std::string& comPort, int numOfChannels, int chBufSize, int usPerSample) {
		numOfChannels_ = numOfChannels;
		chBufSize_ = chBufSize;
		usPerSample_ = usPerSample;

		std::string sNCh = std::to_string(numOfChannels);
		std::string sUsPerSample = std::to_string(usPerSample);

		serial_.Flush();
		if (serial_.Init(comPort.c_str()) == false) {
			std::cout << "Couldn't init serial port\n";
			return false;
		}
		serial_.Write((uint8_t*)"go", 2);
		serial_.Write((uint8_t*)sNCh.c_str(), sNCh.size());
		serial_.Write((uint8_t*)sUsPerSample.c_str(), sUsPerSample.size());

		return true;
	}

	int ReadChunk(uint8_t* data) {
		if (serial_.Read(data, numOfChannels_ * chBufSize_)) 
			return numOfChannels_ * chBufSize_;
		else 
			return 0;
	}

	int GetNumOfChannels() { return numOfChannels_; }
	int GetUsPerSample() { return usPerSample_; }
	int GetRxBufferLen() { return serial_.GetRxBufferLen(); }

};

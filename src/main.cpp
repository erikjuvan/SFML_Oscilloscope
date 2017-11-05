#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <complex>

#include "FFT.h"
#include "MCU.h"
#include "osc.h"

void InitParameters(std::string& sComPort, int& nCh, int& usPerSample, int& chBufSize) {
	std::string sNCh, sUsPerSample, sChBufSize;

	std::ifstream initFile("../res/init.txt");
	if (initFile.is_open()) {
		std::cout << "Found \"init.txt\" Press enter to load parameters from file.";		
		std::getchar();
#if defined(_WIN32)
		system("cls");
#elif defined(__linux__)
		system("clear");
#endif

		initFile >> sComPort;
		initFile >> sNCh;
		initFile >> sUsPerSample;
		initFile >> sChBufSize;
		nCh = std::stoi(sNCh);
		usPerSample = std::stoi(sUsPerSample);
		chBufSize = std::stoi(sChBufSize);

		std::cout << "COM Port: ";
		std::cout << sComPort << std::endl;
		std::cout << "Number of channels: ";
		std::cout << sNCh << std::endl;
		std::cout << "Us per sample: ";
		std::cout << sUsPerSample << std::endl;
		std::cout << "Single channel buffer size (power of 2, e.g. 1024): ";
		std::cout << sChBufSize << std::endl;
	}
	else {
		std::cout << "COM Port: ";
		std::cin >> sComPort;
		std::cout << "Number of channels: ";
		std::cin >> sNCh;
		nCh = std::stoi(sNCh);
		std::cout << "Us per sample: ";
		std::cin >> sUsPerSample;
		usPerSample = std::stoi(sUsPerSample);
		std::cout << "Single channel buffer size (power of 2, e.g. 1024): ";
		std::cin >> sChBufSize;
		chBufSize = std::stoi(sChBufSize);
	}
}


int main() {			
	
	std::string sComPort;
	int nCh, usPerSample, chBufSize;
	InitParameters(sComPort, nCh, usPerSample, chBufSize);

	uint8_t* buffer = nullptr;
	buffer = new uint8_t[chBufSize * nCh];
	if (buffer == nullptr) {
		std::cout << "Couldn't allocate memory. Request of " << chBufSize * nCh << " bytes is too big" << std::endl;
	}

	FFT fft(nCh, chBufSize, usPerSample);
	MCU *mcu;
	try {
		mcu = new MCU(sComPort, nCh, chBufSize, usPerSample);
	}	
	catch (const serial::IOException& e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}
	catch (const std::bad_alloc& e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}

	if (!mcu->IsSerialOpen()) {
		return -1;
	}

	const int MaxNumOfChannels = 10;
	double freq[MaxNumOfChannels] = { 0 };
	double ampl[MaxNumOfChannels] = { 0 };
	int cycles[MaxNumOfChannels] = { 0 };
	
	while (true) {
		if (mcu->ReadChunk((uint8_t*)buffer) > 0) {			
			fft.Compute(buffer, freq, ampl, cycles);
		}		
	}

	if (buffer != nullptr) {
		delete[] buffer;
	}
	if (mcu != nullptr) {
		delete mcu;
	}

	return 0;
}

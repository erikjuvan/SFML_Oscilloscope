#include "parameters.h"
#include <iostream>
#include <fstream>

void Parameters::clearScreen() {
#if defined(_WIN32)
	system("cls");
#elif defined(__linux__)
	system("clear");
#endif
}

Parameters::Parameters() : nCh(0), packetsPerChannel(0), sampleFreq(0), chBufSize(0), margin(0), w(0), h(0), freqPoints(0) {
	set();
}

void Parameters::set() {
	bool succeed = false;
	std::ifstream initFile("../res/init.txt");

	if (initFile.is_open()) {
		std::cout << "Found \"init.txt\" Press enter to load parameters from file.";
		std::getchar();
		clearScreen();

		initFile >> comPort;
		initFile >> sNCh;
		initFile >> sPacketsPerChannel;
		initFile >> sSampleFreq;
		initFile >> sChBufSize;
		initFile >> sMargin;
		initFile >> sW;
		initFile >> sH;
		initFile >> sFreqPoints;

		if (sChBufSize.size() != 0) {
			nCh = std::stoi(sNCh);
			packetsPerChannel = std::stoi(sPacketsPerChannel);
			sampleFreq = std::stoi(sSampleFreq);
			chBufSize = std::stoi(sChBufSize);
			margin = std::stoi(sMargin);
			w = std::stoi(sW);
			h = std::stoi(sH);
			freqPoints = std::stoi(sFreqPoints);
			succeed = true;
		}
		else {
			std::cerr << "Error parsing init file" << std::endl;
		}
	}

	if (!succeed) {
		// Get parameters via command line
		std::cout << "COM Port: "; std::cin >> comPort;
		std::cout << "Number of channels: "; std::cin >> sNCh; nCh = std::stoi(sNCh);
		std::cout << "Packets per channel: "; std::cin >> sPacketsPerChannel; packetsPerChannel = std::stoi(sPacketsPerChannel);
		std::cout << "Sample frequency: "; std::cin >> sSampleFreq; sampleFreq = std::stoi(sSampleFreq);
		std::cout << "Channel buffer size: "; std::cin >> sChBufSize; chBufSize = std::stoi(sChBufSize);
		std::cout << "Margin: "; std::cin >> sMargin; margin = std::stoi(sMargin);
		std::cout << "Width: "; std::cin >> sW; w = std::stoi(sW);
		std::cout << "Height: "; std::cin >> sH; h = std::stoi(sH);
		std::cout << "FFT chart points: "; std::cin >> sFreqPoints; freqPoints = std::stoi(sFreqPoints);
	}
}

void Parameters::display() {
	clearScreen();
	std::cout << "COM Port: " << comPort << std::endl;
	std::cout << "Number of channels: " << sNCh << std::endl;
	std::cout << "Packets per channel: " << sPacketsPerChannel << std::endl;
	std::cout << "Sample frequency: " << sSampleFreq << " Hz" << std::endl;
	std::cout << "Channel buffer size: " << sChBufSize << std::endl;
	std::cout << "Margin: " << sMargin << std::endl;
	std::cout << "Width: " << sW << std::endl;
	std::cout << "Height: " << sH << std::endl;
	std::cout << "FFT chart points: " << sFreqPoints << std::endl;
}


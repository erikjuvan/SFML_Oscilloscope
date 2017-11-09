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

void initParameters(std::string& sComPort, int& nCh, int& usPerSample, int& chBufSize,
	int& margin, int& w, int& h, int& freqPoints) {

	bool fileParse = false;
	std::string sNCh, sUsPerSample, sChBufSize, sMargin, sw, sh, sFreqPoints;

	std::ifstream initFile("../res/init.txt");
	if (initFile.is_open()) {
		std::cout << "Found \"init.txt\" Press enter to load parameters from file.";		
		std::getchar();

		initFile >> sComPort;
		initFile >> sNCh;
		initFile >> sUsPerSample;
		initFile >> sChBufSize;
		initFile >> sMargin;
		initFile >> sw;
		initFile >> sh;		
		initFile >> sFreqPoints;

		if (sFreqPoints.size() == 0) {
			std::cerr << "Error parsing init file" << std::endl;
		} else {
			nCh = std::stoi(sNCh);
			usPerSample = std::stoi(sUsPerSample);
			chBufSize = std::stoi(sChBufSize);
			margin = std::stoi(sMargin);
			w = std::stoi(sw);
			h = std::stoi(sh);			
			freqPoints = std::stoi(sFreqPoints);

			std::cout << "COM Port: " << sComPort << std::endl;
			std::cout << "Number of channels: " << sNCh << std::endl;
			std::cout << "Us per sample: " << sUsPerSample << std::endl;
			std::cout << "Single channel buffer size: " << sChBufSize << std::endl;
			std::cout << "Margin: " << sMargin << std::endl;
			std::cout << "Width: " << sw << std::endl;
			std::cout << "Height: " << sh << std::endl;			
			std::cout << "FFT chart points: " << sFreqPoints << std::endl;

			fileParse = true;
		}		
	}

	if (!fileParse){
		std::cout << "COM Port: "; std::cin >> sComPort;
		std::cout << "Number of channels: "; std::cin >> sNCh; nCh = std::stoi(sNCh);
		std::cout << "Us per sample: "; std::cin >> sUsPerSample; usPerSample = std::stoi(sUsPerSample);
		std::cout << "Single channel buffer size: "; std::cin >> sChBufSize; chBufSize = std::stoi(sChBufSize);
		std::cout << "Margin: "; std::cin >> sMargin; margin = std::stoi(sMargin);
		std::cout << "Width: "; std::cin >> sw; w = std::stoi(sw);
		std::cout << "Height: "; std::cin >> sh; h = std::stoi(sh);
		std::cout << "FFT chart points: "; std::cin >> sFreqPoints; freqPoints = std::stoi(sFreqPoints);
	}
}


int main() {			
		
	std::string sComPort;
	int nCh, usPerSample, chBufSize;
	int w, h, margin;
	int freqPoints;
	initParameters(sComPort, nCh, usPerSample, chBufSize, margin, w, h, freqPoints);

	int freqData = 1e6 / usPerSample;
	int time_x = margin, time_y = h - margin, time_w = w - 2 * margin, time_h = (h - 3 * margin) * 2 / 3;	
	int freq_x = margin, freq_y = h - (time_h + 2 * margin), freq_w = w - 2 * margin, freq_h = h - time_h - 3 * margin;

	sf::RenderWindow window(sf::VideoMode(w, h), "Oscy");

	Time time(window, time_x, time_y, time_w, time_h, nCh, chBufSize /* data */, time_w /* points */);
	Freq freq(window, freq_x, freq_y, freq_w, freq_h, nCh, freqData /* data */, freqPoints /* points */, usPerSample);

	MCU mcu(sComPort, nCh, chBufSize, usPerSample);
	FFT fft(nCh, freqData, usPerSample);

	uint8_t* buffer = new uint8_t[chBufSize * nCh];
	if (buffer == nullptr) std::cerr << "Couldn't allocate buffer memory\n" << std::endl;

	while (window.isOpen()) {
		mcu.readChunk(buffer);		
		time.run(buffer, chBufSize);		
		freq.run(buffer, chBufSize);				
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear();
		time.draw();
		freq.draw();
		window.display();
	}

	if (buffer != nullptr) {
		delete[] buffer;
	}

	return 0;
}
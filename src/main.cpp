#include <Windows.h>

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <complex>

#include <SerialLibrary.h>

#include <fftw3.h>

#include "FFT.h"
#include "MCU.h"


void InitParameters(std::string& sComPort, int& nCh, int& usPerSample, int& chBufSize) {
	std::string sNCh, sUsPerSample, sChBufSize;

	std::ifstream initFile("../res/init.txt");
	if (initFile.is_open()) {
		std::cout << "Found \"init.txt\" Press enter to load parameters from file.";		
		std::getchar();
		system("cls");

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
	HANDLE hStdout;
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	CONSOLE_CURSOR_INFO     cursorInfo;

	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdout == INVALID_HANDLE_VALUE) {
		MessageBox(NULL, TEXT("GetStdHandle"), TEXT("Console Error"), MB_OK);
		return 1;
	}
	if (!GetConsoleScreenBufferInfo(hStdout, &csbiInfo))
	{
		MessageBox(NULL, TEXT("GetConsoleScreenBufferInfo"),
			TEXT("Console Error"), MB_OK);
		return -1;
	}	

	std::string sComPort;
	int nCh, usPerSample, chBufSize;
	InitParameters(sComPort, nCh, usPerSample, chBufSize);

	GetConsoleCursorInfo(hStdout, &cursorInfo);
	cursorInfo.bVisible = false;
	SetConsoleCursorInfo(hStdout, &cursorInfo);
	csbiInfo.dwCursorPosition.X = 0;
	csbiInfo.dwCursorPosition.Y = 5;

	uint8_t* buffer = nullptr;
	buffer = new uint8_t[chBufSize * nCh];
	if (buffer == nullptr) {
		std::cout << "Couldn't allocate memory. Request of " << chBufSize * nCh << " bytes is too big" << std::endl;
	}

	FFT fft(nCh, chBufSize, usPerSample);
	MCU mcu;
	if (mcu.Init(sComPort, nCh, chBufSize, usPerSample) == false) {
		return -1;
	}	

	const int MaxNumOfChannels = 10;
	double freq[MaxNumOfChannels] = { 0 };
	double ampl[MaxNumOfChannels] = { 0 };
	int cycles[MaxNumOfChannels] = { 0 };
	
	while (true) {
		if (mcu.ReadChunk((uint8_t*)buffer) > 0) {
			SetConsoleCursorPosition(hStdout, csbiInfo.dwCursorPosition);
			fft.Compute(buffer, freq, ampl, cycles);

			for (int i = 0; i < nCh; i++) {
				std::cout << "Ch " << i << "\tcycles: " << std::setw(4) << cycles[i] << "\tfreq: " << std::setw(4) << freq[i] << "\tamp: " << std::setw(4) << ampl[i] << std::endl;
			}
		}		
	}

	if (buffer != nullptr) {
		delete[] buffer;
	}

	return 0;
}

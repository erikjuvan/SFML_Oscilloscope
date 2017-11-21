#pragma once
#include <string>
#include <cstdlib>

class Parameters {
private:
	std::string sNCh, sPacketsPerChannel, sSampleFreq, sChBufSize;
	std::string sMargin, sW, sH, sFreqPoints;

	void clearScreen();

public:
	std::string comPort;
	int nCh, packetsPerChannel, sampleFreq, chBufSize;
	int margin, w, h, freqPoints;

	Parameters();
	void set();
	void display();
};

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <complex>

#include "fft.h"
#include "mcu.h"
#include "osc.h"
#include "parameters.h"


int main() {			
	Parameters params;
	params.display();	

	int freqData = 1e6 / params.sampleFreq;
	int time_x = margin, time_y = h - margin, time_w = w - 2 * margin, time_h = (h - 3 * margin) * 2 / 3;	
	int freq_x = margin, freq_y = h - (time_h + 2 * margin), freq_w = w - 2 * margin, freq_h = h - time_h - 3 * margin;

	sf::RenderWindow window(sf::VideoMode(w, h), "Oscy");

	Time time(window, time_x, time_y, time_w, time_h, nCh, chBufSize /* data */, time_w /* points */);
	Freq freq(window, freq_x, freq_y, freq_w, freq_h, nCh, freqData /* data */, freqPoints /* points */, usPerSample);

	MCU mcu(sComPort, nCh, chBufSize, usPerSample);
	FFT fft(nCh, freqData, usPerSample);

	uint8_t* buffer = new uint8_t[chBufSize * nCh];

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
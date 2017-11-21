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

	uint8_t* buffer = new uint8_t[params.chBufSize * params.nCh];

	int freqData = params.sampleFreq;
	int time_x = params.margin, time_y = params.h - params.margin, time_w = params.w - 2 * params.margin, time_h = (params.h - 3 * params.margin) * 2 / 3;
	int freq_x = params.margin, freq_y = params.h - (time_h + 2 * params.margin), freq_w = params.w - 2 * params.margin, freq_h = params.h - time_h - 3 * params.margin;

	sf::RenderWindow window(sf::VideoMode(params.w, params.h), "Oscy");
	
	Time time(window, time_x, time_y, time_w, time_h, params.nCh, params.chBufSize /* data */, time_w /* points */);
	Freq freq(window, freq_x, freq_y, freq_w, freq_h, params.nCh, freqData /* data */, params.freqPoints /* points */, params.sampleFreq, buffer);
	freq.run(buffer, params.chBufSize);
	MCU mcu(params.comPort, params.nCh, params.packetsPerChannel, params.sampleFreq, params.chBufSize);

	while (window.isOpen()) {
		if (mcu.readChunk(buffer) > 0) {
			time.run(buffer, params.chBufSize);
			freq.run(buffer, params.chBufSize);
		}		

		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
			else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
				time.toggleChannel(freq.toggleChannel(sf::Vector2f(event.mouseButton.x, event.mouseButton.y)));
			}
		}

		window.clear();
		time.draw();
		freq.draw();
		window.display();
	}

	delete[] buffer;

	return 0;
}
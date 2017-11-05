#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <complex>

#include <SFML/Graphics.hpp>

void Osc() {


	std::string str;
	bool triggerUsed = false;
	bool saveDataToFile = false;

	std::cout << "COM port (e.g. \"COM6\"): ";
	std::cin >> str;

#ifdef TEST
	SerialEmulated serial;
#else
	serial::Serial serial(str);
#endif


	std::cout << "Set sample frequency in Hz: ";
	std::cin >> str;
	int sampleSpeed = std::stoi(str);
	if (str.size() > 1) {
		str = "CSETF," + str;
		serial.flush();
		serial.write(str);
	}

	str = "N";
	std::cout << "Use trigger[y/N]: ";
	std::getline(std::cin, str);
	if (str == "Y" || str == "y") {
		triggerUsed = true;
	}

	std::getline(std::cin, str);
	str = "N";
	std::cout << "Save data to file[y/N]: ";
	std::getline(std::cin, str);
	if (str == "Y" || str == "y") {
		saveDataToFile = true;
	}

	sf::RenderWindow window(sf::VideoMode(WindowWidth, WindowHeight), "Osc", sf::Style::Close);
	sf::Event event;

	//window.setVerticalSyncEnabled(true);
	//window.setFramerateLimit(80);						

	sf::VertexArray chartTime(sf::PrimitiveType::LinesStrip, 3);
	chartTime[0].color = chartTime[1].color = chartTime[2].color = sf::Color::White;
	chartTime[0].position.x = WidthSpacing;
	chartTime[0].position.y = HeightSpacing;
	chartTime[1].position.x = WidthSpacing;
	chartTime[1].position.y = HeightSpacing + TimeGraphHeight;
	chartTime[2].position.x = TimeGraphWidth + WidthSpacing;
	chartTime[2].position.y = HeightSpacing + TimeGraphHeight;


	sf::VertexArray chartFreq(sf::PrimitiveType::LinesStrip, 3);
	chartFreq[0].color = chartFreq[1].color = chartFreq[2].color = chartTime[0].color;
	chartFreq[0].position.x = WidthSpacing;
	chartFreq[0].position.y = 2 * HeightSpacing + TimeGraphHeight;
	chartFreq[1].position.x = WidthSpacing;
	chartFreq[1].position.y = 2 * HeightSpacing + TimeGraphHeight + FreqGraphHeight;
	chartFreq[2].position.x = FreqGraphWidth + WidthSpacing;
	chartFreq[2].position.y = 2 * HeightSpacing + TimeGraphHeight + FreqGraphHeight;

	sf::VertexArray dataCurrent(sf::PrimitiveType::LinesStrip, HorizontalPixels);
	for (int i = 0; i < HorizontalPixels; i++) {
		dataCurrent[i].position.x = i + WidthSpacing;
		dataCurrent[i].position.y = TimeGraphOriginY;
		dataCurrent[i].color = sf::Color::White;
	}

	sf::VertexArray dataX(sf::PrimitiveType::LinesStrip, HorizontalPixels);
	for (int i = 0; i < HorizontalPixels; i++) {
		dataX[i].position.x = i + WidthSpacing;
		dataX[i].position.y = TimeGraphOriginY;
		dataX[i].color = sf::Color::Red;
	}

	sf::VertexArray dataY(sf::PrimitiveType::LinesStrip, HorizontalPixels);
	for (int i = 0; i < HorizontalPixels; i++) {
		dataY[i].position.x = i + WidthSpacing;
		dataY[i].position.y = TimeGraphOriginY;
		dataY[i].color = sf::Color::Green;
	}

	sf::VertexArray dataZ(sf::PrimitiveType::LinesStrip, HorizontalPixels);
	for (int i = 0; i < HorizontalPixels; i++) {
		dataZ[i].position.x = i + WidthSpacing;
		dataZ[i].position.y = TimeGraphOriginY;
		dataZ[i].color = sf::Color::Blue;
	}

	// FFT graph data
	/////////////////
	sf::VertexArray fftGraphDataCurrent(sf::PrimitiveType::LinesStrip, HorizontalPixels);
	for (int i = 0; i < HorizontalPixels; i++) {
		fftGraphDataCurrent[i].position.x = i + WidthSpacing;
		fftGraphDataCurrent[i].position.y = FreqGraphOriginY;
		fftGraphDataCurrent[i].color = sf::Color::White;
	}

	sf::VertexArray fftGraphDataX(sf::PrimitiveType::LinesStrip, HorizontalPixels);
	for (int i = 0; i < HorizontalPixels; i++) {
		fftGraphDataX[i].position.x = i + WidthSpacing;
		fftGraphDataX[i].position.y = FreqGraphOriginY;
		fftGraphDataX[i].color = sf::Color::Red;
	}

	sf::VertexArray fftGraphDataY(sf::PrimitiveType::LinesStrip, HorizontalPixels);
	for (int i = 0; i < HorizontalPixels; i++) {
		fftGraphDataY[i].position.x = i + WidthSpacing;
		fftGraphDataY[i].position.y = FreqGraphOriginY;
		fftGraphDataY[i].color = sf::Color::Green;
	}

	sf::VertexArray fftGraphDataZ(sf::PrimitiveType::LinesStrip, HorizontalPixels);
	for (int i = 0; i < HorizontalPixels; i++) {
		fftGraphDataZ[i].position.x = i + WidthSpacing;
		fftGraphDataZ[i].position.y = FreqGraphOriginY;
		fftGraphDataZ[i].color = sf::Color::Blue;
	}
	/////////////////

	// FFT data
	///////////
	double fftDataCurrent[2 * HorizontalPixels] = { 0 };
	double fftDataX[2 * HorizontalPixels] = { 0 };
	double fftDataY[2 * HorizontalPixels] = { 0 };
	double fftDataZ[2 * HorizontalPixels] = { 0 };
	///////////

	sf::Font font;
	if (!font.loadFromFile("../res/arial.ttf")) {
		std::cout << "Error in font.loadFromFile" << std::endl;
	}

	sf::Text fpsText;
	fpsText.setFont(font);
	fpsText.setCharacterSize(20);
	fpsText.setFillColor(sf::Color::White);
	fpsText.setStyle(sf::Text::Bold);
	fpsText.setPosition(WindowWidth - 200, 0);

	sf::Text freqText[4];
	for (int i = 0; i < 4; i++) {
		freqText[i].setFont(font);
		freqText[i].setCharacterSize(20);
		freqText[i].setStyle(sf::Text::Bold);
		freqText[i].setPosition(WindowWidth - 40, FreqGraphOriginY - FreqGraphHeight + 30 * i);
		freqText[i].setString("");
	}
	freqText[0].setFillColor(sf::Color::White);
	freqText[1].setFillColor(sf::Color::Red);
	freqText[2].setFillColor(sf::Color::Green);
	freqText[3].setFillColor(sf::Color::Blue);

	bool trigger = false;
	int triggerCnt = 0;
	int triggerVal = 4096 / 2;

	sf::VertexArray triggerLine(sf::PrimitiveType::LinesStrip, 2);
	triggerLine[0].position.x = WindowWidth - 47;
	triggerLine[0].position.y = TimeScale(triggerVal);
	triggerLine[0].color = triggerLine[1].color = sf::Color::Cyan;
	triggerLine[1].position.x = WindowWidth;
	triggerLine[1].position.y = triggerLine[0].position.y;

	std::vector<int> dataStorage[4];

	int fftCalculateCnt = 0;
	const int FFT_calculate = 40;

	while (window.isOpen()) {

		while (window.pollEvent(event)) {
			if (event.type == sf::Event::KeyPressed) {
				const int TriggerIncrement = 10;
				if (event.key.code == sf::Keyboard::Up) {
					if (triggerVal <= (4096 - TriggerIncrement)) {
						triggerVal += TriggerIncrement;
						triggerLine[0].position.y = TimeScale(triggerVal);
						triggerLine[1].position.y = TimeScale(triggerVal);
					}
				}
				else if (event.key.code == sf::Keyboard::Down) {
					if (triggerVal >= TriggerIncrement) {
						triggerVal -= TriggerIncrement;
						triggerLine[0].position.y = TimeScale(triggerVal);
						triggerLine[1].position.y = TimeScale(triggerVal);
					}
				}
#ifdef TEST
				else if (event.key.code == sf::Keyboard::U) {
					serial.freq1 += 0.1;
				}
				else if (event.key.code == sf::Keyboard::J) {
					serial.freq1 -= 0.1;
				}
#endif
			}
			else if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		fpsText.setString("rxbuf: " + std::to_string(serial.available()));

		unsigned char rcvData[SizeOfBuffer];
		if (serial.read(rcvData, sizeof(rcvData))) {
			int newval[SizeOfBuffer / 2];
			int newvalScaled[SizeOfBuffer / 2];

			for (int i = 0; i < SizeOfBuffer / 2; i++) {
				newval[i] = rcvData[2 * i] + (rcvData[2 * i + 1] << 8);
				newvalScaled[i] = TimeScale(newval[i]);

				if (triggerUsed) {
					if (newval[i] < (triggerVal + 10) && newval[i] > (triggerVal - 10)) {
						trigger = true;
						triggerCnt = (HorizontalPixels / DataPerChannel) / 2;
					}
				}
			}

			if (!triggerUsed || (trigger && (triggerCnt-- > 0))) {
				for (int i = 0; i < (HorizontalPixels - DataPerChannel); i++) {
					dataCurrent[i].position.y = dataCurrent[i + DataPerChannel].position.y;
					dataX[i].position.y = dataX[i + DataPerChannel].position.y;
					dataY[i].position.y = dataY[i + DataPerChannel].position.y;
					dataZ[i].position.y = dataZ[i + DataPerChannel].position.y;

					fftDataCurrent[2 * i] = fftDataCurrent[2 * (i + DataPerChannel)];
					fftDataX[2 * i] = fftDataX[2 * (i + DataPerChannel)];
					fftDataY[2 * i] = fftDataY[2 * (i + DataPerChannel)];
					fftDataZ[2 * i] = fftDataZ[2 * (i + DataPerChannel)];
				}

				for (int i = HorizontalPixels - DataPerChannel, j = 0; i < HorizontalPixels; i++, j += NumOfChannels) {
					dataCurrent[i].position.y = newvalScaled[j];
					dataX[i].position.y = newvalScaled[j + 1];
					dataY[i].position.y = newvalScaled[j + 2];
					dataZ[i].position.y = newvalScaled[j + 3];

					fftDataCurrent[2 * i] = static_cast<double>(newval[j]);
					fftDataX[2 * i] = static_cast<double>(newval[j + 1]);
					fftDataY[2 * i] = static_cast<double>(newval[j + 2]);
					fftDataZ[2 * i] = static_cast<double>(newval[j + 3]);

					if (saveDataToFile) {
						dataStorage[0].push_back(newval[j]);
						dataStorage[1].push_back(newval[j + 1]);
						dataStorage[2].push_back(newval[j + 2]);
						dataStorage[3].push_back(newval[j + 3]);
					}
				}


				if (fftCalculateCnt++ > FFT_calculate) {
					fftCalculateCnt = 0;

					double dTmp[2 * HorizontalPixels];
					bool set = false;
					const int starti = 1;
					const int divisionFactor = 1024 * 200;
					const int threashold = 20000;
					const int HorizontalDivision = 8;
					const int HorizontalFFTPoints = HorizontalPixels / HorizontalDivision;
					double dVals[HorizontalFFTPoints];
					int local_max = 0;
					int local_i = 0;

					std::memcpy(dTmp, fftDataCurrent, 2 * HorizontalPixels * sizeof(double));
					FT::fft1(dTmp, HorizontalPixels);
					FT::abs(dTmp, HorizontalFFTPoints, dVals);
					freqText[0].setString("/");
					local_max = 0;
					local_i = 0;
					for (int i = starti; i < HorizontalFFTPoints; i++) {
						if (i != 0 && dVals[i] > local_max && !set) {
							local_max = dVals[i];
							local_i = i;
						}
						fftGraphDataCurrent[HorizontalDivision * i].position.y = FreqScale(dVals[i], divisionFactor);
						for (int j = 1; j < HorizontalDivision; j++) {
							fftGraphDataCurrent[HorizontalDivision * i + j].position.y = FreqGraphOriginY;
						}
					}
					freqText[0].setString(std::to_string(local_i * (sampleSpeed / 1000)));

					std::memcpy(dTmp, fftDataX, 2 * HorizontalPixels * sizeof(double));
					FT::fft1(dTmp, HorizontalPixels);
					FT::abs(dTmp, HorizontalFFTPoints, dVals);
					set = false;
					freqText[1].setString("/");
					local_max = 0;
					local_i = 0;
					for (int i = starti; i < HorizontalFFTPoints; i++) {
						if (i != 0 && dVals[i] > threashold && !set) {
							local_max = dVals[i];
							local_i = i;
						}
						fftGraphDataX[HorizontalDivision * i].position.y = FreqScale(dVals[i], divisionFactor);
						for (int j = 1; j < HorizontalDivision; j++) {
							fftGraphDataX[HorizontalDivision * i + j].position.y = FreqGraphOriginY;
						}
					}
					freqText[1].setString(std::to_string(local_i * (sampleSpeed / 1000)));

					std::memcpy(dTmp, fftDataY, 2 * HorizontalPixels * sizeof(double));
					FT::fft1(dTmp, HorizontalPixels);
					FT::abs(dTmp, HorizontalFFTPoints, dVals);
					set = false;
					freqText[2].setString("/");
					local_max = 0;
					local_i = 0;
					for (int i = starti; i < HorizontalFFTPoints; i++) {
						if (i != 0 && dVals[i] > threashold && !set) {
							local_max = dVals[i];
							local_i = i;
						}
						fftGraphDataY[HorizontalDivision * i].position.y = FreqScale(dVals[i], divisionFactor);
						for (int j = 1; j < HorizontalDivision; j++) {
							fftGraphDataY[HorizontalDivision * i + j].position.y = FreqGraphOriginY;
						}
					}
					freqText[2].setString(std::to_string(local_i * (sampleSpeed / 1000)));

					std::memcpy(dTmp, fftDataZ, 2 * HorizontalPixels * sizeof(double));
					FT::fft1(dTmp, HorizontalPixels);
					FT::abs(dTmp, HorizontalFFTPoints, dVals);
					set = false;
					freqText[3].setString("/");
					local_max = 0;
					local_i = 0;
					for (int i = starti; i < HorizontalFFTPoints; i++) {
						if (i != 0 && dVals[i] > threashold && !set) {
							local_max = dVals[i];
							local_i = i;
						}
						fftGraphDataZ[HorizontalDivision * i].position.y = FreqScale(dVals[i], divisionFactor);
						for (int j = 1; j < HorizontalDivision; j++) {
							fftGraphDataZ[HorizontalDivision * i + j].position.y = FreqGraphOriginY;
						}
					}
					freqText[3].setString(std::to_string(local_i * (sampleSpeed / 1000)));
				}

			}
			else {
				trigger = false;
			}
		}

		window.clear();

		window.draw(chartTime);
		window.draw(chartFreq);
		window.draw(dataCurrent);
		window.draw(dataX);
		window.draw(dataY);
		window.draw(dataZ);
		//FFT
		window.draw(fftGraphDataCurrent);
		window.draw(fftGraphDataX);
		window.draw(fftGraphDataY);
		window.draw(fftGraphDataZ);

		window.draw(fpsText);
		window.draw(freqText[0]);
		window.draw(freqText[1]);
		window.draw(freqText[2]);
		window.draw(freqText[3]);
		if (triggerUsed)
			window.draw(triggerLine);

		window.display();
	}

	if (saveDataToFile) {
		std::ofstream file("data.csv", std::ios::out);
		if (!file.is_open()) {
			std::cout << "Error opening file data.csv for writing" << std::endl;
			return -1;
		}

		int tmp = dataStorage[0].size();
		for (int i = 0; i < NumOfChannels; i++) {
			for (int j = 0; j < tmp; j++) {
				file << dataStorage[i][j];
				if (j < tmp - 1) {
					file << ",";
				}
			}
			file << '\n';
		}

		file.close();
	}
}
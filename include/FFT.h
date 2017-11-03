#pragma once

#include <vector>
#include <complex>
#include <fftw3.h>
#include <iostream>

class FFT {
private:
	struct FFTStruct {
		std::complex<double>* dataIn;
		std::complex<double>* dataOut;
		fftw_plan fftPlan;

		FFTStruct(int bufSize) {
			dataIn = new std::complex<double>[bufSize];
			dataOut = new std::complex<double>[bufSize];
			fftPlan = fftw_plan_dft_1d(bufSize, reinterpret_cast<fftw_complex*>(dataIn),
				reinterpret_cast<fftw_complex*>(dataOut), FFTW_DHT, FFTW_MEASURE);
		}

		~FFTStruct() {
			if (dataIn != nullptr) delete[] dataIn;
			if (dataOut != nullptr) delete[] dataOut;
			fftw_destroy_plan(fftPlan);
		}
	};

	int numOfChannels_;
	int chBufSize_;
	int usPerSample_;

	std::vector<FFTStruct> fftChannels;

public:

	FFT(int numOfCh, int bufSize, int usPerSample) : numOfChannels_(numOfCh), chBufSize_(bufSize), usPerSample_(usPerSample) {
		fftChannels.reserve(numOfCh);
		for (int i = 0; i < numOfCh; i++)
			fftChannels.emplace_back(bufSize);
	}

	~FFT() { }

	void Compute(const uint8_t* buf, double* freq, double* ampl, int* num_cycles) {
		for (int ci = 0; ci < numOfChannels_; ++ci) {
			// Setup data
			for (int i = 0; i < chBufSize_; ++i) {
				int idx = i * numOfChannels_ + ci;
				std::complex<double> buf_in = { static_cast<double>(*(buf + i * numOfChannels_ + ci)), 0.0 };
				fftChannels[ci].dataIn[i] = buf_in;
			}

			// Run FFT
			fftw_execute(fftChannels[ci].fftPlan);

			// Find index with maximum value
			int idx = 0;
			double tmpVal = 0.0;
			for (int i = 1; i < chBufSize_ / 2; i++) {
				double absVal = std::abs(*(fftChannels[ci].dataOut + i));
				if (tmpVal < absVal) {
					tmpVal = absVal;
					idx = i;
				}
			}

			double tmp_cycle = idx;

			freq[ci] = tmp_cycle / ((double)chBufSize_ * (1e-6 * double(usPerSample_)));
			ampl[ci] = tmpVal * 2 / chBufSize_;
			if (num_cycles != nullptr) {
				num_cycles[ci] = static_cast<int>(std::round(tmp_cycle));
			}
		}
	}
};

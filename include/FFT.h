#pragma once

#include <vector>
#include <complex>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <chrono>

#include <fftw3.h>

class FFT {
private:
	struct FFTStruct {
		double* dataIn;
		std::complex<double>* dataOut;
		fftw_plan fftPlan;

		FFTStruct(int bufSize) {
			dataIn = new double[bufSize]();
			dataOut = new std::complex<double>[bufSize]();
			fftPlan = fftw_plan_dft_r2c_1d(bufSize, dataIn, reinterpret_cast<fftw_complex*>(dataOut), FFTW_MEASURE);
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

	std::vector<FFTStruct> fftChannels_;

	void Optimize(FFTStruct& fftCh, double& maxVal, double& idx, int& size) {
		size = chBufSize_;
		int tmpSize = size;
		int target = size - 50;
		int i = 0;

		while (--tmpSize > target) {
			fftCh.fftPlan = fftw_plan_dft_r2c_1d(tmpSize, fftCh.dataIn, reinterpret_cast<fftw_complex*>(fftCh.dataOut), FFTW_MEASURE);
			// Run FFT
			fftw_execute(fftCh.fftPlan);
			// Find max element - optimized
			std::complex<double>* newMaxVal = std::max_element(fftCh.dataOut + 1, fftCh.dataOut + tmpSize / 2,	// discard 0 index (DC offset) and search only the first half
				[](std::complex<double> const & lhs, std::complex<double> const & rhs) { return std::abs(lhs) < std::abs(rhs); });

			if (std::abs(*newMaxVal) > maxVal) {
				maxVal = std::abs(*newMaxVal);
				idx = std::distance(fftCh.dataOut, newMaxVal);
				size = tmpSize;
			}
		}
	}

public:

	FFT(int numOfCh, int bufSize, int usPerSample) : numOfChannels_(numOfCh), chBufSize_(bufSize), usPerSample_(usPerSample) {
		fftChannels_.reserve(numOfCh);
		for (int i = 0; i < numOfCh; i++)
			fftChannels_.emplace_back(bufSize);
	}

	~FFT() { }

	void Run(const std::vector<std::vector<double>>& dataIn, std::vector<std::vector<double>>& dataOut) {

		for (int ci = 0; ci < numOfChannels_; ++ci) {

			// Setup data
			for (int i = 0; i < chBufSize_; ++i) {
				fftChannels_[ci].dataIn[i] = dataIn[ci][i];
			}

			// Run FFT
			fftw_execute(fftChannels_[ci].fftPlan);

			// Copy to output buffer
			//freq[ci] = idx / ((double)fftSize * (1e-6 * double(usPerSample_)));
			int size = dataOut[0].size();
			dataOut[ci][0] = std::abs(fftChannels_[ci].dataOut[0]) / chBufSize_; // offset doesn't multiply by * 2
			for (int i = 1; i < size; ++i) {
				dataOut[ci][i] = std::abs(fftChannels_[ci].dataOut[i]) * 4 / chBufSize_;
			}
		}
	}
};

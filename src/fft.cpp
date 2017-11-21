#include <vector>
#include <complex>
#include <fftw3.h>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <thread>
#include <mutex>

#include "fft.h"

FFT::FFTStruct::FFTStruct(int bufSize, int realDataSize) : freq_(0), ampl_(0), num_cycles_(0), realDataOut(realDataSize) {
	dataIn = new double[bufSize]();
	complexDataOut = new std::complex<double>[bufSize]();
	fftPlan.push_back(fftw_plan_dft_r2c_1d(bufSize, dataIn, reinterpret_cast<fftw_complex*>(complexDataOut), FFTW_MEASURE));
}

FFT::FFTStruct::~FFTStruct() {
	delete[] dataIn;
	delete[] complexDataOut;
	for (auto plan : fftPlan)
		fftw_destroy_plan(plan);
}

FFT::FFT(int numOfCh, int sampleFreq, int bufSize, const uint8_t* buf, int realDataSize) :
	numOfChannels_(numOfCh), sampleFreq_(sampleFreq), chBufSize_(bufSize), realDataSize_(realDataSize),
	buf_(buf), thread_(numOfCh) {

	fftChannels_.reserve(numOfCh);
	for (int i = 0; i < numOfCh; i++) {
		fftChannels_.emplace_back(bufSize, realDataSize_);
	}
}

void FFT::setOptimizationLevel(int optimizationLevel) {
	// Clear all plans but the default one
	for (int chi = 0; chi < numOfChannels_; ++chi)
		while (fftChannels_[chi].fftPlan.size() > 1) fftChannels_[chi].fftPlan.pop_back();

	// Add optimization plans
	for (int chi = 0; chi < numOfChannels_; ++chi)
		for (int i = 1; i <= optimizationLevel; ++i)
			fftChannels_[chi].fftPlan.push_back(fftw_plan_dft_r2c_1d(chBufSize_ - i, fftChannels_[chi].dataIn, reinterpret_cast<fftw_complex*>(fftChannels_[chi].complexDataOut), FFTW_MEASURE));
}

void FFT::threadCompute(int chi) {
	FFTStruct& fftCh = fftChannels_[chi];

	// Setup data
	for (int i = 0; i < chBufSize_; ++i) {
		fftCh.dataIn[i] = static_cast<double>(*(buf_ + i * numOfChannels_ + chi));
	}

	int optimizeCnt = 0;
	double maxVal = 0.0;
	int idx = 0;
	int size = chBufSize_;
	for (auto plan : fftCh.fftPlan) {
		fftw_execute(plan);
		int tmpSize = chBufSize_ - optimizeCnt;
		optimizeCnt++;
		std::complex<double>* newMaxVal = std::max_element(fftCh.complexDataOut + 1, fftCh.complexDataOut + tmpSize / 4,	// discard 0 index (DC offset) and search only the first quarter
			[](std::complex<double> const & lhs, std::complex<double> const & rhs) { return std::abs(lhs) < std::abs(rhs); });

		if (std::abs(*newMaxVal) > maxVal) {
			maxVal = std::abs(*newMaxVal);
			idx = std::distance(fftCh.complexDataOut, newMaxVal);
			size = tmpSize;
		}
	}

	// Return values
	fftCh.freq_ = (double)(idx * sampleFreq_) / (double)size;
	fftCh.ampl_ = maxVal * 2 / size;
	fftCh.num_cycles_ = static_cast<int>(idx);
	fftw_execute(fftCh.fftPlan[0]);	// Fill dataOut with all data points

	// Copy to output buffer
	fftCh.realDataOut[0] = std::abs(fftCh.complexDataOut[0]) / chBufSize_; // offset doesn't multiply by * 2
	for (int i = 1; i < realDataSize_; ++i) {
		fftCh.realDataOut[i] = std::abs(fftCh.complexDataOut[i]) * 4 / chBufSize_;
	}
}

void FFT::run() {
	int i = 0;

	for (auto& t : thread_) {
		t = std::thread(&FFT::threadCompute, this, i);
		i++;
	}

	for (auto& t : thread_) {
		t.join();
	}
}


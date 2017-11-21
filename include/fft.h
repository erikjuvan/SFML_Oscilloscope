#pragma once

#include <vector>
#include <complex>
#include <fftw3.h>
#include <thread>
#include <mutex>

class FFT {
private:

	struct FFTStruct {
		std::vector<fftw_plan> fftPlan;
		double* dataIn;
		std::complex<double>* complexDataOut;
		std::vector<double> realDataOut;		

		double freq_, ampl_;
		int num_cycles_;

		FFTStruct(int bufSize, int realDataSize);
		~FFTStruct();
	};

	std::vector<FFTStruct> fftChannels_;
	std::vector<std::thread> thread_;
	int numOfChannels_;
	int chBufSize_;
	int sampleFreq_;
	int realDataSize_;
	// Thread accessible parameters
	const uint8_t* buf_;

public:

	FFT(int numOfCh, int sampleFreq, int bufSize, const uint8_t* buf, int realDataSize);

	void						setOptimizationLevel(int optimizationLevel);
	void						threadCompute(int chi);
	const double				getFreq(int chi) const { return fftChannels_[chi].freq_; }
	const std::vector<double>&	getData(int chi) const { return fftChannels_[chi].realDataOut; }
	void						run();
};

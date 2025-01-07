#include "DefineParameters.h"

class CDetectEnergy_EPD{

private:
	int BinNumber;
	float NoiseMean[BAND_NUM];
	float NoiseVar[BAND_NUM];
	float SpeechMax[BAND_NUM];
	float SpeechMin[BAND_NUM];
	int SlimBand;
	float SlimRatio;
	int StrongBand;
	float StrongRatio;
	
public:
	CDetectEnergy_EPD(int SampleRate);
	~CDetectEnergy_EPD();
	float MeanMin[BAND_NUM];
	float MeanMax[BAND_NUM];
	float sensitivity[BAND_NUM];
	float Threshold[BAND_NUM];

	int  BandBorder[BAND_NUM][2];
	float BandEnergyBuffer[BAND_NUM][BUFLEN];//用来跟踪很多帧的？
	float MeanBuffer[BAND_NUM][BUFLEN];
	float VarBuffer[BAND_NUM][BUFLEN];

	void ReInit();
	void InitNoise();
	int  InitSensitivity(int FrameCount_in);
	double GetNewEnergy(float *BinEnergy_in, int FrameCount_in);
	void UpdateNoise(int FrameCount_in);
	int  StatSignal(int FrameCount_in);	//用于统计var和mean.
	void UpdateNoiseInSpeech(int FrameCount_in);
	int  DetectRoughStart(int FrameCount_in);
	int  RefineStart(int FrameCount_in, int FirstVoice_in);
	int  DetectRoughEnd(int FrameCount_in);
	int  StatSpeech(int FrameCount_in);
};
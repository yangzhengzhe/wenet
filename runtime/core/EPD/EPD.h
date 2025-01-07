#include "DefineParameters.h"
#include "DetectEnergy.h"
#include "DetectVoice.h"
#include "FFT.h"

class C_EPD
{
private:
	
	FFT fft;
	CDetectVoice_EPD *detectVoice;//check harmonics to detect speech start point.
	CDetectEnergy_EPD *detectEnergy;//能量检测模块

	short *TailData;		//上次剩下的数据
	int  TailLength;		//上次剩下的数据长度
	int  newTailLength;		//这次剩下的数据长度
	float *ffts;			//for fft 
	float *hamwin;			//hamming window
	float *HarmonicEnergy; //bin energy to detect harmonics.
	short *featDetect;
	int isFindStart;
	int isTrueStart;
	int tempStart;
	int tempEnd;
	double *FrameEnergy;   //当EPD检测超过了10s，保存10s到20s的帧能量信息
	double *avgFrameEnergy;//对FrameEnergy进行10帧一个平均，每次移动一帧
	int TenSecondFrameNum ;//10S对应的帧数：10/0.01=1000帧
	int id;
	int BinNumber;
	int FrameCount;			//总计数器，总共处理到第几帧
	int FrameNumber;		//本次计算的帧数（与上次剩余数据和本次送入数据有关）
	int BufferCount;		//本次处理到第几帧
	int GetSensitivity;		//是否已经初始化过灵敏度了。
	
	int EnoughHarmo;
	int Start, End;	//检测出的起止点（单位：点）
	int FRAME_SHIFT;
	int FRAME_LENGTH;
	int FFT_SIZE;
	int BIN_NUMBER;
	int wavSampleRate;
	int end_Count;

public:
	
	C_EPD(int SampleRate,const char* cfgFilename);
	~C_EPD();
	
	int doEPD(short *buf_in, int len_in, int *begin, int *end);
	int re_initEPD();
	int SetParam(char *paramName, char *paramValue);
	int GetParam(char *paramName, char *paramValue);
	int MAX_SILENCE1;
	short *featBuffer;
	int totalLen;
	int blankSpace;

};//CSpeechDetectort

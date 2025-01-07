
#pragma once
#include "DefineParameters.h"

class CDetectVoice_EPD
{
private:
	// 080417 jgao found bug for its global definition in multi-thread, so move here
	int Harmo[WINDOW_FRAMS][4];
	int PartHarmo[3][4];
	int Frame;
	/*
	HarmoResult[Frame]的状态：
	HarmoResult[Frame]=-1表示还没有任何匹配,或者从来没有匹配成功
	HarmoResult[Frame]=1表示检测结束了，不需要再对此帧做匹配操作
	HarmoResult[Frame]=2表示可以继续等待更长的匹配，其better表示这次匹配到的最长的情况*/

	short HarmoResult[WINDOW_FRAMS];//以该帧开头，3连谐波检测并连接的总结果。

	int BinNumber;			//number of bin
	float Resolution;		//Resolution in frequency domain
	int PitchNumber;		//总共有多少个可能的整数基频
	int MaxHarmoMode;		//一帧信号最多总共匹配谐波可能多少种方式（基频为第1谐波）
	int FrameNumber;		//number of frame
	int FrameCount;			//number of the frame in the data buffer of the detector. 

	int LowPitch,HighPitch;	//lowest pitch, highest pitch.
	int HighBand;			//highest frequency of searching harmonics, in bin.
	int LowBand;			//lowest frequency of searching harmonics, in bin. equal of LowPitch.
	int PitchChange;		//两帧之间pitch最大的变化量
	int HighestHarmo;		//可能的最高阶的谐波（即HighBand/LowPitch）


	float *BinEnergy;		//当前计算的一帧的频带能量

	int *max; //当前帧的能量局部极值，可能为谐波或基音
	int AllMax[WINDOW_FRAMS][MAX_PEAK_NUM];
	int *MaxWidth;				//当前帧的极值的宽度；
	int AllMaxWidth[WINDOW_FRAMS][MAX_PEAK_NUM];
	short *AllMatchPitch;		//窗内所有帧信号，匹配到任何可能基频的，任何可能方式
	short *FrameMatchPitch;		//一帧信号匹配到任何可能基频的任何可能方式
	short *nearHarmo;			//一帧信号的极值和某个基频的各次谐波所对应的状况。
	int StrongMax[WINDOW_FRAMS][6];	//每帧最高和最宽的各3个极大值。
	int *StrongPoints;			//用于传递一个指针。
	
public:
	CDetectVoice_EPD(int SampleRate);
	~CDetectVoice_EPD();
	void ReInit();
	void UpdateBuf();
	int  GetMax();
	void MatchPitch();
	int Find(int VoiceNumberRequest);
	int SearchHarmo(int pitch_in,short Mode_in, int FrameNo, int WidthThdSetting);
	int	TestEven(int harmo_in[4], int multi[4]);
	int GetHarmoWidth(int Harmo_in[4], int max_in[MAX_PEAK_NUM], int Width_in[MAX_PEAK_NUM], int Width_Out[4]);
	int DetectVoice(float *AllBinEnergy_in, int VoiceNumber_in);

	int Connect3(int *maxToDo, int FrameToDo, int FrameNow);
	int Connect4(int *maxToDo, int FrameToDo, int FrameNow, short Mode);
	int CountVoice();
	
	float MinBinEnergy;		//允许的最小频带能量，没有初始化？release一度工作不正常！
	int  Results[WINDOW_FRAMS];

};

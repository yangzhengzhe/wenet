#include "stdio.h"
#include "stdlib.h"
#include "EPD.h"
#include "math.h"
#include "string.h"
#include "./config.h"

int g_harmonicMinNumber_EPD; //要判出语音,至少需要多少谐波帧
int g_MAX_SILENCE_EPD;  //time duration (in frames) for waiting speaker's voice at the end. If the user keep silence longer than this parameter, the program will detect the end point.
int g_MIN_HARMO_LEN_EPD; //最短的谐波长度
float g_meanEnergy;
float g_meanEnergy_end;
float g_ENERGY_DECREASE_START;
float g_ENERGY_DECREASE_END;
short silenceList[1800] = {-8,8,8,8,8,8,8,8,8,-8,-8,-8,-24,-24,-8,8,-24,-24,-8,-8,-8,-8,8,8,8,-8,-8,8,-8,-8,8,8,8,8,8,8,-8,8,8,8,8,8,8,8,-8,-8,8,-8,8,-8,-8,8,8,8,-8,8,8,8,8,-8,8,8,8,8,8,8,8,-8,-24,-24,-24,-24,-8,-8,-8,-24,8,8,-8,8,8,8,8,8,8,24,24,24,24,8,8,8,8,24,8,24,8,8,24,8,8,8,8,-8,-8,-8,-8,-8,-24,-24,-24,-24,-24,-40,-40,-40,-40,-40,-40,-24,-24,-24,-24,-24,-24,-24,-8,-8,-8,8,8,8,8,8,8,8,24,8,24,8,24,24,24,24,24,24,24,8,24,24,24,8,8,8,8,8,8,8,-8,-8,-8,-8,-8,-8,-8,-8,-24,-24,-24,-24,-24,-8,-24,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,8,-8,8,8,8,8,8,8,8,-8,-24,-24,-24,-24,-8,-8,-8,-8,-8,-8,-8,-8,8,8,8,-8,8,8,8,8,8,8,8,-8,8,-8,-8,-8,-8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,-8,8,-8,8,8,8,8,-8,-8,-8,-8,-8,8,-8,-8,-8,-8,8,-8,8,8,8,8,8,-8,-8,-8,-8,-8,-8,-8,-8,8,-8,-8,-8,-8,8,-8,-8,-8,-8,-8,-8,-8,-8,8,8,-8,8,8,-8,8,-8,-8,8,8,8,8,8,-8,8,8,8,8,8,-8,8,8,8,-8,-8,8,-8,8,8,8,8,8,8,8,-8,-8,8,8,8,-8,8,8,8,8,8,8,8,-8,-8,-24,-8,-8,-8,-8,-8,-8,-24,-24,-24,-24,-24,-24,-24,-24,-8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,-8,-8,-24,-8,-8,-24,-8,-8,-8,-8,-8,-8,8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,-8,8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,8,-8,8,8,8,8,8,-8,8,8,8,8,-8,-8,-8,-24,-8,-8,8,-8,-24,-8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,-24,-8,-8,8,-8,-8,8,-8,-8,8,8,8,8,8,8,8,8,8,-8,8,-8,-8,-8,-8,-8,8,8,-8,-24,-24,-24,-8,-8,-24,-24,-8,-8,8,8,8,8,8,8,8,8,8,-8,8,8,8,8,8,8,8,8,8,8,-8,-8,-8,-8,-8,-8,-8,-8,8,8,8,8,8,8,8,8,8,8,-8,-8,-8,-8,-8,-8,-8,-8,-8,8,8,8,8,8,8,8,-8,8,-8,-8,8,8,8,-8,-8,-8,-8,-8,-8,-24,-8,-24,-8,-8,-24,-8,-8,-8,8,8,-8,-24,-24,-8,-8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,-8,-8,-8,-8,-8,-8,-8,-8,8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,8,8,-8,-8,-8,8,8,8,8,8,8,8,8,8,-8,8,8,-8,-8,-8,-24,-8,-8,-8,-24,-24,-8,-8,-8,8,-8,-8,-8,-8,-8,-8,-8,8,8,-8,-8,-8,8,8,8,8,8,8,8,8,8,8,8,8,-8,8,8,8,8,8,8,8,8,8,-8,8,8,8,8,8,8,8,-8,-8,8,8,8,-8,-8,8,-8,-8,-8,-8,-8,8,-8,-8,-8,-8,-8,8,-8,-8,8,8,8,8,8,8,8,8,8,8,8,8,8,-8,-8,8,8,8,8,8,8,8,8,8,8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,8,-8,-8,-8,-8,8,8,8,-8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,-8,-8,-8,8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-24,-8,-8,-8,-8,-8,8,-8,8,-8,8,8,8,-8,8,8,8,8,8,8,8,8,8,8,8,8,8,-8,8,8,8,8,-8,-8,8,-8,-24,-8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,-8,-8,8,8,8,8,-8,-8,8,-8,8,8,8,8,-8,-8,8,8,-8,-8,-8,-8,-8,-8,-8,-8,8,-8,-8,-8,8,-8,-8,-8,-8,-8,-8,-8,-8,8,-8,-8,-8,-8,-8,-8,-8,-8,8,-8,-8,-8,8,8,-8,8,8,-8,8,8,8,8,8,8,8,8,-8,8,8,8,8,-8,8,8,8,8,8,8,8,8,8,8,8,8,-8,-8,-8,-8,-8,-8,-8,-8,8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,8,-8,8,8,-8,-8,8,8,8,-8,-8,8,8,8,8,-8,-8,8,8,8,8,8,8,-8,-8,-8,-8,-8,-8,-8,-8,8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,8,8,8,8,8,8,-8,8,8,8,8,8,-8,-8,8,8,8,8,-8,-8,-8,-8,-8,-8,8,8,8,-8,-8,8,8,8,-8,-8,8,8,8,-8,8,8,-8,-24,-24,-24,-8,8,-8,8,8,8,8,8,8,8,8,8,8,8,8,8,-8,-8,8,8,-8,8,-8,-8,8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,8,-8,8,8,8,-8,-8,-8,8,8,-8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,-8,-8,-8,-8,-8,8,8,8,8,-8,-8,8,8,-8,8,8,8,8,8,-8,-8,-8,-8,8,-8,-8,-8,8,8,8,-8,-8,8,8,-24,-8,-8,-8,-8,-24,-8,-8,-8,-24,-8,-8,8,-8,8,8,8,8,8,8,8,8,8,-8,-8,8,-8,-24,-8,-24,-8,-8,-24,-8,-8,-8,-8,-8,-8,8,8,8,8,-8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,-8,-8,-8,-8,-24,-24,-24,-24,-24,-24,-24,-24,-8,8,-8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,24,8,8,8,8,24,8,-8,8,8,-8,-24,-24,-24,-24,-24,-24,-24,-24,-24,-24,-24,-8,-8,-8,-8,8,-24,-8,-24,-8,-8,-8,-8,8,8,8,8,8,8,8,-8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,-8,-24,-24,-24,-8,-8,8,8,8,8,8,8,8,-8,8,-8,-8,-8,-8,8,8,8,8,8,8,-8,-8,8,8,-8,-8,-24,-8,-8,-8,-24,-8,-8,-8,-8,8,8,8,8,-8,-8,-8,8,8,-24,-8,8,8,-8,-8,-8,8,8,-8,-8,-8,-8,-8,8,8,-8,8,-8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,-8,-8,-24,-8,-8,-24,-8,-8,8,8,-24,8,8,8,8,-8,8,-24,-8,-8,-8,-24,-8,8,-24,-8,8,8,8,8,8,8,8,8,8,8,-24,-8,8,-8,8,8,8,-8,8,8,8,8,8,8,8,-8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,-8,-8,-8,-8,-8,-24,8,8,8,8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-24,-8,8,-8,8,8,8,8,8,-8,8,-8,-8,-8,-8,-8,8,8,-8,-8,8,-8,8,8,8,8,8,8,8,8,-8,-8,-8,8,8,-8,8,8,8,-8,-8,-8,-8,-8,-24,-8,-8,-8,-8,-8,-8,-24,8,8,-8,8,8,-24,-8,8,-8,-8,8,8,8,8,8,8,-8,-8,-8,-8,-24,-8,8,8,8,-8,-8,8,8,-8,8,8,8,-8,8,8,8,8
,8,8,8,-24,-8,8,8,8,8,8,8,8,8,8,8,8,-8,-8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,-8,-8,8,-24,-24,-24,-24,-8,-8,-8,-24,-24,-8,8,8,8,8,8,-8,-24,-8,8,8,8,8,8,8,8,8,-8,-8,-24,-24,-8,-8,-8,8,8,8,8,-8,8,8,8,8,8,8,-8,8,8,8,8,8,8,-8,8,8,8,8,8,8,8,8,8,8,8,8,8,-8,-8,-8,-8,-8,-8,-8,-24,-8,-24,8,-8,-8,8,-8,8,-8,-8,-24,-8,-8,8,8,8,8,-24,-8,8,8,8,8,8,-8,-24,-24,-8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,-24,8,8,-24,8,-8,-8,-24,-8,-24,-8,-8,-8,-8,8,8,-8,8,-24,-8,-8,-8,8,8,-8,-24,8,8,8,-8,-8,8,8,8,8,8,8,8};


C_EPD::C_EPD(int SampleRate,const char* cfgFilename)
{
	char sCfg[256];
	memset(sCfg, 0, 256);
	strcpy(sCfg,cfgFilename);

	Config_DNN4EPD *aConfig = new Config_DNN4EPD;
	if (!aConfig->SetConfigFile(sCfg))
	{
		printf("set config file %s error.\n", sCfg);
		exit(1);
	}

	g_harmonicMinNumber_EPD = 8;
	aConfig->ReadConfig("harmonicMinNumber_EPD", g_harmonicMinNumber_EPD);

	g_MAX_SILENCE_EPD = 20;
	aConfig->ReadConfig("MAX_SILENCE_EPD", g_MAX_SILENCE_EPD);

	g_MIN_HARMO_LEN_EPD = 6;
	aConfig->ReadConfig("MIN_HARMO_LEN_EPD", g_MIN_HARMO_LEN_EPD);
	
	g_ENERGY_DECREASE_START = 1;
	aConfig->ReadConfig("ENERGY_DECREASE_START", g_ENERGY_DECREASE_START);

	g_ENERGY_DECREASE_END = 1;
	aConfig->ReadConfig("ENERGY_DECREASE_END", g_ENERGY_DECREASE_END);
	g_meanEnergy = 1;
	aConfig->ReadConfig("meanEnergy", g_meanEnergy);
	g_meanEnergy_end = 1;
	aConfig->ReadConfig("meanEnergy_end", g_meanEnergy_end);
	delete aConfig;	

	wavSampleRate = SampleRate;
	FRAME_SHIFT = (SampleRate/100);		//10 ms, window's shift. It is same with the frameShift
	FRAME_LENGTH = (SampleRate/40);		//25 ms, minimum data to start work
	FFT_SIZE = (512*SampleRate/8000);		//fft points for checking harmonics
	BIN_NUMBER = (256*SampleRate/8000);		//bin numbers for checking harmonics
	detectVoice = new CDetectVoice_EPD(SampleRate);
	detectEnergy = new CDetectEnergy_EPD(SampleRate);
	int ii;
	TailData=(short *)calloc(FRAME_LENGTH,sizeof(short));//残余数据(<1帧)
	TailLength=0;

	FrameCount=0;

	hamwin=(float *)calloc(FRAME_LENGTH,sizeof(float));
	for(ii=0;ii<FRAME_LENGTH/2;ii++)
		hamwin[ii]=float(0.54 - 0.46 * cos(2*PI*((float)ii/(float)FRAME_LENGTH-1)));
	for(ii=FRAME_LENGTH/2;ii<FRAME_LENGTH;ii++)
		hamwin[ii]=hamwin[FRAME_LENGTH-1-ii];
	
	ffts=(float *)calloc(FFT_SIZE,sizeof(float));

	BinNumber=int(FFT_SIZE/2);
	HarmonicEnergy=(float *)calloc(BinNumber*WINDOW_FRAMS, sizeof(float));
	MAX_SILENCE1=g_MAX_SILENCE_EPD;
	featBuffer = (short *)calloc( SampleRate * 60, sizeof(short));

	featDetect = featBuffer;
	totalLen = 0;
	blankSpace = 0;
	isFindStart = 0;
	isTrueStart = 0;
	tempStart = -1;
	tempEnd = -1;
	TenSecondFrameNum = 19/0.01;
	//TenSecondFrameNum = 14/0.01;//20180930 ForceDetectEndPoint14s-15s,when no Detect End Point more than 15s
	FrameEnergy =  new double[TenSecondFrameNum];
	memset(FrameEnergy,0,TenSecondFrameNum);
	avgFrameEnergy = new double[TenSecondFrameNum];
	memset(avgFrameEnergy,0,TenSecondFrameNum);
	id = 0;

}//C_EPD::C_EPD()

C_EPD::~C_EPD()
{
	free(TailData);
	free(ffts);
	free(hamwin);
	free(HarmonicEnergy);
	free(featBuffer);
	delete detectVoice;
	delete detectEnergy;
	delete[] FrameEnergy;
	delete[] avgFrameEnergy;
}//C_EPD::~C_EPD()

int C_EPD::re_initEPD()
{
	memset(TailData,0,sizeof(short)*FRAME_LENGTH);//残余数据清零
	memset(HarmonicEnergy,0,sizeof(float)*WINDOW_FRAMS*BinNumber);
	memset(featBuffer,0,sizeof(short)* wavSampleRate * 60);
	TailLength=0;
	newTailLength=0;

	FrameCount=0;
	Start=-1;
	End=-1;
	GetSensitivity=-1;

	detectEnergy->ReInit();
	detectVoice->ReInit();

	memset(FrameEnergy,0,TenSecondFrameNum);
	memset(avgFrameEnergy,0,TenSecondFrameNum);
	id = 0;
	totalLen = 0;
	featDetect = featBuffer;
	blankSpace = 0;
	isFindStart = 0;
	isTrueStart = 0;
	tempStart = -1;
	tempEnd = -1;
	end_Count = 0;	
	doEPD(silenceList, 1800, &tempStart, &tempEnd);
	return(1);
}

//FILE *epdlog = fopen("epdenrg.txt","wt");
int C_EPD::doEPD(short *buf_in, int len_in, int *begin, int *end)
{
	//* buffer是输入语音数据指针； len, 输入语音数据长度(点数)， * start是检测到的开始位置,  * end是检测到的结束位置
	//函数返回1：检测到起点或者终点;返回0：没有检测到起点或者终点
	
	int i;

//////////////////////////送入数据///////////////////////////////////////////
	if(len_in+TailLength>=FRAME_LENGTH)
	{
		FrameNumber=(len_in+TailLength-FRAME_LENGTH)/FRAME_SHIFT+1;
		newTailLength=len_in+TailLength-FRAME_SHIFT*FrameNumber;
	}
	else
	{
		FrameNumber=0;
		newTailLength=TailLength+len_in;
	}
	memcpy(featDetect, buf_in, len_in * sizeof(short));
	featDetect += len_in; 
	totalLen += len_in;
	
//////////////////////////循环处理各帧///////////////////////////////////////

	BufferCount=0;
	while(BufferCount<FrameNumber)
	{
		//当前帧的数据，准备fft//
		if(BufferCount*FRAME_SHIFT<TailLength)
		{
			for(i=0;i<TailLength-BufferCount*FRAME_SHIFT;i++)
				ffts[i]=TailData[i+BufferCount*FRAME_SHIFT];
			for(i=TailLength-BufferCount*FRAME_SHIFT;i<FRAME_LENGTH;i++)
				ffts[i]=buf_in[i-(TailLength-BufferCount*FRAME_SHIFT)];
		}
		else	//BufferCount*FRAME_SHIFT>=TailLength
		{
			for(i=0;i<FRAME_LENGTH;i++)
				ffts[i]=buf_in[i+BufferCount*FRAME_SHIFT-TailLength];//-(FRAME_LENGTH-FRAME_SHIFT)];
		}
		memset(&ffts[FRAME_LENGTH],0,sizeof(float)*(FFT_SIZE-FRAME_LENGTH));

		for(i=0;i<FRAME_LENGTH;i++)		//加窗
			ffts[i]*=hamwin[i];

		if(PRE_EMPHASIS>0.0)			//预加重
		{
			for(i=FRAME_LENGTH-1;i>0;i--)
				ffts[i]-=ffts[i-1]*PRE_EMPHASIS;
			ffts[0]*=(float)(1.0-PRE_EMPHASIS);
		}
		fft.RealFFT(ffts, FFT_SIZE,0);	//FFT
		
		memmove(&HarmonicEnergy[0],&HarmonicEnergy[BinNumber],sizeof(float)*BinNumber*(WINDOW_FRAMS-1));//更新谐波检测的缓存频带能量
		for(i=0;i<BinNumber;i++)
			HarmonicEnergy[(WINDOW_FRAMS-1)*BinNumber+i]=ffts[2*i]*ffts[2*i]+ffts[2*i+1]*ffts[2*i+1]; //能量计算？

		detectVoice->UpdateBuf();		//更新谐波检测的内部缓存。初始化工作？
		double curFrameEnergy = detectEnergy->GetNewEnergy(&HarmonicEnergy[(WINDOW_FRAMS-1)*BinNumber], FrameCount);

		if(Start==-1)                    //尚未检测到起始点
		{
			if(FrameCount==WINDOW_FRAMS-1)
			{
				detectEnergy->InitNoise();//
				if(GetSensitivity==-1)//如果还没有初始化
					GetSensitivity=detectEnergy->InitSensitivity(FrameCount);

			}//if(FrameCount==WINDOW_FRAMS-1)
			
			if(FrameCount>WINDOW_FRAMS-1)
			{
				if(GetSensitivity==-1)//如果还没有初始化
					GetSensitivity=detectEnergy->InitSensitivity(FrameCount);
				detectEnergy->StatSignal(FrameCount);
				if(GetSensitivity==1)
					detectEnergy->UpdateNoise(FrameCount);

				Start=detectEnergy->DetectRoughStart(FrameCount);
				if(Start>=0)
				{
					EnoughHarmo=detectVoice->DetectVoice(HarmonicEnergy,g_harmonicMinNumber_EPD);
					if(EnoughHarmo>0)	//检测谐波
					{
						i=0;
						while(detectVoice->Results[i]<=0)
							i++;
						//Start=detectEnergy->RefineStart(FrameCount,i);
						int oldStart = Start;
						if(Start > 30)
							Start -= 30;
						else
							Start = 0;
			
						int startPoint = Start* FRAME_SHIFT - blankSpace;
						short *startBuffer;
						startBuffer = featBuffer + startPoint;
						double FrameEnergy = 0;
						int frameMusic,pointMusic;
						for(frameMusic = 0; frameMusic < (oldStart - Start); frameMusic++){
							for(pointMusic = 0; pointMusic < FRAME_LENGTH; pointMusic++){
								FrameEnergy += (*(startBuffer + frameMusic * FRAME_SHIFT + pointMusic)) * (*(startBuffer + frameMusic * FRAME_SHIFT + pointMusic)) / FRAME_LENGTH;
							}
							if(FrameEnergy < 10000)
								Start++;
							else
								break;
							FrameEnergy = 0;
						}
						startPoint = Start * FRAME_SHIFT - blankSpace;
						int temp_WINDOW_FRAMS = int(1.0*(totalLen - startPoint - FRAME_LENGTH) / FRAME_SHIFT);
						if(temp_WINDOW_FRAMS > 10){
							startBuffer = featBuffer + startPoint;
							double *Energy =  new double[temp_WINDOW_FRAMS];
							memset(Energy, 0, sizeof(double)*temp_WINDOW_FRAMS);
							double meanEnergy = 0.0;// varEnergy = 0.0;
							//int frameMusic,pointMusic;
							for(frameMusic = 0; frameMusic < temp_WINDOW_FRAMS; frameMusic++){
								for(pointMusic = 0; pointMusic < FRAME_LENGTH; pointMusic++){
									Energy[frameMusic] += (*(startBuffer + frameMusic * FRAME_SHIFT + pointMusic)) * (*(startBuffer + frameMusic * FRAME_SHIFT + pointMusic)) * 1.0 / FRAME_LENGTH;
								}
							}
							for(frameMusic = 0; frameMusic < temp_WINDOW_FRAMS; frameMusic++){
								meanEnergy += Energy[frameMusic];
							}
							meanEnergy /= temp_WINDOW_FRAMS;
							delete []Energy;
							if(meanEnergy / 100000 >= g_meanEnergy){                 //此处为 能量判决门限，大于阈值判断为语音，目前没有一个非常好的经验值
								if(Start*FRAME_SHIFT - 1800 < 0)
									*begin = 0;
								else	
									*begin=Start*FRAME_SHIFT - 1800;
							}
							else{
								Start=-1;
							}
						}
						else
						{
							if(Start*FRAME_SHIFT - 1800 < 0)
								*begin = 0;
							else	
								*begin=Start*FRAME_SHIFT - 1800;
						}
						End=-1;
						*end=-1;
					}//if(EnoughHarmo>0)	//检测谐波
					else
						Start=-1;
				}//if(Start>=0)
				if(*begin >= 0){
					isFindStart = 1;
				}
			}//if(FrameCount>WINDOW_FRAMS-1)
		}//if(*begin==-1)
	
//////////////////////////////////////////检测终点////////////////////////////////////////////////////		
		else if(*end==-1)
		{
			double lowestEnr = 0.0;
			detectEnergy->StatSignal(FrameCount);
			detectEnergy->UpdateNoiseInSpeech(FrameCount);
				
			if(FrameCount>=Start+SEG_SILENCE)	//after the speeh begin for a while long enough.
			{
				int endPoint = (FrameCount - SEG_SILENCE) * FRAME_SHIFT - blankSpace;
				short *endBuffer;
				endBuffer = featBuffer + endPoint;
				double FrameEnergy_End = 0;
				int pointMusic,frameMusic;
				bool isEndFlag = false;
				for(frameMusic = 0; frameMusic < SEG_SILENCE; frameMusic++){
					for(pointMusic = 0; pointMusic < FRAME_LENGTH; pointMusic++){
							FrameEnergy_End += (*(endBuffer +  frameMusic * FRAME_SHIFT + pointMusic)) * (*(endBuffer + frameMusic * FRAME_SHIFT + pointMusic)) / FRAME_LENGTH;
					}
				}
				FrameEnergy_End /= SEG_SILENCE;
				if(FrameEnergy_End < 10000 * g_meanEnergy_end)
					isEndFlag = true;
				detectEnergy->StatSpeech(FrameCount);
				if(End==-1)
					End=detectEnergy->DetectRoughEnd(FrameCount);
				if(End>=0 || isEndFlag)//粗判出了终点
				{
					EnoughHarmo=detectVoice->DetectVoice(HarmonicEnergy,1);
					if(EnoughHarmo>0 && !isEndFlag)	//检测谐波
						End=-1;
					else
					{
						if(isEndFlag && End == -1)
							End = FrameCount;
						int tmp;
						tmp=detectEnergy->DetectRoughStart(FrameCount);
						if(tmp>0)
						{
							EnoughHarmo=detectVoice->DetectVoice(HarmonicEnergy,1);
							if((FrameCount>=End+MAX_SILENCE1))
							{
								End=FrameCount-MAX_SILENCE1;
								if(End*FRAME_SHIFT - 1800 < 0)
									*end = 0;
								else
									*end=End*FRAME_SHIFT - 1800;
							}
						}
						else if(FrameCount>=End+MAX_SILENCE1)
						{
							End=FrameCount-MAX_SILENCE1;
							if(End*FRAME_SHIFT - 1800 < 0)
								*end = 0;
							else
								*end=End*FRAME_SHIFT - 1800;
						}
						
					}
				}//if(End>=0)
				if (*end==-1)// 处理大于20s没有尾点的情况 [9/5/2016 hongmi]
				{
					//
					if (FrameCount >= Start+TenSecondFrameNum&&FrameCount<Start+2000)
					//if (FrameCount >= Start+TenSecondFrameNum&&FrameCount<Start+1500)//20180930 ForceDetectEndPoint14s-15s,when no Detect End Point more than 15s
					{
						FrameEnergy[id] = curFrameEnergy;
						id++;
					}
					if(FrameCount==Start + 2000)
					//if(FrameCount==Start + 1500)//20180930 ForceDetectEndPoint14s-15s,when no Detect End Point more than 15s
					{
						for (int i=0;i<10;i++)
						{
							avgFrameEnergy[0]+=FrameEnergy[i];
						}
						lowestEnr = avgFrameEnergy[0];
						//fprintf(epdlog,"\n************************************\n\n");
						//fprintf(epdlog,"lowestEnr=%lf\tavgFrameEnergy=%lf\tbegin=%d\tend=%d\n",lowestEnr,avgFrameEnergy[0],*begin,*end);
						int flag = 0;
						for (int i=1;i<FrameCount - Start - TenSecondFrameNum - 10;i++)
						//for (int i=1;i<FrameCount - Start - TenSecondFrameNum - 10;i++)//20180930 ForceDetectEndPoint14s-15s,when no Detect End Point more than 15s
						{
							avgFrameEnergy[i]=avgFrameEnergy[i-1]+FrameEnergy[i+9]-FrameEnergy[i-1];//每10幀求平均，幀移是1幀
							if (avgFrameEnergy[i]-lowestEnr<1e-5)
							{
								lowestEnr = avgFrameEnergy[i];
								flag = i;
							}
							//fprintf(epdlog,"lowestEnr=%lf\tavgFrameEnergy=%lf\tbegin=%d\tend=%d\n",lowestEnr,avgFrameEnergy[i],*begin,*end);
						}
						*end=(Start+TenSecondFrameNum+flag)*FRAME_SHIFT - 1800;
						memset(FrameEnergy,0,TenSecondFrameNum);
						memset(avgFrameEnergy,0,TenSecondFrameNum);
					}
					
				}
				
			}//if(Counter>=StartIdx+SEG_SILENCE)			
		}//else if(*end==-1)

		BufferCount++;
		FrameCount++;
	}//while(BufferCount<FrameNumber)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	if(totalLen > 30 * wavSampleRate && isFindStart == 0)
	{
		int lastLen = totalLen - 20 * wavSampleRate;
		blankSpace += 20 * wavSampleRate;
		memmove(featBuffer, featDetect - lastLen, lastLen * sizeof(short));
		featDetect = featBuffer + lastLen;
		totalLen = lastLen;
	}

/////保存残留数据/////////////////////////////////////////////////////////////////////
	if(len_in>=newTailLength)		//如果新数据足够残存数据长度
		memmove(&TailData[0],&buf_in[len_in-newTailLength],sizeof(short)*newTailLength);
	else			//如果新数据不够残存数据的长度
	{
		memmove(&TailData[0],&TailData[TailLength-(newTailLength-len_in)],sizeof(short)*(newTailLength-len_in));
		memmove(&TailData[newTailLength-len_in],buf_in,sizeof(short)*len_in);
	
	}
	TailLength=newTailLength;
//////////////////////////////////////////////////////////////////////////////////////

	return(1);
}//

int C_EPD::SetParam(char *paramName, char *paramValue)
{
	float numericalValue = atof(paramValue);
	if(!strcmp(paramName, "incompletetimeout") && numericalValue>=0)
		MAX_SILENCE1=int(g_MAX_SILENCE_EPD*numericalValue/50.0);
	else if(!strcmp(paramName, "MAX_SILENCE_EPD") && numericalValue>=0)
		MAX_SILENCE1=int(numericalValue);
	else if(!strcmp(paramName, "sensitivity") && numericalValue>=0 && numericalValue<=100){
		detectEnergy->MeanMax[0]= pow(10, 16-0.12*numericalValue);   // 1e4 + pow(10,20-numericalValue/5.0);   // 越大越不灵敏, 公式可能需要修正
		int i;
		for(i=1;i<BAND_NUM;i++)
			detectEnergy->MeanMax[i]= pow(10, 12-0.08*numericalValue);   // 1e4+pow(10,16-numericalValue/6.25);
	}
	else return -1;
	return 0;
}

int C_EPD::GetParam(char *paramName, char *paramValue)
{
	if(!strcmp(paramName, "incompletetimeout"))
		sprintf(paramValue, "%d", MAX_SILENCE1);
	else if(!strcmp(paramName, "MAX_SILENCE_EPD"))
		sprintf(paramValue, "%d", MAX_SILENCE1);
	else if(!strcmp(paramName, "sensitivity"))
		sprintf(paramValue, "%f", (16-log10(detectEnergy->MeanMax[0]))/0.12);
	else return -1;
	return 0;
}

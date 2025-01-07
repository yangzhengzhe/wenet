#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "math.h"
#include "DetectEnergy.h"
#include "DefineParameters.h"

extern float g_ENERGY_DECREASE_START;
extern float g_ENERGY_DECREASE_END;

CDetectEnergy_EPD::CDetectEnergy_EPD(int SampleRate)
{
	int FFT_SIZE = (512*SampleRate/8000);
	BinNumber=int(FFT_SIZE/2);
	
	BandBorder[0][0]=0;
	BandBorder[0][1]=BinNumber/8;
	BandBorder[1][0]=BinNumber/8;
	BandBorder[1][1]=BinNumber/4;
	BandBorder[2][0]=BinNumber/4;
	BandBorder[2][1]=BinNumber/2;
	BandBorder[3][0]=BinNumber/2;
	BandBorder[3][1]=BinNumber;

	MeanMax[0]=1e10;
	MeanMax[1]=1e8;
	MeanMax[2]=1e8;
	MeanMax[3]=1e8;
	MeanMin[0]=1e4;
	MeanMin[1]=1e4;
	MeanMin[2]=1e4;
 	MeanMin[3]=1e4;

}//CDetectEnergy::CDetectEnergy()


CDetectEnergy_EPD::~CDetectEnergy_EPD()
{
}//CDetectEnergy::~CDetectEnergy()


void CDetectEnergy_EPD::ReInit()
{
	memset(BandEnergyBuffer,0,sizeof(float)*BAND_NUM*BUFLEN);
	memset(SpeechMax, 0,sizeof(float)*BAND_NUM);
	memset(SpeechMin, 0,sizeof(float)*BAND_NUM);
	
}//void CDetectEnergy::ReInit()

//计算各个子带的能量并存储,并返回该帧的总能量
double CDetectEnergy_EPD::GetNewEnergy(float *BinEnergy_in, int FrameCount_in)
{
	int ii,jj;
	float energy;
	double totalEnergy=0;
	for(ii=0;ii<BAND_NUM;ii++)
	{
		energy=0;
		for(jj=BandBorder[ii][0];jj<BandBorder[ii][1];jj++)
			energy+=BinEnergy_in[jj];
		BandEnergyBuffer[ii][FrameCount_in%BUFLEN]=energy;
		totalEnergy+= energy;
	}
	return totalEnergy;

}//void CDetectEnergy::GetNewEnergy()

//初始化噪声模型参数
void CDetectEnergy_EPD::InitNoise()
{  
	float mean[BAND_NUM],var[BAND_NUM];
	int ii,jj, frame;

	for(ii=0;ii<BAND_NUM;ii++)
	{
		mean[ii]=0;
		for(jj=0;jj<ENERGY_FRAMS;jj++)
			mean[ii]+=BandEnergyBuffer[ii][jj];
		mean[ii]/=ENERGY_FRAMS;//样本均值

		var[ii]=0;
		for(jj=0;jj<ENERGY_FRAMS;jj++)
			var[ii]+=(BandEnergyBuffer[ii][jj]-mean[ii])*(BandEnergyBuffer[ii][jj]-mean[ii]);

		if(mean[ii]>0)
			var[ii]=float(sqrt(var[ii]/(mean[ii]*mean[ii]*ENERGY_FRAMS)));//论文page5的公式
		else 
			var[ii]=0;

		VarBuffer[ii][0]=var[ii];
		MeanBuffer[ii][0]=mean[ii];
	}//for(ii=0;ii<BAND_NUM;ii++)
//////////////////////////////////////////////////////////////////////////
//？不懂了
	for(frame=1;frame<=WINDOW_FRAMS-ENERGY_FRAMS;frame++)
	{
		for(ii=0;ii<BAND_NUM;ii++)
		{
			MeanBuffer[ii][frame]=MeanBuffer[ii][frame-1]+(BandEnergyBuffer[ii][frame+ENERGY_FRAMS-1]-BandEnergyBuffer[ii][frame-1])/ENERGY_FRAMS;
			mean[ii]=MeanBuffer[ii][frame];

			var[ii]=0;
			for(jj=frame;jj<frame+ENERGY_FRAMS;jj++)
				var[ii]+=(BandEnergyBuffer[ii][jj]-mean[ii])*(BandEnergyBuffer[ii][jj]-mean[ii]);

			if(mean[ii]>0)
			{
				var[ii]/=ENERGY_FRAMS*mean[ii]*mean[ii];
				var[ii]=float(sqrt(var[ii]));
			}
			else 
				var[ii]=0;

			VarBuffer[ii][frame]=var[ii];
		}//for(ii=0;ii<BAND_NUM;ii++)
	}

	for(ii=0;ii<BAND_NUM;ii++)
	{
		NoiseMean[ii]=MeanBuffer[ii][0];
		NoiseVar[ii]=VarBuffer[ii][0];
	}
	
}//void CDetectEnergy::InitNoise()

int CDetectEnergy_EPD::InitSensitivity(int FrameCount_in)
{
	int ii,jj;
	int GetRecord=1;    //所有频带在当前滑动窗的前ENERGY_FRAMS帧都有信号，则为1，否则为0
	float tmpMean,tmpVar;

	for(jj=FrameCount_in-WINDOW_FRAMS+1;(jj<FrameCount_in-WINDOW_FRAMS+1+ENERGY_FRAMS)&&(GetRecord);jj++)
		for(ii=0;ii<BAND_NUM;ii++)
		{
			if(BandEnergyBuffer[ii][jj%BUFLEN]<=MeanMin[ii])
			{
				GetRecord=0;
				break;
			}
		}//for for

	if(GetRecord==1)//存在录音，则根据频带能量均值和方差进行灵敏度初始化
	{
		for(ii=0;ii<BAND_NUM;ii++)
		{
			tmpMean=MeanBuffer[ii][(FrameCount_in-WINDOW_FRAMS+1)%BUFLEN];
			tmpVar=VarBuffer[ii][(FrameCount_in-WINDOW_FRAMS+1)%BUFLEN];
			if(tmpMean*(1+tmpVar)>=MeanMax[ii]) //if the noise is too high, then the sensitivity should be high
				sensitivity[ii]=MAX_SENSITIVITY;
			else if(tmpMean<=MeanMin[ii])
					sensitivity[ii]=MIN_SENSITIVITY;
			else
				sensitivity[ii]=(MAX_SENSITIVITY-MIN_SENSITIVITY)/(MeanMax[ii]-MeanMin[ii])*(tmpMean-MeanMin[ii])+MIN_SENSITIVITY;
			
			Threshold[ii]=tmpMean+tmpMean*tmpVar/sensitivity[ii];
			NoiseMean[ii]=tmpMean;
			NoiseVar[ii]=tmpVar;
		}
		return(1);//表示已经完成了初始化
	}//if(GetRecord==1)
	else 
	{
		for(ii=0;ii<BAND_NUM;ii++)
			Threshold[ii]=MeanMax[ii]*0.3;
		return(-1);
	}//else

}//int InitSensitivity()

void CDetectEnergy_EPD::UpdateNoise(int FrameCount_in)
{
	float mean[BAND_NUM],var[BAND_NUM];
	float pre_mean[BAND_NUM], pre_var[BAND_NUM];
	float next_mean[BAND_NUM],next_var[BAND_NUM];
	int ii;
	for(ii=0;ii<BAND_NUM;ii++)
	{
		var[ii]=VarBuffer[ii][(FrameCount_in-WINDOW_FRAMS+1)%BUFLEN];
		mean[ii]=MeanBuffer[ii][(FrameCount_in-WINDOW_FRAMS+1)%BUFLEN];
		pre_var[ii]=VarBuffer[ii][(FrameCount_in-WINDOW_FRAMS)%BUFLEN];
		pre_mean[ii]=MeanBuffer[ii][(FrameCount_in-WINDOW_FRAMS)%BUFLEN];
		next_var[ii]=VarBuffer[ii][(FrameCount_in-WINDOW_FRAMS+2)%BUFLEN];
		next_mean[ii]=MeanBuffer[ii][(FrameCount_in-WINDOW_FRAMS+2)%BUFLEN];

		if(mean[ii]<MeanMin[ii])
		{
			NoiseMean[ii]=MeanMin[ii];
			if(var[ii]<VAR_MIN)
				NoiseVar[ii]=VAR_MIN;
		}
		else if ((var[ii]<VAR_MIN)&&(mean[ii]<=pre_mean[ii])&&(next_mean[ii]<=mean[ii]))
		{	//方差非常小，且均值正在不断降低，所以把方差设为最小值，
			NoiseMean[ii]=mean[ii];
			NoiseVar[ii]=VAR_MIN;
		}
		else if((mean[ii]<=NoiseMean[ii]*UPDATE_NOISE)&&(var[ii]<=NoiseVar[ii]*UPDATE_NOISE))
		{//都在慢变，可以更新。
			NoiseMean[ii]=mean[ii];
			NoiseVar[ii]=var[ii];
		}
		else if((var[ii]<pre_var[ii])&&(next_var[ii]<var[ii])&&(mean[ii]<=pre_mean[ii])&&(mean[ii]<NoiseMean[ii]*UPDATE_NOISE))
		{//方差正在下降，均值也在下降，且均值不大，说明强噪声已经过去
			NoiseMean[ii]=mean[ii];
			NoiseVar[ii]=next_var[ii];
		}
		else if(mean[ii]*(var[ii]+1)<NoiseMean[ii]*(NoiseVar[ii]+1))
		{//噪声无论如何是降低了，只是方差还比较大
			NoiseMean[ii]=mean[ii];
			NoiseVar[ii]=var[ii];
		}
		else if((NoiseMean[ii]<=MeanMin[ii])&&(NoiseVar[ii]<=VAR_MIN+0.00001)&&(mean[ii]<=pre_mean[ii])&&(next_mean[ii]<=mean[ii])&&(var[ii]<=pre_var[ii])&&(next_var[ii]<=var[ii]))
		{//这里是用于从静音中恢复到录音信号的状态，以防止静音带来的噪声死锁。
			NoiseMean[ii]=mean[ii];
			NoiseVar[ii]=var[ii];
		}
		Threshold[ii]=NoiseMean[ii]+NoiseMean[ii]*NoiseVar[ii]/sensitivity[ii];

	}//for(ii=0;ii<BAND_NUM;ii++)
	
}//void CDetectEnergy::UpdateNoise()


void CDetectEnergy_EPD::UpdateNoiseInSpeech(int FrameCount_in)
{
	float mean[BAND_NUM],var[BAND_NUM];
	int ii;
	
	for(ii=0;ii<BAND_NUM;ii++)
	{
		var[ii]=VarBuffer[ii][(FrameCount_in-WINDOW_FRAMS+1)%BUFLEN];
		mean[ii]=MeanBuffer[ii][(FrameCount_in-WINDOW_FRAMS+1)%BUFLEN];
	
		if(mean[ii]<MeanMin[ii])
		{
			NoiseMean[ii]=MeanMin[ii];
			if(var[ii]<VAR_MIN)
				var[ii]=VAR_MIN;
		}
		else if(mean[ii]*(var[ii]+1)<NoiseMean[ii]*(NoiseVar[ii]+1))
		{//噪声无论如何是降低了，即使方差还比较大
			NoiseMean[ii]=mean[ii];
			NoiseVar[ii]=var[ii];
		}
		else if((mean[ii]<NoiseMean[ii]*UPDATE_NOISE)&&(var[ii]<NoiseVar[ii]))
		{//均值增大不多，而方差确实减小了。
			NoiseMean[ii]=mean[ii];	//这里会带来问题：语音段中高频能量少，会更新成语音能量的！
			NoiseVar[ii]=var[ii];
		}
		Threshold[ii]=NoiseMean[ii]+NoiseMean[ii]*NoiseVar[ii];
	}//for(ii=0;ii<BAND_NUM;ii++)
	
}//void CDetectEnergy::UpdateNoiseInSpeech()

//统计信号情况
int CDetectEnergy_EPD::StatSignal(int FrameCount_in)
{
	float mean[BAND_NUM],var[BAND_NUM];
	int ii,jj;
	
	for(ii=0;ii<BAND_NUM;ii++)
	{
		mean[ii]=MeanBuffer[ii][(FrameCount_in-ENERGY_FRAMS)%BUFLEN];
		mean[ii]+=(BandEnergyBuffer[ii][FrameCount_in%BUFLEN]-BandEnergyBuffer[ii][(FrameCount_in-ENERGY_FRAMS)%BUFLEN])/ENERGY_FRAMS;
		MeanBuffer[ii][(FrameCount_in-ENERGY_FRAMS+1)%BUFLEN]=mean[ii];

		var[ii]=0;
		for(jj=FrameCount_in-ENERGY_FRAMS+1;jj<=FrameCount_in;jj++)
			var[ii]+=(BandEnergyBuffer[ii][jj%BUFLEN]-mean[ii])*(BandEnergyBuffer[ii][jj%BUFLEN]-mean[ii]);

		if(mean[ii]>0)
			VarBuffer[ii][(FrameCount_in-ENERGY_FRAMS+1)%BUFLEN]=float(sqrt(var[ii]/(mean[ii]*mean[ii]*ENERGY_FRAMS)));
		else 
			VarBuffer[ii][(FrameCount_in-ENERGY_FRAMS+1)%BUFLEN]=0;

	}//for(ii)
	return(1);

}//int CDetectEnergy::StatSignal()


int CDetectEnergy_EPD::DetectRoughStart(int FrameCount_in)
{
	int ii,jj;	//原理：当前窗中至少有3个频带满足能量大于阈值的要求，且都大于噪声
	int StrongBandNum;
	int WeakBandIdx;

	for(jj=ENERGY_FRAMS;jj<=WINDOW_FRAMS-ENERGY_FRAMS;jj++)
	{
		StrongBandNum=0;
		WeakBandIdx=-1;
		for(ii=0;ii<BAND_NUM;ii++)
		{
			if(MeanBuffer[ii][(FrameCount_in-WINDOW_FRAMS+1+jj)%BUFLEN]>Threshold[ii] * g_ENERGY_DECREASE_START)
				StrongBandNum++;
			else 
				WeakBandIdx=ii;
		}//for(ii)

		if(StrongBandNum==BAND_NUM)
			return(FrameCount_in-WINDOW_FRAMS+1);
		if(StrongBandNum==BAND_NUM-1)
		{
			if(MeanBuffer[WeakBandIdx][(FrameCount_in-WINDOW_FRAMS+1+jj)%BUFLEN]>NoiseMean[WeakBandIdx]*(1+NoiseVar[WeakBandIdx]))
				return(FrameCount_in-WINDOW_FRAMS+1);
		}//if(StrongBandNum==3)
	
			
	}//for(jj=ENERGY_FRAMS;jj<WINDOW_FRAMS;jj++)
		
	return(-1);//返回-1，表示没有找到起点
}//int CDetectEnergy::DetectRoughStart()


int CDetectEnergy_EPD::RefineStart(int FrameCount_in, int FirstVoice_in)
{
	int ii, count;
	int start;
	int find;
	int BiggerthanNoise;

	if(FirstVoice_in>WINDOW_FRAMS-ENERGY_FRAMS)
		start=FrameCount_in-ENERGY_FRAMS+1;
	else 
		start=FrameCount_in-WINDOW_FRAMS+1+FirstVoice_in;//从浊音开始往前找

	find=1;
	while((start>FrameCount_in-BUFLEN+1)&&(find==1)&&(start>=0))
	{
		BiggerthanNoise=1;//首先在所有频带能量都比噪声强的情况下,才可以向前找精确起点
		for(ii=0;ii<BAND_NUM;ii++)
		{
			if(MeanBuffer[ii][start%BUFLEN]<NoiseMean[ii])
				BiggerthanNoise=0;
		}
		if(BiggerthanNoise==0)
			break;

		find=0;
		for(ii=0;ii<BAND_NUM;ii++)
		{
			if(MeanBuffer[ii][start%BUFLEN]>10*Threshold[ii])//语音很强，大于阈值的10倍
				find=1;
		}//for(ii)
		
		for(ii=0;ii<BAND_NUM;ii++)
		{
			if(MeanBuffer[ii][start%BUFLEN]>MeanBuffer[ii][(start-1)%BUFLEN])
				find=1;
			if((start<=FrameCount_in-BUFLEN+1)||(start==0))//超出缓存范围了,或者已经是0。
				break;
		}//for(ii)
		if(find==1)// 不管符合上述两个要求中的哪一个，都认为起点可以向前推。
			start--;
		if(find==0)
			break;
	}//while

	//加上一段往后找的处理，防止找到能量比噪声还低的位置。各帧依次往后找，只要有一个频带比噪声低，就认为不是语音
	BiggerthanNoise=0;
	while((start<FrameCount_in-WINDOW_FRAMS+1+FirstVoice_in)&&(BiggerthanNoise==0))
	{
		BiggerthanNoise=1;
		for(ii=0;ii<BAND_NUM;ii++)
		{
			if(BandEnergyBuffer[ii][start%BUFLEN]<NoiseMean[ii])
				BiggerthanNoise=0;
		}//for(ii)

		if(BiggerthanNoise==1)
			break;
		if(BiggerthanNoise==0)
			start++;
	}//while	
	
	SlimBand=-1;
	SlimRatio=1e12;
	StrongBand=-1;
	StrongRatio=0;
	return(start);
}//int CDetectEnergy::RefineStart()


int CDetectEnergy_EPD::DetectRoughEnd(int FrameCount_in)
{
	int ii,jj;
	int AllPulse=0;
	int pulse;
	float tmpThd;

	//除了最弱的1个频带，其余所有的频带在SEG_SILENCE帧中Mean一直小于最小语音的10%。
	for(ii=0;ii<BAND_NUM;ii++)
	{
		if(ii==SlimBand)
			continue;

		tmpThd=0.9*NoiseMean[ii]*(1+NoiseVar[ii])+0.1*SpeechMin[ii];
		pulse=0;
		for(jj=FrameCount_in-SEG_SILENCE;jj<FrameCount_in-ENERGY_FRAMS;jj++)
		{
			if(MeanBuffer[ii][jj%BUFLEN]>tmpThd * g_ENERGY_DECREASE_END)
			{
				pulse=1;
				break;
			}
		}
		if(pulse==0)
			AllPulse++;
	}//for(ii=0;ii<BAND_NUM;ii++)
	if(AllPulse>=BAND_NUM-1)
	{ 
		return (FrameCount_in);
		//return(FrameCount_in-SEG_SILENCE);
	}
	//最强的频带，能量一直小于语音能量的8%
	pulse=0;
	tmpThd=NoiseMean[StrongBand]*(1+NoiseVar[StrongBand])*0.95+0.05*SpeechMin[StrongBand];
	for(jj=FrameCount_in-SEG_SILENCE;jj<FrameCount_in-ENERGY_FRAMS;jj++)
	{
		if(MeanBuffer[StrongBand][jj%BUFLEN]>tmpThd * g_ENERGY_DECREASE_END)
		{
			pulse=1;
			break;
		}
	}
	if(pulse==0)
	{
		return(FrameCount_in);
		//return(FrameCount_in-SEG_SILENCE);
	}
	return(-1);
}//int CDetectEnergy::DetectRoughEnd()


int CDetectEnergy_EPD::StatSpeech(int FrameCount_in)
{
	int ii,jj;
	memset(SpeechMax,0,sizeof(float)*BAND_NUM);
	StrongRatio=0;
	SlimRatio=1e13;

	for(ii=0;ii<BAND_NUM;ii++)
	{
		for(jj=FrameCount_in-WINDOW_FRAMS+1;jj<FrameCount_in-ENERGY_FRAMS;jj++)
		{
			if(MeanBuffer[ii][jj%BUFLEN]>SpeechMax[ii])
			{
				SpeechMax[ii]=MeanBuffer[ii][jj%BUFLEN];
				SpeechMin[ii]=MeanBuffer[ii][(jj-ENERGY_FRAMS)%BUFLEN];
			}
		}
		if(SpeechMax[ii]/NoiseMean[ii]<SlimRatio)//找到语音能量最弱的一个频带 
		{
			SlimRatio=SpeechMax[ii]/NoiseMean[ii];
			SlimBand=ii;
		}

		if(SpeechMax[ii]/NoiseMean[ii]>StrongRatio)//找到语音能量最强的一个频带 
		{
			StrongRatio=SpeechMax[ii]/NoiseMean[ii];
			StrongBand=ii;
		}
	}//for(ii)

	return(1);
}//int StatSpeech()

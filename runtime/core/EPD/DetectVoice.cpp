#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "math.h"
#include "DetectVoice.h"

CDetectVoice_EPD::CDetectVoice_EPD(int SampleRate)
{
	MinBinEnergy=100; //maybe bugs,but debug treat well,I don't know ! ysun 061020
	int FFT_SIZE = (512*SampleRate/8000);
	BinNumber=int(FFT_SIZE/2);
	Resolution=float(SampleRate)/float(FFT_SIZE);	//谱分辨率，单位Hz
	LowPitch=int(LOW_PITCH_HZ/Resolution);	//搜索的最低和最高基音
	HighPitch=int(HIGH_PITCH_HZ/Resolution);
	HighBand=int(HIGH_BAND_HZ/Resolution);	//搜索的最高和最低频率（bin）
	LowBand=LowPitch;
	PitchChange=int(PITCH_CHANGE/Resolution);
	HighestHarmo=int(HighBand/LowPitch+1);

	MaxHarmoMode=HighestHarmo-2;
	PitchNumber=HighPitch-LowPitch+1;
	AllMatchPitch=(short *)calloc(WINDOW_FRAMS*PitchNumber*MaxHarmoMode,sizeof(short));
	nearHarmo=(short *)calloc(HighestHarmo,sizeof(short));

}//CDetectVoice_EPD()

CDetectVoice_EPD::~CDetectVoice_EPD()
{
	free(AllMatchPitch);
	free(nearHarmo);
	
}//~CDetectVoice_EPD()

void CDetectVoice_EPD::ReInit()
{
	int ii;
	memset(AllMax,0,sizeof(int)*MAX_PEAK_NUM*WINDOW_FRAMS);
	memset(AllMaxWidth,0,sizeof(int)*MAX_PEAK_NUM*WINDOW_FRAMS);
	memset(AllMatchPitch,0,sizeof(short)*WINDOW_FRAMS*PitchNumber*MaxHarmoMode);
	for(ii=0;ii<WINDOW_FRAMS;ii++)
	{
		Results[ii]=-1;
		HarmoResult[ii]=-1;
	}
}//ReInit()

void CDetectVoice_EPD::UpdateBuf()//更新谐波检测的内部缓存
{
	memmove(&Results[0],&Results[1],sizeof(int)*(WINDOW_FRAMS-1));
	memmove(&HarmoResult[0],&HarmoResult[1],sizeof(short)*(WINDOW_FRAMS-1));
	memmove(&AllMax[0][0],&AllMax[1][0],sizeof(int)*(WINDOW_FRAMS-1)*MAX_PEAK_NUM);
	memmove(&AllMaxWidth[0][0],&AllMaxWidth[1][0],sizeof(int)*(WINDOW_FRAMS-1)*MAX_PEAK_NUM);
	memmove(&AllMatchPitch[0],&AllMatchPitch[PitchNumber*MaxHarmoMode],sizeof(short)*(WINDOW_FRAMS-1)*PitchNumber*MaxHarmoMode);
	memset(&AllMax[WINDOW_FRAMS-1][0],0,sizeof(int)*MAX_PEAK_NUM);
	memmove(&StrongMax[0][0],&StrongMax[1][0],sizeof(int)*(WINDOW_FRAMS-1)*6);
	Results[WINDOW_FRAMS-1]=-1;
	HarmoResult[WINDOW_FRAMS-1]=-1;
	int ii;
	for(ii=0;ii<PitchNumber*MaxHarmoMode;ii++)
		AllMatchPitch[(WINDOW_FRAMS-1)*PitchNumber*MaxHarmoMode+ii]=-1;

}//void UpdateBuf()


//对BinEnergy求局部极值
int CDetectVoice_EPD::GetMax()
{
	int ii,jj,idx,min_idx;
	float minEnergy;
	int min[MAX_PEAK_NUM][2];
	int max2[MAX_PEAK_NUM];
	int min2[MAX_PEAK_NUM*3];
	memset(StrongPoints,0,sizeof(int)*6);

///////////////////基本设置/////////////////////
	memset(max,0,sizeof(int)*MAX_PEAK_NUM);
	memset(MaxWidth,0,sizeof(int)*MAX_PEAK_NUM);
	minEnergy=BinEnergy[LowBand];
	
	for(ii=LowBand;ii<HighBand;ii++)
	{
		if(BinEnergy[ii]<minEnergy)
			minEnergy=BinEnergy[ii];
	}
	if(minEnergy<MinBinEnergy)
		minEnergy=MinBinEnergy;
	minEnergy*=MIN_HARMO_SCALE;


////////////////////初步搜索极大极小值并计算对应性质///////////////////
	//找频谱的极大值
	memset(max2,0,sizeof(int)*MAX_PEAK_NUM);
	idx=0;
	for(ii=LowBand;ii<HighBand-1;ii++)
	{
		if((BinEnergy[ii]>BinEnergy[ii-1])&&(BinEnergy[ii]>BinEnergy[ii+1])&&(BinEnergy[ii]>minEnergy))
		{
			max2[idx]=ii;
			idx++;
			if(idx>=MAX_PEAK_NUM)	//最多记录MAX_PEAK_NUM个极小值点，极大点最多MAX_PEAK_NUM个。
				break;
		}//if()
	}//for(ii)

	memset(min2,0,sizeof(int)*MAX_PEAK_NUM*3);
	min_idx=1;
	min2[0]=LowBand;
	for(ii=LowBand+1;ii<HighBand-1;ii++)
	{
		if(((BinEnergy[ii]<=BinEnergy[ii-1])&&(BinEnergy[ii]<BinEnergy[ii+1]))
			||((BinEnergy[ii]<=BinEnergy[ii-1])&&(BinEnergy[ii]<BinEnergy[ii+1])))
		{
			min2[min_idx]=ii;
			min_idx++;
			if(min_idx>=MAX_PEAK_NUM*3)	//最多记录MAX_PEAK_NUM*3个极小值点，极大点最多MAX_PEAK_NUM个。
				break;
		}//if()
	}//for(ii)
	if(min_idx<MAX_PEAK_NUM*3)//一般认为极小值点个数不超过这个范围
	{
		min2[min_idx]=HighBand-1;
		min_idx++;//min2共有min_idx个，编号为0~min_idx-1。其中第0个为LowBand，第min_idx个为HighBand-1
	}//if(min_idx<MAX_PEAK_NUM*3)


	//找极大点左右的极小点
	memset(min,0,sizeof(int)*MAX_PEAK_NUM*2);
	for(ii=0;ii<idx;ii++)//max2一共有idx个，编号为0~idx-1
	{
		jj=0;
		while((min2[jj]<max2[ii])&&(jj<MAX_PEAK_NUM*3))
			jj++;
		if(jj>0)
			jj--;
		min[ii][0]=min2[jj];//左边的紧邻极小点

		jj=min_idx-1;
		while((min2[jj]>max2[ii])&&(jj>=0))
			jj--;
		if(jj<min_idx-1)
			jj++;
		min[ii][1]=min2[jj];//右边的紧邻极小点

	}//for(ii=0;ii<idx;ii++)

	//计算各个极大值对应的当前宽度（各主峰单独的宽度）
	int Width[MAX_PEAK_NUM];
	memset(Width,0,sizeof(int)*MAX_PEAK_NUM);	
	for(ii=0;ii<idx;ii++)
		Width[ii]=min[ii][1]-min[ii][0];


	//把极大值的峰扩大到合适的范围，把主峰强调出来。即把主峰加宽，使其包含副峰。
	for(ii=0;ii<idx;ii++)
	{
		jj=ii-1;
		while((jj>-1)&&(Width[ii]>Width[jj])&&(BinEnergy[max2[ii]]>BinEnergy[max2[jj]])
			&&(BinEnergy[min[ii][0]]>BinEnergy[min[jj][1]])&&(BinEnergy[min[ii][0]]>BinEnergy[min[jj][0]]))
			jj--;
		min[ii][0]=min[jj+1][0];

		jj=ii+1;
		while((jj<idx)&&(Width[ii]>Width[jj])&&(BinEnergy[max2[ii]]>BinEnergy[max2[jj]])
			&&(BinEnergy[min[ii][1]]>BinEnergy[min[jj][1]])&&(BinEnergy[min[ii][1]]>BinEnergy[min[jj][0]]))
			jj++;
		min[ii][1]=min[jj-1][1];

//		if((abs(max2[ii]-(min[ii][1]+min[ii][0])/2))>=2)
//			max2[ii]=(min[ii][1]+min[ii][0])/2;
	}//for(ii=0;ii<idx;ii++)

	for(ii=0;ii<idx;ii++)//重新计算各个峰的宽度
		Width[ii]=min[ii][1]-min[ii][0];

	//计算各个峰值的高度
	float Height[MAX_PEAK_NUM];
	memset(Height,0,sizeof(float)*MAX_PEAK_NUM);
	for(ii=0;ii<idx;ii++)
	{
		if(max2[ii]>0)
		{
			if(BinEnergy[min[ii][0]]<BinEnergy[min[ii][1]])
				Height[ii]=BinEnergy[max2[ii]]-BinEnergy[min[ii][0]];
			else 
				Height[ii]=BinEnergy[max2[ii]]-BinEnergy[min[ii][1]];
		}
	}//for(ii=0;ii<idx;ii++)

	//保留最高和最宽的各3个峰
	int tmpWidth;		
	float tmpHeight;	
	int tmpIdx;
	memset(StrongPoints,0,sizeof(int)*6);
	for(ii=0;ii<3;ii++)
	{
		tmpWidth=0;
		tmpIdx=0;
		for(jj=0;jj<idx;jj++)
		{
			if((Width[jj]>tmpWidth)&&(max2[jj]>0))//宽度相等的peak，优先保留低频的。
				if((StrongPoints[0]!=max2[jj])&&(StrongPoints[1]!=max2[jj])&&(StrongPoints[2]!=max2[jj]))
					if((StrongPoints[3]!=max2[jj])&&(StrongPoints[4]!=max2[jj])&&(StrongPoints[5]!=max2[jj]))
					{
						tmpWidth=Width[jj];
						tmpIdx=max2[jj];
					}
		}
		if(tmpWidth>0)
			StrongPoints[ii+3]=tmpIdx;

		tmpHeight=0;
		tmpIdx=0;
		for(jj=0;jj<idx;jj++)
		{
			if((Height[jj]>tmpHeight)&&(max2[jj]>0))
				if((StrongPoints[0]!=max2[jj])&&(StrongPoints[1]!=max2[jj])&&(StrongPoints[2]!=max2[jj]))
					if((StrongPoints[3]!=max2[jj])&&(StrongPoints[4]!=max2[jj])&&(StrongPoints[5]!=max2[jj]))
					{
						tmpHeight=Height[jj];
						tmpIdx=max2[jj];
					}
		}
		if(tmpHeight>0)
			StrongPoints[ii]=tmpIdx;
	}//for(ii=0;ii<3;ii++)


	for(ii=0;ii<idx-1;ii++)//宽度很小，距离很接近，能量很接近 
	{
		if((Width[ii]<=4)&&(Width[ii+1]<=3)&&(Width[ii]+Width[ii+1]<=7))
			if((BinEnergy[max2[ii]]<BinEnergy[max2[ii+1]]*2)&&(BinEnergy[max2[ii+1]]<BinEnergy[max2[ii]]*2))
				max2[ii]=max2[ii+1]=0;
	}
	//删除那些宽度很小，且能量不大的极值点。
	for(ii=1;ii<idx-1;ii++)
	{
		if((Width[ii]<=4)&&(max2[ii]>0))//宽度很小，而且位于一个谷底（两边的极小值都比这个peak的极小值大）。且比旁边的一个peak低
			if((BinEnergy[max2[ii]]<BinEnergy[max2[ii+1]])&&((BinEnergy[max2[ii]]<BinEnergy[min[ii+1][0]])||(BinEnergy[max2[ii]]<BinEnergy[min[ii+1][1]])))
				if((BinEnergy[max2[ii]]<BinEnergy[max2[ii-1]])&&((BinEnergy[max2[ii]]<BinEnergy[min[ii-1][0]])||(BinEnergy[max2[ii]]<BinEnergy[min[ii-1][1]])))
					max2[ii]=0;
	}

/////////////////////////////////选择保留有效极点/////////////
	int MAX_LAST_PK=10;
	float height_thd;
	for(ii=0;ii<MAX_LAST_PK;ii++)
	{
		tmpWidth=0;
		tmpIdx=0;
		for(jj=0;jj<idx;jj++)
		{
			if((Width[jj]>tmpWidth)&&(max2[jj]>0))
			{
				tmpWidth=Width[jj];
				tmpIdx=jj;
			}
		}//for(jj=0;jj<idx;jj++)
		if(ii==0)
			height_thd=Height[tmpIdx];
		if((Height[tmpIdx]>0)&&(Height[tmpIdx]<height_thd)&&(ii<4))//对宽度最大的4个，取其高度的最低值，其它极值点只要高度大于它，就可以保留。
			height_thd=Height[tmpIdx];

		if(tmpWidth>0)
		{
			max[ii]=max2[tmpIdx];
			MaxWidth[ii]=tmpWidth;
			max2[tmpIdx]=0;
		}//if(tmpWidth>0)
		else 
			break;
	}//for(ii=0;ii<MAX_LAST_PK;ii++)

	ii=0;
	while((max[ii]>0)&&(ii+1<MAX_PEAK_NUM))
		ii++;		//max记录了ii个极点。
	
	for(jj=0;jj<idx;jj++)
	{
		if((Width[jj]>=5)&&(max2[jj]>0))//把宽度大于5的peak也保存下来
		{
			max[ii]=max2[jj];
			max2[jj]=0;
			MaxWidth[ii]=Width[jj];
			ii++;
		}

		if((Height[jj]>=height_thd)&&(max2[jj]>0)&&(max2[jj]!=max[ii-1]))//高度大于前4宽度的极值点，也保留。
		{
			max[ii]=max2[jj];
			max2[jj]=0;
			MaxWidth[ii]=Width[jj];
			ii++;
		}
	}//for(jj=0;jj<idx;jj++)

	//根据频率高低排序，保留极值点
	idx=ii;		//ii现在记录了max中的记录个数。
	int tmp;
	for(ii=0;ii<idx;ii++)
		for(jj=ii+1;jj<idx;jj++)
		{
			if(max[jj]<max[ii])
			{
				tmp=max[ii];
				max[ii]=max[jj];
				max[jj]=tmp;

				tmpWidth=MaxWidth[ii];
				MaxWidth[ii]=MaxWidth[jj];
				MaxWidth[jj]=tmpWidth;
			}
		}
	return(0);

}//void GetMax()


int CDetectVoice_EPD::DetectVoice(float *AllBinEnergy_in, int VoiceNumber_in)
{
	int VoiceNum;
	int Count;
	
	for(Count=0;Count<WINDOW_FRAMS;Count++)
	{
		if(AllMax[Count][0]>0)
			continue;
		BinEnergy=&AllBinEnergy_in[Count*BinNumber];
		max=&AllMax[Count][0];
		MaxWidth=&AllMaxWidth[Count][0];//窗内每个极点的对应强度（此处为peak的宽度）
		StrongPoints=&StrongMax[Count][0];//最高和最宽的各3个极点。
		GetMax();

		FrameMatchPitch=&AllMatchPitch[Count*PitchNumber*MaxHarmoMode];
		MatchPitch();
	}
	
	VoiceNum=Find(VoiceNumber_in);

	return(VoiceNum);
}//int detect(short *data, int WavLength, short *voice_out)
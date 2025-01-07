#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "math.h"
#include "DetectVoice.h"
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#define min(a,b)  (((a) < (b)) ? (a) : (b))

extern int g_MIN_HARMO_LEN_EPD;
int CDetectVoice_EPD::Find(int VoiceNumberRequest)
{
	int pitch0,pitch1,pitch2,MaxPitch1,MaxPitch2,MinPitch1,MinPitch2;
	short *MatchPitch0,*MatchPitch1,*MatchPitch2;
	short *MatchPitch0Mode,*MatchPitch1Mode,*MatchPitch2Mode;
	short TestHarmoMode;
	short getHarmo0,getHarmo1,getHarmo2;
	short FwdGetHarmo,BkwdGetHarmo;//前后连接的结果，是否可连接
	int FwdFrame,BkwdFrame;	//前后连接的帧号
	int better;
	int HighMode;//对某个基频，最高可能的谐波匹配模式。
	int maxPitch;//连接的3个基频中，最低的1个基频。
	int ii,jj;
	
	for(Frame=0;Frame<WINDOW_FRAMS-2;Frame++)
	{
		if(HarmoResult[Frame]==1)	//如果已经完全匹配和连接过了，则不再重复
			continue;
		MatchPitch0=&AllMatchPitch[Frame*PitchNumber*MaxHarmoMode];
		MatchPitch1=&AllMatchPitch[(Frame+1)*PitchNumber*MaxHarmoMode];
		MatchPitch2=&AllMatchPitch[(Frame+2)*PitchNumber*MaxHarmoMode];

		memset(PartHarmo,0,sizeof(int)*3*4);
		better=0;
		for(pitch0=LowPitch;pitch0<=HighPitch;pitch0++)
		{
			if(MatchPitch0[(pitch0-LowPitch)*MaxHarmoMode]==-100)
				continue;
			MaxPitch1=min(pitch0+PitchChange,HighPitch);
			MinPitch1=max(pitch0-PitchChange,LowPitch);
			for(pitch1=MinPitch1; pitch1<=MaxPitch1;pitch1++)
			{
				if(MatchPitch1[(pitch1-LowPitch)*MaxHarmoMode]==-100)
					continue;
				MinPitch2=max(pitch1-PitchChange,LowPitch);
				MaxPitch2=min(pitch1+PitchChange,HighPitch);
				for(pitch2=MinPitch2;pitch2<=MaxPitch2;pitch2++)
				{
					if(MatchPitch2[(pitch2-LowPitch)*MaxHarmoMode]==-100)
						continue;
					maxPitch=max(pitch0,pitch1);
					maxPitch=max(pitch2,maxPitch);
					HighMode=HighBand/maxPitch-4;//因为连接的模式其实由每帧决定，所以反倒是最大的基频决定了最高可连的模式。
					MatchPitch0Mode=&MatchPitch0[(pitch0-LowPitch)*MaxHarmoMode];
					MatchPitch1Mode=&MatchPitch1[(pitch1-LowPitch)*MaxHarmoMode];
					MatchPitch2Mode=&MatchPitch2[(pitch2-LowPitch)*MaxHarmoMode];
					//核心处理，匹配3帧的谐波
					for(TestHarmoMode=0;TestHarmoMode<HighMode;TestHarmoMode++)
					{
						if(MatchPitch0Mode[TestHarmoMode]!=1)
							continue;
						if(MatchPitch1Mode[TestHarmoMode]!=1)
							continue;
						if(MatchPitch2Mode[TestHarmoMode]!=1)
							continue;

						//检验这3帧是否符合对应的pitch的对应次数的谐波。条件：均匀等距离，且宽度达到较高要求
						getHarmo0=SearchHarmo(pitch0,TestHarmoMode,Frame, 1);
						if(getHarmo0<0)
							continue;
						getHarmo1=SearchHarmo(pitch1,TestHarmoMode,Frame+1,1);
						if(getHarmo1<0)
							continue;
						getHarmo2=SearchHarmo(pitch2,TestHarmoMode,Frame+2,1);
						if(getHarmo2<0)
							continue;
						
						int ContinueTest=0;//这里是因为，不同的基频，匹配出来的谐波可能完全一样，不需要再搜索和连接一次了。
						for(ii=0;(ii<3)&&(ContinueTest==0);ii++)
							for(jj=0;(jj<4)&&(ContinueTest==0);jj++)
							{
								if(Harmo[Frame+ii][jj]!=PartHarmo[ii][jj])//只要找到一个不一样的，就说明这3帧谐波还没检测过，可以继续检测
									ContinueTest=1;
							}//for(ii,jj)
						if(ContinueTest==0)
							continue;

						memmove(PartHarmo,&Harmo[Frame][0],sizeof(int)*3*4);

/////////////////////////////////////////////前后连接谐波//////////////////////////////////
						FwdFrame=Frame;//可以连接上的最前面一帧
						BkwdFrame=Frame+2;//可以连接上的最后面一帧
						//第0,1,2帧都均匀，前后连接
						if(TestHarmoMode==0)//连接1，2，3谐波
						{
							FwdGetHarmo=3;
							while((FwdGetHarmo>0)&&(FwdFrame-1>=0))//向前判断是否有均匀3条谐波
							{
								FwdGetHarmo=Connect3(AllMax[FwdFrame-1],FwdFrame-1, FwdFrame);
								if(FwdGetHarmo>0)
									FwdFrame--;
							}//while((FwdGetHarmo==1)&&(FwdFrame>=0))
							BkwdGetHarmo=3;
							while((BkwdGetHarmo>0)&&(BkwdFrame+1<WINDOW_FRAMS))//向后判断是否有均匀3条谐波
							{
								BkwdGetHarmo=Connect3(AllMax[BkwdFrame+1],BkwdFrame+1, BkwdFrame);
								if(BkwdGetHarmo>0)
									BkwdFrame++;
							}//while((FwdGetHarmo==1)&&(FwdFrame>=0))
						}//if(TestHarmoMode==0)

						if(TestHarmoMode>0)//连接高次谐波
						{
							FwdGetHarmo=4;
							while((FwdGetHarmo>0)&&(FwdFrame-1>=0))//向前判断是否有均匀4条谐波
							{
								FwdGetHarmo=Connect4(AllMax[FwdFrame-1],FwdFrame-1, FwdFrame,TestHarmoMode);
								if(FwdGetHarmo>0)
									FwdFrame--;
							}//while((FwdGetHarmo==1)&&(FwdFrame>=0))
							BkwdGetHarmo=4;
							while((BkwdGetHarmo>0)&&(BkwdFrame+1<WINDOW_FRAMS))//向后判断是否有均匀4条谐波
							{
								BkwdGetHarmo=Connect4(AllMax[BkwdFrame+1],BkwdFrame+1, BkwdFrame,TestHarmoMode);
								if(BkwdGetHarmo>0)
									BkwdFrame++;
							}//while((FwdGetHarmo==1)&&(FwdFrame>=0))
						}//if(TestHarmoMode>0)

//////////////////////////////////连接完毕，判断是否浊音/////////////////////////////////////////
						if(BkwdFrame<WINDOW_FRAMS-1)//如果连接完毕，可以做判断
						{
							if(BkwdFrame-FwdFrame+1>=g_MIN_HARMO_LEN_EPD)//如果够长
							{	
								HarmoResult[Frame]=1;//这一帧匹配完成了
								if(TestHarmoMode==0)
								{
									if(Results[ii]==-1)
										Results[ii]=5;
									if(CountVoice()>=VoiceNumberRequest)
										return(1);
								}//if(TestHarmoMode==0)
								if(TestHarmoMode>0)                                 
								{
									for(ii=FwdFrame;ii<=BkwdFrame;ii++)
										Results[ii]=15;
									if(CountVoice()>=VoiceNumberRequest)
										return(1);
								}//if(TestHarmoMode>0)
							}//if(BkwdFrame-FwdFrame+1>=MIN_HARMO_LEN)//如果够长
							else //如果不够长
							{
								if((HarmoResult[Frame]==2)&&(better==0))//如果把上次没连接完的连好了，且没发现更长的谐波
									HarmoResult[Frame]=-1;//就当这次没有连上
							}
						}//if(BkwdFrame<WINDOW_FRAMS-1)//连接完毕
		
						if(BkwdFrame==WINDOW_FRAMS-1)//如果连接未完，暂不做判决
						{
							if(BkwdFrame-FwdFrame+1>=g_MIN_HARMO_LEN_EPD)//已经有了足够长度的谐波,可以确认这几帧肯定是谐波
							{
								for(ii=FwdFrame;ii<=BkwdFrame;ii++)
									Results[ii]=5;
							}
							if(CountVoice()>=VoiceNumberRequest)
								return(1);
							HarmoResult[Frame]=2;//表示可以继续等待匹配.
							if(better<BkwdFrame-FwdFrame+1)
								better=BkwdFrame-FwdFrame+1;
						}//if(BkwdFrame==WINDOW_FRAMS-1)
					}//for(TestHarmoMode=0;TestHarmoMode<MaxHarmoMode;TestHarmoMode++)
				}//for(pitch2)
			}//for(pitch1)
		}//for(pitch0)

		if(HarmoResult[Frame]==-1)//如果匹配测试完毕，且从未匹配到，则标记不需要再做测试了。
			HarmoResult[Frame]=1;//已经匹配完毕了

	}//for(Frame)

	return(0);
}//int Find()

//这里肯定会找到谐波，只是找其中最平均的而已。
//找到谐波后，检验其是否符合谐波宽度要求。WidthThdSetting为1，则要求谐波宽度较大，为0则不要求宽度太大。
//谐波宽度不够时，返-1，否则返回谐波总宽度。
int CDetectVoice_EPD::SearchHarmo(int pitch_in,short Mode_in, int FrameNo, int WidthThdSetting)
{
	int Indx[4];
	int even=3;
	int tmp_even;
	int testmax[4],mode[4];//用于调用函数，测试极值均匀程度
	int ii;
	int *max_in;
	int peak_number;
	
	int width[4];
	int *width_in;
	int width_thd;
	int count;
	float SettingAll,SettingBig;
	int TotalWidth;

	//设定对谐波宽度的要求
	if(WidthThdSetting==1)
	{	SettingAll=0.5;	SettingBig=0.9;	}
	else if(WidthThdSetting==0)
	{	SettingAll=0.3;	SettingBig=0.7;	}
	
	if(pitch_in>10)
		width_thd=int(pitch_in*SettingAll);
	else width_thd=int(pitch_in*SettingBig);

	width_in=&AllMaxWidth[FrameNo][0];
	memset(Harmo[FrameNo],0, sizeof(int)*4);

	max_in=&AllMax[FrameNo][0];//对应帧的极值点
	peak_number=0;//实际极值点的个数
	while((max_in[peak_number]>0)&&(peak_number<MAX_PEAK_NUM))
		peak_number++;

	///////////////////////////////////1，2，3次谐波(基频和相邻两条谐波)////
	if(Mode_in==0)//匹配1，2，3次谐波即可。Indx[0]无用。
	{
		mode[0]=0;	mode[1]=1; mode[2]=2; mode[3]=3;
		memset(Indx,0,sizeof(int)*4);
		Harmo[FrameNo][0]=0;
		testmax[0]=0;
		for(Indx[1]=0;(Indx[1]<peak_number-2)&&(max_in[Indx[1]]<pitch_in+1.01);Indx[1]++)
		{
			if(max_in[Indx[1]]<pitch_in-1)
				continue;
			
			for(Indx[2]=Indx[1]+1;(Indx[2]<peak_number-1)&&(max_in[Indx[2]]<pitch_in+1.01+max_in[Indx[1]]);Indx[2]++)
			{
				if(max_in[Indx[2]]<pitch_in-1+max_in[Indx[1]])
					continue;
				
				for(Indx[3]=Indx[2]+1;(Indx[3]<peak_number)&&(max_in[Indx[3]]<pitch_in+1.01+max_in[Indx[2]]);Indx[3]++)
				{
					if(max_in[Indx[3]]<pitch_in-1+max_in[Indx[2]])
						continue;
					
					for(ii=1;ii<4;ii++)
						testmax[ii]=max_in[Indx[ii]];
			
					tmp_even=TestEven(testmax,mode);
					if(tmp_even<even)
					{
						even=tmp_even;
						memmove(Harmo[FrameNo],testmax, sizeof(int)*4);
					}
				}//for(Indx[3])
			}//for(Indx[2])
		}//for(Indx[1])
	
		GetHarmoWidth(Harmo[FrameNo],max_in, width_in,width);
		if((width[1]<width_thd)&&(width[2]<width_thd)&&(width[3]<width_thd))
			return(-1);

		count=0;
		for(ii=1;ii<4;ii++)
		{
			if(width[ii]>=width_thd)
				count++;
		}
		if((pitch_in>=10)&&(count<1))
			return(-1);
		if((pitch_in<10)&&(count<3))
			return(-1);
		
		TotalWidth=width[1]+width[2]+width[3];
	}//if(Mode_in==0)
				

	/////////////////////////////4条高次谐波//////////////////
	if(Mode_in>0)		//对应第Mode_in+1，+2，+3，+4条谐波
	{
		mode[0]=Mode_in+1;	mode[1]=Mode_in+2; mode[2]=Mode_in+3; mode[3]=Mode_in+4;

		for(Indx[0]=0;(Indx[0]<peak_number-3)&&(max_in[Indx[0]]<(pitch_in+1)*mode[0]+0.01);Indx[0]++)
		{
			if(max_in[Indx[0]]<pitch_in*mode[0]-1)
				continue;
			
			for(Indx[1]=Indx[0]+1;(Indx[1]<peak_number-2)&&(max_in[Indx[1]]<pitch_in+1.01+max_in[Indx[0]]);Indx[1]++)
			{
				if(max_in[Indx[1]]<pitch_in-1+max_in[Indx[0]])
					continue;

				for(Indx[2]=Indx[1]+1;(Indx[2]<peak_number-1)&&(max_in[Indx[2]]<pitch_in+1.01+max_in[Indx[1]]);Indx[2]++)
				{
					if(max_in[Indx[2]]<pitch_in-1+max_in[Indx[1]])
						continue;

					for(Indx[3]=Indx[2]+1;(Indx[3]<peak_number)&&(max_in[Indx[3]]<pitch_in+1.01+max_in[Indx[2]]);Indx[3]++)
					{
						if(max_in[Indx[3]]<pitch_in-1+max_in[Indx[2]])
							continue;

						for(ii=0;ii<4;ii++)
							testmax[ii]=max_in[Indx[ii]];
						tmp_even=TestEven(testmax,mode);
						if(tmp_even<even)
						{
							even=tmp_even;
							memmove(Harmo[FrameNo],testmax, sizeof(int)*4);
						}//if(tmp_even<even)
					}//for(Indx[3])
				}//for(Indx[2])
			}//for(Indx[1])
		}//for(Indx[0])

		GetHarmoWidth(Harmo[FrameNo],max_in, width_in,width);
		if((width[0]<width_thd)&&(width[1]<width_thd)&&(width[2]<width_thd)&&(width[3]<width_thd))
			return(-1);
		
		count=0;
		for(ii=0;ii<4;ii++)
		{
			if(width[ii]>=width_thd)
				count++;
		}
		if((pitch_in>=10)&&(count<2))
			return(-1);
		if((pitch_in<10)&&(count<3))
			return(-1);
		TotalWidth=width[0]+width[1]+width[2]+width[3];
	}//if(Mode_in>0)

	return(TotalWidth);
}// int SearchHarmo(int pitch_in,short Mode_in)

 
//从第FrameToDo帧的极值maxToDo中，选择一些值与FrameNowHarmo连接，连接的模式是pitch的1，2，3倍，即Harmo[FrameNow][3]肯定为0
int CDetectVoice_EPD::Connect3(int *maxToDo, int FrameToDo, int FrameNow)
{
	int *HarmoNow;//只连接pitch的1，2，3倍，对应HarmoNow[1]~HarmoNow[3]
	int *HarmoToDo;	//要连接的结果
	int Indx[4];	//
	int mode[4];
	float tmpPitch;
	int intPitch, MaxIntPitch, MinIntPitch;
	int blankHarmo;
	int EveryWidth[4];
	int MinWidthIdx;
	short MatchPitchMode;
	int ii;
	int allWidth,MaxWidth = -1;   // 080228 syq fix bug found by vs2005 Run-Time Check Failure
	int HarmoTest[4];
	int stat_peak_number, peak_number;

	HarmoToDo=Harmo[FrameToDo];
	memset(HarmoToDo,0,sizeof(int)*4);	//把要连接帧的谐波信息都清零
	HarmoNow=Harmo[FrameNow];
	
	mode[0]=0;mode[1]=1;mode[2]=2;mode[3]=3;

	if((HarmoNow[1]>0)&&(HarmoNow[2]>0)&&(HarmoNow[3]>0))//连接3条，如果能够全连上，则只保留最宽的
	{
		MinIntPitch=max(HarmoNow[1]-PitchChange,LowPitch);
		MaxIntPitch=min(HarmoNow[1]+PitchChange,HighPitch);
		MaxWidth=-1;
		memset(HarmoTest,0,sizeof(int)*4);
		for(intPitch=MinIntPitch;intPitch<=MaxIntPitch;intPitch++)
		{
			MatchPitchMode=AllMatchPitch[(FrameToDo*PitchNumber+intPitch-LowPitch)*MaxHarmoMode];
			if(MatchPitchMode>0)
			{
				allWidth=SearchHarmo(intPitch, 0, FrameToDo,0);//用较低的阈值检测连接出的谐波是否够宽
				if(allWidth<0)//检验谐波宽度是否足够
				{
					memmove(HarmoToDo,HarmoTest,sizeof(int)*4);
					continue;
				}//if(allWidth<0)
				if(allWidth>MaxWidth)//如果有更宽的谐波，则替代掉原来的
				{
					MaxWidth=allWidth;
					memmove(HarmoTest,HarmoToDo,sizeof(int)*4);
				}//if(allWidth>MaxWidth)
				else
					memmove(HarmoToDo,HarmoTest,sizeof(int)*4);
			}//if(MatchPitchMode>0)
		}//for(intPitch)
	}//if((HarmoNow[1]>0)&&(HarmoNow[2]>0)&&(HarmoNow[3]>0))

	if(MaxWidth>0)
		return(3);
	else
	{
		MaxWidth=GetHarmoWidth(HarmoNow, AllMax[FrameNow], AllMaxWidth[FrameNow], EveryWidth);
		MinWidthIdx=1;
		if(EveryWidth[2]<EveryWidth[MinWidthIdx])
			MinWidthIdx=2;
		if(EveryWidth[3]<EveryWidth[MinWidthIdx])
			MinWidthIdx=3;
		HarmoNow[MinWidthIdx]=0;
	}//else
	
	if((HarmoNow[1]==0)||(HarmoNow[2]==0)||(HarmoNow[3]==0))//只连其中两根
	{
		int maxTest[MAX_PEAK_NUM];
		int maxAdd;
		int maxWidthTest[MAX_PEAK_NUM];
		int tmp_even,even;

		stat_peak_number=0;//实际极值点的个数
		while((AllMaxWidth[FrameToDo][stat_peak_number]>0)&&(stat_peak_number<MAX_PEAK_NUM))
			stat_peak_number++;

		blankHarmo=1;
		while(HarmoNow[blankHarmo]>0)
			blankHarmo++;
		tmpPitch=(float(HarmoNow[1])+float(HarmoNow[2])/2+float(HarmoNow[3])/3)/2;

		even=min(int(tmpPitch*0.3), 3);
		MaxIntPitch=min(int(tmpPitch)+PitchChange,HighPitch);
		MinIntPitch=max(int(tmpPitch)-PitchChange,LowPitch);
		for(intPitch=MinIntPitch;intPitch<=MaxIntPitch;intPitch++)
		{
			maxAdd=int(tmpPitch*blankHarmo);
			ii=0;
			while((maxToDo[ii]<maxAdd)&&(ii<MAX_PEAK_NUM-1)&&(maxToDo[ii+1]>0))
				ii++;
			
			if(maxToDo[ii]==maxAdd)//如果存在这个极值，就不需要再插入此值了
			{
				memmove(maxTest,maxToDo,sizeof(int)*MAX_PEAK_NUM);	
				memmove(maxWidthTest,AllMaxWidth[FrameToDo],sizeof(int)*MAX_PEAK_NUM);
				peak_number=stat_peak_number;
			}
			else
			{	//插入一个极值点
				memmove(maxTest,maxToDo,sizeof(int)*ii);
				maxTest[ii]=maxAdd;
				memmove(&maxTest[ii+1],&maxToDo[ii],sizeof(int)*(MAX_PEAK_NUM-ii-1));
				memmove(maxWidthTest, AllMaxWidth[FrameToDo], sizeof(int)*ii);
				maxWidthTest[ii]=1;
				memmove(&maxWidthTest[ii+1], &AllMaxWidth[FrameToDo][ii], sizeof(int)*(MAX_PEAK_NUM-ii-1));
				peak_number=stat_peak_number+1;
			}//else

			for(Indx[1]=0;(Indx[1]<peak_number-2)&&(maxTest[Indx[1]]<intPitch+1.01);Indx[1]++)
			{
				if(maxTest[Indx[1]]<intPitch-1)
					continue;
				
				for(Indx[2]=Indx[1]+1;(Indx[2]<peak_number-1)&&(maxTest[Indx[2]]<maxTest[Indx[1]]+intPitch+1.01);Indx[2]++)
				{
					if(maxTest[Indx[2]]<maxTest[Indx[1]]+intPitch-1)
						continue;
					
					for(Indx[3]=Indx[2]+1;(Indx[3]<peak_number)&&(maxTest[Indx[3]]<maxTest[Indx[2]]+intPitch+1.01);Indx[3]++)
					{
						if(maxTest[Indx[3]]<maxTest[Indx[2]]+intPitch-1)
							continue;

						HarmoTest[1]=maxTest[Indx[1]];
						HarmoTest[2]=maxTest[Indx[2]];
						HarmoTest[3]=maxTest[Indx[3]];
						
						MaxWidth=GetHarmoWidth(HarmoTest, maxTest, maxWidthTest, EveryWidth);
						if(MaxWidth<intPitch/2)
							continue;
						tmp_even=TestEven(HarmoTest,mode);
						if(tmp_even<even)
						{
							even=tmp_even;
							HarmoToDo[1]=HarmoTest[1];
							HarmoToDo[2]=HarmoTest[2];
							HarmoToDo[3]=HarmoTest[3];
						}//if(tmp_even<even)
					}//for(Indx[3])
				}//for(Indx[2])
			}//for(Indx[1])
		}//for(intPitch)
	HarmoToDo[blankHarmo]=0;
	}//if((HarmoNow[1]==0)||(HarmoNow[2]==0)||(HarmoNow[3]==0))
	
	if((HarmoToDo[1]>0)||(HarmoToDo[2]>0)||(HarmoToDo[3]>0))
		return(2);
	else return(-1);

}//int Connect3()

//从第FrameToDo帧的极值maxToDo中，选择一些值与FrameNowHarmo连接
//连接的模式是pitch的Mode+1，Mode+2，Mode+3，Mode+4倍
int CDetectVoice_EPD::Connect4(int *maxToDo, int FrameToDo, int FrameNow, short Mode)
{
	int *HarmoNow;//只连接pitch的1，2，3倍，对应HarmoNow[1]~HarmoNow[3]
	int *HarmoToDo;	//要连接的结果
	int HarmoTest[4];
	int Indx[4];	//
	int mode[4];

	short MatchPitchMode;
	int ii;
	int allWidth,MaxWidth = -1;   // 080228 syq fix bug found by vs2005 Run-Time Check Failure
	
	float tmpPitch=0;
	int intPitch, MaxIntPitch, MinIntPitch;
	int blankHarmo;
	int tmp_even;
	int even=3;
	int EveryWidth[4];
	int MinWidthIdx;

	HarmoToDo=Harmo[FrameToDo];
	memset(HarmoToDo,0,sizeof(int)*4);	//把要连接帧的谐波信息都清零
	HarmoNow=Harmo[FrameNow];
	mode[0]=Mode+1;mode[1]=Mode+2;mode[2]=Mode+3;mode[3]=Mode+4;

	if((HarmoNow[0]>0)&&(HarmoNow[1]>0)&&(HarmoNow[2]>0)&&(HarmoNow[3]>0))
	{
		tmpPitch=float(HarmoNow[0]+HarmoNow[1]+HarmoNow[2]+HarmoNow[3])/(Mode*4+10);
		MaxWidth=-1;
		memset(HarmoTest,0,sizeof(int)*4);
		MinIntPitch=max(int(tmpPitch)-PitchChange,LowPitch);
		MaxIntPitch=min(int(tmpPitch)+PitchChange,HighPitch);
	
		for(intPitch=MinIntPitch;intPitch<=MaxIntPitch;intPitch++)
		{
			MatchPitchMode=AllMatchPitch[(FrameToDo*PitchNumber+intPitch-LowPitch)*MaxHarmoMode+Mode];
			if(MatchPitchMode>0)
			{
				allWidth=SearchHarmo(intPitch, Mode, FrameToDo,0);//用较低的阈值检测连接出的谐波是否够宽
				if(allWidth<0)//检验谐波宽度是否足够
				{
					memmove(HarmoToDo,HarmoTest,sizeof(int)*4);
					continue;
				}//if(allWidth<0)
				if(allWidth>MaxWidth)//如果有更宽的谐波，则替代掉原来的
				{
					MaxWidth=allWidth;
					memmove(HarmoTest,HarmoToDo,sizeof(int)*4);
				}//if(allWidth>MaxWidth)
				else
					memmove(HarmoToDo,HarmoTest,sizeof(int)*4);
			}//if(MatchPitchMode>0)
		}//for(intPitch)
	}//if((HarmoNow[0]>0)&&(HarmoNow[1]>0)&&(HarmoNow[2]>0)&&(HarmoNow[3]>0))

	if(MaxWidth>0)
		return(4);
	else
	{
		MaxWidth=GetHarmoWidth(HarmoNow, AllMax[FrameNow], AllMaxWidth[FrameNow], EveryWidth);
		MinWidthIdx=0;
		if(EveryWidth[1]<EveryWidth[MinWidthIdx])
			MinWidthIdx=1;
		if(EveryWidth[2]<EveryWidth[MinWidthIdx])
			MinWidthIdx=2;
		if(EveryWidth[3]<EveryWidth[MinWidthIdx])
			MinWidthIdx=3;
		HarmoNow[MinWidthIdx]=0;
	}//else
	
	if((HarmoNow[0]==0)||(HarmoNow[1]==0)||(HarmoNow[2]==0)||(HarmoNow[3]==0))//只连其中3根
	{
		int maxTest[MAX_PEAK_NUM];
		int maxAdd;
		int maxWidthTest[MAX_PEAK_NUM];
		int stat_peak_number, peak_number;

		stat_peak_number=0;//实际极值点的个数
		while((maxToDo[stat_peak_number]>0)&&(stat_peak_number<MAX_PEAK_NUM))
			stat_peak_number++;

		blankHarmo=0;
		while(HarmoNow[blankHarmo]>0)
			blankHarmo++;
		
		tmpPitch=(float(HarmoNow[0])/mode[0]+float(HarmoNow[1])/mode[1]+float(HarmoNow[2])/mode[2]+float(HarmoNow[3])/mode[3])/3;

		even=min(int(tmpPitch*0.3),2);
		MinIntPitch=max(int(tmpPitch)-PitchChange,LowPitch);
		MaxIntPitch=min(int(tmpPitch)+PitchChange,HighPitch);
	
		for(intPitch=MinIntPitch;intPitch<=MaxIntPitch;intPitch++)
		{
			/////////for test here插入理想的极值点，设其宽度为1
					maxAdd=intPitch*mode[blankHarmo];
					ii=0;
					while((maxToDo[ii]<maxAdd)&&(ii<MAX_PEAK_NUM-1)&&(maxToDo[ii+1]>0))
						ii++;
					if(maxToDo[ii]==maxAdd)//如果存在这个极值，就不需要再插入此值了
					{
						memmove(maxTest,maxToDo,sizeof(int)*MAX_PEAK_NUM);	
						memmove(maxWidthTest,AllMaxWidth[FrameToDo],sizeof(int)*MAX_PEAK_NUM);
						peak_number=stat_peak_number;
					}
					else
					{	//插入一个极值点
						memmove(maxTest,maxToDo,sizeof(int)*ii);
						maxTest[ii]=maxAdd;
						memmove(&maxTest[ii+1],&maxToDo[ii],sizeof(int)*(MAX_PEAK_NUM-ii-1));
						memmove(maxWidthTest, AllMaxWidth[FrameToDo], sizeof(int)*ii);
						maxWidthTest[ii]=1;
						memmove(&maxWidthTest[ii+1], &AllMaxWidth[FrameToDo][ii], sizeof(int)*(MAX_PEAK_NUM-ii-1));
						peak_number=stat_peak_number+1;
					}//else

			/////////////
			
			for(Indx[0]=0;(Indx[0]<peak_number-3)&&(maxTest[Indx[0]]<(intPitch+1)*mode[0]+0.01);Indx[0]++)
			{
				if(maxTest[Indx[0]]<intPitch*mode[0])
					continue;
				for(Indx[1]=Indx[0]+1;(Indx[1]<peak_number-2)&&(maxTest[Indx[1]]<maxTest[Indx[0]]+intPitch+1.01);Indx[1]++)
				{
					if(maxTest[Indx[1]]<maxTest[Indx[0]]+intPitch-1)
						continue;
					for(Indx[2]=Indx[1]+1;(Indx[2]<peak_number-1)&&(maxTest[Indx[2]]<maxTest[Indx[1]]+intPitch+1.01);Indx[2]++)
					{
						if(maxTest[Indx[2]]<maxTest[Indx[1]]+intPitch-1)
							continue;
						for(Indx[3]=Indx[2]+1;(Indx[3]<peak_number)&&(maxTest[Indx[3]]<maxTest[Indx[2]]+intPitch+1.01);Indx[3]++)
						{
							if(maxTest[Indx[3]]<maxTest[Indx[2]]+intPitch-1)
								continue;
						
							HarmoTest[0]=maxTest[Indx[0]];
							HarmoTest[1]=maxTest[Indx[1]];
							HarmoTest[2]=maxTest[Indx[2]];
							HarmoTest[3]=maxTest[Indx[3]];
							
							MaxWidth=GetHarmoWidth(HarmoTest, maxTest, maxWidthTest, EveryWidth);
							if(MaxWidth<intPitch/2)
								continue;
							tmp_even=TestEven(HarmoTest,mode);
							if(tmp_even<even)
							{
								even=tmp_even;
								HarmoToDo[0]=HarmoTest[0];
								HarmoToDo[1]=HarmoTest[1];
								HarmoToDo[2]=HarmoTest[2];
								HarmoToDo[3]=HarmoTest[3];
							}//if(tmp_even<even)
						}//Indx[3]
					}//Indx[2]
				}//Indx[1]
			}//for(Indx[0])
		}//for(intPitch)
		HarmoToDo[blankHarmo]=0;
	}//if((HarmoNow[1]==0)||(HarmoNow[2]==0)||(HarmoNow[3]==0))
	
	if((HarmoToDo[0]>0)||(HarmoToDo[1]>0)||(HarmoToDo[2]>0)||(HarmoToDo[3]>0))
		return(3);
	else return(-1);
}//Connect4()


int CDetectVoice_EPD::TestEven(int harmo_in[4], int multi[4])
{
	int dif_max;
	int dif[4];
	int tmp, ii,jj;

	if(multi[0]==0)
	{
		dif[0]=harmo_in[1];
		dif[1]=harmo_in[2]-harmo_in[1];
		dif[2]=harmo_in[3]-harmo_in[2];
		dif_max=max(abs(dif[0]-dif[1]),abs(dif[1]-dif[2]));
		dif_max=max(abs(dif[0]-dif[2]),dif_max);
	}//if(multi[0]==0)

	if(multi[0]>0)
	{
		dif[0]=int(harmo_in[0]/multi[0]);
		dif[1]=harmo_in[1]-harmo_in[0];
		dif[2]=harmo_in[2]-harmo_in[1];
		dif[3]=harmo_in[3]-harmo_in[2];
		dif_max=0;
		for(ii=0;ii<3;ii++)
			for(jj=ii+1;jj<4;jj++)
			{
				tmp=abs(dif[ii]-dif[jj]);
				dif_max=max(dif_max,tmp);
			}//for for
	}//if(multi[0]>0)
	
	return(dif_max);
}//int TestEven(int harmo_in[4], int multi[4])

//计算当前存在的谐波的各自宽度，并输出最大谐波的宽度
int CDetectVoice_EPD::GetHarmoWidth(int Harmo_in[4], int max_in[MAX_PEAK_NUM], int Width_in[MAX_PEAK_NUM], int Width_Out[4])
{
	int ii, jj;
	int maxWidth=0;
	
	memset(Width_Out,0, sizeof(int)*4);
	jj=1;
	for(ii=0;ii<4;ii++)
	{
		if(Harmo_in[ii]==0)
			continue;
		jj--;
		while((jj<MAX_PEAK_NUM)&&(max_in[jj]<Harmo_in[ii]))
			jj++;
		if(Harmo_in[ii]==max_in[jj])
			Width_Out[ii]=Width_in[jj];
		if(Width_Out[ii]>maxWidth)
			maxWidth=Width_Out[ii];
	}//for(ii=0;ii<4;ii++)
	
	return(maxWidth);
}

int CDetectVoice_EPD::CountVoice()
{
	int ii;
	int count=0;
	for(ii=0;ii<WINDOW_FRAMS;ii++)
	{
		if(Results[ii]>0)
			count++;
	}//for(ii)
	return(count);
}
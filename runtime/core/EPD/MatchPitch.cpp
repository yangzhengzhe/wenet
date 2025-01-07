#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "math.h"
#include "DetectVoice.h"


//2006.05.31改进：针对当前极值，对非整数的基频匹配谐波，并最终对应到整数基频上去。
void CDetectVoice_EPD::MatchPitch()//对当前一帧的max做基本的匹配，看是否可以基本符合某些谐波
{
	int ii,jj;
	int pitch=LowPitch;
	float pitchflt=float(LowPitch);
	float currentharmo;//理想的谐波位置及其对应的可能上下范围
	int CanMatch;	//如果不能匹配到任何谐波，就把FrameMatchPitch[pitch][0]置为-100，表示不需要考虑此pitch了。
	int multi;		//谐波次数。基频为1次谐波。
	short *matchpitch;	//当前一帧信号对应到某个基频，对应一组谐波的任何可能方式
	float most_dif=1.01;


	matchpitch=&FrameMatchPitch[(pitch-LowPitch)*MaxHarmoMode];//记录这帧信号中1个基频的谐波对应情况，极值点是否能够对应上某次谐波
	memset(matchpitch,0,sizeof(short)*MaxHarmoMode);
	CanMatch=0;
	while(pitchflt<=HighPitch)
	{
		memset(nearHarmo,0,sizeof(short)*(HighestHarmo));//对各次谐波的匹配情况
		
		//对基频为pitchflt的浊音谐波的匹配
		jj=0;
		multi=1;
		currentharmo=pitchflt;
		while((currentharmo<HighBand)&&(max[jj]>0)&&(jj<MAX_PEAK_NUM))	//匹配极值和谐波
		{
			while((max[jj]<int(currentharmo))&&(max[jj]>0)&&(jj<MAX_PEAK_NUM))
				jj++;
			if((max[jj]<=currentharmo+1.01)&&(max[jj]>0)&&(jj<MAX_PEAK_NUM))
				nearHarmo[multi]=max[jj];//nearHarmo[x]记录了基频的x倍是否有对应的极值，nearHarmo[0]无用
			currentharmo+=pitchflt;		//准备匹配下一条谐波
			multi++;
		}//while((currentharmo<HighBand)&&(max[jj]>0)&&(jj<MAX_PEAK_NUM))
		
		//检查是否连续4个谐波都匹配上（最低频为3个谐波）
		if(matchpitch[0]==0)//如果还没有匹配过
			if((nearHarmo[1]>0)&&(nearHarmo[2]>0)&&(nearHarmo[3]>0))
				if((nearHarmo[1]>=pitchflt-most_dif)&&(nearHarmo[2]-nearHarmo[1]>=pitchflt-most_dif)&&(nearHarmo[3]-nearHarmo[2]>=pitchflt-most_dif))
					if((nearHarmo[1]<=pitchflt+most_dif)&&(nearHarmo[2]-nearHarmo[1]<=pitchflt+most_dif)&&(nearHarmo[3]-nearHarmo[2]<=pitchflt+most_dif))
					{
						matchpitch[0]=1;
						CanMatch=1;
					}
		
		currentharmo=5*pitchflt;
		ii=1;
		while(currentharmo<HighBand)
		{
			if(matchpitch[ii]==0)//如果还没有匹配到这4级谐波的组合
				if((nearHarmo[ii+1]>0)&&(nearHarmo[ii+2]>0)&&(nearHarmo[ii+3]>0)&&(nearHarmo[ii+4]>0))
					if((nearHarmo[ii+2]-nearHarmo[ii+1]>=pitchflt-most_dif)&&(nearHarmo[ii+3]-nearHarmo[ii+2]>=pitchflt-most_dif)&&(nearHarmo[ii+4]-nearHarmo[ii+3]>=pitchflt-most_dif))
						if((nearHarmo[ii+2]-nearHarmo[ii+1]<=pitchflt+most_dif)&&(nearHarmo[ii+3]-nearHarmo[ii+2]<=pitchflt+most_dif)&&(nearHarmo[ii+4]-nearHarmo[ii+3]<=pitchflt+most_dif))
						{
							matchpitch[ii]=1;
							CanMatch=1;		//用于记录总的匹配结果
						}//if
			currentharmo+=pitchflt;
			ii++;
		}//while(currentharmo<HighBand)
		
		pitchflt+=PITCH_STEP;
		if(floor(pitchflt)>pitch)//如果整数部分已经大过了上个整数基频，则开始做这个整数基频的初始化。
		{
			if(CanMatch==0)
				matchpitch[0]=-100;//对上个整数基频作总结
			pitch=floor(pitchflt);
			if(pitch<=HighPitch)
			{
				matchpitch=&FrameMatchPitch[(pitch-LowPitch)*MaxHarmoMode];//记录这帧信号中1个基频的谐波对应情况，极值点是否能够对应上某次谐波
				memset(matchpitch,0,sizeof(short)*MaxHarmoMode);
				CanMatch=0;
			}
		}//if(floor(pitchflt)>pitch)
	}//while(pitch<=HighPitch)
}//void MatchHarmo()


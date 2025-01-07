#ifndef PI
	#define PI		3.14159265359 // 3.14159265428
#endif
//#ifndef SampleRate
//	#define SampleRate          16000
//#endif
//extern int SampleRate;
//	#define SAMPLE_RATE			SampleRate   // 采样率
//	#define FRAME_SHIFT			(SampleRate/100)		//10 ms, window's shift. It is same with the frameShift
//	#define FRAME_LENGTH		(SampleRate/40)		//25 ms, minimum data to start work
	#define BUFLEN				100		//频带能量缓存的长度

	#define PRE_EMPHASIS		0.97f	//pre_emphasis


	#define WINDOW_FRAMS		30
	#define BEGIN_SILENT_FRAMS	10		//shortest beginning silence(in frames). If the silence is less, detector will fail to detect the correct start point.

//	#define FFT_SIZE	(512*SampleRate/8000)		//fft points for checking harmonics
//	#define BIN_NUMBER	(256*SampleRate/8000)		//bin numbers for checking harmonics
	////////////////////////basic settings of endpoint detector////////////////////////
	#define harmonicMinNumber		8		//要判出语音,至少需要多少谐波帧
	
	////////////////////////setting for checking harmonics///////////////////////////////////////////
	#define LOW_PITCH_HZ		65		//lowest pitch: 65Hz
	#define HIGH_PITCH_HZ		450		//highest pitch: 450Hz
	#define HIGH_BAND_HZ		2000	//Highest frequency of searching harmonic.
	#define MAX_PEAK_NUM		20		//每帧在搜索范围中最多出现多少个极值

	#define MIN_HARMO_SCALE		20		//谐波bin能量至少是最小bin能量的多少倍
	#define PITCH_STEP			0.1		//基频粗匹配时，测试基频的步长。
	
	#define PITCH_CHANGE		40		//单位：Hz. 两帧之间,基音最快变化为60Hz
	#define MIN_HARMO_LEN		6		//最短的谐波长度


////////////////////////////////////parameters for user's speech/////////////////////////
#define SAFE_REGION			10		//在前后10帧寻找精细起点。

#define SEG_SILENCE			20		//先检测的一段静音长度，用于检测终点
#define MAX_SILENCE			20      // (SampleRate/400)		//time duration (in frames) for waiting speaker's voice at the end. If the user keep silence longer than this parameter, the program will detect the end point.

//////////////////////////////////////////////////////////////
#define BAND_NUM		4
#define ENERGY_FRAMS	10		//估计噪声所用帧数
#define UPDATE_NOISE	1.1		//parameter for update noise. The noise can be updated only when the difference is less than it.

#define VAR_MIN			0.1		//最小的方差
#define MAX_ENERGY		1e12	//最大的语音信号频带能量
#define MIN_ENERGY		1000	//最小的噪声频带能量

#define SPEECH_HIGH_to_LOW	10	//最大语音的能量是正常语音能量的多少倍
#define MAX_SENSITIVITY	0.1		//最高灵敏度，无论如何变化貌似没什么用
#define MIN_SENSITIVITY	0.01	//最低灵敏度，无论如何变化貌似没什么用
#define STAT_THD_FRAMS	10		//通过连续多少帧来做系统阈值的初始化


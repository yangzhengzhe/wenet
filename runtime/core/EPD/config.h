/******************************DOCUMENT*COMMENT***********************************
*D
*D 文件名称            : config.h
*D
*D 项目名称            : 
*D
*D 版本号              : 1.1.0002
*D
*D 文件描述            :
*D
*D
*D 文件修改记录
*D ------------------------------------------------------------------------------ 
*D 版本号       修改日期       修改人     改动内容
*D ------------------------------------------------------------------------------ 
*D 1.1.0001                               创建文件
*D 1.1.0002     2007.08.29     plu        修改构造函数,增加SetCfgFile函数
*D*******************************************************************************/

#ifndef _CONFIGURATION_HCCL_
#define _CONFIGURATION_HCCL_

//#include "common.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
typedef unsigned short		ushort;
typedef unsigned int		uint;
typedef unsigned char		uchar;
typedef			 char		char32[32];
typedef			 char		char64[64];
typedef			 char		char128[128];
typedef			 char		char256[256];
typedef			 char       char512[512];
typedef			 char		char1024[1024];

struct aConfigEnv_DNN4EPD {
   aConfigEnv_DNN4EPD *next;
   char256 env;
   char256 def;
};

class Config_DNN4EPD {
   protected:

      aConfigEnv_DNN4EPD *envs;
      FILE *fenv;
      char *value;
      bool exam;

      char *GetEnv(char *env);
      
   public:
      // =%d 
      bool ReadConfig(char *line,int& num);
      // =%d %d
      bool ReadConfig(char *line,int& num1,int& num2);
      // =true/false
      bool ReadConfig(char *line,bool& bln);
      // =%g
      bool ReadConfig(char *line,float& num);
      // =%g %g
      bool ReadConfig(char *line,float& num1,float& num2);
      // =%g %g %g
      bool ReadConfig(char *line,float& num1,float& num2,float& num3);
      // =%g,%g %g,%g
      bool ReadConfig(char *line,float& n1,float& n2,float& n3,float& n4);
      // =%s
      bool ReadConfig(char *line,char *str);
      // =%s %s
      bool ReadConfig(char *line,char *str1,char *str2);
      
      /*// plu 2007.08.29_12:40:15
      Config(int argc,char *argv[],char *head=NULL);
      */// plu 2007.08.29_12:40:15
	  Config_DNN4EPD();
      ~Config_DNN4EPD();

	  // 2007.08.29 plu : 
	  bool SetConfigFile(const char *cfgFile);
};

#endif

/******************************DOCUMENT*COMMENT***********************************
*D
*D 文件名称            : config.cpp
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
*D 1.1.0001                                创建文件
*D 1.1.0002     2007.08.29     plu         将ReadConfig的返回类型改为bool
*D*******************************************************************************/
//#include "common.h"
#include "config.h"

//LJ-- static void SetEnv(aConfigEnv* &envs,char *config) {
void SetEnv(aConfigEnv_DNN4EPD* &envs,char *config) 
{
   char *pt;
   while ((pt=strchr(config,'"'))) strcpy(pt,pt+1);
   if ((pt=strchr(config,'='))) {
      aConfigEnv_DNN4EPD *tmp=new aConfigEnv_DNN4EPD;
      tmp->next=envs; 
	  envs=tmp; 
      *pt='\0'; 
	  strcpy(tmp->env,config); 
      strcpy(tmp->def,pt+1);
	  *pt='=';
   }
}

Config_DNN4EPD::Config_DNN4EPD()
{
	fenv=NULL;
	exam=false;
	envs=NULL;
}

Config_DNN4EPD::~Config_DNN4EPD(void)
{
	if (envs!=NULL)
	{
		for (aConfigEnv_DNN4EPD *tenv,*env=envs;(tenv=env);delete tenv)
			env=env->next;
	}

	if (exam)	exit(3);

	if (fenv!=NULL) 
	{
		fprintf(fenv,"Compiling date: %s\n",__DATE__);
		fclose(fenv);
	}
}

// 2007.08.29 plu : add
bool Config_DNN4EPD::SetConfigFile(const char *cfgFile)
{
	FILE *fin;
	fin=fopen(cfgFile,"rt");
	if (fin==NULL)		
	{
		printf("Error open %s for read!\n",cfgFile);
		return false;
	}
	
	char256 config="";
    while (!feof(fin)) 
	{
		if (fscanf(fin,"\n %[^\n]s",config)<1) break;
        SetEnv(envs,config);
    }
    fclose(fin);

	return true;
}

char *Config_DNN4EPD::GetEnv(char *env) 
{
   for (aConfigEnv_DNN4EPD *ev=envs;ev;ev=ev->next)
      if (!strcmp(ev->env,env)) return ev->def;
   char *tmp=getenv(env);
   if (tmp==NULL)
	   printf("parameter %s not set\n",env);
   return tmp;
}

bool Config_DNN4EPD::ReadConfig(char *line,int& num) 
{
	char *pRst = GetEnv(line);
	if (pRst==NULL)	return false;

	sscanf(pRst,"%d",&num);
	if (fenv) fprintf(fenv,"%s=%d\n",line,num);

	return true;
}

bool Config_DNN4EPD::ReadConfig(char *line,int& num1,int& num2) 
{
	char *pRst = GetEnv(line);
	if (pRst==NULL)	return false;

	sscanf(pRst,"%d %d",&num1,&num2);
	if (fenv) fprintf(fenv,"%s=%d %d\n",line,num1,num2);

	return true;
}

bool Config_DNN4EPD::ReadConfig(char *line,bool& bln) 
{
	char *pRst = GetEnv(line);
	if (pRst==NULL)	return false;

	bln=(!strncmp(pRst,"true",4));
	if (fenv) fprintf(fenv,"%s=%s\n",line,(bln)?"true":"false");

	return true;
}

bool Config_DNN4EPD::ReadConfig(char *line,float& num) 
{
	char *pRst = GetEnv(line);
	if (pRst==NULL)	return false;

	sscanf(pRst,"%g",&num);
	if (fenv) fprintf(fenv,"%s=%g\n",line,num);

	return true;
}

bool Config_DNN4EPD::ReadConfig(char *line,float& num1,float& num2) 
{
	char *pRst = GetEnv(line);
	if (pRst==NULL)	return false;

	sscanf(pRst,"%g %g",&num1,&num2);
	if (fenv) fprintf(fenv,"%s=%g %g\n",line,num1,num2);

	return true;
}

bool Config_DNN4EPD::ReadConfig(char *line,float& num1,float& num2,float& num3) 
{
	char *pRst = GetEnv(line);
	if (pRst==NULL)	return false;

	sscanf(pRst,"%g %g %g",&num1,&num2,&num3);
	if (fenv) fprintf(fenv,"%s=%g %g %g\n",line,num1,num2,num3);

	return true;
}

bool Config_DNN4EPD::ReadConfig(char *line,float& n1,float& n2,float& n3,float& n4) 
{
	char *pRst = GetEnv(line);
	if (pRst==NULL)	return false;

	if (sscanf(pRst,"%g ,%g %g ,%g",&n1,&n2,&n3,&n4)<3) { n3=n1;n4=n2; }
	if (fenv) fprintf(fenv,"%s=%g,%g %g,%g\n",line,n1,n2,n3,n4);

	return true;
}

bool Config_DNN4EPD::ReadConfig(char *line,char *str) 
{
	char *pRst = GetEnv(line);
	if (pRst==NULL)	return false;

	str[0]='\0'; sscanf(pRst,"%[^\n]s",str);
	if (fenv) fprintf(fenv,"%s=%s\n",line,str);

	return true;
}

bool Config_DNN4EPD::ReadConfig(char *line,char *str1,char *str2) 
{
	char *pRst = GetEnv(line);
	if (pRst==NULL)	return false;

	if (sscanf(pRst,"%s %s",str1,str2)<2) strcpy(str2,str1);
	if (!strcmp(str1,"null") || !strcmp(str1,"NULL")) str1[0]='\0';
	if (!strcmp(str2,"null") || !strcmp(str2,"NULL")) str2[0]='\0';
	if (fenv) fprintf(fenv,"%s=%s %s\n",line,
	  (str1[0])?str1:"null",(str2[0])?str2:"null");

	return true;
}


/*// plu 2007.08.29_12:38:59
#ifdef __GNUC__
Config::Config(int argc,char *argv[],char *head=NULL) {
#else
Config::Config(int argc,char *argv[],char *head) {
#endif
   char256 readme; int i;

   fenv=NULL; exam=false; envs=NULL;
#ifdef __GNUC__
   char *pt=strrchr(argv[0],'/'); 
#else
   char *pt=strrchr(argv[0],'\\'); 
#endif
   sprintf(readme,"%s.env",head?head:(pt?pt+1:argv[0]));
   for (i=1;i<argc;i++) {
      if (strchr(argv[i],'=')) {
      } else if (!strcmp(argv[i],"-readme")) {
         strcpy(readme,argv[++i]);
      } else if (!strcmp(argv[i],"-config")) {
         //LJ-- static 
			 char256 config="";
         FILE *fin; i++; TextReadOpen(fin,argv[i]);
         while (!feof(fin)) {
            if (fscanf(fin,"\n %[^\n]s",config)<1) break;
            SetEnv(envs,config);
         }
         fclose(fin);
      } else if (!strcmp(argv[i],"-d")) {
         i++;
      } else if (!strcmp(argv[i],"-e")) {
         exam=true;
      } else if (!strcmp(argv[i],"-h")) {
         printf("Usage: %s [options ...]\n",argv[0]);
         printf("  -d dir       chdir to the specified dir first\n");
         printf("  -e           examine configurations\n");
         printf("  -config file configuration file\n");
         printf("  -readme file define readme file name\n");
         printf("  xxx=xxx      environment setting\n");
         printf("  -h           this help\n\n");
         printf("Compiling date: %s\n\n",__DATE__);
         exit(3);
      }
   }
   for (i=1;i<argc;i++) {
      if (strchr(argv[i],'=')) {
         SetEnv(envs,argv[i]);
      } else if (!strcmp(argv[i],"-d")) {
         _chdir(argv[++i]);
      }
   }
   fenv=stdout;
   if (!exam) TextWriteOpen(fenv,readme);
   fprintf(fenv,"Environment settings for %s\n\n",argv[0]);
   for (i=0;i<argc;i++)
      if (!strchr(argv[i],'=')) fprintf(fenv,"%s ",argv[i]);
   fprintf(fenv,"\n\n");
}
*/// plu 2007.08.29_12:38:59


#ifndef EPD_INTERFACE_H
#define EPD_INTERFACE_H

class C_EPD;

class EPD_Interface {
private:
    C_EPD* epdInstance; 
public:
    EPD_Interface(int SampleRate,const char* cfgFilename);
    ~EPD_Interface();

    int doEPD(short *buf_in, int len_in, int *begin, int *end);
    int re_initEPD();
    int SetParam(char *paramName, char *paramValue);
    int GetParam(char *paramName, char *paramValue);
};

#endif // EPD_INTERFACE_H

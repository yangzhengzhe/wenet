#include "EPD_Interface.h"
#include "EPD.h" // 包含C_EPD的头文件
#include <thread>

EPD_Interface::EPD_Interface(int SampleRate,const char* cfgFilename) : epdInstance(nullptr) {
    // 在构造函数中初始化C_EPD的实例
    epdInstance = new C_EPD(SampleRate, cfgFilename);
}

EPD_Interface::~EPD_Interface() {
    // 在析构函数中释放C_EPD的实例
    delete epdInstance;
}

int EPD_Interface::doEPD(short *buf_in, int len_in, int *begin, int *end) {
    // 调用C_EPD的doEPD方法
    return epdInstance->doEPD(buf_in, len_in, begin, end);
}

int EPD_Interface::re_initEPD() {
    // 调用C_EPD的re_initEPD方法
    return epdInstance->re_initEPD();
}

int EPD_Interface::SetParam(char *paramName, char *paramValue) {
    // 调用C_EPD的SetParam方法
    return epdInstance->SetParam(paramName, paramValue);
}

int EPD_Interface::GetParam(char *paramName, char *paramValue) {
    // 调用C_EPD的GetParam方法
    return epdInstance->GetParam(paramName, paramValue);
}

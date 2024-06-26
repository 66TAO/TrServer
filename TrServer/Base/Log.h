//#pragma once
//#include <time.h>
//#include <string>
//#include <vector>
//
//static std::string getFile(std::string file) {
//#ifndef WIN32
//    std::string pattern = "/";
//#else
//    std::string pattern = "\\";
//#endif // !WIN32
//
//
//    std::string::size_type pos;
//    std::vector<std::string> result;
//    file += pattern;                    //扩展字符串以方便操作
//    int size = file.size();
//    for (int i = 0; i < size; i++) {
//        pos = file.find(pattern, i);
//        if (pos < size) {
//            std::string s = file.substr(i, pos - i);
//            result.push_back(s);
//            i = pos + pattern.size() - 1;
//        }
//    }
//    return result.back();
//}
////  __FILE__ 获取源文件的相对路径和名字
////  __LINE__ 获取该行代码在文件中的行号
////  __func__ 或 __FUNCTION__ 获取函数名
////  ##__VA_ARGS__ 这是用于将可变参数传递给fprintf的语法。##在这里用于处理没有可变参数时的情况，确保即使没有可变参数，宏也能正确展开。
//
//#define LOGI(format, ...)  fprintf(stderr," [%s:%d] " format "\n",__FILE__,__LINE__,##__VA_ARGS__)      //输出示例：LOGI("Entering the main function"); →[TrConnection.c:14] Entering the main function
//#define LOGE(format, ...)  fprintf(stderr," [%s:%d] " format "\n",__FILE__,__LINE__,##__VA_ARGS__)




#pragma once
#include <time.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

// 提取文件名的函数
static std::string getFile(std::string file) {
#ifndef WIN32
    std::string pattern = "/";
#else
    std::string pattern = "\\";
#endif // !WIN32

    std::string::size_type pos;
    std::vector<std::string> result;
    file += pattern; // 扩展字符串以方便操作
    int size = file.size();
    for (int i = 0; i < size; i++) {
        pos = file.find(pattern, i);
        if (pos < size) {
            std::string s = file.substr(i, pos - i);
            result.push_back(s);
            i = pos + pattern.size() - 1;
        }
    }
    return result.back();
}

// 获取当前时间的字符串
static std::string getCurrentTime() {
    time_t now = time(0);
    tm* localtm = localtime(&now);
    char buf[80];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtm);
    return std::string(buf);
}

// 日志记录到文件
static void logToFile(const std::string& message) {
    std::ofstream logFile("log.txt", std::ios_base::app); // 以追加模式打开文件
    if (logFile.is_open()) {
        logFile << message << std::endl;
        logFile.close();
    }
    else {
        std::cerr << "Unable to open log file!" << std::endl;
    }
}

// 日志宏定义
#define LOGI(format, ...)  do { \
    char buffer[1024]; \
    snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
    std::string logMessage = "[" + getCurrentTime() + "] [INFO] [" + getFile(__FILE__) + ":" + std::to_string(__LINE__) + "] " + buffer; \
    fprintf(stderr, "%s\n", logMessage.c_str()); \
    logToFile(logMessage); \
} while (0)

#define LOGE(format, ...)  do { \
    char buffer[1024]; \
    snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
    std::string logMessage = "[" + getCurrentTime() + "] [ERROR] [" + getFile(__FILE__) + ":" + std::to_string(__LINE__) + "] " + buffer; \
    fprintf(stderr, "%s\n", logMessage.c_str()); \
    logToFile(logMessage); \
} while (0)



#include <iostream>
#include <sstream>
#include <string>

#include <opencv2/opencv.hpp>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")
#else
#include <unistd.h>
#include <sys/vfs.h>

#endif

static inline std::string getRootHiddenDir() {
#ifdef WIN32

#ifdef UNICODE
    wchar_t szPath[MAX_PATH];
#else
    TCHAR szPath[MAX_PATH];
#endif

    if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath)))
        return "";

    char snfile_path[MAX_PATH];

#ifndef UNICODE

    size_t newsize = strlen(szPath) + 1;
    wchar_t * wcstring = new wchar_t[newsize];
    // Convert char* string to a wchar_t* string.  
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars, wcstring, newsize, szPath, _TRUNCATE);
    wcstombs(snfile_path, wcstring, MAX_PATH);
#else
    wcstombs(snfile_path, szPath, MAX_PATH);
#endif

    std::string filename(snfile_path);
    filename += "\\Stereolabs\\";

#else //LINUX
    std::string filename = "/usr/local/zed/";
#endif

    return filename;
}

/*return the path to the Sl ZED hidden dir*/
static inline std::string getHiddenDir() {
    std::string filename = getRootHiddenDir();
#ifdef WIN32
    filename += "settings\\";
#else //LINUX
    filename += "settings/";
#endif
    return filename;
}

bool downloadCalib(unsigned int serial_number, std::string &filepath, std::string &calibration_file);
bool checkFile(std::string path);
bool convertCalibration(std::string path_zed_calib, std::string path_opencv_calib, cv::Size2i image_size = cv::Size2i(1280, 720));
void initCalibration(std::string path_opencv_calib, cv::Mat &map1_left, cv::Mat &map2_left, cv::Mat &cameraMatrix_left,
        cv::Mat &map1_right, cv::Mat &map2_right, cv::Mat &cameraMatrix_right);
void rectify(cv::Mat &left_raw, cv::Mat &left_rect, cv::Mat &right_raw, cv::Mat &right_rect, cv::Mat &map1_left, cv::Mat &map2_left, cv::Mat &map1_right, cv::Mat &map2_right);


#include <iostream>
#include <sstream>
#include <string>

#include "confManager.hpp"
#include "calibration.hpp"

using namespace cv;
using namespace std;

bool downloadCalib(unsigned int serial_number, std::string &filepath, std::string &calibration_file) {
#ifndef _WIN32
    std::string path = getHiddenDir();
    char specific_name[128];
    sprintf(specific_name, "SN%d.conf", serial_number);
    filepath = path + specific_name;
    calibration_file = path + "SN" + std::to_string(serial_number) + ".yml";
    if (!checkFile(filepath)) {
        // Download the file
        std::string url("'http://calib.stereolabs.com/?SN=");
        std::string cmd;
        cmd = "wget " + url + to_string(serial_number) + "' -O " + filepath;
        cout << cmd << endl;
        system(cmd.c_str());
        
        if (!checkFile(filepath)) {
            cout << "Invalid calibration file" << endl;
            return 1;
        }
    }
#else
    std::string path = getHiddenDir();
    char specific_name[128];
    sprintf(specific_name, "SN%d.conf", serial_number);
    filepath = path + specific_name;
    calibration_file = path + "SN" + std::to_string(serial_number) + ".yml";
    if (!checkFile(filepath)) {
        TCHAR *settingFolder = new TCHAR[path.size() + 1];
        settingFolder[path.size()] = 0;
        std::copy(path.begin(), path.end(), settingFolder);
        SHCreateDirectoryEx(NULL, settingFolder, NULL); //recursive creation

        std::string url("http://calib.stereolabs.com/?SN=");
        url += std::to_string(serial_number);
        TCHAR *address = new TCHAR[url.size() + 1];
        address[url.size()] = 0;
        std::copy(url.begin(), url.end(), address);
        TCHAR *calibPath = new TCHAR[filepath.size() + 1];
        calibPath[filepath.size()] = 0;
        std::copy(filepath.begin(), filepath.end(), calibPath);

        HRESULT hr = URLDownloadToFile(NULL, address, calibPath, 0, NULL);
        if (hr != 0) {
            cout << "Fail to download calibration file" << endl;
            return 2;
        }

        if (!checkFile(filepath)) {
            cout << "Invalid calibration file" << endl;
            return 1;
        }
    }
#endif
    return 0;
}



bool checkFile(std::string path) {
    ifstream f(path.c_str());
    return f.good();
}

bool convertCalibration(std::string path_zed_calib, std::string path_opencv_calib,
        cv::Size2i image_size) {

    if (!checkFile(path_zed_calib)) {
        cout << "Calibration missing" << endl;
        return 0;
    }

    // Open ZED INI file

    ConfManager camerareader(path_zed_calib.c_str());
    if (!camerareader.isOpened())
        return 0;

    std::string resolution_str;
    bool check_data = true;
    switch ((int) image_size.width) {
        case 2208:
            resolution_str = "2k";
            break;
        case 1920:
            resolution_str = "fhd";
            break;
        case 1280:
            resolution_str = "hd";
            break;
        case 672:
            resolution_str = "vga";
            break;
        default:
            resolution_str = "hd";
            check_data = false;
            break;
    }

    // Read rotations
    float T[3];

    // Read translations
    T[0] = camerareader.getValue("stereo:baseline", 0.0f);
    T[1] = camerareader.getValue("stereo:ty_" + resolution_str, 0.f);
    T[2] = camerareader.getValue("stereo:tz_" + resolution_str, 0.f);

    // Read left params
    float left_cam_cx = camerareader.getValue("left_cam_" + resolution_str + ":cx", 0.0f);
    float left_cam_cy = camerareader.getValue("left_cam_" + resolution_str + ":cy", 0.0f);
    float left_cam_fx = camerareader.getValue("left_cam_" + resolution_str + ":fx", 0.0f);
    float left_cam_fy = camerareader.getValue("left_cam_" + resolution_str + ":fy", 0.0f);
    float left_cam_k1 = camerareader.getValue("left_cam_" + resolution_str + ":k1", 0.0f);
    float left_cam_k2 = camerareader.getValue("left_cam_" + resolution_str + ":k2", 0.0f);
    float left_cam_p1 = camerareader.getValue("left_cam_" + resolution_str + ":p1", 0.0f);
    float left_cam_p2 = camerareader.getValue("left_cam_" + resolution_str + ":p2", 0.0f);
    float left_cam_k3 = camerareader.getValue("left_cam_" + resolution_str + ":k3", 0.0f);

    // Read right params
    float right_cam_cx = camerareader.getValue("right_cam_" + resolution_str + ":cx", 0.0f);
    float right_cam_cy = camerareader.getValue("right_cam_" + resolution_str + ":cy", 0.0f);
    float right_cam_fx = camerareader.getValue("right_cam_" + resolution_str + ":fx", 0.0f);
    float right_cam_fy = camerareader.getValue("right_cam_" + resolution_str + ":fy", 0.0f);
    float right_cam_k1 = camerareader.getValue("right_cam_" + resolution_str + ":k1", 0.0f);
    float right_cam_k2 = camerareader.getValue("right_cam_" + resolution_str + ":k2", 0.0f);
    float right_cam_p1 = camerareader.getValue("right_cam_" + resolution_str + ":p1", 0.0f);
    float right_cam_p2 = camerareader.getValue("right_cam_" + resolution_str + ":p2", 0.0f);
    float right_cam_k3 = camerareader.getValue("right_cam_" + resolution_str + ":k3", 0.0f);

    //Security Check A : (Linux Only) : Wrong "." or "," reading in file conf.
#ifndef _WIN32
    if (right_cam_k1 == 0 && left_cam_k1 == 0 && left_cam_k2 == 0 && right_cam_k2 == 0) {
        cout << "ZED File invalid" << endl;
        exit(1);
    }
#endif

    cv::Mat R_zed = (Mat_<double>(1, 3) << camerareader.getValue("stereo:rx_" + resolution_str, 0.f), camerareader.getValue("stereo:cv_" + resolution_str, 0.f), camerareader.getValue("stereo:rz_" + resolution_str, 0.f));
    cv::Mat RotMatrix3x3;

    cv::Rodrigues(R_zed /*in*/, RotMatrix3x3 /*out*/);

    // Saving calibration
    FileStorage fs(path_opencv_calib, FileStorage::WRITE);
    time_t rawtime;
    time(&rawtime);
    fs << "calibrationDate" << asctime(localtime(&rawtime));
    fs << "imageSize_width" << image_size.width << "imageSize_height" << image_size.height;

    // Left
    Mat cameraMatrix1 = (Mat_<double>(3, 3) << left_cam_fx, 0, left_cam_cx, 0, left_cam_fy, left_cam_cy, 0, 0, 1);
    Mat distCoeffs1 = (Mat_<double>(5, 1) << left_cam_k1, left_cam_k2, left_cam_p1, left_cam_p2, left_cam_k3);
    fs << "cameraMatrix1" << cameraMatrix1 << "distCoeffs1" << distCoeffs1;

    // Right
    Mat cameraMatrix2 = (Mat_<double>(3, 3) << right_cam_fx, 0, right_cam_cx, 0, right_cam_fy, right_cam_cy, 0, 0, 1);
    Mat distCoeffs2 = (Mat_<double>(5, 1) << right_cam_k1, right_cam_k2, right_cam_p1, right_cam_p2, right_cam_k3);
    fs << "cameraMatrix2" << cameraMatrix2 << "distCoeffs2" << distCoeffs2;

    // Stereo
    Mat T_ = (Mat_<double>(3, 1) << T[0], T[1], T[2]);
    fs << "R" << RotMatrix3x3 << "T" << T_;

    fs.release();
    return 0;
}

void initCalibration(std::string path_opencv_calib, cv::Mat &map1_left, cv::Mat &map2_left, cv::Mat &cameraMatrix_left,
        cv::Mat &map1_right, cv::Mat &map2_right, cv::Mat &cameraMatrix_right) {

    // Load the calibration
    FileStorage fs(path_opencv_calib, FileStorage::READ);
    std::string date;
    fs["calibrationDate"] >> date;
    cv::Size2i resolution;

    fs["imageSize_width"] >> resolution.width;
    fs["imageSize_height"] >> resolution.height;

    // Left
    Mat cameraMatrix1, distCoeffs1;
    fs["cameraMatrix1"] >> cameraMatrix1;
    fs["distCoeffs1"] >> distCoeffs1;

    // Right
    Mat cameraMatrix2, distCoeffs2;
    fs["cameraMatrix2"] >> cameraMatrix2;
    fs["distCoeffs2"] >> distCoeffs2;

    // Stereo
    Mat R, T;
    fs["R"] >> R;
    fs["T"] >> T;
    fs.release();


    cv::Mat R1, R2, P1, P2, Q;
    stereoRectify(cameraMatrix1, distCoeffs1, cameraMatrix2, distCoeffs2, resolution, R, T, R1, R2, P1, P2, Q, CALIB_ZERO_DISPARITY, 0, resolution);

    // Compute MX MY for each side
    initUndistortRectifyMap(cameraMatrix1, distCoeffs1, R1, P1, resolution, CV_32FC1, map1_left, map2_left);
    initUndistortRectifyMap(cameraMatrix2, distCoeffs2, R2, P2, resolution, CV_32FC1, map1_right, map2_right);

    cameraMatrix_left = P1;
    cameraMatrix_right = P2;
}

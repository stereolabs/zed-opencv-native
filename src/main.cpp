#include <iostream>
#include <sstream>
#include <string>

#include <opencv2/opencv.hpp>
#include "calibration.hpp"

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
    // argument : SN OR opencv calibration file (.yml or xml)
    if (argc == 1) {
        cout << "Please provide either the ZED Serial number or the opencv calibration file path (.yml)" << endl;
        return 1;
    }

    cv::Size2i image_size = cv::Size2i(1280, 720);

    std::string calibration_file;
    std::string fn(argv[1]);
    if (fn.substr(fn.find_last_of(".") + 1) == "yml") {
        // Calibration file already exists
        if (!checkFile(fn)) {
            cout << "Invalid calibration file" << endl;
            return 1;
        }
        calibration_file = fn;
    } else {
        // ZED Calibration
        unsigned int serial_number = std::stoi(fn);
        std::string filepath = "unknown";

        if (downloadCalib(serial_number, filepath, calibration_file)) return 1;
        cout << "ZED calibration found ! Converting..." << endl;
        convertCalibration(filepath, calibration_file, image_size);
    }

    cv::Mat map1_left, map2_left, map1_right, map2_right;
    cv::Mat cameraMatrix_left, cameraMatrix_right;
    initCalibration(calibration_file, map1_left, map2_left, cameraMatrix_left, map1_right, map2_right, cameraMatrix_right);

    cout << " Camera Matrix L: \n" << cameraMatrix_left << endl << endl;
    cout << " Camera Matrix R: \n" << cameraMatrix_right << endl << endl;


    char key = 'r';

    /////////// IMPORTANT ///////////////
    // Assuming here that ZED is connected under device 0 (/dev/video0 for Linux or first camera listed on Windows)
    // On laptops, you may have to change to 1 if the first listed camera is the integrated webcam.
    ////////////////////////////////////
    VideoCapture cap(0);
    if (!cap.isOpened())
        return -1;
    cap.grab();
    // Set the video resolution (2*Width * Height)
    cap.set(CV_CAP_PROP_FRAME_WIDTH, image_size.width * 2);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, image_size.height);
    cap.grab();

    Mat frame, left_raw, left_rect, right_raw, right_rect;

    while (key != 'q') {
        // Get a new frame from camera
        cap >> frame;
        // Extract left and right images from side-by-side
        left_raw = frame(cv::Rect(0, 0, frame.cols / 2, frame.rows));
        right_raw = frame(cv::Rect(frame.cols / 2, 0, frame.cols / 2, frame.rows));
        // Display images    
        imshow("left RAW", left_raw);

        remap(left_raw, left_rect, map1_left, map2_left, INTER_LINEAR);
        remap(right_raw, right_rect, map1_right, map2_right, INTER_LINEAR);

        imshow("right RECT", right_rect);
        imshow("left RECT", left_rect);

        key = waitKey(30);
    }
    return 0;
}
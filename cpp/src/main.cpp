///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2018, STEREOLABS.
//
// All rights reserved.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

/*****************************************************************************************
 ** This sample demonstrates how to capture stereo images and calibration parameters    **
 ** from the ZED camera with OpenCV without using the ZED SDK.                          **
 *****************************************************************************************/

// General includes
#include <iostream>
#include <sstream>
#include <string>

// OpenCV includes
#include <opencv2/opencv.hpp>

// Sample includes
#include "calibration.hpp"

// Namespaces
using namespace cv;
using namespace std;

int main(int argc, char** argv) {

    // argument : SN
    if (argc == 1) {
        cout << "Please provide the ZED Serial number" << endl;
        return 1;
    }

    cv::Size2i image_size = cv::Size2i(1280, 720);

    std::string calibration_file;
    // ZED Calibration
    unsigned int serial_number = std::stoi(argv[1]);
    // Download camera calibration file
    if (downloadCalibrationFile(serial_number, calibration_file)) return 1;
    cout << "Calibration file found. Loading..." << endl;

    cv::Mat map_left_x, map_left_y;
    cv::Mat map_right_x, map_right_y;
    cv::Mat cameraMatrix_left, cameraMatrix_right;
    initCalibration(calibration_file, image_size, map_left_x, map_left_y, map_right_x, map_right_y, cameraMatrix_left, cameraMatrix_right);

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
    cap.set(cv::CAP_PROP_FRAME_WIDTH, image_size.width * 2);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, image_size.height);
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

        remap(left_raw, left_rect, map_left_x, map_left_y, INTER_LINEAR);
        remap(right_raw, right_rect, map_right_x, map_right_y, INTER_LINEAR);

        imshow("right RECT", right_rect);
        imshow("left RECT", left_rect);

        key = waitKey(30);
    }
    return 0;
}

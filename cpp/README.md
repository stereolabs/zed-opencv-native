# Stereolabs ZED -  OpenCV Native Capture

This sample shows how to capture rectified images with the ZED or ZED-M Camera and OpenCV, without the ZED SDK.

If you want to use OpenCV with the ZED SDK features, check our sample [here](https://github.com/stereolabs/zed-opencv).

## OS

- Windows, Linux, MacOS

## Build the program

Download the sample and follow the instructions below: [More](https://www.stereolabs.com/docs/getting-started/application-development/)

### Build for Windows

- Create a "build" folder in the source folder
- Open cmake-gui and select the source and build folders
- Generate the Visual Studio `Win64` solution
- Open the resulting solution and change configuration to `Release`
- Build solution

### Build for Linux

Open a terminal in the sample directory and execute the following command:

    mkdir build
    cd build
    cmake ..
    make

## Run the program

- Navigate to the build directory and launch the executable
- Or open a terminal in the build directory and run the sample :

      ./ZED_Opencv_Native <camera_serial_number>

## Notes

- You need to know the serial number of the camera you are trying to open. To do so, you can use ZED Explorer tools (ZED SDK tools) and check the serial number on the top right of ZED Explorer window.

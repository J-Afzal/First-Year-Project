#ifndef MAIN_HPP_INCLUDED
#define MAIN_HPP_INCLUDED

#include <iostream>
#include "opencv_aee.hpp"
#include "main.hpp"     // You can use this file for declaring defined values and functions
#include "pi2c.h"
#include <stdlib.h>

using namespace std;

Pi2c car(0x22); // Configure the I2C interface to the Car as a global variable
Pi2c sensor(0x23);

// Function declarations
int main(int argc, char** argv);



Mat GetFrameHSVForPink (const Mat frame)
{
    Mat frameHSV;
    int lowH = 150, highH = 170, lowS = 32, highS = 255, lowV = 32, highV = 255;

    cvtColor(frame, frameHSV, COLOR_BGR2HSV);
    inRange(frameHSV, Scalar(lowH, lowS, lowV), Scalar(highH, highS, highV), frameHSV);

    return frameHSV;
}

Mat GetFrameHSVForBlack (const Mat frame)
{
    Mat frameHSV;
    int lowH = 0, highH = 179, lowS = 0, highS = 255, lowV = 0, highV = 50;

    cvtColor(frame, frameHSV, COLOR_BGR2HSV);
    inRange(frameHSV, Scalar(lowH, lowS, lowV), Scalar(highH, highS, highV), frameHSV);

    return frameHSV;
}

Mat GetFrameHSVForRed (const Mat frame)
{
    Mat frameHSV;
    int lowH = 170, highH = 180, lowS = 32, highS = 255, lowV = 32, highV = 255;

    cvtColor(frame, frameHSV, COLOR_BGR2HSV);
    inRange(frameHSV, Scalar(lowH, lowS, lowV), Scalar(highH, highS, highV), frameHSV);

    return frameHSV;
}

Mat GetFrameHSVForGreen (const Mat frame)
{
    Mat frameHSV;
    int lowH = 70, highH = 90, lowS = 32, highS = 255, lowV = 32, highV = 255;

    cvtColor(frame, frameHSV, COLOR_BGR2HSV);
    inRange(frameHSV, Scalar(lowH, lowS, lowV), Scalar(highH, highS, highV), frameHSV);

    return frameHSV;
}


void setup(void)
{
    setupCamera(320, 240);  // Enable the camera for OpenCV
}

Mat CaptureFrameFromCamera (void)
{
    Mat frame;

    while(frame.empty())
        frame = captureFrame(); // Capture a frame from the camera and store in a new matrix variable

    return frame;
}

void HSVCamera (void)
{
    namedWindow("HSV Tester");

    int lowH = 0, highH = 179, lowS = 0, highS = 255, lowV = 0, highV = 255;

    createTrackbar("Low Hue", "HSV Tester", &lowH, 179, NULL);
    createTrackbar("High Hue", "HSV Tester", &highH, 179, NULL);

    createTrackbar("Low Sat", "HSV Tester", &lowS, 255, NULL);
    createTrackbar("High Sat", "HSV Tester", &highS, 255, NULL);

    createTrackbar("Low Value", "HSV Tester", &lowV, 255, NULL);
    createTrackbar("High Value", "HSV Tester", &highV, 255, NULL);


    while(1)
    {
        lowH = getTrackbarPos("Low Hue", "HSV Tester");
        highH = getTrackbarPos("High Hue", "HSV Tester");
        lowS = getTrackbarPos("Low Sat", "HSV Tester");
        highS = getTrackbarPos("High Sat", "HSV Tester");
        lowV = getTrackbarPos("Low Value", "HSV Tester");
        highV = getTrackbarPos("High Value", "HSV Tester");

        Mat frame = CaptureFrameFromCamera();

        Mat GaussianBlurred;
        cv::GaussianBlur(frame, GaussianBlurred, Size(11,11), 0, 0);

        Mat frameHSV;
        cvtColor(frame, frameHSV, COLOR_BGR2HSV);
        inRange(frameHSV, Scalar(lowH, lowS, lowV), Scalar(highH, highS, highV), frameHSV);

        Mat comparison;
        cvtColor(frameHSV, frameHSV, COLOR_GRAY2BGR);
        hconcat(frame, frameHSV, comparison);

        imshow("HSV Tester", comparison);

        int key = cv::waitKey(1);
        key = (key==255) ? -1 : key;
        if (key == 27)
            break;
    }

    destroyAllWindows();

}

int NumberOfPinkPixels(Mat frame)
{
    Mat framePinkSquareHSV = GetFrameHSVForPink(frame);

    return countNonZero(framePinkSquareHSV);
}

int DetectSymbol(void)
{
    float MaxPercentageMatch = 0.0;
    int Command;

    while (MaxPercentageMatch < 67.0)
    {
        /*Capture and filter the frame*/
        Mat Symbolframe = CaptureFrameFromCamera();

        Mat GaussianBlurred;
        cv::GaussianBlur(Symbolframe, GaussianBlurred, Size(11,11), 0, 0);

        Mat SymbolPinkHSV = GetFrameHSVForPink(GaussianBlurred);


        /*Display seperate steps of capturing and filtering frame for debugging*/
        cv::imshow("Symbolframe", Symbolframe);
        cv::waitKey(1);
        cv::imshow("GaussianBlurred", GaussianBlurred);
        cv::waitKey(1);
        cv::imshow("SymbolPinkHSV", SymbolPinkHSV);
        cv::waitKey(1);


        /*Find the contours*/
        std::vector < std::vector<Point> > Contours;            // Variable for list of Contours
        std::vector<Vec4i> Hierarchy;                           // Variable for image topology data
        findContours(SymbolPinkHSV, Contours, Hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
        if(Contours.size() == 0)
        {
            std::cout << "Error in reading contour data" << std::endl;
            continue;
        }


        /*Find the contour with the max area and thus the one the encloses the symbol*/
        int MaxArea = 0, MaxContour, CurrentArea;
        for(int i = 0; i < Contours.size(); i++)
        {
            CurrentArea = cv::contourArea(Contours[i]);

            if(CurrentArea > MaxArea)
            {
                MaxArea = CurrentArea;
                MaxContour = i;
            }
        }


        /*Simplify the biggest contour to 4 points*/
        vector<cv::Point> ApproxedContours; // Array or new Contours
        cv::approxPolyDP(Contours[MaxContour], ApproxedContours, 10, true); // Approximate the contour
        if(ApproxedContours.size() == 0)
        {
            std::cout<< "ERROR: Unable to read approx contour data" << std::endl;
            continue;
        }


        /*Transform the image so that the symbol is rectangular and in the centre and fills the whole image*/
        Mat Transformedframe = transformPerspective(ApproxedContours, SymbolPinkHSV, 320, 240); // Transform the perspective of the four points found in frame into a 320x240 image and return this into transformed.
        if(Transformedframe.data == NULL)
        {
            cout<< "ERROR: Unable to read Transformedframe data" << ApproxedContours.size() << endl;
            continue;
        }



        /*Display transoformed image for debugging*/
        cv::imshow("Transformed", Transformedframe);
        cv::waitKey(1);



        /*Directory for all possible symbols*/
        char Files [8][64] =
        {
            {"/home/pi/Desktop/OpenCV_Template_RPi/FollowBlack.PNG"},
            {"/home/pi/Desktop/OpenCV_Template_RPi/RedShortCut.PNG"},
            {"/home/pi/Desktop/OpenCV_Template_RPi/GreenShortCut.PNG"},
            {"/home/pi/Desktop/OpenCV_Template_RPi/DistanceMeasurement.PNG"},
            {"/home/pi/Desktop/OpenCV_Template_RPi/Football.PNG"},
            {"/home/pi/Desktop/OpenCV_Template_RPi/InclineMeasurement.PNG"},
            {"/home/pi/Desktop/OpenCV_Template_RPi/ShapeCounter.PNG"},
            {"/home/pi/Desktop/OpenCV_Template_RPi/StopLight.PNG"},
        };



        /*Find the symbol that matches most with the Transformedframe*/
        float CurrerntPercentage;
        int MaxPercentagePosition;

        for (int i = 0; i < 8; i++)
        {
            /*Load in the current image file*/
            Mat Comparisonframe = imread(Files[i]);
            if (Comparisonframe.data == NULL)
            {
                std::cout << "No image found " <<i<< "! Check path." << std::endl;
                exit(1);
            }

            /*Put loaded file through the same filtering for a better comparison*/
            Mat ComparisonGaussianBlurred;
            cv::GaussianBlur(Comparisonframe, ComparisonGaussianBlurred, Size(11,11), 0, 0);

            Mat ComparisonframePinkHSV = GetFrameHSVForPink(ComparisonGaussianBlurred);

            /*Display seperate steps of capturing and filtering Comparisonframe for debugging*/
            cv::imshow("Comparisonframe", Comparisonframe);
            cv::waitKey(1);
            cv::imshow("ComparisonGaussianBlurred", ComparisonGaussianBlurred);
            cv::waitKey(1);
            cv::imshow("ComparisonframePinkHSV", ComparisonframePinkHSV);
            cv::waitKey(1);

            /*Calculate the percentage match of current comparisonframe and the Transformedframe*/
            CurrerntPercentage = compareImages(Transformedframe, ComparisonframePinkHSV);

            /*Keep track of current highest percentage and the symbol that produced it*/
            if (CurrerntPercentage > MaxPercentageMatch)
            {
                MaxPercentageMatch = CurrerntPercentage;
                Command = i;
            }
        }

        /*Display the file directory and its percentage match for debugging purposes*/
        std::cout << "Symbol is " << Files[MaxPercentagePosition] << "with a percentage match of " << MaxPercentageMatch << std::endl;
    }

    return Command;
}




int CalculateSpeedAndDirectionForLineFollowing (Mat frameHSV)
{
    int x;

    uchar* p = frameHSV.ptr<uchar>(1);
    int DistanceFromLeft;
    for(x = 0, DistanceFromLeft = 0; x < frameHSV.cols; x++)
    {
        DistanceFromLeft++;

        p[x];

        uchar pixel = p[x];

        if(p[x] == 255)
            break;
    }

    uchar* q = frameHSV.ptr<uchar>(1); //1 is chosen so bottom of image is read
    int DistanceFromRight;
    for(x = 319, DistanceFromRight = 0; x >= 0; x--)
    {
        DistanceFromRight++;

        q[x];

        uchar pixel = q[x];

        if(q[x] == 255)
            break;
    }

//    std::cout << "Distance from the left = " << DistanceFromLeft << "\t" <<"Distance from the right = " << DistanceFromRight << std::endl;

    int Speed;

    if((DistanceFromLeft == 320 && DistanceFromRight == 320) || (DistanceFromLeft == 1 && DistanceFromRight == 1))
        Speed = 0;

    else if (DistanceFromLeft == 1)
        Speed = 255 + 256;

    else if (DistanceFromRight == 1)
        Speed = 255;

    else if(DistanceFromLeft > DistanceFromRight)
    {
        Speed = DistanceFromLeft - DistanceFromRight;
        Speed = (255.0/320.0)*Speed;
    }

    else if (DistanceFromRight > DistanceFromLeft)
    {
        Speed = DistanceFromRight - DistanceFromLeft;
        Speed = (255.0/320.0)*Speed + 256;
    }

//    std::cout << "Speed = " << Speed << std::endl;

    return Speed;
}

void FollowBlackLine(Mat frame)
{
    Mat frameBlackHSV = GetFrameHSVForBlack(frame);

    int Speed = CalculateSpeedAndDirectionForLineFollowing(frameBlackHSV);

    if (car.i2cWriteArduinoInt(Speed) < 0)
        std::cout << "Error in sending speed for black line" << std::endl;
}

void FollowRedLine(Mat frame)
{
    Mat frameRedHSV = GetFrameHSVForRed(frame);

    int Speed = CalculateSpeedAndDirectionForLineFollowing(frameRedHSV);

    if (car.i2cWriteArduinoInt(Speed) < 0)
        std::cout << "Error in sending speed for red line" << std::endl;
}

void FollowGreenLine(Mat frame)
{
    Mat frameGreenHSV = GetFrameHSVForGreen(frame);

    int Speed = CalculateSpeedAndDirectionForLineFollowing(frameGreenHSV);

    if (car.i2cWriteArduinoInt(Speed) < 0)
        std::cout << "Error in sending speed for green line" << std::endl;
}

void StopCar(void)
{
    if (car.i2cWriteArduinoInt(0) < 0)
        std::cout << "Error in stopping car" << std::endl;
}

void TiltCameraUp(void)
{
    if (sensor.i2cWriteArduinoInt(1) < 0)
            std::cout << "Error in sending tilt camera up command" << std::endl;
}

void TiltCameraDown(void)
{
    if (sensor.i2cWriteArduinoInt(2) < 0)
        std::cout << "Error in sending tilt camera down command" << std::endl;
}

void MeasureDistance(void)
{
    if (sensor.i2cWriteArduinoInt(3) < 0)
        std::cout << "Error in sending measure distance command" << std::endl;
}

void Football(void)
{



}

void MeasureIncline(void)
{
    if (sensor.i2cWriteArduinoInt(4) < 0)
        std::cout << "Error in sending measure distance command" << std::endl;
}

void CountShapes(void)
{



}

void StopLight(void)
{



}

#endif // MAIN_HPP_INCLUDED

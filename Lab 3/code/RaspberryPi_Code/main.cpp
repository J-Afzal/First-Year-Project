#include <iostream>
#include "opencv_aee.hpp"
#include "main.hpp"
#include <stdlib.h>

using namespace std;

int main(int argc, char** argv)
{
    setup();    // Call a setup function to prepare IO and devices

    //HSVCamera();

    int Command = 0;

    while(1)    // Main loop to perform image processing
    {
        Mat frame = CaptureFrameFromCamera();
        FollowBlackLine(frame);

        /*Check if the pink square is present*/
        if (NumberOfPinkPixels(frame) >= 250)
        {
            StopCar();

            TiltCameraUp();

            Command = DetectSymbol();

            TiltCameraDown();
        }

        /*Work out the correct command */
        switch (Command)
        {
        case 0:
            std::cout << "Folllow black line command" << std::endl;
            FollowBlackLine(frame);
            break;

        case 1:
            std::cout << "Red short cut command" << std::endl;
            FollowRedLine(frame);
            break;

        case 2:
            std::cout << "Green short cut command" << std::endl;
            FollowGreenLine(frame);
            break;

        case 3:
            std::cout << "Distance measuremnt command" << std::endl;
            MeasureDistance();
            FollowBlackLine(frame);
            break;

        case 4:
            std::cout << "Football command" << std::endl;
            Football();
            FollowBlackLine(frame);
            break;

        case 5:
            std::cout << "Incline measurement command" << std::endl;
            MeasureIncline();
            FollowBlackLine(frame);
            break;

        case 6:
            std::cout << "Shape counting command" << std::endl;
            CountShapes();
            FollowBlackLine(frame);
            break;

        case 7:
            std::cout << "Stop light command" << std::endl;
            StopLight();
            FollowBlackLine(frame);
            break;

        default:
            std::cout << "Error in switch statement for command" << std::endl;
            break;
        }

        cv::imshow("frame", frame);

        int key = cv::waitKey(1);   // Wait 1ms for a keypress (required to update windows)
        key = (key==255) ? -1 : key;    // Check if the ESC key has been pressed
        if (key == 27)
            break;
    }

    closeCV();  // Disable the camera and close any windows

    return 0;
}

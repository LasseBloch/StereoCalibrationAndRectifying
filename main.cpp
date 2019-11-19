#include <iostream>
#include <opencv2/opencv.hpp>
const float squareSideLength = 0.035; // in meters

// Get a video capture for a camra, and do some validating that we can read an image from it
cv::VideoCapture getVideoCapture(int camNr)
{
    // Cam we open the capture device
    auto captureDev = cv::VideoCapture(camNr);
    if (captureDev.isOpened())
    {
        // Can we read a valid frame from the capture device
        cv::Mat image;
        if (captureDev.read(image))
        {
            // NOTE: Note sure if this check is needed or read does all the required word
            if (image.empty())
            {
                std::cout << "Read empty frame from VideoCapture(" << camNr << ")\n";
                exit(-1);
            }
            return captureDev;
        }
        std::cout << "Could not read frame from VideoCapture(" << camNr << ")\n";
        exit(-1);
    }
    else
    {
        std::cout << "Could not open VideoCapture(" << camNr << ")\n";
        exit(-1);
    }
}


void calibrateStereoCam(cv::Size boardSize, const int nrCalibPicturesToTake)
{
    // Capture calibration pictures
    int goodCalibrationParses{0};
    while(goodCalibrationParses < nrCalibPicturesToTake)
    {
        // When we find chessboard in both cams
        goodCalibrationParses++;
    }
}


int main()
{
    // board width and hight is the actually the number of inner corners on the calibration chessboard
    const auto boardWight{7};
    const auto boardHeight{5};
    const cv::Size boardSize{boardWight, boardHeight};

    // Number of pictures to use for the stereo calibration, by capturing them directly from the cam
    // TODO: Change this to read a series of pictures from disk
    const int nrCalibrationPictures{10};

    auto cam0 = getVideoCapture(1);


    calibrateStereoCam(boardSize, nrCalibrationPictures);

    return 0;
}
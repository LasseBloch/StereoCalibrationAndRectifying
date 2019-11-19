#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

const float squareSideLength = 0.035; // in meters

std::vector<cv::Point3f> calculateCornerPositonsForBoard(int boardWidth, int boardHeight, float squareSideLenght)
{
    std::vector<cv::Point3f> result;
    for(int i = 0; i < boardWidth; i++)
    {
        for (int j = 0; j < boardHeight; j++)
        {
            result.emplace_back(cv::Point3f{i * squareSideLenght, j * squareSideLenght, 0.0f} );
        }
    }
    return result;
}

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
            // NOTE: not sure if this check is needed or read does all the required word
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


void calibrateStereoCam(cv::Size boardSize, const int nrCalibPicturesToTake, cv::VideoCapture& cam0, cv::VideoCapture& cam1)
{
    // Setup
    cv::Mat img0, img1;
    // Get image size by capturing a frame
    cam0 >> img0;
    cv::Size imgSize = img0.size();

    // imagePoints: Where the we will store the found locations of the chessboard corners
    std::vector<std::vector<cv::Point2f> > imagePoints[2];
    std::vector<std::vector<cv::Point3f> > objectPoints;

    // Capture calibration pictures
    int goodCalibrationParses{0};
    while(goodCalibrationParses < nrCalibPicturesToTake)
    {
        // Capture from both cams
        cam0 >> img0;
        cam1 >> img1;
        if (img0.empty() || img1.empty())
        {
            std::cout << "Captured empty image \n";
            exit(-1);
        }

        std::vector<cv::Point2f> corners0;
        std::vector<cv::Point2f> corners1;
        // Look for corners of the chessboard
        auto found0 = findChessboardCorners(img0, boardSize, corners0);
        auto found1 = findChessboardCorners(img0, boardSize, corners1);

        if (found0 && found1)
        {
            // For img0
            cv::Mat cimg0, cimg01;
            // For img1
            cv::Mat cimg1, cimg11;
            // Greyscale them images
            cvtColor(img0, cimg0, cv::COLOR_GRAY2BGR);
            cvtColor(img1, cimg1, cv::COLOR_GRAY2BGR);
            // Draw corners on the greyscales
            drawChessboardCorners(cimg0, boardSize, corners0, found0);
            drawChessboardCorners(cimg1, boardSize, corners1, found1);
            // Calculate scale factor (should same for both)
            double sf = 640./MAX(img0.rows, img0.cols);
            // Resize both images
            resize(cimg0, cimg01, cv::Size(), sf, sf, cv::INTER_LINEAR_EXACT);
            resize(cimg1, cimg11, cv::Size(), sf, sf, cv::INTER_LINEAR_EXACT);

            imshow("corners0", cimg01);
            imshow("corners1", cimg11);
            // When we find chessboard in both cams
            char c = (char)cv::waitKey(500);
            if( c == 27 || c == 'q' || c == 'Q' ) //Allow ESC to quit
            {
                exit(-1);
            }
            goodCalibrationParses++;
        }
        // Display images
        cv::imshow("img0", img0);
        cv::imshow("img1", img1);
        char c = (char)cv::waitKey(20);
        if( c == 27 || c == 'q' || c == 'Q' ) //Allow ESC to quit
        {
            exit(-1);
        }
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

    auto cam0 = getVideoCapture(0);
    auto cam1 = getVideoCapture(1);

    calibrateStereoCam(boardSize, nrCalibrationPictures, cam0, cam1);

    return 0;
}
#include <iostream>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <future>
#include <vector>

const float squareSideLength = 0.035; // in meters

std::vector<cv::Point3f> calculateCornerPositionsForBoard(int boardWidth, int boardHeight, float squaresSideLength)
{
    std::vector<cv::Point3f> result;
    for(int i = 0; i < boardWidth; i++)
    {
        for (int j = 0; j < boardHeight; j++)
        {
            result.emplace_back(cv::Point3f{i * squaresSideLength, j * squaresSideLength, 0.0f} );
        }
    }
    return result;
}

// Get a video capture for a camera, and do some validating that we can read an image from it
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

// findChessboardCorners takes a long time if the picture does not contain a chessboard
// cvCheckChessboard should be much faster, and returns whether a pictures contains a chessboard or not
bool CheckIfBothPicturesContainChessboard(const cv::Mat& img0, const cv::Mat& img1, const cv::Size& boardSize)
{
    auto startTime = std::chrono::system_clock::now();
    std::chrono::duration<float> elapsedTime = std::chrono::system_clock::now() - startTime;
    bool found0 = cv::checkChessboard(img0, boardSize);
    bool found1 =  cv::checkChessboard(img0, boardSize);
    std::cout << "It took " << elapsedTime.count() << "To see if both pictures contain a chessboard\n";
    return found0 && found1;
}


void calibrateStereoCam(cv::Size boardSize, const int nrCalibPicturesToTake, cv::VideoCapture& cam0, cv::VideoCapture& cam1)
{
    // Setup
    cv::Mat rawImg0, rawImg1, img0, img1;
    // Get image size by capturing a frame
    cam0 >> rawImg0;
    cv::Size imgSize = rawImg0.size();

    // imagePoints: Where the we will store the found locations of the chessboard corners
    std::vector<std::vector<cv::Point2f> > imagePoints[2];
    std::vector<std::vector<cv::Point3f> > objectPoints;

    // Capture calibration pictures
    int goodCalibrationParses{0};
    while(goodCalibrationParses < nrCalibPicturesToTake)
    {
        // Capture from both cams
        cam0 >> rawImg0;
        cam1 >> rawImg1;
        if (rawImg0.empty() || rawImg1.empty())
        {
            std::cout << "Captured empty image \n";
            exit(-1);
        }

        // Convert images to grey scale
        cv::cvtColor(rawImg0, img0, cv::COLOR_BGR2GRAY);
        cv::cvtColor(rawImg1, img1, cv::COLOR_BGR2GRAY);
        std::vector<cv::Point2f> corners0;
        std::vector<cv::Point2f> corners1;
        if (CheckIfBothPicturesContainChessboard(img0, img1, boardSize))
        {
            // Look for corners of the chessboard
            std::cout << "looking for corners in pictures\n";
            auto startTime = std::chrono::system_clock::now();
            // Wrap call to findChessboardCorners in future's to utilize multiple cores
            std::future<bool> findCBCornersImg0 = std::async(std::launch::async, [&]{return findChessboardCorners(img0, boardSize, corners0); });
            std::future<bool> findCBCornersImg1 = std::async(std::launch::async, [&]{return findChessboardCorners(img1, boardSize, corners1); });
            findCBCornersImg0.wait();
            findCBCornersImg1.wait();
            auto found0 = findCBCornersImg0.get();
            auto found1 = findCBCornersImg1.get();
            std::chrono::duration<float> elapsedTime = std::chrono::system_clock::now() - startTime;
            std::cout << "It took " << elapsedTime.count() << '\n';

            if (found0 && found1)
            {
                std::cout << "Found chessboard in both images\n";
                // For img0
                cv::Mat img0WithCorners, img0WithCornersUpScaled;
                // For img1
                cv::Mat img1WithCorners, img1WithCornersUpScaled;

                img0WithCorners = img0.clone();
                img1WithCorners = img1.clone();

                // Draw corners on the greyscale pictures
                drawChessboardCorners(img0WithCorners, boardSize, corners0, found0);
                drawChessboardCorners(img1WithCorners, boardSize, corners1, found1);

                // Upscale pictures so it is easier to see if the corners are place correctly
                // Scale factor
                double sf = 1.5;
                std::cout << sf << '\n';
                // Resize both images
                resize(img0WithCorners, img0WithCornersUpScaled, cv::Size(), sf, sf, cv::INTER_LINEAR_EXACT);
                resize(img1WithCorners, img1WithCornersUpScaled, cv::Size(), sf, sf, cv::INTER_LINEAR_EXACT);

                imshow("corners0", img0WithCornersUpScaled);
                imshow("corners1", img1WithCornersUpScaled);
                // Todo: Make logic to keep or discard images
                // When we find chessboard in both cams
                char c = (char)cv::waitKey(500);
                if( c == 27 || c == 'q' || c == 'Q' ) //Allow ESC to quit
                {
                    exit(-1);
                }
                goodCalibrationParses++;
            }
        }


        // Display images
        cv::imshow("img0", img0);
        cv::imshow("img1", img1);
        std::cout << "Looping around \n";
        char c = (char)cv::waitKey(10);
        if( c == 27 || c == 'q' || c == 'Q' ) //Allow ESC to quit
        {
            exit(-1);
        }
    }
}


int main()
{
    // board width and height is the actually the number of inner corners on the calibration chessboard
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
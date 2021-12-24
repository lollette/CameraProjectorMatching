#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <bitset>
#include <string>
#include <iostream>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>


using namespace std;
using namespace cv;


Mat capturePatternFct(VideoCapture capture)
{
    int nonValidFrame = 20;
    Mat gray;
    while(nonValidFrame >0)
    {
        Mat frame;
        capture >> frame;
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        waitKey(40);
        nonValidFrame -=1;
    }
    return gray;
}


int  findScreen(Mat maskScreen, Size projSize)
{
    Mat imageCamPattern(projSize.height,projSize.width, CV_8U, 255);
    Mat contourScreen = Mat::zeros(maskScreen.rows,maskScreen.cols, CV_8U);
    Mat  thresh;

    vector<vector<Point> > contours;
    vector<double> area;

    VideoCapture capture(0);
    if(!capture.isOpened())
    return -1;

    imshow( "Display window", imageCamPattern );

    Mat captureScreenPattern = capturePatternFct(capture);

    for(int i=0; i < captureScreenPattern.rows; ++i)
    {
        for(int j =0; j < captureScreenPattern.cols; ++j)
        {
            int tmp = captureScreenPattern.at<uchar>(i,j);
            if(tmp > 60)
            {
                contourScreen.at<uchar>(i,j) = 255;
            }

        }
    }

    threshold( contourScreen, thresh, 127, 255, THRESH_BINARY );
    findContours( thresh, contours,  RETR_EXTERNAL, CHAIN_APPROX_NONE);

    for( int i = 0; i< contours.size(); i++ )
    {
        area.push_back(contourArea(contours[i]));
    }
    int maxArea = distance(area.begin(), max_element(area.begin(), area.end()));
    drawContours( maskScreen, contours, maxArea, 255,cv::FILLED, 8 );
    capture.release();
    return 0;
}

Mat processCapture(Mat capture, Mat maskScreen)
{
    Mat crop;
    bitwise_and(capture, maskScreen, crop);
    return crop;
}

int pattern2Code(vector<Mat> &imagePatternVect,
                  Size camSize,
                  Mat maskScreen,
                  vector<Mat> &codeVect)
{
    namedWindow( "Display window", WND_PROP_FULLSCREEN );
    setWindowProperty("Display window", WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);

    int idxImgPat;
    for(idxImgPat=0; idxImgPat<imagePatternVect.size(); ++idxImgPat)
    {
        VideoCapture capture(0);
        if(!capture.isOpened())
        return -1;

        imshow("Display window", imagePatternVect[idxImgPat]);
        Mat capturePattern = capturePatternFct(capture);
        Mat capturePatternCrop = processCapture(capturePattern, maskScreen);
        //imshow("capturePatternCrop", capturePatternCrop);
        //waitKey(0);

        Mat imagePatternInv;
        imagePatternVect[idxImgPat].copyTo(imagePatternInv);
        imagePatternInv = 255 -imagePatternInv;
        imshow("Display window", imagePatternInv);
        Mat capturePatternInv = capturePatternFct(capture);
        Mat capturePatternCropInv = processCapture(capturePatternInv,
                                                   maskScreen);
        //imshow("capturePatternCropInv", capturePatternCropInv);
        //waitKey(0);

        Mat subtractPattern = Mat::zeros(camSize.height,camSize.width, CV_8U);
        subtractPattern = capturePatternCrop - capturePatternCropInv;

        int i,j;
        for(i=0; i<subtractPattern.rows; ++i)
        {
            for(j=0; j<subtractPattern.cols; ++j)
            {
                if(subtractPattern.at<uchar>(i,j)>0)
                {
                    subtractPattern.at<uchar>(i,j) = 1;
                }
                else
                {
                    subtractPattern.at<uchar>(i,j) = 0;
                }
            }
        }

        codeVect.push_back(subtractPattern.clone());
        capture.release();
    }
}

string int2String(int i)
{
    stringstream s;
    s << i;
    return s.str();
}

#endif

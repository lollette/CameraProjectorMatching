#include <iostream>
#include <sstream>
#include <string>

#include <cstdio>
#include <dirent.h>
#include <math.h>
#include <fstream>

#include <opencv2/core.hpp>

#include <opencv2/highgui.hpp>

#include "grayCodePattern.hpp"
#include "utils.hpp"

using namespace std;
using namespace cv;

void cameraProjectorMatching(GrayCodeGenerator pat,
                             Size camSize,
                             vector<Mat> &codeVect,
                             int *matchingArr)
{
    int i,j, idxPat;
    unsigned int bin2IntVar, gray2IntVar;

    for(i=0; i<camSize.height; ++i)
    {
        for(j=0; j<camSize.width; ++j)
        {
            string int2StringVar;
            for(idxPat=0; idxPat<codeVect.size(); ++idxPat)
            {
                int2StringVar += pat.int2String(
                            (int)(codeVect[idxPat].at<uchar>(i, j)));
            }
            bin2IntVar = pat.binary2Int(int2StringVar);
            gray2IntVar = pat.gray2Int(bin2IntVar);

            matchingArr[(i*camSize.width +j)]= gray2IntVar;
        }
    }
}


int main (int argc, char* argv[])
{
    namedWindow( "Display window", WND_PROP_FULLSCREEN );
    setWindowProperty("Display window", WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);

    Size camSize(640,480);
    Size projSize(1920,1080);

    Mat maskScreen = Mat::zeros(camSize.height,camSize.width, CV_8U);
    findScreen(maskScreen, projSize);

    GrayCodeGenerator pat;
    pat.getGrayCodeH(projSize);
    vector<Mat> horizCodeVect;
    pattern2Code(pat.m_imagePatternVcet, camSize, maskScreen, horizCodeVect);

    int *horizMatchingArr;
    horizMatchingArr = (int *)malloc(camSize.width*camSize.height*sizeof(int));
    cameraProjectorMatching(pat, camSize, horizCodeVect, horizMatchingArr);
    //cout<<horizMatchingArr[237*640+316]<<endl;

    pat.getGrayCodeV(projSize);
    vector<Mat> vertiCodeVect;
    pattern2Code(pat.m_imagePatternVcet, camSize, maskScreen, vertiCodeVect);

    int *vertiMatchingArr;
    vertiMatchingArr = (int *)malloc(camSize.width*camSize.height*sizeof(int));
    cameraProjectorMatching(pat, camSize,vertiCodeVect, vertiMatchingArr);
    //cout<<vertiMatchingArr[237*640+316]<<endl;

    // matching map

    Mat lutMap = Mat::zeros(camSize.height, camSize.width, CV_16UC3);
    int i, j;
    for(i=0; i<camSize.height; ++i)
    {
        for(j=0; j<camSize.width; ++j)
        {
            lutMap.at<Vec3w>(i,j)[0] =0;
            lutMap.at<Vec3w>(i,j)[1] = (horizMatchingArr[i*camSize.width+j]
                    * 65536) / projSize.height;
            lutMap.at<Vec3w>(i,j)[2] = (vertiMatchingArr[i*camSize.width+j]
                    * 65536) / projSize.width;

        }
    }
    imshow("Matching Map", lutMap);

    // test center

    Mat centerMap = Mat::zeros(projSize.height, projSize.width, CV_8U);
    for(i=0; i<projSize.height; ++i)
    {
        centerMap.at<uchar>(i,vertiMatchingArr[237*camSize.width+316]) =255;
    }
    for(int i=0; i<projSize.width; ++i)
    {
        centerMap.at<uchar>(horizMatchingArr[237*camSize.width+316],i) =255;
    }
    imshow("Display window", centerMap);
    VideoCapture capture(0);
    if(!capture.isOpened())
    return -1;

    Mat capturePattern = capturePatternFct(capture);
    imshow("Centetr Map", capturePattern);
    waitKey(0);

    return 0;
}

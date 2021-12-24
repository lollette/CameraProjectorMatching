#include <iostream>
#include <sstream>
#include <string>

#include <cstdio>
#include <dirent.h>
#include <math.h>
#include <fstream>

#include <opencv2/core.hpp>

#include <opencv2/highgui.hpp>

#include "moduloCodePattern.hpp"
#include "utils.hpp"

using namespace std;
using namespace cv;

void cameraProjectorMatching(ModuloCodeGenerator pat, int pSize,
                             int *patCode, Size camSize,
                             vector<Mat> &codeVect,
                             int *matchingArr)
{
    int i, j, k, idxPat;
    int target;
    unsigned int bin2IntVar1, bin2IntVar2;

    for(i=0; i<camSize.height; ++i)
    {
        for(j=0; j<camSize.width; ++j)
        {
            string int2StringVar1;
            string int2StringVar2;
            for(idxPat=0; idxPat<codeVect.size()/2; ++idxPat)
            {
                int2StringVar1 += pat.int2String(
                            (int)(codeVect[idxPat].at<uchar>(i, j)));
                int2StringVar2 += pat.int2String(
                            (int)(codeVect[idxPat+codeVect.size()/2].at<uchar>
                            (i, j)));
            }

            bin2IntVar1 = pat.binary2Int(int2StringVar1);
            bin2IntVar2 = pat.binary2Int(int2StringVar2);
            int md1mod2 = 102;

            for (k =0; k<pSize; k++)
            {
                if(abs(bin2IntVar1- patCode[(k*2 +0)]) +
                        abs(bin2IntVar2- patCode[(k*2 +1)])< md1mod2)
                {
                    md1mod2 = abs(bin2IntVar1- patCode[(k*2 +0)]) +
                            abs(bin2IntVar2- patCode[(k*2 +1)]);
                    target = k;

                }
            }



            matchingArr[(i*camSize.width +j)]= target;
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

    ModuloCodeGenerator pat;
    int *patCodeH;
    patCodeH= (int *)malloc(projSize.height*2*sizeof(int));
    for(int i = 0; i<projSize.height; i++)
    {
        patCodeH[(i*2 +0)]=i%59 ;
        patCodeH[(i*2 +1)]=i%43 ;
    }
    int *patCodeV;
    patCodeV= (int *)malloc(projSize.width*2*sizeof(int));
    for(int i = 0; i<projSize.width; i++)
    {
        patCodeV[(i*2 +0)]=i%59 ;
        patCodeV[(i*2 +1)]=i%43 ;
    }

    pat.getModuloCodeH(projSize);
    vector<Mat> horizCodeVect;
    pattern2Code(pat.m_imagePatternVcet, camSize, maskScreen, horizCodeVect);
    int *horizMatchingArr;
    horizMatchingArr = (int *)malloc(camSize.width*camSize.height*sizeof(int));
    cameraProjectorMatching(pat, projSize.height, patCodeH,
                            camSize, horizCodeVect, horizMatchingArr);

    cout<<horizMatchingArr[237*640+316]<<endl;

    pat.getModuloCodeV(projSize);
    vector<Mat> vertiCodeVect;
    pattern2Code(pat.m_imagePatternVcet, camSize, maskScreen, vertiCodeVect);

    int *vertiMatchingArr;
    vertiMatchingArr = (int *)malloc(camSize.width*camSize.height*sizeof(int));
    cameraProjectorMatching(pat, projSize.width, patCodeV,
                            camSize,vertiCodeVect, vertiMatchingArr);
    cout<<vertiMatchingArr[237*640+316]<<endl;

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

#include <iostream>
#include <sstream>
#include <string>

#include <cstdio>
#include <dirent.h>
#include <math.h>
#include <fstream>
#include <sys/stat.h>

#include <gmp.h>

#include <opencv2/core.hpp>

#include <opencv2/highgui.hpp>

#include <time.h>

#include "leopardPattern.hpp"
#include "utils.hpp"

using namespace std;
using namespace cv;

string checkDir(string path, string pathDir, string mode)
{
    struct stat sb;
    if (stat(path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))
    {
        string patternDir = path + string("/") + pathDir;
        if (stat(patternDir.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))
        {
            DIR *dir;
            struct dirent* readFile = NULL;
            dir = opendir (patternDir.c_str());
            if (dir == NULL)
                exit(-1);
            int nbrFile =0;
            while ((readFile = readdir(dir)) != NULL)
            {
                nbrFile++;
            }
            if(nbrFile>2)
            {
                if(mode == "Capture")
                {
                    cout << "the "+ pathDir +" directory is not empty choose "
                            "another directory or delete this one. "
                                            "Application stopping. "
                         << endl;
                    exit(-1);
                }
                if(mode == "Read")
                {
                    return patternDir;
                }

            }
            else
            {
                if(mode == "Capture")
                {
                    return patternDir;
                }
                if(mode == "Read")
                {
                    cout << "the "+ pathDir +" directory is empty. "
                            "please choose capture mode"
                         << endl;
                    exit(-1);
                }
            }
        }
        else
        {
            if(mode == "Capture")
            {
                mkdir(patternDir.c_str(),  0777);
                return patternDir;
            }
            if(mode == "Read")
            {
                cout << "the "+ pathDir +" directory does not exist. "
                        "please choose capture mode"
                     << endl;
                exit(-1);
            }
        }
    }
    else
    {
        if(mode == "Capture")
        {
            mkdir(path.c_str(),  0777);
            string patternDir = path + string("/") + pathDir;
            mkdir(patternDir.c_str(),  0777);
            return patternDir;
        }
        if(mode == "Read")
        {
            cout << "the"+ path +"directory does not exist. "
                    "please choose capture mode"
                 << endl;
            exit(-1);
        }
    }
}


void readModeFct(string path2CamPat, string path2ProjPat,int nbrPat,
                 Size camSize, Size projSize, unsigned char *projImg,
                 unsigned char *camImg)
{
    for(int idxPat=0; idxPat<nbrPat; ++idxPat)
    {
        Mat capturePatternCrop = imread(
                    cv::format( (path2CamPat+"/cap-0%d.png").c_str(),
                                idxPat ), 0);

        for(int i=0; i<camSize.width*camSize.height; i++)
            camImg[idxPat*camSize.width*camSize.height + i] =
                    capturePatternCrop.at<uchar>(i);

        Mat Pattern = imread(
                    cv::format( (path2ProjPat+"/ref-0%d.png").c_str(),
                                idxPat ), 0);
        for(int i=0; i<projSize.width*projSize.height; i++)
            projImg[idxPat*projSize.width*projSize.height + i] =
                    Pattern.at<uchar>(i);
    }
}


void captureModeFct(string path2CamPat, string path2ProjPat,int nbrPat,
                 Size camSize, Size projSize, unsigned char *projImg,
                 unsigned char *camImg)
{
    namedWindow( "Display window", WND_PROP_FULLSCREEN );
    setWindowProperty("Display window", WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);

    Mat maskScreen = Mat::zeros(camSize.height,camSize.width, CV_8UC1);
    findScreen(maskScreen, projSize);

    LeopardCodeGenerator pattern;
    pattern.setParameters(projSize, 32, true);

    for(int idxPat=0; idxPat<nbrPat; ++idxPat)
    {
        Mat Pattern = pattern.getLeopard();
        VideoCapture capture(0);
        if(!capture.isOpened())  // check if we succeeded
            exit(-1);

        imshow("Display window", Pattern);
        Mat capturePattern = capturePatternFct(capture);
        Mat capturePatternCrop = processCapture(capturePattern, maskScreen);


        for(int i=0; i<camSize.width*camSize.height; i++)
            camImg[idxPat*camSize.width*camSize.height + i] =
                    capturePatternCrop.at<uchar>(i);

        for(int i=0; i<projSize.width*projSize.height; i++)
            projImg[idxPat*projSize.width*projSize.height + i] =
                    Pattern.at<uchar>(i);

        imwrite(cv::format( (path2CamPat+"/cap-0%d.png").c_str(), idxPat ),
                capturePatternCrop);
        imwrite( cv::format( (path2ProjPat+"/ref-0%d.png").c_str(), idxPat ),
                 Pattern);
        capture.release();
    }
}

void codeGenerator(int nbrPat,
                   Size imgSize,
                   unsigned char *arrImg,
                   mpz_t *arrCode)
{
    cout << "je rentre  "<<endl;

    for(int i=0; i<imgSize.width*imgSize.height; ++i)
    {
        mpz_t hash;
        mpz_init(hash);
        int idx =(nbrPat*(nbrPat-1))/2 -1;

        for(int idxarrImg=0; idxarrImg<nbrPat-1; ++idxarrImg)
        {
            for(int idxarrImgPlus=idxarrImg+1; idxarrImgPlus<nbrPat;
                ++idxarrImgPlus)
            {
                if((int)(
                         arrImg[idxarrImg*imgSize.width*imgSize.height + i]-
                         arrImg[idxarrImgPlus*imgSize.width*imgSize.height + i])
                        >0)
                {
                    mpz_setbit(hash,idx);
                }
                else
                {
                    if((int)(
                             arrImg[idxarrImg*imgSize.width*imgSize.height + i]-
                             arrImg[idxarrImgPlus*imgSize.width*imgSize.height
                                + i])  == 0)
                    {
                        if(rand() % 2 == 1)mpz_setbit(hash,idx);
                    }

                }
                --idx;
            }
        }
        mpz_set(arrCode[i], hash);
    }
}
int main (int argc, char* argv[])
{
    clock_t tStart = clock();

    //  check settings
    if (argc<5)
    {
        cout<< "Usage Capture: " << argv[0] << "\n c: capture "
                     "new patterns  or r: read patterns\n number of"
                     "patterns\n outputFileNameCapturePattern or "
                     "inputFileNamePattern\n outputFileNameResults" <<endl;
        return 1;
    }

    char* captureRead = argv[1];
    int nbrPat = atoi(argv[2]);
    string patternDir = argv[3];
    string resultDir = argv[4];

    if (nbrPat<4)
    {
        cout<<"Invalid number of patterns (<4). Application stopping." <<endl;
        return -1;
    }
    if (captureRead[0]!='c' && captureRead[0]!='r')
    {
        cout<<"Invalid input detected (c or r). Application stopping."<<endl;
        return -1;
    }
    string path2ProjPat;
    string path2CamPat;

    Size camSize(640,480);
    Size projSize(1920,1080);

    unsigned char *projImg;
    unsigned char *camImg;

    projImg=(unsigned char  *)malloc(1920*1080*nbrPat*sizeof(unsigned char ));
    camImg=(unsigned char *)malloc(640*480*nbrPat*sizeof(unsigned char));

    if (captureRead[0]=='c')
    {
        path2ProjPat = checkDir(patternDir, "projector", "Capture") + "/";
        path2CamPat = checkDir(patternDir, "camera", "Capture") + "/";
        captureModeFct(path2CamPat, path2ProjPat, nbrPat, camSize, projSize, projImg, camImg);
    }
    if (captureRead[0]=='r')
    {
        path2ProjPat = checkDir(patternDir, "projector", "Read") + "/";
        path2CamPat = checkDir(patternDir, "camera", "Read") + "/";
        readModeFct(path2CamPat, path2ProjPat, nbrPat, camSize, projSize, projImg, camImg);
    }
    mpz_t *projCode;
    mpz_t *camCode;

    projCode =(mpz_t *)malloc(projSize.width*projSize.height*sizeof(mpz_t));
    camCode =(mpz_t *)malloc(camSize.width*camSize.height*sizeof(mpz_t));

    for(int i=0; i<projSize.width*projSize.height; i++) mpz_init(projCode[i]);
    for(int i=0; i<camSize.width*camSize.height; i++) mpz_init(camCode[i]);

    codeGenerator( nbrPat, camSize, camImg, camCode);
    printf("Time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
    codeGenerator( nbrPat, projSize, projImg, projCode);
    printf("Time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);

    int *matchingArr;
    matchingArr = (int *)malloc(camSize.width*camSize.height*3*sizeof(int));
    for(int i=0; i<camSize.width*camSize.height*3; i++) matchingArr[i] = 0;

    for(int iCam=65; iCam<390/*camSize.height*/; ++iCam)
    {
      for(int jCam =92; jCam<527/*camSize.width*/; ++jCam)
      {
        mp_bitcnt_t distance= ((nbrPat*(nbrPat-1))/2)+1;
        for(int iProj=0; iProj<projSize.height; iProj+=4/*++iPro*/)
        {
            for(int jProj=0; jProj<projSize.width; jProj+=4/*++jProj*/)
            {
                mp_bitcnt_t tmpDist = mpz_hamdist(
                            camCode[iCam*camSize.width + jCam],
                            projCode[iProj*projSize.width + jProj]);
                if((int)tmpDist<(int)distance)
                {
                    distance = tmpDist;
                    matchingArr[iCam*camSize.width*3 +jCam*3 + 0] =
                            (int)tmpDist;
                    matchingArr[iCam*camSize.width*3 +jCam*3 + 1] = iProj;
                    matchingArr[iCam*camSize.width*3 +jCam*3 + 2]=jProj;
                }
            }
       }
      }
    }
    printf("Time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);


    Mat lutMap = Mat::zeros(camSize.height, camSize.width, CV_16UC3);
    for(int i=0; i<camSize.height; ++i)
    {
        for(int j=0; j<camSize.width; ++j)
        {
            lutMap.at<Vec3w>(i,j)[0] =(matchingArr[i*camSize.width*3 + j*3 + 0]
                      * 65536) / (nbrPat*(nbrPat-1))/2;
            lutMap.at<Vec3w>(i,j)[1] = (matchingArr[i*camSize.width*3 + j*3 + 1]
                      * 65536) / projSize.height;
            lutMap.at<Vec3w>(i,j)[2] = (matchingArr[i*camSize.width*3 + j*3 + 2]
                      * 65536) / projSize.width;

        }
    }

    imshow("Matching Map", lutMap);

    struct stat sb;
    cout<<resultDir.c_str()<<endl;
    if (stat(resultDir.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))
    {cout << "herr non mk"<<endl;
       imwrite( cv::format( (resultDir+"resultmap_%d.png").c_str(), nbrPat ), lutMap);
       string name = resultDir+"/resultmap_"+int2String(nbrPat)+".txt";
       ofstream matchFile ((name).c_str());
       if(matchFile.is_open())
       {
           for(int i=65; i<390/*camSize.height*/; ++i)
           {
               for(int j=92; j<527/*camSize.width*/; ++j)
               {
                   matchFile << i << " ";
                   matchFile << j << " ";
                   matchFile << matchingArr[i*camSize.width*3 + j*3 + 1] << " ";
                   matchFile << matchingArr[i*camSize.width*3 + j*3 + 2] << " ";
                   matchFile << matchingArr[i*camSize.width*3 + j*3 + 0] << " ";
                   matchFile <<"\n";
               }
           }
       }
    }
    else
    {
        mkdir(resultDir.c_str(),  0777);
        imwrite( cv::format( (resultDir+"/resultmap_%d.png").c_str(), nbrPat ), lutMap);
        string name = resultDir+"/resultmap_"+int2String(nbrPat)+".txt";
        ofstream matchFile ((name).c_str());
        if(matchFile.is_open())
        {
            for(int i=65; i<390/*camSize.height*/; ++i)
            {
                for(int j=92; j<527/*camSize.width*/; ++j)
                {
                    matchFile << i << " ";
                    matchFile << j << " ";
                    matchFile << matchingArr[i*camSize.width*3 + j*3 + 1] << " ";
                    matchFile << matchingArr[i*camSize.width*3 + j*3 + 2] << " ";
                    matchFile << matchingArr[i*camSize.width*3 + j*3 + 0] << " ";
                    matchFile <<"\n";
                }
            }
        }

    }

    cout<<matchingArr[237*camSize.width*3 + 316*3 + 2]<<
          " "<< matchingArr[237*camSize.width*3 + 316*3 + 1]<<
          " "<< matchingArr[237*camSize.width*3 + 316*3 + 0]<<endl;


   waitKey(0);



    return 0;
}

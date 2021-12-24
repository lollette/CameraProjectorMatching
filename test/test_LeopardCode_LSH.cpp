#include <iostream>
#include <sstream>
#include <string>

#include <cstdio>
#include <dirent.h>
#include <math.h>
#include <fstream>
#include <sys/stat.h>
#include <limits>

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
                if(mode == "Match")
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
                if(mode == "Match")
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
            if(mode == "Match")
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
        if(mode == "Match")
        {
            cout << "the"+ path +"directory does not exist. "
                    "please choose capture mode"
                 << endl;
            exit(-1);
        }
    }
}


void matchModeFct(string path2CamPat, string path2ProjPat,int nbrPat,
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
        if(!capture.isOpened())
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

void readModeFct(string path2CamPat, string path2ProjPat,int nbrPat,
                 Size camSize, Size projSize, unsigned char *projImg,
                 unsigned char *camImg)
{
    namedWindow( "Display window", WND_PROP_FULLSCREEN );
    setWindowProperty("Display window", WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);

    Mat maskScreen = Mat::zeros(camSize.height,camSize.width, CV_8UC1);
    findScreen(maskScreen, projSize);
    //imshow("Display ", maskScreen);
    //waitKey(0);

    for(int idxPat=0; idxPat<nbrPat; ++idxPat)
    {
        Mat Pattern = imread(
                    cv::format( (path2ProjPat+"/ref-0%d.png").c_str(),
                                idxPat ), 0);
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

void hashFct(int kSize, int nbrPat, unsigned short *hash)
{
    for(int i=0;i<kSize;i++) hash[i]=rand()%((nbrPat*(nbrPat-1))/2);
}

int main (int argc, char* argv[])
{
    clock_t tStart = clock();

    //  check settings
    if (argc<5)
    {
        cout<< "Usage Capture: " << argv[0] << "\n c: capture "
                     "new patterns  or r: read patterns or m: matching\n"
                     " number of patterns\n  "
                     "outputFileNameCapturePattern or "
                     "inputFileNamePattern\n "
                     "outputFileNameResults\n"
                     "if m outputFileNameCam\n" <<endl;
        return 1;
    }

    char* captureRead = argv[1];
    int nbrPat = atoi(argv[2]);
    string patternDir = argv[3];
    string resultDir = argv[4];
    string camDir = argc>=6?argv[5]:"";


    if (nbrPat<4)
    {
        cout<<"Invalid number of patterns (<4). Application stopping." <<endl;
        return -1;
    }
    if (captureRead[0]!='c' && captureRead[0]!='r' && captureRead[0]!='m')
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

    projImg=(unsigned char  *)malloc(
                projSize.width*projSize.height*nbrPat*sizeof(unsigned char ));
    camImg=(unsigned char *)malloc(
                camSize.width*camSize.height*nbrPat*sizeof(unsigned char));

    // Capture new patterns
    if (captureRead[0]=='c')
    {
        path2ProjPat = checkDir(patternDir, "projector", "Capture") + "/";
        path2CamPat = checkDir(patternDir, "camera", "Capture") + "/";
        captureModeFct(path2CamPat, path2ProjPat, nbrPat, camSize, projSize,
                       projImg, camImg);
    }
    // Match an existing pair of Camera/Projector images
    if (captureRead[0]=='m')
    {
        path2ProjPat = checkDir(patternDir, "projector", "Match") + "/";
        path2CamPat = checkDir(patternDir, "camera", "Match") + "/";
        cout<<path2ProjPat<<endl;
        cout<<path2CamPat<<endl;
        matchModeFct(path2CamPat, path2ProjPat, nbrPat, camSize, projSize,
                    projImg, camImg);
    }
    // Read an existing patterns and capture camera images
    if (captureRead[0]=='r')
    {
        path2ProjPat = checkDir(patternDir, "projector", "Match") + "/";
        cout<<path2ProjPat<<endl;
        path2CamPat = checkDir(camDir, "camera", "Capture") + "/";
        cout<<path2CamPat<<endl;
        readModeFct(path2CamPat, path2ProjPat, nbrPat, camSize, projSize,
                    projImg, camImg);
    }
    mpz_t *projCode;
    mpz_t *camCode;

    projCode =(mpz_t *)malloc(projSize.width*projSize.height*sizeof(mpz_t));
    camCode =(mpz_t *)malloc(camSize.width*camSize.height*sizeof(mpz_t));

    for(int i=0; i<projSize.width*projSize.height; i++) mpz_init(projCode[i]);
    for(int i=0; i<camSize.width*camSize.height; i++) mpz_init(camCode[i]);

    //Generate a (nbrPat*(nbrPat-1))/2) bits code for camera and projector
    // eg: 60 patterns projected => 1770 bits code
    codeGenerator( nbrPat, camSize, camImg, camCode);
    printf("Time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
    codeGenerator( nbrPat, projSize, projImg, projCode);
    printf("Time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);

    free(camImg);
    free(projImg);

    // compute the hashing key size that should cover at least the number
    //log(WpxHp) of pixels in the projector such that expected number of codes
    //hashed by a single key is around 1
    int kSize = log2 (projSize.height*projSize.width);
    cout << "kSize = "<< kSize<<endl;

    //init match costs to infinity
    int *matchCostArr;
    matchCostArr=(int *)malloc(camSize.height*camSize.width*sizeof(int));
    for(int i=0;i<camSize.height*camSize.width;i++) matchCostArr[i] =
            numeric_limits<int>::max();;

    //init match array to -1
    int *matchCorrArr;
    matchCorrArr=(int *)malloc(camSize.height*camSize.width*sizeof(int));
    for(int i=0;i<camSize.height*camSize.width;i++) matchCorrArr[i] = -1;

    // compute the hashing table size
    int projHashTabSize = pow(2,kSize+1);
    int idx, iter, i, k;
    int pSize = projSize.height*projSize.width;
    int cSize = camSize.height*camSize.width;
    mp_bitcnt_t tmpDists;

    // hashing index
    unsigned short *hashIdx;
    hashIdx=(unsigned short *)malloc(kSize*sizeof(unsigned short));

    // hashing table
    int *projHash;
    projHash=(int *)malloc(projHashTabSize*sizeof(int));

    int hash;

    for(iter=0; iter<200; ++iter)
    {
        cout<< "itr = "<<iter<<endl;

        //select k bits out of N
        hashFct( kSize,  nbrPat,  hashIdx);

        for(i=0; i<projHashTabSize;i++) projHash[i]=-1;

        //construct hashing table
        for(i=0; i<pSize; ++i)
        {
            hash = 0;

            for(k=0; k<kSize; k++)
                if(mpz_tstbit(projCode[i], hashIdx[k])) hash|=(1<<k);

            projHash[hash]=i;
        }

        // closest projector code to Cam[i]
        for(i=0/*(237*640)+316*/; i</*(237*640)+316+1*/cSize; ++i)
        {
            hash = 0;

            //select same k bits out of N
            for(k=0; k<kSize; ++k)
                if(mpz_tstbit(camCode[i], hashIdx[k])) hash|=(1<<k);

            idx = projHash[hash];
            if( idx<0 ) continue;

            // if the k hash found compare the N bits code
            tmpDists = mpz_hamdist(camCode[i], projCode[idx]);

            //if current distance is smaller than the one in matchCostArr
            //replace it and save  projector's pix coord
            if((int)tmpDists<matchCostArr[i])
            {
                matchCostArr[i] = (int)tmpDists;
                matchCorrArr[i] = idx;
            }
        }
    }
    free(hashIdx);
    free(projHash);

    cout<< "matchCostArr[(237*640)+316] = "<<matchCostArr[(237*640)+316]<<endl;

    int l= matchCorrArr[(237*640)+316]/1920;
    int c= matchCorrArr[(237*640)+316] - (l*1920);
    cout<< "matchCorrArr[(237*640)+316] = "<<matchCorrArr[(237*640)+316]<<endl;
    cout<< "l = "<<l<<endl;
    cout<< "c = "<<c<<endl;

    printf("Time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);

    // map the matching
    Mat lutMap = Mat::zeros(camSize.height, camSize.width, CV_16UC3);
    for(int i=0; i<camSize.height; ++i)
    {
        for(int j=0; j<camSize.width; ++j)
        {
            if(matchCostArr[i*camSize.width + j] <450)
            {
                lutMap.at<Vec3w>(i,j)[0] =(matchCostArr[i*camSize.width + j]
                          * 65536) / (nbrPat*(nbrPat-1))/2;
                lutMap.at<Vec3w>(i,j)[1] = ((matchCorrArr[(i*camSize.width)+j]
                          /1920) * 65536) / projSize.height;
                lutMap.at<Vec3w>(i,j)[2] =
                        ((matchCorrArr[(i*camSize.width)+j] -
                         ((matchCorrArr[(i*camSize.width)+j]/1920)*1920))
                          * 65536) / projSize.width;
           }
        }
    }

    imshow("Matching Map", lutMap);

    //map the center
    Mat centerMap = Mat::zeros(projSize.height, projSize.width, CV_8U);
    for(int i=0; i<projSize.height; ++i)
    {
        centerMap.at<uchar>(
                    i,c) = 255;
    }
    for(int i=0; i<projSize.width; ++i)
    {
        centerMap.at<uchar>(
                    l,i) =255;
    }
    imshow("Display window", centerMap);
    VideoCapture capture(0);
    if(!capture.isOpened())
        return -1;

    Mat capturePattern = capturePatternFct(capture);

    imshow("Centetr Map", capturePattern);

    //save the matching
    struct stat sb;
    cout<<resultDir.c_str()<<endl;
    if (stat(resultDir.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))
    {
       imwrite( cv::format( (resultDir+"/resultmap_%d.png").c_str(), nbrPat ),
                lutMap);
       imwrite(  cv::format( (resultDir+"/Centre_%d.png").c_str(), nbrPat ),
                 capturePattern);
       string name = resultDir+"/resultmap_"+int2String(nbrPat)+".txt";
       cout<<"if ="<<name<<endl;
       ofstream matchFile ((name).c_str());
       if(matchFile.is_open())
       {
           for(int i=0; i<camSize.height; ++i)
           {
               for(int j=0; j<camSize.width; ++j)
               {
                   if(matchCostArr[i*camSize.width + j] <450)
                   {
                       matchFile << i << " ";
                       matchFile << j << " ";
                       matchFile << matchCorrArr[(i*camSize.width)+j]/1920
                               << " ";
                       matchFile << matchCorrArr[(i*camSize.width)+j] -
                               ((matchCorrArr[(i*camSize.width)+j]/1920)*1920)
                               << " ";
                       matchFile << matchCostArr[i*camSize.width + j] << " ";
                       matchFile <<"\n";
                   }
               }
           }
       }
    }
    else
    {
        mkdir(resultDir.c_str(),  0777);
        cout<<(resultDir+"/resultmap_%d.png").c_str()<<endl;
        imwrite( cv::format( (resultDir+"/resultmap_%d.png").c_str(), nbrPat ),
                 lutMap);
        imwrite( cv::format( (resultDir+"/Centre_%d.png").c_str(), nbrPat ),
                 capturePattern);
        string name = resultDir+"/resultmap_"+int2String(nbrPat)+".txt";
        cout<<"else = "<<name<<endl;
        ofstream matchFile ((name).c_str());
        if(matchFile.is_open())
        {
            for(int i=0; i<camSize.height; ++i)
            {
                for(int j=0; j<camSize.width; ++j)
                {
                    if(matchCostArr[i*camSize.width + j] <450)
                    {
                        matchFile << i << " ";
                        matchFile << j << " ";
                        matchFile << matchCorrArr[(i*camSize.width)+j]/1920
                                << " ";
                        matchFile << matchCorrArr[(i*camSize.width)+j] -
                                ((matchCorrArr[(i*camSize.width)+j]/1920)*1920)
                                << " ";
                        matchFile << matchCostArr[i*camSize.width + j] << " ";
                        matchFile <<"\n";
                    }
                }
            }
        }

   }





   waitKey(0);



    return 0;
}

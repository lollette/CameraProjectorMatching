#ifndef MODULOCODE_H
#define MODULOCODE_H

#include <algorithm>
#include <bitset>
#include <string>
#include <iostream>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>


using namespace std;
using namespace cv;

class ModuloCodeGenerator
{
    private:
            Size m_projectorSize;

    public:
            vector<Mat> m_imagePatternVcet;

            void setParameters(Size projectorSize)
            {
                m_projectorSize = projectorSize;
            }

            string int2Binary(unsigned int x)
            {
                int mod1 = x%59;
                string mod1s = bitset<6>(mod1).to_string();
                int mod2 = x%43;
                string mod2s = bitset<6>(mod2).to_string();
                //cout <<mod1s <<" " <<mod2s<<endl;
                return mod1s+mod2s;
            }
            unsigned int binary2Int(string x)
            {
                return bitset<16>(x).to_ulong();
            }
            string int2String(int i)
            {
                stringstream s;
                s << i;
                return s.str();
            }
            void getModuloCodeH(Size projectorSize)
            {
                m_imagePatternVcet.clear();
                setParameters(projectorSize);
                for(int c=0; c<12; c++)
                {
                    Mat tmpMat;
                    tmpMat.create(m_projectorSize, CV_8U);
                    tmpMat = Scalar::all(0);
                    for (int j=0; j<m_projectorSize.height; j++)
                    {
                        string s = int2Binary(j);
                        char cstr = s[s.length()-12+c];
                        for (int i=0; i<m_projectorSize.width; ++i)
                        {
                            tmpMat.at<uchar>(j,i) = (cstr -'0')*255;
                        }
                    }
                    m_imagePatternVcet.push_back( tmpMat);
               }
            }
            void getModuloCodeV(Size projectorSize)
            {
                m_imagePatternVcet.clear();
                setParameters(projectorSize);
                for(int c=0; c<12; c++)
                {
                    Mat tmpMat;
                    tmpMat.create(m_projectorSize, CV_8U);
                    tmpMat = Scalar::all(0);
                    for (int j=0; j<m_projectorSize.width; j++)
                    {
                        string s = int2Binary(j);
                        char cstr = s[s.length()-12+c];
                        for (int i=0; i<m_projectorSize.height; ++i)
                        {
                            tmpMat.at<uchar>(i,j) = (cstr -'0')*255;
                        }
                    }
                    m_imagePatternVcet.push_back( tmpMat);
                }
            }

};

#endif

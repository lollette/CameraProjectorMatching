#ifndef GRAYCODE_H
#define GRAYCODE_H

#include <algorithm>
#include <bitset>
#include <string>
#include <iostream>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>


using namespace std;
using namespace cv;

class GrayCodeGenerator
{
    private:
            Size m_projectorSize;
            unsigned m_bitNumber;

    public:
            vector<Mat> m_imagePatternVcet;
            unsigned bitNumberCount(unsigned x)
            {
                unsigned bits, var =(x < 0) ? -x : x;
                for(bits = 0; var != 0; ++bits) var >>= 1;
                return bits;
            }
            unsigned maxBitsNumber()
            {
                return max(bitNumberCount(m_projectorSize.height),
                           bitNumberCount(m_projectorSize.width));
            }
            void setParameters(Size projectorSize)
            {
                m_projectorSize = projectorSize;
                m_bitNumber = maxBitsNumber();
            }
            unsigned int int2Gray(unsigned int x)
            {
                return (x >> 1) ^ x;
            }
            unsigned int gray2Int(unsigned int x)
            {
                unsigned int mask;
                for (mask = x >> 1; mask != 0; mask = mask >> 1)
                {
                    x = x ^ mask;
                }
                return x;
            }
            string int2Binary(unsigned int x)
            {
                return bitset<16>(x).to_string();
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
            void getGrayCodeH(Size projectorSize)
            {
                m_imagePatternVcet.clear();
                setParameters(projectorSize);
                for(int c=0; c<m_bitNumber; c++)
                {
                    Mat tmpMat;
                    tmpMat.create(m_projectorSize, CV_8U);
                    tmpMat = Scalar::all(0);
                    for (int j=0; j<m_projectorSize.height; j++)
                    {
                        string s = int2Binary(int2Gray(j));
                        char cstr = s[s.length()-m_bitNumber+c];
                        for (int i=0; i<m_projectorSize.width; ++i)
                        {
                            tmpMat.at<uchar>(j,i) = (cstr -'0')*255;
                        }
                    }
                    m_imagePatternVcet.push_back( tmpMat);
                }
            }
            void getGrayCodeV(Size projectorSize)
            {
                m_imagePatternVcet.clear();
                setParameters(projectorSize);
                for(int c=0; c<m_bitNumber; c++)
                {
                    Mat tmpMat;
                    tmpMat.create(m_projectorSize, CV_8U);
                    tmpMat = Scalar::all(0);
                    for (int j=0; j<m_projectorSize.width; j++)
                    {
                        string s = int2Binary(int2Gray(j));
                        char cstr = s[s.length()-m_bitNumber+c];
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

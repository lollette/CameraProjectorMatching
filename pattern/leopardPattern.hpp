#ifndef LEOPARDCODE_H
#define LEOPARDCODE_H_H

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>


using namespace std;
using namespace cv;

RNG *rng= new RNG(0xFFFFFFFF);

class LeopardCodeGenerator
{
    private:
            Size m_projectorSize;
            int m_frequence;
            bool m_blurStep;
            Mat m_pattern;
    public:
            void setParameters(Size projectorSize, int frequence, bool blurStep)
            {
                m_projectorSize = projectorSize;
                m_frequence = frequence;
                m_blurStep = blurStep;
            }
            Mat getLeopard()
            {
                Size tmpSize = m_projectorSize + Size(
                            m_projectorSize.height*0.1,
                            m_projectorSize.width*0.1);
                tmpSize.width = getOptimalDFTSize(tmpSize.width);
                tmpSize.height = getOptimalDFTSize(tmpSize.height);

                Mat matR = Mat::zeros(tmpSize.height, tmpSize.width, CV_32F);
                Mat matI = Mat::zeros(tmpSize.height, tmpSize.width, CV_32F);
                Mat matthresh = Mat::zeros(
                            tmpSize.height, tmpSize.width, CV_32F);
                Mat matComplex;

                int minFreq = m_frequence;
                int maxFreq = m_frequence*2;

                double freqX, freqY;
                double ratio = double(tmpSize.width)/tmpSize.height;
                double magnitude, phase, cosPhase, sinPhase;

                for(int j=1; j<tmpSize.height/2; ++j)
                {
                    int invJ = tmpSize.height-j;

                    float *ptrmatR = matR.ptr<float>(j);
                    float *ptrmatRInv = matR.ptr<float>(invJ);
                    float *ptrmatI = matI.ptr<float>(j);
                    float *ptrmatIInv = matI.ptr<float>(invJ);

                    freqY = j*ratio;

                    for (int i=1; i<tmpSize.width/2; ++i)
                    {
                        int invC = tmpSize.width-i;

                        freqX = i;
                        magnitude = (freqX * freqX)  + (freqY * freqY);

                        if (magnitude <= maxFreq*maxFreq &&
                                magnitude >= minFreq*minFreq)
                        {
                            phase = rng->uniform(0., 2.*M_PI);
                            cosPhase = cos(phase);
                            sinPhase = sin(phase);
                            ptrmatR[i] = cosPhase;
                            ptrmatI[i] = sinPhase;
                            ptrmatRInv[invC] = cosPhase;
                            ptrmatIInv[invC] = -sinPhase;

                            phase = rng->uniform(0., 2.*M_PI);
                            cosPhase = cos(phase);
                            sinPhase = sin(phase);
                            ptrmatR[invC] = cosPhase;
                            ptrmatI[invC] = sinPhase;
                            ptrmatRInv[i] = cosPhase;
                            ptrmatIInv[i] = -sinPhase;
                        }
                    }
                }
                Mat tmp[] = {matR, matI};
                merge(tmp, 2, matComplex);
                idft(matComplex, matR, DFT_REAL_OUTPUT);

                threshold(matR, matthresh, 0, 255., THRESH_BINARY);
                matthresh = Mat_<uchar>(matthresh);
                if (m_blurStep) {
                    double avgFreq = (minFreq+maxFreq)/2.;
                    double sigmaX = (tmpSize.width/avgFreq)/6.;
                    double sigmaY = (tmpSize.height/avgFreq)/6.;
                    GaussianBlur(matthresh, matthresh, Size(), sigmaX, sigmaY);
                }
                matthresh(Rect((tmpSize.width-m_projectorSize.width)/2.,
                      (tmpSize.height-m_projectorSize.height)/2.,
                      m_projectorSize.width, m_projectorSize.height)).copyTo(
                            m_pattern);
               return m_pattern;
            }

};

#endif

#ifndef FILTER_H
#define FILTER_H

#include "opencv2/opencv.hpp"
#include <QWidget>
#include "ui_widget.h"


class Filter
{

public:
    void init(cv::Mat img);//初始化对象
    void addPepperSaltNoise(void);//添加椒盐噪声
    void addGaussNoise(void);//添加高斯噪声
    void addPoissonNoise(void);//添加泊松噪声
    void medianFilter(void);//中值滤波
    void meanFilter(void);//均值滤波
    void Fourier(void);//傅里叶降噪
    cv::Mat getResultImg(void);

private:
    void sort(unsigned char array[], int n);//冒泡排序，用于中值滤波
    unsigned char getMeanValue(unsigned char array[], int n);//求平均值，用于均值滤波

public:
    cv::Mat src,dst;//源图像，目标图像
    int rows,cols,channels;

};



class Quality
{

public:
    Ui::Widget* exUI;//为了在这个类里面操作ui
    cv::Mat src,noiseSrc;//原图和噪声图
    double MSE,PSNR;

    Quality(Ui::Widget* UI):exUI(UI){}
    void setSrc(cv::Mat img);
    void setNoiseSrc(cv::Mat img);
    double calculateMSE(void);
    double calculatePSNR(void);
    void showMSE_PSNR(double mse, double psnr);
};

#endif // FILTER_H

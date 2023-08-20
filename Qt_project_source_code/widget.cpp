#include "widget.h"
#include "ui_widget.h"
#include "opencv2/opencv.hpp"
#include "QDebug"
#include "QFileDialog"
#include "QDir"
#include "QMessageBox"
#include "QPushButton"


Widget::Widget(QWidget *parent): QWidget(parent), ui(new Ui::Widget), myQuality(ui)
{
    ui->setupUi(this);

//    Quality myQuality(ui);//创建一个Quality对象，并把ui指针传给他

    int distance1=10,distance2=10;//图片和顶部的距离；图片之间以及图片和左右边界的距离
    ui->label_srcImg->setGeometry(distance2,distance1,100,100);//设置显示位置
    ui->label_dstImg->setGeometry(1300/2+distance2,distance1,100,100);

    ui->comboBox_choseNoise->setCurrentIndex(0);//设置下拉框默认索引值

    //选择图像的按钮和对应的槽函数
    connect(ui->pb_choseImg,&QPushButton::clicked,this,&Widget::choseImgSlot);

    //添加噪声的槽函数
//    void (QComboBox::*funcSignal)(int)=&QComboBox::currentIndexChanged;
    void (QComboBox::*funcSignal)(int)=&QComboBox::activated;
    connect(ui->comboBox_choseNoise,funcSignal,this,&Widget::addNoiseSlot);

    //中值滤波和均值滤波和傅里叶降噪的按钮和槽函数连接
    connect(ui->pb_meanFilt,&QPushButton::clicked,this,&Widget::meanFiltSlot);
    connect(ui->pb_medianFilt,&QPushButton::clicked,this,&Widget::medianFiltSlot);
    connect(ui->pb_Fourier,&QPushButton::clicked,this,&Widget::FourierSlot);

//    void(Quality::*funcsignal)(double,double)=&Quality::showMSE_PSNR;
//    void(Widget::*funcslot)(double,double)=&Widget::showmsepsnr;
//    connect(&(this->myQuality),funcsignal,this,funcslot);
}

//将opencv里的Mat类型图像转化为Qt中可用的QImage类型
QImage Widget::changeImgType(cv::Mat src)
{
    return QImage((const unsigned char*)(src.data),src.cols,src.rows,src.step,QImage::Format_BGR888);
}

Widget::~Widget()
{
    delete ui;
}

//弹出选择图像的文件对话框的槽函数。通过这个对话框，初始化滤波器对象
void Widget::choseImgSlot(void)
{
    QString curPath=QDir::currentPath();
    QString filter="(*.bmp *.BMP *.png *.PNG *.jpg *.JPG)";
    this->choseImgPath=QFileDialog::getOpenFileName(this,"选择一幅图片",curPath,filter);//弹出文件对话框
    if(!this->choseImgPath.isEmpty())
    {
        this->imgTemp=cv::imread(this->choseImgPath.toLocal8Bit().toStdString());
        this->myFilter.init(this->imgTemp);
        this->qimshow(1,imgTemp);
    }
}

//添加噪声槽函数
void Widget::addNoiseSlot(int index)
{
    switch(index)
    {
        case 0:{//显示原图
            this->qimshow(1,imgTemp);
            this->myFilter.init(imgTemp);
            this->ui->label_MSE->setText(QString(" "));
            this->ui->label_PSNR->setText(QString(" "));
        }break;

        case 1:{//添加椒盐噪声
            this->myFilter.addPepperSaltNoise();
            this->qimshow(1,this->myFilter.src);
            this->ui->label_MSE->setText(QString(" "));
            this->ui->label_PSNR->setText(QString(" "));
        }break;

        case 2:{//添加高斯噪声
            this->myFilter.init(imgTemp);
            this->myFilter.addGaussNoise();
            this->qimshow(1,this->myFilter.src);
            this->ui->label_MSE->setText(QString(" "));
            this->ui->label_PSNR->setText(QString(" "));
        }break;

        case 3:{//添加泊松噪声
            this->myFilter.init(imgTemp);
            this->myFilter.addPoissonNoise();
            this->qimshow(1,this->myFilter.src);
            this->ui->label_MSE->setText(QString(" "));
            this->ui->label_PSNR->setText(QString(" "));
        }break;

        default:break;
    }
}

//在label中显示一幅图像
void Widget::qimshow(int whichLabel,cv::Mat img)
{
    QImage qImg=this->changeImgType(img);//先从Mat类型转化为QImage类型

    if(whichLabel==1)
    {
        ui->label_srcImg->setPixmap(QPixmap::fromImage(qImg));
        ui->label_srcImg->resize(qImg.size());//根据图像的大小来设置label的大小
        ui->label_srcImg->show();
    }
    else if(whichLabel==2)
    {
        ui->label_dstImg->setPixmap(QPixmap::fromImage(qImg));
        ui->label_dstImg->resize(qImg.size());//根据图像的大小来设置label的大小
        ui->label_dstImg->show();
    }
}

//均值滤波按钮的槽函数
void Widget::meanFiltSlot(void)
{
    this->myFilter.meanFilter();
    this->qimshow(2,this->myFilter.getResultImg());

    this->myQuality.setSrc(this->imgTemp);
    this->myQuality.setNoiseSrc(this->myFilter.dst);
    double mse,psnr;
    mse=this->myQuality.calculateMSE();
    psnr=this->myQuality.calculatePSNR();
    this->myQuality.showMSE_PSNR(mse,psnr);
}

//中值滤波按钮的槽函数
void Widget::medianFiltSlot(void)
{
    this->myFilter.medianFilter();
    this->qimshow(2,this->myFilter.getResultImg());

    this->myQuality.setSrc(this->imgTemp);
    this->myQuality.setNoiseSrc(this->myFilter.dst);
    double mse,psnr;
    mse=this->myQuality.calculateMSE();
    psnr=this->myQuality.calculatePSNR();
    this->myQuality.showMSE_PSNR(mse,psnr);
}

//傅里叶降噪的槽函数
void Widget::FourierSlot(void)
{
    this->myFilter.Fourier();

    this->myQuality.setSrc(this->imgTemp);
    this->myQuality.setNoiseSrc(this->myFilter.dst);

    double mse,psnr;
    mse=this->myQuality.calculateMSE();
    psnr=this->myQuality.calculatePSNR();
    this->myQuality.showMSE_PSNR(mse,psnr);

    cv::Mat image1,image2;
    image1=this->myFilter.getResultImg();
    image1.convertTo(image2,CV_8UC3);

    this->qimshow(2,image2);

}



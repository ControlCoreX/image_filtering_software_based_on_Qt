#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "opencv2/opencv.hpp"
#include "QDebug"
#include "QDir"
#include "QFileDialog"


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString curPath=QDir::currentPath();//得到程序当前目录，作为标准文件对话框的起始定位目录
    QString filter="(*.jpg *.bmp *.png)";//文件类型过滤器
    QString imgPath=QFileDialog::getOpenFileName(this,"选择一个图像",curPath,filter);
    if(!imgPath.isEmpty())
    {
        cv::Mat src=cv::imread("imgPath");
        if(src.empty())
        {
            qDebug()<<"图像加载错误"<<endl;
            //弹出一个对话框，提示图片加载错误吧
        }

//        cv::imshow("图像",src);


    }

}

MainWindow::~MainWindow()
{
    delete ui;
}


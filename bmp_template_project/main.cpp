// ImgOpt.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <iostream>
#include <Windows.h>
#include <malloc.h>
#include <vector>

using namespace std;

string imgPath = "E:/打工人/基于Visual C++的图像滤波技术 - 500/Lena.bmp";
string saveImgPath = "E:/打工人/基于Visual C++的图像滤波技术 - 500/Lena1.bmp";

//自定义了一个ImgInfo的结构体，包含BMP文件头、BMP信息头和像素点的RGB值。
//目前只支持24位图像的读取和显示
typedef struct 
{
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
    vector<vector<char>> imgData;
}ImgInfo;

//根据图片路径读取Bmp图像，生成ImgInfo对象
ImgInfo readBitmap(string imgPath) 
{
    ImgInfo imgInfo;
    char* buf;//定义文件读取缓冲区
    char* p;

    FILE* fp;
    fopen_s(&fp, imgPath.c_str(), "rb");
    if (fp == NULL) 
    {
        cout << "打开文件失败!" << endl;
        exit(0);
    }

    fread(&imgInfo.bf, sizeof(BITMAPFILEHEADER), 1, fp);//读出文件头
    fread(&imgInfo.bi, sizeof(BITMAPINFOHEADER), 1, fp);//读出信息头

    if (imgInfo.bi.biBitCount != 24)//判断色深是否为24bit
    {
        cout << "不支持该格式的BMP位图！" << endl;
        exit(0);
    }

    fseek(fp, imgInfo.bf.bfOffBits, 0);//根据文件头中的信息，将读写标记定位到数据区

    buf = (char*)malloc(imgInfo.bi.biWidth * imgInfo.bi.biHeight * 3);//申请保存所有像素值的内存
    fread(buf, 1, imgInfo.bi.biWidth * imgInfo.bi.biHeight * 3, fp);//读出所有像素值信息放到buf里

    p = buf;

    vector<vector<char>> imgData;//每一个像素点是一个vector<char>类型，把所有像素点都存到一个vector里，直接存成一排
    int x = 0, y = 0;
    for (y = 0; y < imgInfo.bi.biHeight; y++)//控制行数
    {
        for (x = 0; x < imgInfo.bi.biWidth; x++)//控制列数
        {
            vector<char> vRGB;//存储一个像素点的各通道值

            vRGB.push_back(*(p++));//blue
            vRGB.push_back(*(p++));//green
            vRGB.push_back(*(p++));//red

            if (x == imgInfo.bi.biWidth - 1)//读完每一行，保证指针4字节对齐，少几个字节不是4的倍数就偏移几次
            {
                for (int k = 0; k < imgInfo.bi.biWidth % 4; k++) p++;
            }
            imgData.push_back(vRGB);//将一个像素点放到容器里
        }
    }
    fclose(fp);
    imgInfo.imgData = imgData;
    return imgInfo;
}

void showBitmap(ImgInfo imgInfo) 
{
    HWND hWindow;//窗口句柄
    HDC hDc;//绘图设备环境句柄
    int yOffset = 150;
    hWindow = GetForegroundWindow();
    hDc = GetDC(hWindow);

    int posX, posY;
    char blue = 0, green = 0, red = 0;
    //遍历每一个像素点
    for (int i = 0; i < imgInfo.imgData.size(); i++)//imgInfo.imgData.size()是像素点的个数
    {
        blue = imgInfo.imgData.at(i).at(0);//第i个像素点的第0个通道的值
        green = imgInfo.imgData.at(i).at(1);//...
        red = imgInfo.imgData.at(i).at(2);//...

        posX = i % imgInfo.bi.biWidth;//这又是一个递增，结果循环的常规操作啊！posX应该也是从0开始的
        posY = imgInfo.bi.biHeight - i / imgInfo.bi.biWidth + yOffset;
        SetPixel(hDc, posX, posY, RGB(red, green, blue));
    }
}

void saveBitmap(ImgInfo imgInfo)
{
    FILE* fpw;
    fopen_s(&fpw, saveImgPath.c_str(), "wb");
    fwrite(&imgInfo.bf, sizeof(BITMAPFILEHEADER), 1, fpw);//写入文件头
    fwrite(&imgInfo.bi, sizeof(BITMAPINFOHEADER), 1, fpw);//写入文件头信息

    int size = imgInfo.bi.biWidth * imgInfo.bi.biHeight;
    for (int i = 0; i < size; i++)
    {
        fwrite(&imgInfo.imgData.at(i).at(0), 1, 1, fpw);
        fwrite(&imgInfo.imgData.at(i).at(1), 1, 1, fpw);
        fwrite(&imgInfo.imgData.at(i).at(2), 1, 1, fpw);

        if (i % imgInfo.bi.biWidth == imgInfo.bi.biWidth - 1)
        {
            char ch = '0';
            for (int j = 0; j < imgInfo.bi.biWidth % 4; j++)
            {
                fwrite(&ch, 1, 1, fpw);
            }
        }
    }
    fclose(fpw);
    cout << "已保存图像至: " + saveImgPath << endl;
}

int main(void)
{
    ImgInfo imgInfo = readBitmap(imgPath);
    showBitmap(imgInfo);
    saveBitmap(imgInfo);
}

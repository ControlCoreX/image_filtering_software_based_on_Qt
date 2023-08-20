// ImgOpt.cpp : ���ļ����� "main" ����������ִ�н��ڴ˴���ʼ��������
//
#include <iostream>
#include <Windows.h>
#include <malloc.h>
#include <vector>

using namespace std;

string imgPath = "E:/����/����Visual C++��ͼ���˲����� - 500/Lena.bmp";
string saveImgPath = "E:/����/����Visual C++��ͼ���˲����� - 500/Lena1.bmp";

//�Զ�����һ��ImgInfo�Ľṹ�壬����BMP�ļ�ͷ��BMP��Ϣͷ�����ص��RGBֵ��
//Ŀǰֻ֧��24λͼ��Ķ�ȡ����ʾ
typedef struct 
{
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
    vector<vector<char>> imgData;
}ImgInfo;

//����ͼƬ·����ȡBmpͼ������ImgInfo����
ImgInfo readBitmap(string imgPath) 
{
    ImgInfo imgInfo;
    char* buf;//�����ļ���ȡ������
    char* p;

    FILE* fp;
    fopen_s(&fp, imgPath.c_str(), "rb");
    if (fp == NULL) 
    {
        cout << "���ļ�ʧ��!" << endl;
        exit(0);
    }

    fread(&imgInfo.bf, sizeof(BITMAPFILEHEADER), 1, fp);//�����ļ�ͷ
    fread(&imgInfo.bi, sizeof(BITMAPINFOHEADER), 1, fp);//������Ϣͷ

    if (imgInfo.bi.biBitCount != 24)//�ж�ɫ���Ƿ�Ϊ24bit
    {
        cout << "��֧�ָø�ʽ��BMPλͼ��" << endl;
        exit(0);
    }

    fseek(fp, imgInfo.bf.bfOffBits, 0);//�����ļ�ͷ�е���Ϣ������д��Ƕ�λ��������

    buf = (char*)malloc(imgInfo.bi.biWidth * imgInfo.bi.biHeight * 3);//���뱣����������ֵ���ڴ�
    fread(buf, 1, imgInfo.bi.biWidth * imgInfo.bi.biHeight * 3, fp);//������������ֵ��Ϣ�ŵ�buf��

    p = buf;

    vector<vector<char>> imgData;//ÿһ�����ص���һ��vector<char>���ͣ����������ص㶼�浽һ��vector�ֱ�Ӵ��һ��
    int x = 0, y = 0;
    for (y = 0; y < imgInfo.bi.biHeight; y++)//��������
    {
        for (x = 0; x < imgInfo.bi.biWidth; x++)//��������
        {
            vector<char> vRGB;//�洢һ�����ص�ĸ�ͨ��ֵ

            vRGB.push_back(*(p++));//blue
            vRGB.push_back(*(p++));//green
            vRGB.push_back(*(p++));//red

            if (x == imgInfo.bi.biWidth - 1)//����ÿһ�У���ָ֤��4�ֽڶ��룬�ټ����ֽڲ���4�ı�����ƫ�Ƽ���
            {
                for (int k = 0; k < imgInfo.bi.biWidth % 4; k++) p++;
            }
            imgData.push_back(vRGB);//��һ�����ص�ŵ�������
        }
    }
    fclose(fp);
    imgInfo.imgData = imgData;
    return imgInfo;
}

void showBitmap(ImgInfo imgInfo) 
{
    HWND hWindow;//���ھ��
    HDC hDc;//��ͼ�豸�������
    int yOffset = 150;
    hWindow = GetForegroundWindow();
    hDc = GetDC(hWindow);

    int posX, posY;
    char blue = 0, green = 0, red = 0;
    //����ÿһ�����ص�
    for (int i = 0; i < imgInfo.imgData.size(); i++)//imgInfo.imgData.size()�����ص�ĸ���
    {
        blue = imgInfo.imgData.at(i).at(0);//��i�����ص�ĵ�0��ͨ����ֵ
        green = imgInfo.imgData.at(i).at(1);//...
        red = imgInfo.imgData.at(i).at(2);//...

        posX = i % imgInfo.bi.biWidth;//������һ�����������ѭ���ĳ����������posXӦ��Ҳ�Ǵ�0��ʼ��
        posY = imgInfo.bi.biHeight - i / imgInfo.bi.biWidth + yOffset;
        SetPixel(hDc, posX, posY, RGB(red, green, blue));
    }
}

void saveBitmap(ImgInfo imgInfo)
{
    FILE* fpw;
    fopen_s(&fpw, saveImgPath.c_str(), "wb");
    fwrite(&imgInfo.bf, sizeof(BITMAPFILEHEADER), 1, fpw);//д���ļ�ͷ
    fwrite(&imgInfo.bi, sizeof(BITMAPINFOHEADER), 1, fpw);//д���ļ�ͷ��Ϣ

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
    cout << "�ѱ���ͼ����: " + saveImgPath << endl;
}

int main(void)
{
    ImgInfo imgInfo = readBitmap(imgPath);
    showBitmap(imgInfo);
    saveBitmap(imgInfo);
}

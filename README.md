# image_filtering_software_based_on_Qt
基于Qt和opencv的图像滤波软件

这也是我在网上接的一个bishe项目, 任务指导书上是这样写的:
![任务指导书](./assets/%E5%AE%A2%E6%88%B7%E9%9C%80%E6%B1%82.jpg)

大体上也就是自己用代码实现一下这3种算法. 

## 1. 解决方案
当然第一想法就是直接使用opencv提供的接口进行图像滤波, 但是这样未免太简单了, 客户的需求肯定也不是这样的. 要自己用代码实现这几种算法. 那还会有一个想法, 因为我要自己写代码滤波, 那我就要获取到这些像素点啊, 那怎么获得这些像素点呢?

1. 可以使用opencv库, 使用里面的Mat类, 读入图像, 然后就可以通过访问Mat对象里的成员就可以获取到像素信息.
2. 自己手动解析图像文件, 直接用以二进制的形式访问文件, 找到存储图像像素信息的内存地址, 然后对这部分地址进行操作就可以了.

我评估了一下这两种方案, 显然第一种是更简单一些的. 但是我本着**做项目的同时也是为了锻炼自己, 网上接单也不只是单纯赚钱, 更是为了学习**的想法, 决定给自己加点难度(因为我本来也没有手动解析过图像文件, 这就要现学现卖了), 直接解析文件. (也不知道我当时胆子咋那么大, 哈哈哈哈, 想来也一直都是这样, 接单全都是现学现卖, 但我都有预先评估项目难度的, 不会耽误工期)

## 2. bmp位图文件格式解析
首先为什么要选择解析bmp(bit map)位图文件呢? 很简单, 因为这种文件它没有进行压缩, 直接把图像中的所有像素信息都存储起来了, 所以我可以很简单的获取到这些像素值. 而其他的一些图像存储格式, 如jpg, png等都使用了一些压缩算法, 所以我无法很容易的获取到**RAW数据**. 这也是我在看一些**DIP数字图像处理**的课的时候了解到的.

bmp位图文件载入到内存中后, 总共分为4个部分:

1. bmp文件头file header(**14byte**): 提供文件格式, 文件大小(byte), **图像数据的偏移量**, 还有一些保留字节.
2. 位图信息图info header(**40byte**): **图像的宽度**, **图像的高度**, 压缩类型(不压缩), 等等.
3. 调色板: 如果是**真彩色24bit**图像的话就不需要调色板, 只有伪彩色图像才需要调色板, 相当于一个颜色的索引表.
4. 位图数据: 就是真正存储图像像素信息的地方.

其中file header和info header, Windows都为其定义了结构体, 分别如下: 

```c
//! 我们只需要使用其中一部分成员即可.
typedef struct tagBITMAPFILEHEADER 
{  
    UINT16 bfType;    
    DWORD bfSize; 
    UINT16 bfReserved1; 
    UINT16 bfReserved2; 
    DWORD bfOffBits;//!< 解析格式时需要使用该成员.
}BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize; 
    LONG biWidth;//!< 图像宽度.
    LONG biHeight;//!< 图像高度.
    WORD biPlanes; 
    WORD biBitCount; 
    DWORD biCompression; 
    DWORD biSizeImage; 
    LONG biXPelsPerMeter; 
    LONG biYPelsPerMeter; 
    DWORD biClrUsed; 
    DWORD biClrImportant;
}BITMAPINFOHEADER;
```
其实也就是说, 你一二进制的方式读入一个bmp文件, 然后其开头就是上面这两个结构体, 大小是固定的字节数, 下面使用Notepad++来打开一个bmp文件验证前面的说法, 那就以数字图像处理领域最有名的**Lena**为例吧!

![Lena](./assets/%E4%BD%8D%E5%9B%BE%E6%A0%BC%E5%BC%8F%E5%88%86%E6%9E%90.png)
**注意**: 由于这里是以十六进制的方式显示的, 而且显示的直接是读入内存时的数据, 也就是图像在内存中真实的样子, 所以这里还涉及到一个多字节数据在内存中存储时的**字节序**的问题, 即计算机体系架构中的**大小端**:

1. 如果数据的低字节在低地址处, 则该模式为**小端模式little endian**.
2. 如果数据的高字节在低地址处, 则该模式为**大端模式big endian**.

而Intel体系结构一般采用的是小端模式, 所以在看上图的时候需要注意.

**分析**: 
1. 第一个红框对应file header结构体中的`bfType`成员, `UINT16`类型, 16bit, 所以是2个字节, 用于标识文件类型, 这里是`4d42`.
2. 第二个红框对应`bfSize`成员, 表示文件大小, 看到其值为`000a4cd6`, 转化为十进制为675030, 单位是byte, 也就是675030/1024=659KB.

![属性](./assets/%E6%96%87%E4%BB%B6%E5%B1%9E%E6%80%A7%E8%8F%9C%E5%8D%95.png)
看到跟文件的属性菜单中显示的大小一样, 因为其实你在电脑上右键点属性, 文件资源管理器其实不也就是用同样的方法读了一下file header的`bfSize`成员么!

3. 第三个红框对应的是两个保留部分, 这里都是0.
4. 第四个红框对应于结构体中的`bfOffBits`成员, 即内存中图像的像素数据离文件头的偏移量, 这里是`00000036`, 即十进制54, 即14+40, 就刚好是file header + info header, 随后就是像素数据了, 说明这里面就没有调色板, 也验证了前面的说法.
5. 下面就不分析了, 跟上面的一样, 看一下info header的信息就可以获取到图像的宽度和高度.

## 3. 代码实现文件读取
下面只贴出比较重要的代码部分: 
```c++
//! 自定义一个结构体:
typedef struct 
{
    BITMAPFILEHEADER bf;//!< file header.
    BITMAPINFOHEADER bi;//!< info header.
    vector<vector<char>> imgData;//!< 像素数据.
}ImgInfo;

//! 以二进制方式读出文件头和信息头.
FILE* fp;
fopen_s(&fp, imgPath.c_str(), "rb");
if (fp == NULL) 
{
    cout << "打开文件失败!" << endl;
    exit(0);
}
fread(&imgInfo.bf, sizeof(BITMAPFILEHEADER), 1, fp);//!< 读出文件头
fread(&imgInfo.bi, sizeof(BITMAPINFOHEADER), 1, fp);//!< 读出信息头

//! 根据文件头中的信息，将读写标记定位到数据区.
fseek(fp, imgInfo.bf.bfOffBits, 0);

//! 申请保存所有像素值的内存. 根据info header中的图像宽高信息申请大小, 因为是真彩色, 所以每个像素有3个通道, 所以乘以3.
buf = (char*)malloc(imgInfo.bi.biWidth * imgInfo.bi.biHeight * 3);
//! 读出所有像素值信息放到buf里.
fread(buf, 1, imgInfo.bi.biWidth * imgInfo.bi.biHeight * 3, fp);

//! 每一个像素点是一个vector<char>类型, 因为有3个通道.
//! 把所有像素点都存到一个vector里, 也就是直接排成一排.
vector<vector<char>> imgData;
int x = 0, y = 0;
for (y = 0; y < imgInfo.bi.biHeight; y++)//!< 控制行数.
{
    for (x = 0; x < imgInfo.bi.biWidth; x++)//!< 控制列数.
    {
        vector<char> vRGB;//!< 存储一个像素点的各通道值.

        vRGB.push_back(*(p++));//!< blue
        vRGB.push_back(*(p++));//!< green
        vRGB.push_back(*(p++));//!< red

        if (x == imgInfo.bi.biWidth - 1)//!< 读完每一行, 保证指针4字节对齐, 少几个字节不是4的倍数就偏移几次.
        {
            for (int k = 0; k < imgInfo.bi.biWidth % 4; k++) 
                p++;
        }
        imgData.push_back(vRGB);//!< 将一个像素点放到容器里.
    }
}
fclose(fp);
imgInfo.imgData = imgData;
```
**注意**: 上面代码中读取像素数据的时候是有一个**对齐**操作的(这涉及到计算机体系结构中的内存对齐问题, 主要是为了CPU的访问速度而考量的), 如果不做这一步对齐, 会导致读取的图像信息错误, 最终显示出来就很奇怪.

## 4. 图像滤波算法实现

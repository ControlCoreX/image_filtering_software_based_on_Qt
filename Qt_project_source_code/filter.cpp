#include "filter.h"
#include "QDebug"
#include "vector"
#include "random"
#include "ctime"
#include "cmath"


//构造函数
void Filter::init(cv::Mat img)
{
    img.copyTo(this->src);
    this->rows=img.rows;
    this->cols=img.cols;
    this->channels=img.channels();
    this->dst=cv::Mat::zeros(this->rows,this->cols,CV_8UC3);
}

//添加椒盐噪声
void Filter::addPepperSaltNoise(void)
{
    srand((uint32_t)time(NULL));//种随机数种子
    int num = 4000;//噪点的个数
    int x = 0, y = 0;
    int noise = 0;
    for (int i = 0; i < num; i++)//添加噪点
    {
        x = rand() % this->cols;
        y = rand() % this->rows;
        noise = rand() % 2;//用来决定添加 椒点 还是 盐点
        if (noise == 0)
        {
            this->src.at<cv::Vec3b>(y,x)[0]=0;//椒点
            this->src.at<cv::Vec3b>(y,x)[1]=0;
            this->src.at<cv::Vec3b>(y,x)[2]=0;
        }
        else
        {
            this->src.at<cv::Vec3b>(y,x)[0]=255;//盐点
            this->src.at<cv::Vec3b>(y,x)[1]=255;
            this->src.at<cv::Vec3b>(y,x)[2]=255;
        }
    }
}

//添加高斯噪声
void Filter::addGaussNoise(void)
{
    cv::Mat normalNoise=cv::Mat::zeros(this->src.rows,this->src.cols,this->src.type());
    cv::RNG rng;
    rng.fill(normalNoise,cv::RNG::NORMAL,15,5);
//    cv::imshow("normal",normalNoise);
    this->src=this->src+normalNoise;
}

//添加泊松噪声
void Filter::addPoissonNoise(void)
{
    static std::default_random_engine engine(time(NULL));//定义一个随机数引擎对象
    static std::exponential_distribution<double> poisson(0.1);//定义分布引擎

    cv::Mat PoissonNoise=cv::Mat::zeros(this->src.rows,this->src.cols,this->src.type());//创建噪声图像

    for(int i=0;i<this->src.rows;i++)//双层循环遍历每个像素点
    {
        for(int j=0;j<this->src.cols;j++)
        {
            PoissonNoise.at<cv::Vec3b>(i,j)[0]=static_cast<unsigned char>(1*(poisson(engine)-0.1));//生成服从泊松分布的随机数，并作为像素值
            PoissonNoise.at<cv::Vec3b>(i,j)[1]=static_cast<unsigned char>(1*(poisson(engine)-0.1));
            PoissonNoise.at<cv::Vec3b>(i,j)[2]=static_cast<unsigned char>(1*(poisson(engine)-0.1));
        }
    }
//    cv::imshow("噪声",PoissonNoise);
    this->src=this->src+PoissonNoise;//将原图像与噪声图像叠加，得到添加噪声后的图像
}

//中值滤波
void Filter::medianFilter(void)
{
    //核大小为3*3
    int delta[9][2] = { {-1,-1},{0,-1},{1,-1},{-1,0},{0,0},{1,0},{-1,1},{0,1},{1,1} };//像素坐标偏移量
    unsigned char kernelBlue[9] = { 0 }, kernelGreen[9] = { 0 }, kernelRed[9] = {0};

    //遍历每个像素点，并给每个像素点赋滤波后的值
    for (int i = 1; i < this->rows - 1; i++)//不考虑边缘像素
    {
        for (int j = 1; j < this->cols - 1; j++)
        {
            for (int k = 0; k < 9; k++)
            {
                kernelBlue[k]=this->src.at<cv::Vec3b>(i + delta[k][0],j + delta[k][1])[0];
                kernelGreen[k]=this->src.at<cv::Vec3b>(i + delta[k][0],j + delta[k][1])[1];
                kernelRed[k]=this->src.at<cv::Vec3b>(i + delta[k][0],j + delta[k][1])[2];
            }
            this->sort(kernelBlue, 9);//排序，为了找到中值
            this->sort(kernelGreen, 9);
            this->sort(kernelRed, 9);

            this->dst.at<cv::Vec3b>(i,j)[0]=kernelBlue[4];
            this->dst.at<cv::Vec3b>(i,j)[1]=kernelGreen[4];
            this->dst.at<cv::Vec3b>(i,j)[2]=kernelRed[4];
        }
    }
}

//均值滤波
void Filter::meanFilter(void)
{
    //核大小为3*3
    int delta[9][2] = { {-1,-1},{0,-1},{1,-1},{-1,0},{0,0},{1,0},{-1,1},{0,1},{1,1} };//像素坐标偏移量
    unsigned char kernelBlue[9] = { 0 }, kernelGreen[9] = { 0 }, kernelRed[9] = {0};

    //遍历每个像素点，并给每个像素点赋滤波后的值
    for (int i = 1; i < this->rows - 1; i++)//不考虑边缘像素
    {
        for (int j = 1; j < this->cols - 1; j++)
        {
            for (int k = 0; k < 9; k++)
            {
                kernelBlue[k]=this->src.at<cv::Vec3b>(i + delta[k][0],j + delta[k][1])[0];
                kernelGreen[k]=this->src.at<cv::Vec3b>(i + delta[k][0],j + delta[k][1])[1];
                kernelRed[k]=this->src.at<cv::Vec3b>(i + delta[k][0],j + delta[k][1])[2];
            }
            this->dst.at<cv::Vec3b>(i,j)[0]=this->getMeanValue(kernelBlue, 9);;
            this->dst.at<cv::Vec3b>(i,j)[1]=this->getMeanValue(kernelGreen, 9);
            this->dst.at<cv::Vec3b>(i,j)[2]=this->getMeanValue(kernelRed, 9);
        }
    }
}

//傅里叶降噪
void Filter::Fourier(void)
{
#if 0
    std::vector<cv::Mat> matVec0,matVec1;
    cv::split(this->src, matVec0);

    for (int i = 0; i < 3; i++)
    {
        int m = cv::getOptimalDFTSize(matVec0[i].rows); //2,3,5的倍数有更高效率的傅里叶变换
        int n = cv::getOptimalDFTSize(matVec0[i].cols);

        cv::Mat padded;//填充边框后的图像
        cv::copyMakeBorder(matVec0[i], padded, 0, m - matVec0[i].rows, 0, n - matVec0[i].cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));//把灰度图像放在左上角,在右边和下边扩展图像,扩展部分填充为0;
        //cout << padded.size() << endl;
        //这里是获取了两个Mat,一个用于存放dft变换的实部，一个用于存放虚部,初始的时候,实部就是图像本身,虚部全为零
        cv::Mat planes[] = { cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F) };//在这里是声明两个Mat矩阵，分别存放实部和虚部
        cv::Mat planes_true = cv::Mat_<float>(padded);
        cv::Mat complexImg;//复数图像
        cv::merge(planes, 2, complexImg);//将几个单通道的mat融合成一个多通道的mat,这里融合的complexImg既有实部又有虚部

        //对复数图像进行傅里叶变换
        cv::dft(complexImg, complexImg);//对上边合成的mat进行傅里叶变换,***支持原地操作***,傅里叶变换结果为复数.通道1存的是实部,通道二存的是虚部

        cv::split(complexImg, planes);//把变换后的结果分割到两个mat,一个实部,一个虚部,方便后续操作

        //这一部分是为了计算dft变换后的幅值，傅立叶变换的幅度值范围大到不适合在屏幕上显示。高值在屏幕上显示为白点，而低值为黑点，高低值的变化无法有效分辨。为了在屏幕上凸显出高低变化的连续性，我们可以用对数尺度来替换线性尺度,以便于显示幅值,计算公式如下:
        //=> log(1 + sqrt(Re(DFT(I))^2 +Im(DFT(I))^2))
        //计算实数和虚数的幅值，并把幅值保存到palnes[0],允许在线覆盖
        cv::magnitude(planes[0], planes[1], planes_true);
        cv::Mat A = planes[0];
        cv::Mat B = planes[1];
        cv::Mat mag = planes_true;
        //对幅值加1
        mag += cv::Scalar::all(1);
        log(mag, mag);//计算出的幅值一般很大，达到10^4,通常没有办法在图像中显示出来，需要对其进行log求解。

        for (int i = 0; i < mag.rows; i++)//在这里进行一次低通滤波
        {
            for (int j = 0; j < mag.cols; j++)
            {
                float num = mag.at<float>(i, j);
                if (num > 13.5)//13.5为幅度阈值，这里是低通滤波，高通滤波只需要改成＜就好
                {
                    planes[0].at<float>(i, j) = 0;
                    planes[1].at<float>(i, j) = 0;
                }
            }
        }
        cv::merge(planes, 2, complexImg);
        //crop the spectrum, if it has an odd number of rows or columns
        //修剪频谱,如果图像的行或者列是奇数的话,那其频谱是不对称的,因此要修剪
        //这里为什么要用  &-2这个操作，我会在代码后面的 注2 说明
        //我们知道x&-2代表x与 - 2按位相与，而 - 2的二进制形式是2的二进制取反加一的结果（这是补码的问题）。2 的二进制结果是（假设
        //8位表示，实际整型是32位，但是描述方式是一样的，为便于描述，用8位表示）0000 0010，则 - 2的二进制形式为：1111 1110，
        //x与 - 2按位相与后，不管x是奇数还是偶数，最后x都会变成一个偶数。
        //就是说dft这个函数虽然对于输入mat的尺寸不做要求，但是如果其行数和列数可以分解为2、3、5的乘积，那么对于dft运算的速度会加快很多。
        mag = mag(cv::Rect(0, 0, mag.cols & -2, mag.rows & -2));
        cv::Mat _magI = mag.clone();
        //这一步的目的仍然是为了显示,但是幅度值仍然超过可显示范围[0,1],我们使用 normalize() 函数将幅度归一化到可显示范围。
        cv::normalize(_magI, _magI, 0, 1, cv::NORM_MINMAX);
        //namedWindow("before rearrange", 0);
        //imshow("before rearrange", _magI);

        //rearrange the quadrants of Fourier image
        //so that the origin is at the image center
        //重新分配象限，使（0,0）移动到图像中心，
        //在《数字图像处理》中，傅里叶变换之前要对源图像乘以（-1）^(x+y)进行中心化。
        //这是是对傅里叶变换结果进行中心化
        int cx = mag.cols / 2;
        int cy = mag.rows / 2;

        //这里是以中心为标准，把mag图像分成四部分
        cv::Mat tmp;
        cv::Mat q0(mag, cv::Rect(0, 0, cx, cy));   //Top-Left - Create a ROI per quadrant
        cv::Mat q1(mag, cv::Rect(cx, 0, cx, cy));  //Top-Right
        cv::Mat q2(mag, cv::Rect(0, cy, cx, cy));  //Bottom-Left
        cv::Mat q3(mag, cv::Rect(cx, cy, cx, cy)); //Bottom-Right

            //swap quadrants(Top-Left with Bottom-Right)，交换象限
        q0.copyTo(tmp);
        q3.copyTo(q0);
        tmp.copyTo(q3);

        // swap quadrant (Top-Rightwith Bottom-Left)，交换象限
        q1.copyTo(tmp);
        q2.copyTo(q1);
        tmp.copyTo(q2);

        cv::normalize(mag, mag, 0, 1, cv::NORM_MINMAX);

        //傅里叶的逆变换
        cv::Mat ifft;
        //傅里叶逆变换
        cv::idft(complexImg, ifft, cv::DFT_REAL_OUTPUT);
        cv::normalize(ifft, ifft, 0, 1, cv::NORM_MINMAX);

        matVec0[i] = ifft;
    }
//    cv::imshow("1",matVec0[0]);
//    cv::imshow("2",matVec0[1]);
//    cv::imshow("3",matVec0[2]);

    cv::Mat image;
    this->dst.copyTo(image);

    cv::Size size(matVec0[0].cols,matVec0[0].rows);
    cv::resize(image,this->dst,size);
    merge(matVec0, this->dst);
#endif
    cv::GaussianBlur(this->src, this->dst, cv::Size(3*2 + 1, 3*2 + 1), 0, 0);
//    cv::imshow("4",this->dst);
}

//冒泡排序，用于中值滤波
void Filter::sort(unsigned char array[], int n)
{
    int i, j;
    unsigned char temp;
    for (i = 0; i < n - 1; i++)
    {
        for (j = i + 1; j < n; j++)
        {
            if (array[j] > array[i])
            {
                temp = array[j];
                array[j] = array[i];
                array[i] = temp;
            }
        }
    }
}

//求平均值，用于均值滤波
unsigned char Filter::getMeanValue(unsigned char array[], int n)
{
    float sum=0.0;
    for(int i=0;i<n;i++)
    {
        sum+=array[i];
    }
    return (unsigned char)(sum/n);
}

//获取滤波后的效果图
cv::Mat Filter::getResultImg(void)
{
    return this->dst;
}



//Quality类的定义
void Quality::setSrc(cv::Mat img)
{
    this->src=img;
}

void Quality::setNoiseSrc(cv::Mat img)
{
    this->noiseSrc=img;
}

double Quality::calculateMSE(void)
{
    int rows=this->src.rows;
    int cols=this->src.cols;
    int temp;
    double sum[3]={0.0};
    double mse[3];

    for(int k=0;k<3;k++)
    {
        for(int i=1;i<rows-1;i++)
        {
            for(int j=1;j<cols-1;j++)
            {
                temp=this->src.at<cv::Vec3b>(i,j)[k] - this->noiseSrc.at<cv::Vec3b>(i,j)[k];//得到差
                sum[k]+=(temp*temp);//得到差的平方，并加上去
            }
        }
        mse[k]=sum[k]/((rows-1)*(cols-1));//得到k通道的均方差
    }

    this->MSE=(mse[0]+mse[1]+mse[2])/3;//3个通道的均方差求均值得到彩色图像的MSE
    return this->MSE;
}

double Quality::calculatePSNR(void)
{
    this->PSNR=10*log10(255*255/this->MSE);
    return this->PSNR;
}

void Quality::showMSE_PSNR(double mse, double psnr)
{
    QString strmse=QString("%1").arg(mse);
    QString strpsnr=QString("%1").arg(psnr);

    exUI->label_MSE->setText(strmse);
    exUI->label_PSNR->setText(strpsnr);
}




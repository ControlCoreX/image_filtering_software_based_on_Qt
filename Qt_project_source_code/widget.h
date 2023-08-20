#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "opencv2/opencv.hpp"
#include "filter.h"


QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    QImage changeImgType(cv::Mat);
    void qimshow(int whichLabel,cv::Mat img);

    ~Widget();

public slots:
    void choseImgSlot(void);
    void addNoiseSlot(int index);
    void meanFiltSlot(void);
    void medianFiltSlot(void);
    void FourierSlot(void);

private:
    Ui::Widget *ui;
    QString choseImgPath;
    Filter myFilter;
    Quality myQuality;
    cv::Mat imgTemp;
};

#endif // WIDGET_H

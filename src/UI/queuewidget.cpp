#include "queuewidget.h"

#ifdef QT_DEBUG
#include <QtDebug>
#endif

QueueWidget::QueueWidget(FFmpeg *ffmpeg, QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    _ffmpeg = ffmpeg;
    _inputMedias = new MediaList();

    addInput();
    addOutput();

    //hide close buttons on add buttons
    QTabBar *outputTabBar = outputTab->tabBar();
    QWidget *closeButton = outputTabBar->tabButton(1, QTabBar::RightSide);
    if (closeButton == nullptr) closeButton = outputTabBar->tabButton(1, QTabBar::LeftSide);
    if (closeButton != nullptr) closeButton->hide();

    QTabBar *inputTabBar = inputTab->tabBar();
    closeButton = inputTabBar->tabButton(1, QTabBar::RightSide);
    if (closeButton == nullptr) closeButton = inputTabBar->tabButton(1, QTabBar::LeftSide);
    if (closeButton != nullptr) closeButton->hide();

    // splitter sizes
    QSettings settings;
    settings.beginGroup("queueSplitter");
    QList<int>sizes;
    sizes << settings.value("inputSize",50).toInt();
    sizes << settings.value("outputSize",50).toInt();
    splitter->setSizes(sizes);
    settings.endGroup();
}

QList<MediaInfo *> QueueWidget::getInputMedia()
{
    QList<MediaInfo *> input;
    foreach(InputWidget *iw,inputWidgets)
    {
        input << iw->getMediaInfo();
    }
    return input;
}

QList<MediaInfo *> QueueWidget::getOutputMedia()
{
    QList<MediaInfo *> output;
    foreach(OutputWidget *ow,outputWidgets)
    {
        output << ow->getMediaInfo();
    }
    return output;
}

void QueueWidget::addInputFile(QString file)
{
    inputWidgets[ inputTab->currentIndex() ]->openFile(file);
}

void QueueWidget::addInputFile(QUrl file)
{
    inputWidgets[ inputTab->currentIndex() ]->openFile(file);
}

void QueueWidget::presetsPathChanged()
{
    foreach(OutputWidget *ow,outputWidgets)
    {
        ow->loadPresets();
    }
}

void QueueWidget::on_outputTab_tabCloseRequested(int index)
{
    int numTabs = outputTab->count();
    if (index == numTabs-1) return;
    if (numTabs <= 2) return;

    //remove tab
    outputTab->removeTab(index);
    //delete corresponding widget
    OutputWidget *w = outputWidgets.takeAt(index);
    w->deleteLater();
    //update current index
    if (outputTab->currentIndex() == outputTab->count()-1)
    {
        outputTab->setCurrentIndex(outputTab->count()-2);
    }
}

void QueueWidget::on_outputTab_tabBarClicked(int index)
{
    if (index != outputTab->count()-1) return;
    //add new
    addOutput();
}

void QueueWidget::on_inputTab_tabCloseRequested(int index)
{
    int numTabs = inputTab->count();
    if (index == numTabs-1) return;
    if (numTabs <= 2) return;

    //remove tab
    inputTab->removeTab(index);
    //delete corresponding widget and media
    InputWidget *w = inputWidgets.takeAt(index);
    _inputMedias->removeMedia( w->mediaInfo() );
    w->deleteLater();
    //update current index
    if (inputTab->currentIndex() == inputTab->count()-1)
    {
        inputTab->setCurrentIndex(inputTab->count()-2);
    }
}

void QueueWidget::on_inputTab_tabBarClicked(int index)
{
    if (index != inputTab->count()-1) return;
    //add new
    addInput();
}

void QueueWidget::log(QString log, LogUtils::LogType lt )
{
    emit newLog(log, lt);
}

void QueueWidget::addInput()
{
    int num = inputWidgets.count();

    //create widget
    InputWidget *inputWidget = new InputWidget(_ffmpeg, num, this);
    inputWidgets << inputWidget;
    _inputMedias->addMedia( inputWidget->mediaInfo() );
    inputTab->insertTab( inputTab->count()-1, inputWidget, "Input " + QString::number(num + 1));

    //set tab index
    inputTab->setCurrentIndex(inputTab->count()-2);
}

void QueueWidget::addOutput()
{
    //number of the output
    int num = outputWidgets.count()+1;

    //create widget
    OutputWidget *ow = new OutputWidget(_ffmpeg, num, _inputMedias, this);
    outputWidgets << ow;
    outputTab->insertTab(outputTab->count()-1,ow,"Output " + QString::number(num));

    //connect console
    connect(ow,SIGNAL(newLog(QString,LogUtils::LogType)),this,SLOT(log(QString,LogUtils::LogType)));
    //set tab index
    outputTab->setCurrentIndex(outputTab->count()-2);
}

void QueueWidget::on_splitter_splitterMoved()
{
    QSettings settings;
    settings.beginGroup("queueSplitter");
    settings.setValue("inputSize", splitter->sizes()[0]);
    settings.setValue("outputSize", splitter->sizes()[1]);
    settings.endGroup();
}
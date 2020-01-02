#include "uiinputwidget.h"

#ifdef QT_DEBUG
#include <QtDebug>
#endif

UIInputWidget::UIInputWidget(FFmpeg *ff, QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    _mediaInfo = new MediaInfo( ff, this);

    // CREATE MENUS
    blocksMenu = new QMenu(this);
    blocksMenu->setTearOffEnabled(true);
    addBlockButton->setMenu( blocksMenu );

    // CREATE BLOCKS
    blockFrameRateContent = new BlockFrameRate( _mediaInfo );
    blockFrameRate = addBlock( blockFrameRateContent, actionFramerate );
    blockColorContent = new BlockColor( _mediaInfo );
    blockColor = addBlock( blockColorContent, actionColor );
    blockEXRContent = new BlockEXR( _mediaInfo );
    blockEXR = addBlock( blockEXRContent, actionEXR );
    blockAECompContent = new BlockAEComp( _mediaInfo );
    blockAEComp = addBlock( blockAECompContent, actionAfter_Effects_composition);

    updateOptions();
}

MediaInfo *UIInputWidget::getMediaInfo()
{
    //update custom params before returning
    _mediaInfo->clearFFmpegOptions();

    for (int i = 0 ; i < _customParamEdits.count() ; i++)
    {
        QString param = _customParamEdits[i]->text();
        if (param != "")
        {
            QStringList option(param);
            option << _customValueEdits[i]->text();
            _mediaInfo->addFFmpegOption(option);
        }
    }

    return _mediaInfo;
}

void UIInputWidget::openFile(QString file)
{
    QSettings settings;
    if (file == "") return;
    file = QDir::toNativeSeparators( file );

    //Text
    QString mediaInfoString = "Media information";

    QFileInfo fileInfo(file);
    _mediaInfo->updateInfo( fileInfo );

    //update UI
    inputEdit->setText( file );
    //keep in settings
    settings.setValue("input/path", fileInfo.path() );

    if ( _mediaInfo->isAep() )
    {
        _mediaInfo->setAepNumThreads(threadsBox->value());
        if (fileInfo.suffix() == "aep") mediaInfoString += "\n\nAfter Effects project.";
        if (fileInfo.suffix() == "aet") mediaInfoString += "\n\nAfter Effects template.";
        if (fileInfo.suffix() == "aepx") mediaInfoString += "\n\nAfter Effects XML project.";
    }

    if ( _mediaInfo->muxer() != nullptr )
    {
        mediaInfoString += "\n\nFile format: " + _mediaInfo->muxer()->prettyName();
    }
    else
    {
        mediaInfoString += "\n\nContainers: " + _mediaInfo->extensions().join(",");
    }


    if (_mediaInfo->duration() != 0.0)
    {
        QTime duration(0,0,0);
        duration = duration.addSecs( int( _mediaInfo->duration() ) );
        mediaInfoString += "\nDuration: " + duration.toString("hh:mm:ss.zzz");
    }
    else if (_mediaInfo->isImageSequence())
    {
        mediaInfoString += "\nDuration: " + QString::number(  _mediaInfo->frames().count() ) + " frames";
        mediaInfoString += "\nStart Frame Number: " + QString::number( _mediaInfo->startNumber() );
    }

    qint64 size = _mediaInfo->size( );
    mediaInfoString += "\nSize: " + MediaUtils::sizeString( size );

    if ( !_mediaInfo->isAep() )
    {
        qint64 bitrate = _mediaInfo->bitrate();
        mediaInfoString += "\nGlobal bitrate: " + MediaUtils::bitrateString( bitrate );

        mediaInfoString += "\nContains video: ";
        if (_mediaInfo->hasVideo()) mediaInfoString += "yes";
        else mediaInfoString += "no";
        mediaInfoString += "\nContains audio: ";
        if (_mediaInfo->hasAudio()) mediaInfoString += "yes";
        else mediaInfoString += "no";
        mediaInfoString += "\nImage sequence: ";
        if (_mediaInfo->isImageSequence()) mediaInfoString += "yes";
        else mediaInfoString += "no";

        if (_mediaInfo->hasVideo())
        {
            mediaInfoString += "\n\nVideo codec: ";
            if(_mediaInfo->videoCodec() != nullptr)
            {
                mediaInfoString += _mediaInfo->videoCodec()->prettyName();
            }
            mediaInfoString += "\nResolution: " + QString::number(_mediaInfo->videoWidth()) + "x" + QString::number(_mediaInfo->videoHeight());
            mediaInfoString += "\nVideo Aspect: " + QString::number( int( _mediaInfo->videoAspect()*100+0.5 ) / 100.0) + ":1";
            mediaInfoString += "\nFramerate: " + QString::number(_mediaInfo->videoFramerate()) + " fps";
            qint64 bitrate = _mediaInfo->videoBitrate( );
            if (bitrate != 0) mediaInfoString += "\nBitrate: " + MediaUtils::bitrateString(bitrate);
            mediaInfoString += "\nPixel Aspect: " + QString::number( int(_mediaInfo->pixAspect()*100+0.5)/ 100.0) + ":1";
            if (_mediaInfo->pixFormat() != nullptr)
            {
                mediaInfoString += "\nPixel Format: " + _mediaInfo->pixFormat()->prettyName();
                if (_mediaInfo->pixFormat()->hasAlpha()) mediaInfoString += "\nAlpha: yes";
                else mediaInfoString += "\nAlpha: no";
            }
        }

        if (_mediaInfo->hasAudio())
        {
            mediaInfoString += "\n\nAudio codec: ";
            if(_mediaInfo->audioCodec() != nullptr)
            {
                mediaInfoString += _mediaInfo->audioCodec()->prettyName();
            }
            mediaInfoString += "\nSampling rate: " + QString::number(_mediaInfo->audioSamplingRate()) + " Hz";
            if (_mediaInfo->audioChannels() != "")
            {
                mediaInfoString += "\nChannels: " + _mediaInfo->audioChannels();
            }
            int abitrate = int( _mediaInfo->audioBitrate( ) );
            if (abitrate != 0) mediaInfoString += "\nBitrate: " + MediaUtils::bitrateString(abitrate);
        }
    }

    mediaInfosText->setText(mediaInfoString);

    updateOptions();

    emit newMediaLoaded(_mediaInfo);
}

void UIInputWidget::openFile(QUrl file)
{
    openFile(file.toLocalFile());
}

void UIInputWidget::on_inputBrowseButton_clicked()
{
    QSettings settings;
    QString inputPath = QFileDialog::getOpenFileName(this,"Select the media file to transcode",settings.value("input/path","").toString());
    if (inputPath == "") return;
    openFile(inputPath);
}

void UIInputWidget::on_inputEdit_editingFinished()
{
    //check if file exists, try to read url
    QUrl test(inputEdit->text());
    if (!test.isEmpty())
    {
        if (test.isValid())
        {
            inputEdit->setText(test.toLocalFile());
        }
    }

    openFile(inputEdit->text());
}

void UIInputWidget::on_addParamButton_clicked()
{
    /*//add a param and a value
    QLineEdit *customParam = new QLineEdit(this);
    customParam->setPlaceholderText("-param");
    customParam->setMinimumWidth(100);
    customParam->setMaximumWidth(100);
    //the value edit
    QLineEdit *customValue = new QLineEdit(this);
    customValue->setPlaceholderText("Value");
    //add to layout and lists
    customParamsLayout->insertRow(customParamsLayout->rowCount(),customParam,customValue);
    _customParamEdits << customParam;
    _customValueEdits << customValue;*/
}

UIBlockWidget *UIInputWidget::addBlock(UIBlockContent *content, QAction *action)
{
    // create block
    UIBlockWidget *b = new UIBlockWidget( action->text(), content, blocksWidget);
    blocksLayout->addWidget( b );
    //add and connect action
    blocksMenu->addAction( action );
    connect( action, SIGNAL( triggered(bool) ), b, SLOT( setVisible(bool) ) );
    connect( b, SIGNAL( activated(bool) ), action, SLOT( setChecked( bool ) ) );

    return b;
}

void UIInputWidget::on_threadsButton_toggled(bool checked)
{
    threadsBox->setEnabled(checked);
    threadsBox->setValue(QThread::idealThreadCount());
}

void UIInputWidget::on_threadsBox_valueChanged(int arg1)
{
    _mediaInfo->setAepNumThreads(arg1);
}

void UIInputWidget::updateOptions()
{
    //frame rate
    blockFrameRate->hide();
    //exr prerender
    blockEXR->hide();
    //Aep
    blockAEComp->hide();
    threadsBox->hide();
    threadsButton->hide();


    //get media default extension (needed to adjust some options)
    QString extension = "";
    if (_mediaInfo->extensions().count() > 0) extension = _mediaInfo->extensions()[0];

    // framerate buttons
    if (_mediaInfo->isImageSequence() || _mediaInfo->isAep())
    {
        actionFramerate->setVisible( true );
        blockFrameRate->show();
    }
    else
    {
        actionFramerate->setVisible( false );
    }

    //exr prerender
    actionEXR->setVisible( extension == "exr_pipe" );

    if (_mediaInfo->isAep())
    {
        actionAfter_Effects_composition->setVisible(true);
        blockAEComp->show();
        threadsBox->show();
        threadsButton->show();
        threadsButton->setChecked(false);
        //for now, using half the threads.
        //TODO: count depending on RAM (3Go per thread for example)
        threadsBox->setValue(QThread::idealThreadCount()/2);
    }
    else
    {
        actionAfter_Effects_composition->setVisible(false);
        blockAEComp->hide();
    }

    emit newMediaLoaded(_mediaInfo);
}

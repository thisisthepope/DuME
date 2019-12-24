﻿#include "Renderer/renderqueue.h"

RenderQueue::RenderQueue(FFmpeg *ffmpeg, AfterEffects *afterEffects, QObject *parent ) : QObject(parent)
{
    setStatus( Initializing );

    // === FFmpeg ===

    // The transcoder
    _ffmpeg = ffmpeg;
    // Create the renderer
    _ffmpegRenderer = new FFmpegRenderer( _ffmpeg->binary() );
    // Connections
    connect( _ffmpeg, &FFmpeg::binaryChanged, _ffmpegRenderer, &FFmpegRenderer::setBinary ) ;
    connect( _ffmpegRenderer, &FFmpegRenderer::newError, this, &RenderQueue::ffmpegError ) ;
    connect( _ffmpegRenderer, &FFmpegRenderer::newLog, this, &RenderQueue::ffmpegLog ) ;
    connect( _ffmpegRenderer, &FFmpegRenderer::newOutput, this, &RenderQueue::ffmpegOutput ) ;
    connect( _ffmpegRenderer, &FFmpegRenderer::finished, this, &RenderQueue::ffmpegFinished ) ;
    connect( _ffmpegRenderer, &FFmpegRenderer::progress, this, &RenderQueue::ffmpegProgress ) ;


    // === After Effects ===

    // The renderer
    _ae = afterEffects;
    // Create the renderer
    _aeRenderer = new AERenderer( _ae->binary() );
    // Connections
    connect( _ae, &AfterEffects::currentAeVersionChanged, _aeRenderer, &AERenderer::setBinary ) ;
    connect( _aeRenderer, &AERenderer::newError, this, &RenderQueue::aeError ) ;
    connect( _aeRenderer, &AERenderer::newLog, this, &RenderQueue::aeLog ) ;
    connect( _aeRenderer, &AERenderer::newOutput, this, &RenderQueue::aeOutput ) ;
    connect( _aeRenderer, &AERenderer::finished, this, &RenderQueue::aeFinished ) ;
    connect( _aeRenderer, &AERenderer::progress, this, &RenderQueue::aeProgress ) ;

    // A timer to keep track of the rendering process
    timer = new QTimer( this );
    timer->setSingleShot(true);

    setStatus( Waiting );
}

RenderQueue::~RenderQueue()
{
    stop(100);
    postRenderCleanUp();
}

void RenderQueue::postRenderCleanUp()
{
    setStatus(Cleaning);

    _currentItem->postRenderCleanUp();

    encodeNextItem();
}

void RenderQueue::setStatus(RenderQueue::Status st)
{
    _status = st;
    emit statusChanged(_status);
}

void RenderQueue::log(QString message)
{
    qInfo() << message;
    emit newLog( message );
}

void RenderQueue::error(QString message)
{
    qWarning() << message;
    emit newError( message );
}

void RenderQueue::output(QString message)
{
    qDebug() << message;
    emit newOutput( message );
}

void RenderQueue::ffmpegLog(QString message)
{
    message = "FFmpeg | " + message;
    log( message );
}

void RenderQueue::ffmpegError(QString message)
{
    message = "FFmpeg | " + message;
    error( message );

    setStatus( Error );
    _currentItem->setStatus( QueueItem::Error );
    _encodingHistory << _currentItem;
}

void RenderQueue::ffmpegOutput(QString message)
{
    message = "FFmpeg | " + message;
    output( message );
}

void RenderQueue::ffmpegFinished()
{
    log("FFmpeg Transcoding process successfully finished");
    finished();
}

void RenderQueue::ffmpegProgress()
{
    //Relay progress information
    _numFrames = _ffmpegRenderer->numFrames();
    _frameRate = _ffmpegRenderer->frameRate();
    _currentFrame = _ffmpegRenderer->currentFrame();
    _startTime = _ffmpegRenderer->startTime();
    _outputSize = _ffmpegRenderer->outputSize();
    _outputBitrate = _ffmpegRenderer->outputBitrate();
    _expectedSize = _ffmpegRenderer->expectedSize();
    _encodingSpeed = _ffmpegRenderer->encodingSpeed();
    _timeRemaining = _ffmpegRenderer->timeRemaining();
    _elapsedTime = _ffmpegRenderer->elapsedTime();
    emit progress();
}

void RenderQueue::renderFFmpeg(QueueItem *item)
{
    //generate arguments
    QStringList arguments("-stats");
    arguments << "-y";

    //add inputs
    foreach(MediaInfo *input, item->getInputMedias())
    {
        QString inputFileName = input->fileName();
        //add custom options
        foreach(QStringList option,input->ffmpegOptions())
        {
            arguments << option[0];
            if (option.count() > 1)
            {
                if (option[1] != "") arguments << option[1];
            }
        }
        //add sequence options
        if (input->isImageSequence())
        {
            arguments << "-framerate" << QString::number(input->videoFramerate());
            arguments << "-start_number" << QString::number(input->startNumber());
            inputFileName = input->ffmpegSequenceName();
        }
        //add trc
        if (input->trc() != "")
        {
            arguments << "-apply_trc" << input->trc();
        }
        //add input file
        arguments << "-i" << QDir::toNativeSeparators(inputFileName);
    }
    //add outputs
    foreach(MediaInfo *output, item->getOutputMedias())
    {
        //muxer
        QString muxer = "";
        if (output->muxer() != nullptr)
        {
            muxer = output->muxer()->name();
            if (output->muxer()->isSequence()) muxer = "image2";
        }
        if (muxer != "")
        {
            arguments << "-f" << muxer;
        }

        //add custom options
        foreach(QStringList option,output->ffmpegOptions())
        {
            arguments << option[0];
            if (option.count() > 1)
            {
                if (option[1] != "") arguments << option[1];
            }
        }

        //video
        QString codec = "";
        if (output->videoCodec() != nullptr) codec = output->videoCodec()->name();


        if (output->hasVideo())
        {
            //codec
            if (codec != "") arguments << "-vcodec" << codec;

            if (codec != "copy")
            {
                //bitrate
                int bitrate = int( output->videoBitrate() );
                if (bitrate != 0)
                {
                    arguments << "-b:v" << QString::number(bitrate);
                }

                //size
                int width = output->videoWidth();
                int height = output->videoHeight();
                //fix odd sizes (for h264)
                if (codec == "h264" && width % 2 != 0)
                {
                    width--;
                    log("Adjusting width for h264 compatibility. New width: " + QString::number(width));
                }
                if (codec == "h264" && height % 2 != 0)
                {
                    height--;
                    log("Adjusting height for h264 compatibility. New height: " + QString::number(height));
                }
                if (width != 0 && height != 0)
                {
                    arguments << "-s" << QString::number(width) + "x" + QString::number(height);
                }

                //framerate
                double framerate = output->videoFramerate();
                if (framerate != 0.0)
                {
                    arguments << "-r" << QString::number(framerate);
                }

                //loop (gif)
                if (codec == "gif")
                {
                    int loop = output->loop();
                    arguments << "-loop" << QString::number(loop);
                }

                //profile
                int profile = output->videoProfile();
                if (profile > -1)
                {
                    arguments << "-profile" << QString::number(profile);
                }

                //quality (h264)
                int quality = output->videoQuality();
                if (codec == "h264" && quality > 0 )
                {
                    quality = 100-quality;
                    //adjust to CRF values
                    if (quality < 10)
                    {
                        //convert to range 0-15 // visually lossless
                        quality = quality*15/10;
                    }
                    else if (quality < 25)
                    {
                        //convert to range 15-21 // very good
                        quality = quality-10;
                        quality = quality*6/15;
                        quality = quality+15;
                    }
                    else if (quality < 50)
                    {
                        //convert to range 22-28 // good
                        quality = quality-25;
                        quality = quality*6/25;
                        quality = quality+21;
                    }
                    else if (quality < 75)
                    {
                        //convert to range 29-34 // bad
                        quality = quality-50;
                        quality = quality*6/25;
                        quality = quality+28;
                    }
                    else
                    {
                        //convert to range 35-51 // very bad
                        quality = quality-75;
                        quality = quality*17/25;
                        quality = quality+34;
                    }
                    arguments << "-crf" << QString::number(quality);
                }

                //start number (sequences)
                if (muxer == "image2")
                {
                    int startNumber = output->startNumber();
                    arguments << "-start_number" << QString::number(startNumber);
                }

                //pixel format
                QString pixFmt = "";
                if (output->pixFormat() != nullptr) pixFmt = output->pixFormat()->name();
                //set default for h264 to yuv420 (ffmpeg generates 444 by default which is not standard)
                if (pixFmt == "" && codec == "h264") pixFmt = "yuv420p";
                if (pixFmt != "") arguments << "-pix_fmt" << pixFmt;

                //b-pyramids
                //set as none to h264: not really useful (only on very static footage), but has compatibility issues
                if (codec == "h264") arguments << "-x264opts" << "b_pyramid=0";

                //unpremultiply
                bool unpremultiply = !output->premultipliedAlpha();
                if (unpremultiply) arguments << "-vf" << "unpremultiply=inplace=1";
            }

            //update the values which do not change from input
            //get the first input which has video
            MediaInfo *input = nullptr;
            foreach( MediaInfo *in, item->getInputMedias() )
            {
                if (in->hasVideo())
                {
                    input = in;
                    break;
                }
            }
            if (input != nullptr)
            {
                if ( output->videoFramerate() == 0.0 ) output->setVideoFramerate( input->videoFramerate() );
                if ( output->duration() == 0.0 ) output->setDuration( input->duration() );
            }

        }
        else
        {
            //no video
            arguments << "-vn";
        }

        //audio
        QString acodec = "";
        if (output->audioCodec() != nullptr) acodec = output->audioCodec()->name();

        if (output->hasAudio())
        {
            //codec
            if (acodec != "") arguments << "-acodec" << acodec;

            if (acodec != "copy")
            {
                //bitrate
                int bitrate = int( output->audioBitrate() );
                if (bitrate != 0)
                {
                    arguments << "-b:a" << QString::number(output->audioBitrate());
                }

                //sampling
                int sampling = output->audioSamplingRate();
                if (sampling != 0)
                {
                    arguments << "-ar" << QString::number(sampling);
                }
            }
        }
        else
        {
            //no audio
            arguments << "-an";
        }

        //file
        QString outputPath = QDir::toNativeSeparators(output->fileName());

        //if sequence, digits
        if (output->muxer() != nullptr)
        {
            if (output->isImageSequence())
            {
                outputPath = output->ffmpegSequenceName();
            }
        }

        arguments << outputPath;
    }

    log("Beginning new encoding\nUsing FFmpeg commands:\n" + arguments.join(" | "));

    //launch
    _ffmpegRenderer->setOutputFileName( item->getOutputMedias()[0]->fileName() );
    _ffmpegRenderer->setNumFrames( int( item->getOutputMedias()[0]->duration() * item->getOutputMedias()[0]->videoFramerate() ) );
    _ffmpegRenderer->setFrameRate( item->getOutputMedias()[0]->videoFramerate() );
    _ffmpegRenderer->start( arguments );
    _currentItem->setStatus(QueueItem::InProgress);
    emit encodingStarted( _currentItem );
    setStatus(FFmpegEncoding);
}

void RenderQueue::aeLog(QString message)
{
    message = "After Effects | " + message;
    log( message );
}

void RenderQueue::aeError(QString message)
{
    message = "After Effects | " + message;
    error( message );

    setStatus( Error );
    _currentItem->setStatus( QueueItem::Error );
    _encodingHistory << _currentItem;
}

void RenderQueue::aeOutput(QString message)
{
    message = "After Effects | " + message;
    output( message );
}

void RenderQueue::aeProgress()
{
    //Relay progress information
    _numFrames = _aeRenderer->numFrames();
    _frameRate = _aeRenderer->frameRate();
    _currentFrame = _aeRenderer->currentFrame();
    _startTime = _aeRenderer->startTime();
    _outputSize = _aeRenderer->outputSize();
    _outputBitrate = _aeRenderer->outputBitrate();
    _expectedSize = _aeRenderer->expectedSize();
    _encodingSpeed = _aeRenderer->encodingSpeed();
    _timeRemaining = _aeRenderer->timeRemaining();
    _elapsedTime = _aeRenderer->elapsedTime();
    emit progress();
}

RenderQueue::Status RenderQueue::status() const
{
    return _status;
}

QueueItem *RenderQueue::getCurrentItem()
{
    return _currentItem;
}

void RenderQueue::encode()
{
    if (_status == FFmpegEncoding || _status == AERendering ) return;

    setStatus(Launching);
    //launch first item
    encodeNextItem();
}

void RenderQueue::encode(QueueItem *item)
{
    _encodingQueue.clear();
    _encodingQueue << item;
    encode();
}

void RenderQueue::encode(QList<QueueItem*> list)
{
    _encodingQueue = list;
    encode();
}

void RenderQueue::encode(MediaInfo *input, QList<MediaInfo *> outputs)
{
    QueueItem *item = new QueueItem(input,outputs,this);
    _encodingQueue.clear();
    _encodingQueue << item;
    encode();
}

void RenderQueue::encode(MediaInfo *input, MediaInfo *output)
{
    QueueItem *item = new QueueItem(input,output,this);
    _encodingQueue.clear();
    _encodingQueue << item;
    encode();
}

int RenderQueue::addQueueItem(QueueItem *item)
{
    _encodingQueue << item;
    return _encodingQueue.count()-1;
}

void RenderQueue::removeQueueItem(int id)
{
    QueueItem *i = _encodingQueue.takeAt(id);
    i->deleteLater();
}

QueueItem *RenderQueue::takeQueueItem(int id)
{
    return _encodingQueue.takeAt(id);
}

void RenderQueue::clearQueue()
{
    while(_encodingQueue.count() > 0)
    {
        removeQueueItem(0);
    }
}

void RenderQueue::stop(int timeout)
{
    log( "Stopping queue" );

    if ( _status == FFmpegEncoding )
    {
        _ffmpegRenderer->stop( timeout );
    }
    else if ( _status == AERendering )
    {
        _aeRenderer->stop( timeout );
    }

    setStatus(Waiting);

    log( "Queue stopped" );
}

void RenderQueue::finished()
{
    if (_status == FFmpegEncoding || _status == AERendering || _status == BlenderRendering )
    {
        _currentItem->setStatus( QueueItem::Finished );
        //move to history
        _encodingHistory << _currentItem;

        setStatus(Cleaning);

        disconnect(timer, SIGNAL(timeout()), nullptr, nullptr);
        connect(timer,SIGNAL(timeout()), this, SLOT(postRenderCleanUp()));
        timer->start(3000);

        encodeNextItem();
    }
    else
    {
        setStatus(Waiting);
    }
}

void RenderQueue::encodeNextItem()
{
    if (_encodingQueue.count() == 0)
    {
        setStatus(Waiting);
        return;
    }

    setStatus(Launching);

    _currentItem = _encodingQueue.takeAt(0);

    //Check if there are AEP to render
    foreach(MediaInfo *input, _currentItem->getInputMedias())
    {
        if (input->isAep())
        {
            //check if we need audio
            bool needAudio = false;
            foreach(MediaInfo *output, _currentItem->getOutputMedias())
            {
                if (output->hasAudio())
                {
                    needAudio = true;
                    break;
                }
            }

            renderAep(input, needAudio);
            return;
        }
    }

    //Now all aep are rendered, transcode with ffmpeg
    renderFFmpeg( _currentItem );
}

void RenderQueue::aeFinished()
{
    MediaInfo *input = _currentItem->getInputMedias()[0];

    log("After Effects Render process successfully finished");

    //encode rendered EXR
    if (!input->aeUseRQueue())
    {
        //set exr
        //get one file
        QString aeTempPath = input->cacheDir()->path();
        QDir aeTempDir(aeTempPath);
        QStringList filters("DuME_*.exr");
        QStringList files = aeTempDir.entryList(filters,QDir::Files | QDir::NoDotAndDotDot);

        //if nothing has been rendered, set to error and go on with next queue item
        if (files.count() == 0)
        {
            _currentItem->setStatus(QueueItem::AEError);
            emit encodingFinished( );
            //move to history
            _encodingHistory << _currentItem;
            encodeNextItem();
            return;
        }

        //set file and launch
        QString prevTrc = input->trc();
        double frameRate = input->videoFramerate();
        input->updateInfo( _ffmpeg->analyseMedia(aeTempPath + "/" + files[0]));
        if (prevTrc == "") input->setTrc("gamma22");
        else input->setTrc(prevTrc);
        if (int( frameRate ) != 0) input->setVideoFramerate(frameRate);

        //reInsert at first place in renderqueue
        _encodingQueue.insert(0,_currentItem);

        encodeNextItem();
    }
    else
    {
        finished();
    }
}

void RenderQueue::renderAep(MediaInfo *input, bool audio)
{
    QStringList arguments("-project");
    QStringList audioArguments;
    arguments <<  QDir::toNativeSeparators(input->fileName());

    QString tempPath = "";

    //if not using the existing render queue
    if (!input->aeUseRQueue())
    {
        //get the cache dir
        QTemporaryDir *aeTempDir = new QTemporaryDir( settings.value("aerender/cache","" ).toString());
        input->setCacheDir(aeTempDir);

        //if a comp name is specified, render this comp
        if (input->aepCompName() != "") arguments << "-comp" << input->aepCompName();
        //else use the sepecified renderqueue item
        else if (input->aepRqindex() > 0) arguments << "-rqindex" << QString::number(input->aepRqindex());
        //or the first one if not specified
        else arguments << "-rqindex" << "1";

        //and finally, append arguments
        audioArguments = arguments;

        arguments << "-RStemplate" << "DuMultiMachine";
        arguments << "-OMtemplate" << "DuEXR";
        tempPath = QDir::toNativeSeparators(aeTempDir->path() + "/" + "DuME_[#####]");
        arguments << "-output" << tempPath;

        if (audio)
        {
            audioArguments << "-RStemplate" << "DuBest";
            audioArguments << "-OMtemplate" << "DuWAV";
            QString audioTempPath = QDir::toNativeSeparators(aeTempDir->path() + "/" + "DuME");
            audioArguments << "-output" << audioTempPath;
        }
        else
        {
            audioArguments.clear();
        }
    }

    log("Beginning After Effects rendering\nUsing aerender commands:\n" + arguments.join(" | "));

    //adjust the number of threads
    //keep one available for exporting the audio
    int numThreads = input->aepNumThreads();
    if (audio) numThreads--;

    // video
    _aeRenderer->setNumFrames( int( input->duration() * input->videoFramerate() ) );
    _aeRenderer->setFrameRate( input->videoFramerate() );
    _aeRenderer->setOutputFileName( tempPath );
    _aeRenderer->start( arguments, numThreads );
    // audio
    if (audio) _aeRenderer->start( audioArguments );

    _currentItem->setStatus(QueueItem::InProgress);
    setStatus(AERendering);
    emit encodingStarted(_currentItem);
}
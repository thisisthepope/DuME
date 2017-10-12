#include "ffmpeg.h"

FFmpeg::FFmpeg(QString path,QObject *parent) : QObject(parent)
{
    ffmpeg = new QProcess(this);
    ffmpeg->setProgram(path);
    getEncoders();
    getHelp();
    getLongHelp();


    currentFrame = 0;
    startTime = QTime(0,0,0);
    outputSize = 0.0;
    outputBitrate = 0;
    encodingSpeed = 0.0;

    //Connect process
    connect(ffmpeg,SIGNAL(readyReadStandardError()),this,SLOT(stdError()));
    connect(ffmpeg,SIGNAL(readyReadStandardOutput()),this,SLOT(stdOutput()));
    connect(ffmpeg,SIGNAL(started()),this,SLOT(started()));
    connect(ffmpeg,SIGNAL(finished(int)),this,SLOT(finished()));
    connect(ffmpeg,SIGNAL(errorOccurred(QProcess::ProcessError)),this,SLOT(errorOccurred(QProcess::ProcessError)));
}

QList<FFmpegCodec> FFmpeg::getEncoders()
{
    if (audioEncoders.count() == 0 || videoEncoders.count() == 0)
    {
        //TODO use signals instead of waiting? it's very quick maybe not needed
        ffmpeg->setArguments(QStringList("-encoders"));
        ffmpeg->start(QIODevice::ReadOnly);
        if (ffmpeg->waitForFinished(3000))
        {
            gotCodecs(ffmpeg->readAll());
        }
    }

    QList<FFmpegCodec> encoders = videoEncoders;
    encoders.append(audioEncoders);

    return encoders;
}

QList<FFmpegCodec> FFmpeg::getVideoEncoders()
{
    if (videoEncoders.count() > 0)
    {
        return videoEncoders;
    }
    getEncoders();
    return videoEncoders;
}

QList<FFmpegCodec> FFmpeg::getAudioEncoders()
{
    if (audioEncoders.count() > 0)
    {
        return audioEncoders;
    }
    getEncoders();
    return audioEncoders;
}

QString FFmpeg::getHelp()
{
    if (help != "") return help;

    //if first run, get the help
    //TODO use signals instead of waiting? it's very quick maybe not needed
    ffmpeg->setArguments(QStringList("-h"));
    ffmpeg->start(QIODevice::ReadOnly);
    if (ffmpeg->waitForFinished(3000))
    {
        help = ffmpeg->readAll();
    }
    return help;
}

QString FFmpeg::getLongHelp()
{
    if (longHelp != "") return longHelp;

    //if first run, get the long help
    //TODO use signals instead of waiting? it's very quick maybe not needed
    QStringList args("-h");
    args << "long";
    ffmpeg->setArguments(args);
    ffmpeg->start(QIODevice::ReadOnly);
    if (ffmpeg->waitForFinished(3000))
    {
        longHelp = ffmpeg->readAll();
    }
    return longHelp;
}

void FFmpeg::stdError()
{
    QString output = ffmpeg->readAllStandardError();
    readyRead(output);
}

void FFmpeg::stdOutput()
{
    QString output = ffmpeg->readAllStandardOutput();
    readyRead(output);
}

void FFmpeg::started()
{

}

void FFmpeg::finished()
{

}

void FFmpeg::errorOccurred(QProcess::ProcessError e)
{

}

void FFmpeg::gotCodecs(QString output)
{
    videoEncoders.clear();
    audioEncoders.clear();
    //get codecs
    QStringList codecs = output.split("\n");
    for (int i = 10 ; i < codecs.count() ; i++)
    {
        QString codec = codecs[i];

        //TODO adjust regexp to detect if encoding and/or decoding

        //if video encoding
        QRegExp reVideo("[\\.D]EV[\\.I][\\.L][\\.S] ([\\S]+)\\s+([^\\(\\n]+)");
        if (reVideo.indexIn(codec) != -1)
        {
            QString codecName = reVideo.cap(1);
            QString codecPrettyName = reVideo.cap(2);
            FFmpegCodec co(codecName,codecPrettyName,true);
            videoEncoders << co;
        }
        //if audio encoding
        QRegExp reAudio("[\\.D]EA[\\.I][\\.L][\\.S] ([\\S]+)\\s+([^\\(\\n]+)");
        if (reAudio.indexIn(codec) != -1)
        {
            QString codecName = reAudio.cap(1);
            QString codecPrettyName = reAudio.cap(2);
            FFmpegCodec co(codecName,codecPrettyName,false);
            audioEncoders << co;
        }
    }
}

void FFmpeg::readyRead(QString output)
{
    emit newOutput(output);

    ffmpegOutput = ffmpegOutput + output;

    QRegularExpression reProgress("frame= *(\\d+) fps= *(\\d+).+ L?size= *(\\d+)kB time=(\\d\\d:\\d\\d:\\d\\d.\\d\\d) bitrate= *(\\d+).\\d+kbits\\/s.* speed= *(\\d+.\\d*)x");
    QRegularExpressionMatch match = reProgress.match(output);
    //if progress, update UI
    if (match.hasMatch())
    {
        QString frame = match.captured(1);
        QString size = match.captured(3);
        QString bitrate = match.captured(5);
        QString speed = match.captured(6);

        //frame
        currentFrame = frame.toInt();

        //size
        int sizeKB = size.toInt();
        outputSize = sizeKB/1024;
        int sizeMBRounded = outputSize*100+0.5;
        outputSize = sizeMBRounded / 100.0;

        //size
        int bitrateKB = bitrate.toInt();
        outputSize = bitrateKB/1024;

        //speed
        encodingSpeed = speed.toDouble();

        //time remaining
        if (inputInfos->getDuration() > 0)
        {
            int max = inputInfos->getDuration() * inputInfos->getVideoFramerate();
            if (currentFrame > 0)
            {
                int elapsed = startTime.elapsed() / 1000;
                int remaining = elapsed*max/currentFrame - elapsed;
                timeRemaining = QTime(0,0,0).addSecs(remaining);
            }
        }
    }
    else
    {
        //if beginning, get infos (duration) of current input
        delete inputInfos;
        inputInfos = new MediaInfo(ffmpegOutput,this);
    }
}
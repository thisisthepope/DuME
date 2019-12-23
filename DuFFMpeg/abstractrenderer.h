#ifndef ABSTRACTRENDERER_H
#define ABSTRACTRENDERER_H

#include <QObject>
#include <QTime>
#include <QProcess>
#include <QTimer>
#include <QRegularExpression>
#include <QFileInfoList>
#include <QDir>

#include "utils.h"

/**
 * @brief The AbstractRenderer class is the base class for all renderers: ffmpeg, after effects, blender...
 */
class AbstractRenderer : public QObject
{
    Q_OBJECT
public:
    explicit AbstractRenderer(QObject *parent = nullptr);

    // RENDERING PROGRESS INFO
    /**
     * @brief currentFrame The frame being rendered
     * @return
     */
    int currentFrame() const;
    /**
     * @brief setCurrentFrame Sets the current frame being rendered and updates all progress info (size, time, bitrate...)
     * @param currentFrame
     */
    void setCurrentFrame(int currentFrame);
    /**
     * @brief startTime The start time of the rendering process
     * @return
     */
    QTime startTime() const;
    /**
     * @brief outputSize The expected output file size
     * @return
     */
    double outputSize(MediaUtils::SizeUnit unit = MediaUtils::MB) const;
    /**
     * @brief outputBitrate The output bitrate
     * @return
     */
    double outputBitrate(MediaUtils::BitrateUnit unit = MediaUtils::Kbps) const;
    /**
     * @brief encodingSpeed The rendering speed, compared to the media duration
     * @return
     */
    double encodingSpeed() const;
    /**
     * @brief timeRemaining The expected remaining time to finish the rendering
     * @return
     */
    QTime timeRemaining() const;
    /**
     * @brief elapsedTime The elapsed time since the rendering process has started
     * @return
     */
    QTime elapsedTime() const;

    // OUTPUT FILE INFO
    /**
     * @brief outputFileName The output file name.
     * @return
     */
    QString outputFileName() const;
    /**
     * @brief setNumFrames The number of frames to render
     * @param numFrames
     */
    void setNumFrames(int numFrames);
    /**
     * @brief setOutputFileName Sets the output file name.
     * @param outputFileName
     */
    void setOutputFileName(const QString &outputFileName);
    /**
     * @brief setFrameRate The framerate of the output video
     * @param frameRate
     */
    void setFrameRate(double frameRate);

    // MANAGE THE RENDERING PROCESS
    /**
     * @brief setStopCommand Sets the command to send to the renderer to stop it, if any.
     * @param stopCommand
     * @default ""
     */
    void setStopCommand(const QString &stopCommand);
    /**
     * @brief start Starts a rendering process
     * @param arguments The arguments to pass to the renderer
     */
    void start( QStringList arguments );
    /**
     * @brief stop Stops the current process(es)
     * @param timeout Kills the process after timeout if it does not respond to the stop commands. In milliseconds.
     */
    void stop(int timeout = 10000);
    /**
     * @brief numThreads The number of threads to use when launching the rendering process
     * @return
     */
    int numThreads() const;
    /**
     * @brief setNumThreads Sets the number of threads to use
     * @param numThreads
     */
    void setNumThreads(int numThreads);

    // CONFIGURE RENDERER
    void setBinaryFileName(const QString &binaryFileName);


signals:
    /**
     * @brief newError Emitted when a blocking error occurs. Contains the description of the error.
     */
    void newError(QString);
    /**
     * @brief newLog Emitted when some debug infos are available
     */
    void newLog(QString);
    /**
     * @brief newOutput Emitted when the process(es) output something
     */
    void newOutput(QString);
    /**
     * @brief started Emitted when the rendering has just started
     */
    void started();
    /**
     * @brief finished Emitted when the rendering has finished
     */
    void finished();
    /**
     * @brief progress Emitted each time the transcoding process outputs new stats
     */
    void progress();

protected:

    // RENDERING PROCESS

    // the output fileName
    QString _outputFileName;
    // the number of frames to render
    int _numFrames;
    // the framerate of the video
    double _frameRate;
    // the current frame being renderered.
    int _currentFrame;
    // the starttime of the rendering process
    QTime _startTime;
    // the current output size of the rendered file in Bytes
    double _outputSize;
    // the output bitrate of the renderered file in bps
    double _outputBitrate;
    // the expected output size
    double _expectedSize;
    // the speed of the encoding compared to the speed of the video
    double _encodingSpeed;
    // the time remaining before rendering completion
    QTime _timeRemaining;
    // the elapsed time
    QTime _elapsedTime;

    // METHODS

    //Called when the process outputs something on stdError or stdOutput. Reimplement this method to interpret the output. It has to emit progress() at the end, and can use setCurrentFrame().
    void readyRead(QString output);

private slots:
    // gets the output from the render process(es)
    void processStdError();
    void processStdOutput();
    void processStarted();
    void processFinished();
    void processErrorOccurred(QProcess::ProcessError e);
    // kills all render processes
    void killRenderProcesses();

private:
    // The process(es)
    QList<QProcess *> _renderProcesses;
    QString _binaryFileName;

    // CONFIGURATION

    QString _stopCommand;
    int _numThreads;

    //Launches a new process
    void launchProcess(QStringList arguments );
};

#endif // ABSTRACTRENDERER_H
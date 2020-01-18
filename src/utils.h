﻿#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QFile>
#include <QObject>
#include <QRegularExpression>
#include <QDebug>

#include "languagelist.h"

namespace FileUtils
{
    void setReadWrite(QFile *file);
    void setReadWrite(QString path);
    void move(QString from, QString to);
    void copy(QString from, QString to);
    void remove(QString path);
};

namespace MediaUtils
{
    Q_NAMESPACE
    /**
     * @brief The unit used for bitrates
     */
    enum BitrateUnit { bps, kbps, Mbps};
    Q_ENUM_NS(BitrateUnit)

    /**
     * @brief The unit used for sizes
     */
    enum SizeUnit { Bytes, kB, MB, GB};
    Q_ENUM_NS(SizeUnit)

    /**
     * @brief The Status enum Used to describe the current status of the renderer
     */
    enum RenderStatus { Initializing, Waiting, Launching, Encoding, FramesConversion, FFmpegEncoding, AERendering, BlenderRendering, Cleaning, Finished, Stopped, Error, Other };
    Q_ENUM_NS(RenderStatus)

    /**
     * @brief isEncoding Checks if the current status is a busy (encoding or pre/post steps for encoding) status.
     * @param status
     * @return
     */
    bool isBusy(MediaUtils::RenderStatus status);

    /**
     * @brief statusString Converts the status as a human readable string to be used in the UI
     * @param status
     * @return
     */
    QString statusString(MediaUtils::RenderStatus status);

    /**
     * @brief sizeString converts a size in bytes to a human-readable string
     * @param size
     * @return
     */
    QString sizeString(qint64 size);

    QString bitrateString(qint64 bitrate);

    /**
     * @brief convertBitrate Converts a bitrate from bps to another unit.
     * @param to
     * @return
     */
    double convertFromBps( qint64 value, BitrateUnit to );

    /**
     * @brief convertBitrate Converts a file size from Bytes to another unit.
     * @param to
     * @return
     */
    double convertFromBytes( qint64 value, SizeUnit to );

    /**
     * @brief convertBitrate Converts a bitrate to bps from another unit.
     * @param to
     * @return
     */
    qint64 convertToBps( qint64 value, BitrateUnit from );

    /**
     * @brief convertBitrate Converts a file size to Bytes from another unit.
     * @param to
     * @return
     */
    qint64 convertToBytes( qint64 value, SizeUnit from );
};

namespace LogUtils
{
    Q_NAMESPACE

    /**
     * @brief The LogType enum Log level for printing the debug log
     */
    enum LogType { Debug, Information, Warning, Critical, Fatal };
    Q_ENUM_NS(LogType)
};

namespace RegExUtils {
    QRegularExpression getRegEx( QString name );
};

namespace LanguageUtils
{
    QString get(QString id, LanguageList::LanguageIDType from = LanguageList::ISO639_2, LanguageList::LanguageIDType to = LanguageList::NATIVE_NAME);
    static const LanguageList *languageList = new LanguageList();
};

namespace Interpolations {
    double linear( double val, double fromMin, double fromMax, double toMin = 0, double toMax = 100);
}

#endif // UTILS_H

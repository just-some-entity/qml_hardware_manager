#pragma once

#include <qobject.h>
#include <qfile.h>
#include <qurl.h>
#include <qdir.h>
#include <qfilesystemwatcher.h>
#include <qqmlintegration.h>
#include <qiodevice.h>
#include <qtimer.h>

class Brightness : public QObject
{
    Q_OBJECT;

    Q_PROPERTY(qreal backlightAbsolute READ brightnessAbsolute WRITE brightnessAbsolute NOTIFY brightnessChanged)
    Q_PROPERTY(qreal backlightPercent READ brightnessPercent WRITE brightnessPercent NOTIFY brightnessChanged)
    Q_PROPERTY(qreal backlightMax READ brightnessMax)

    Q_PROPERTY(qreal ledAbsolute READ brightnessLedAbsolute WRITE brightnessLedAbsolute NOTIFY brightnessLedChanged)
    Q_PROPERTY(qreal ledPercent READ brightnessLedPercent WRITE brightnessLedPercent NOTIFY brightnessLedChanged)
    Q_PROPERTY(qreal ledMax READ brightnessLedMax)

    QML_SINGLETON
    QML_NAMED_ELEMENT(BrightnessController);

public:
    explicit Brightness(QObject* parent = nullptr);

    [[nodiscard]] qreal brightnessAbsolute();
    void brightnessAbsolute(qreal value);

    [[nodiscard]] qreal brightnessPercent();
    void brightnessPercent(qreal value);

    [[nodiscard]] qreal brightnessMax();

    [[nodiscard]] qreal brightnessLedAbsolute();
    void brightnessLedAbsolute(qreal value);

    [[nodiscard]] qreal brightnessLedPercent();
    void brightnessLedPercent(qreal value);

    [[nodiscard]] qreal brightnessLedMax();

signals:
    void brightnessChanged();
    void brightnessLedChanged();

private:
    struct Entry
    {
        int current;
        int max;

        QString device;

        QString pathCurrent;
        QString pathMax;
    };

    static QMap<QString, Entry> parseDir(const QDir& dir);

    QFileSystemWatcher _watcher;
    QMap<QString, Entry> _backlights;
    QMap<QString, Entry> _leds;

    QMap<QString, QPair<int, QTimer*>> _pendingWrites;

    void setBrightness(const QString& clazz, const QString& device, int value);
};
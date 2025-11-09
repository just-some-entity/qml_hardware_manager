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

    void setBrightness(QString clazz, QString device, const int value)
    {
        constexpr int delay = 50;

        const QString path = QString("/sys/class/%1/%2/brightness").arg(clazz, device);

        if (_pendingWrites.contains(path))
            _pendingWrites[path].second->stop();
        else
        {
            auto timer = new QTimer(this);
            timer->setSingleShot(true);
            connect(timer, &QTimer::timeout, this, [this, path]
            {
                if (_pendingWrites.contains(path))
                {
                    const int val = _pendingWrites[path].first;

                    if (QFile file(path); file.open(QIODevice::WriteOnly))
                    {
                        file.write(QByteArray::number(val));
                        file.flush();
                        file.close();
                    }

                    _pendingWrites[path].second->deleteLater();
                    _pendingWrites.remove(path);
                }
            });

            _pendingWrites[path] = qMakePair(value, timer);
        }

        _pendingWrites[path].first = value;
        _pendingWrites[path].second->start(delay);
    }
};
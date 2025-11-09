#include "brightness.h"

#include <iostream>
#include <qregularexpression.h>

Brightness::Brightness(QObject* parent)
: QObject(parent)
, _backlights(parseDir(QDir("/sys/class/backlight")))
, _leds(parseDir(QDir("/sys/class/leds")))
{
    for (const auto& entry : _backlights)
        _watcher.addPath(entry.pathCurrent);
    for (const auto& entry : _leds)
        _watcher.addPath(entry.pathCurrent);

    connect(&_watcher, &QFileSystemWatcher::fileChanged, this, [this](const QString& path)
    {
        if (QFile f(path); f.open(QIODevice::ReadOnly))
        {
            const int newVal = f.readAll().trimmed().toInt();
            f.close();

            const QRegularExpression re("^/sys/class/([^/]+)/([^/]+)/brightness$");
            if (const QRegularExpressionMatch match = re.match(path); match.hasMatch())
            {
                const QString className = match.captured(1);  // backlight or leds
                const QString deviceName = match.captured(2); // intel_backlight, etc.

                if (className == "backlight")
                    _backlights[deviceName].current = newVal;
                else if (className == "leds")
                    _backlights[deviceName].current = newVal;
            }
        }

        /// As a safety measure, many applications save an open
        /// file by writing a new file and then deleting the old one.
        /// So if it's missing add it back
        _watcher.addPath(path);
    });
}

qreal Brightness::brightnessAbsolute()
{
    if (_backlights.isEmpty()) return 0.0;
    return _backlights.first().current;
}

void Brightness::brightnessAbsolute(const qreal value)
{
    if (_backlights.isEmpty())
    {
        qWarning("Failed to set backlight brightness. No devices found");
        return;
    }

    auto& entry = _backlights.first();
    entry.current = value;
    setBrightness("backlight", entry.device, entry.current);
}

qreal Brightness::brightnessPercent()
{
    if (_backlights.isEmpty()) return 0;
    return _backlights.first().current / _backlights.first().max;
}

void Brightness::brightnessPercent(const qreal value)
{
    if (_backlights.isEmpty())
    {
        qWarning("Failed to set backlight brightness. No devices found");
        return;
    }

    auto& entry = _backlights.first();
    entry.current = (value / 100) * entry.max;
    setBrightness("backlight", entry.device, entry.current);
}

qreal Brightness::brightnessMax()
{
    if (_backlights.isEmpty()) return 0.0;
    return _backlights.first().max;
}

qreal Brightness::brightnessLedAbsolute()
{
    if (_leds.isEmpty()) return 0.0;
    return _leds.first().current;
}

void Brightness::brightnessLedAbsolute(const qreal value)
{
    if (_leds.isEmpty())
    {
        qWarning("Failed to set led brightness. No devices found");
        return;
    }

    auto& entry = _leds.first();
    entry.current = value;
    setBrightness("backlight", entry.device, value);
}

qreal Brightness::brightnessLedPercent()
{
    if (_leds.isEmpty()) return 0;
    return _leds.first().current / _leds.first().max;
}

void Brightness::brightnessLedPercent(const qreal value)
{
    if (_leds.isEmpty())
    {
        qWarning("Failed to set led brightness. No devices found");
        return;
    }

    auto& entry = _leds.first();
    entry.current = (value / 100) * entry.max;
    setBrightness("backlight", entry.device, entry.current / entry.max);
}

qreal Brightness::brightnessLedMax()
{
    if (_leds.isEmpty()) return 0.0;
    return _leds.first().max;
}

QMap<QString, Brightness::Entry> Brightness::parseDir(const QDir& dir)
{
    QMap<QString, Entry> controllers;

    for (const QFileInfo& fileInfo : dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        QString pathB = fileInfo.absoluteFilePath() + "/brightness";
        QString pathM = fileInfo.absoluteFilePath() + "/max_brightness";

        QFile fBrightness(pathB);
        QFile fBrightnessMax(pathM);

        if (!fBrightness.open(QIODevice::ReadOnly))
            continue;

        QByteArray bData = fBrightness.readAll();
        fBrightness.close();

        if (!fBrightnessMax.open(QIODevice::ReadOnly))
            continue;

        QByteArray mData = fBrightnessMax.readAll();
        fBrightnessMax.close();

        Entry entry {
            bData.trimmed().toInt(),
            mData.trimmed().toInt(),
            pathB,
            pathM
        };

        controllers.insert(fileInfo.fileName(), entry);
    }

    return controllers;
}
#include "brightness.h"

#include <iostream>
#include <qregularexpression.h>

static const QRegularExpression regex("^/sys/class/([^/]+)/([^/]+)/brightness$");

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

            if (const QRegularExpressionMatch match = regex.match(path); match.hasMatch())
            {
                const QString className = match.captured(1);  // backlight or leds
                const QString deviceName = match.captured(2); // intel_backlight, etc.

                if (className == "backlight")
                    _backlights[deviceName].current = newVal;
                else if (className == "leds")
                    _leds[deviceName].current = newVal;
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
    entry.current = static_cast<int>(value);
    setBrightness("backlight", entry.device, entry.current);
}

qreal Brightness::brightnessPercent()
{
    if (_backlights.isEmpty()) return 0;
    return static_cast<qreal>(_backlights.first().current) / _backlights.first().max;
}

void Brightness::brightnessPercent(const qreal value)
{
    if (_backlights.isEmpty())
    {
        qWarning("Failed to set backlight brightness. No devices found");
        return;
    }

    auto& entry = _backlights.first();
    entry.current = static_cast<int>((value / 100.0) * entry.max);
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
    entry.current = static_cast<int>(value);
    setBrightness("backlight", entry.device, entry.current);
}

qreal Brightness::brightnessLedPercent()
{
    if (_leds.isEmpty()) return 0;
    return static_cast<qreal>(_leds.first().current) / _leds.first().max;
}

void Brightness::brightnessLedPercent(const qreal value)
{
    if (_leds.isEmpty())
    {
        qWarning("Failed to set led brightness. No devices found");
        return;
    }

    auto& entry = _leds.first();
    entry.current = static_cast<int>((value / 100) * entry.max);
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

void Brightness::setBrightness(const QString& clazz, const QString& device, const int value)
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
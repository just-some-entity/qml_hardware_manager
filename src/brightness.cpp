#include "brightness.h"

#include <qregularexpression.h>
#include <QDateTime>

#ifdef ENABLE_LOGIND
# if defined(HAVE_LIBSYSTEMD)
#  include <systemd/sd-bus.h>
# elif defined(HAVE_LIBELOGIND)
#  include <elogind/sd-bus.h>
# elif defined(HAVE_BASU)
#  include <basu/sd-bus.h>
# else
#  error "No dbus provider found"
# endif
#endif

static const QRegularExpression regex("^/sys/class/([^/]+)/([^/]+)/brightness$");

static constexpr auto basePath       = "/sys/class/";
static constexpr auto backlightClass = "backlight";
static constexpr auto ledClass       = "leds";

BrightnessEntry::BrightnessEntry(
    QObject* parent,
    const Class clazz,
    const Id_t& id,
    const int current,
    const int max,
    const QString& path_current)
: QObject(parent)
, _class(clazz)
, _id(id)
, _current(current)
, _max(max)
, _pathCurrent(path_current)
{}

int BrightnessEntry::current() const
{
    return _current;
}

void BrightnessEntry::current(const int value)
{
    _current = value;
    requestSync();
}

qreal BrightnessEntry::currentNormalized() const
{
    return _current / _max;
}

void BrightnessEntry::currentNormalized(const qreal value)
{
    _current = static_cast<int>((std::min(value, 1.0) * _max) / _max);
    requestSync();
}

int BrightnessEntry::max() const
{
    return _max;
}

BrightnessEntry::Class BrightnessEntry::clazz() const
{
    return _class;
}

const QString& BrightnessEntry::id() const
{
    return _id;
}

const QString& BrightnessEntry::pathCurrent() const
{
    return _pathCurrent;
}

QString BrightnessEntry::classAsString(const Class clazz)
{
    return clazz == Class::Backlight ? backlightClass : ledClass;
}

void BrightnessEntry::requestSync()
{
    const auto now = QDateTime::currentMSecsSinceEpoch();
    auto& [timeoutUntil, queuedValue, timer] = _sync;

    if (timer || now > timeoutUntil) // Can write now
    {
        writeChanges();
        timeoutUntil = now + _updateDelay;
    }

    else // Not ready yet
    {
        // Timer already running, just update the value it will apply
        if (timer)
        {
            queuedValue = _current;
            return;
        }

        timer = new QTimer(this);
        timer->setSingleShot(true);

        connect(timer, &QTimer::timeout, this, [this]
        {
            _current = _sync.queuedValue;
            writeChanges();
            _sync.timer->deleteLater();
        });
    }
}

void BrightnessEntry::writeChanges()
{
#ifdef ENABLE_LOGIND
    sd_bus* bus = nullptr;
    int r = sd_bus_default_system(&bus);
    if (r < 0)
        qWarning() << "Can't connect to system bus:" << strerror(-r);

    r = sd_bus_call_method(bus,
                           "org.freedesktop.login1",
                           "/org/freedesktop/login1/session/auto",
                           "org.freedesktop.login1.Session",
                           "SetBrightness",
                           nullptr,
                           nullptr,
                           "ssu",
                           classAsString(_class).toUtf8().constData(),
                           _id.toUtf8().constData(),
                           _current);

    if (r < 0)
        qWarning() << "Failed to set brightness: " << strerror(-r);

    sd_bus_unref(bus);
#else
    if (QFile file(_pathCurrent); file.open(QIODevice::WriteOnly))
    {
        file.write(QByteArray::number(_current));
        file.flush();
        file.close();
    }

    else
        qWarning() << "Failed to set brightness: " << file.errorString();
#endif
}


Brightness::Brightness(QObject* parent)
: QObject(parent)
, _backlights(parseClass(this, BrightnessEntry::Class::Backlight))
, _leds(parseClass(this, BrightnessEntry::Class::Led))
{
    for (const auto& entry : _backlights)
        _watcher.addPath(entry->pathCurrent());
    for (const auto& entry : _leds)
        _watcher.addPath(entry->pathCurrent());

    connect(&_watcher, &QFileSystemWatcher::fileChanged, this, [this](const QString& path)
    {
        if (QFile f(path); f.open(QIODevice::ReadOnly))
        {
            const int newVal = f.readAll().trimmed().toInt();
            f.close();

            if (const QRegularExpressionMatch match = regex.match(path); match.hasMatch())
            {
                const QString className  = match.captured(1);  // backlight or leds
                const QString id         = match.captured(2);  // intel_backlight, etc.

                if (className != backlightClass && className != ledClass) return;

                QList<BrightnessEntry*> entries = className == backlightClass ? _backlights : _leds;
                for (const auto& backlight : entries)
                    if (backlight->id() == id)
                        backlight->_current = newVal;
            }
        }

        /// As a safety measure, many applications save an open
        /// file by writing a new file and then deleting the old one.
        /// So if it's missing add it back
        _watcher.addPath(path);
    });
}

int Brightness::updateDelay() const
{
    return _updateDelay;
}

void Brightness::updateDelay(const int value)
{
    _updateDelay = value;

    for (const auto backlight : _backlights)
        backlight->_updateDelay = value;

    for (const auto backlight : _leds)
        backlight->_updateDelay = value;
}

// Backlight defaults
int Brightness::backlight() const
{
    if (_backlights.empty())
    {
        qWarning() << "No Backlights found on system";
        return 0;
    }

    return _backlights.first()->current();
}

void Brightness::backlight(const int value)
{
    if (_backlights.empty())
    {
        qWarning() << "No Backlights found on system";
        return;
    }

    _backlights.first()->current(value);
}

qreal Brightness::backlightNormalized() const
{
    if (_backlights.empty())
    {
        qWarning() << "No Backlights found on system";
        return 0;
    }

    return _backlights.first()->currentNormalized();
}

void Brightness::backlightNormalized(const qreal value)
{
    if (_backlights.empty())
    {
        qWarning() << "No Backlights found on system";
        return;
    }

    return _backlights.first()->currentNormalized(value);
}

int Brightness::backlightMax() const
{
    if (_backlights.empty())
    {
        qWarning() << "No Backlights found on system";
        return 0;
    }

    return _backlights.first()->max();
}

QList<BrightnessEntry*> Brightness::backlights()
{
    return _backlights;
}

QList<BrightnessEntry*> Brightness::leds()
{
    return _leds;
}

QList<BrightnessEntry*> Brightness::parseClass(Brightness* thiz, const BrightnessEntry::Class clazz)
{
    QList<BrightnessEntry*> controllers;

    const QString classStr = BrightnessEntry::classAsString(clazz);

    const QDir dir(basePath + classStr);

    for (const QFileInfo& fileInfo : dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        QString pathB = fileInfo.absoluteFilePath() + "/brightness";
        QString pathM = fileInfo.absoluteFilePath() + "/max_brightness";

        QFile fBrightness(pathB);
        QFile fBrightnessMax(pathM);

        if (!fBrightness.open(QIODevice::ReadOnly))
            continue;

        QByteArray currentData = fBrightness.readAll();
        fBrightness.close();

        if (!fBrightnessMax.open(QIODevice::ReadOnly))
            continue;

        QByteArray maxData = fBrightnessMax.readAll();
        fBrightnessMax.close();

        const auto id = fileInfo.fileName();

        // Qt Manages lifetime so *should* not be a memory leak
        // ReSharper disable once CppDFAMemoryLeak
        const auto entry = new BrightnessEntry(
            thiz,
            clazz,
            id,
            currentData.trimmed().toInt(),
            maxData.trimmed().toInt(),
        pathB);

        controllers.push_back(entry);
    }

    return controllers;
}
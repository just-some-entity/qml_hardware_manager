#pragma once

#include <QVariant>
#include <qstring.h>
#include <qtypes.h>

struct Data_Cpu
{
    struct Stats
    {
        quint64 user       = 0; // time spent in user mode
        quint64 nice       = 0; // time spent in user mode with low priority
        quint64 system     = 0; // time spent in kernel mode
        quint64 idle       = 0; // idle time
        quint64 iowait     = 0; // time waiting for I/O
        quint64 irq        = 0; // time servicing hardware interrupts
        quint64 softirq    = 0; // time servicing software interrupts
        quint64 steal      = 0; // time stolen by hypervisor
        quint64 guest      = 0; // running a virtual CPU
        quint64 guest_nice = 0; // guest time with low priority

        [[nodiscard]] quint64 total() const
        {
            return nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice;
        }

        Stats& operator+=(const Stats& other)
        {
            user       += other.user;
            nice       += other.nice;
            system     += other.system;
            idle       += other.idle;
            iowait     += other.iowait;
            irq        += other.irq;
            softirq    += other.softirq;
            steal      += other.steal;
            guest      += other.guest;
            guest_nice += other.guest_nice;
            return *this;
        }
    };

    struct EntryBase
    {
        float freqMin = 0.0;
        float freqMax = 0.0;
        float freqNow = 0.0;
        float temp    = 0.0;
    };

    struct Entry : EntryBase
    {
        Stats stats;
    };

    struct CoreData : Entry
    {
        QVariantMap cpuInfoEntries;
    };

    struct CpuData : Entry
    {
        QString name = nullptr;
        float   draw = 0.0;

        QVector<CoreData> cores;
    };

    struct StatsGlobal
    {
        Stats totalCpuStats;

        QVector<quint64> interrupts;

        quint64 contextSwitches = 0;
        quint64 bootTime        = 0;
        quint64 processes       = 0;
        quint64 procsRunning    = 0;
        quint64 procsBlocked    = 0;

        QVector<quint64> softIrqs;
    };

    float load1  = 0; // 1-minute load average
    float load5  = 0; // 5-minute load average
    float load15 = 0; // 15-minute load average

    StatsGlobal globalStats;

    QVector<CpuData> cpus;
};
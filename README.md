# QML Hardware Controls

A Qt/QML module to monitor and control hardware on Linux.

## Current Features

- Detects available **backlight** and **LED devices** under `/sys/class/backlight` and `/sys/class/leds`, and allows writing to them.
- Monitors and exposes **CPU statistics** by reading from `/proc/stat`, `/proc/cpuinfo` ,`/sys/devices/system/cpu/` and `/proc/loadavg`,

## Planned Features

- Memory Stats
- Temperature Monitoring for Cpus
- System Uptime and Load Average
- Network Information (Wi-Fi, Ethernet, Signal Strength)
- Battery and Power Management
- Storage Usage and Disk I/O
- GPU Information and Load
- Fan Speed and Thermal Sensors (Depending on difficulty)
- Audio Devices and Volume Control (Maybe)
- Display Configuration and Brightness Profiles (Maybe)

## Currently Available Interfaces

### BrightnessController
| Property              | Type                      | Access      | Description                                 |
|-----------------------|---------------------------|-------------|---------------------------------------------|
| `updateDelay`         | `int`                     | Read/Write  | Delay between brightness updates (ms).      |
| `backlight`           | `qreal`                   | Read/Write  | Current backlight value.                    |
| `backlightNormalized` | `qreal`                   | Read/Write  | Backlight value normalized between 0 and 1. |
| `backlightMax`        | `qreal`                   | Read-only   | Maximum backlight value.                    |
| `backlights`          | `QList<BrightnessEntry*>` | Read-only   | List of available backlight devices.        |
| `leds`                | `QList<BrightnessEntry*>` | Read-only   | List of available LED devices.              |


### BrightnessEntry

| Property             | Type        | Access      | Description                                            |
|----------------------|-------------|-------------|--------------------------------------------------------|
| `current`            | `qreal`     | Read/Write  | Current brightness value.                              |
| `currentNormalized`  | `qreal`     | Read/Write  | Brightness value normalized between 0 and 1.           |
| `max`                | `qreal`     | Read-only   | Maximum brightness value supported by the device.      |
| `id`                 | `QString`   | Read-only   | Unique identifier of the device.                       |


### CpuDataSampler (For simple monitoring tools)

| Property       | Type     | Access     | Description                                                           |
|----------------|----------|------------|-----------------------------------------------------------------------|
| `name`         | `string` | Read-only  | Name of the CPU (e.g., `"Intel(R) Core(TM) i7-9700K CPU @ 3.60GHz"`). |
| `load1`        | `qreal`  | Read-only  | System load average over 1 minute.                                    |
| `load5`        | `qreal`  | Read-only  | System load average over 5 minutes.                                   |
| `load15`       | `qreal`  | Read-only  | System load average over 15 minutes.                                  |
| `frequencyMin` | `qreal`  | Read-only  | First CPU's minimum frequency.                                        |
| `frequencyMax` | `qreal`  | Read-only  | First CPU's maximum frequency.                                        |
| `frequency`    | `qreal`  | Read-only  | Current CPU frequency.                                                |
| `temperature`  | `qreal`  | Read-only  | Current CPU temperature.                                              |
| `utilization`  | `qreal`  | Read-only  | CPU utilization ratio (0–1).                                          |
| `powerDraw`    | `qreal`  | Read-only  | Estimated CPU power draw in watts.                                    |
| `maxSamples`   | `int`    | Read/Write | Max number of data samples to collect                                 |

### CpuDataSnapshotModel Properties

| Role          | Type      | Description                      |
|---------------|-----------|----------------------------------|
| `Temperature` | `qreal`   | Snapshot temperature value.      |
| `Frequency`   | `qreal`   | Snapshot frequency value.        |
| `Utilization` | `qreal`   | Snapshot CPU utilization ratio.  |
| `PowerDraw`   | `qreal`   | Snapshot estimated power draw.   |

## Example Usage

```qml
import HardwareControls
 
 ...
CpuDataSampler
{
    id: cpuData
}

Text
{
    text: "cpu-name: " + cpuData.name + " : " + cpuData.utilization
}

Text {
    text: "cpu utilization:" + BrightnessController.backlight
}

Component.onCompleted: {

    console.log("backlights");
    console.log("----------");
    for (const backlight of BrightnessController.backlights)
    {
        console.log(led.id);
        console.log(led.current);
        console.log(led.max);
        console.log("");
    }

    console.log("leds");
    console.log("----------");
    for (const led of BrightnessController.leds)
    {
        console.log(led.id);
        console.log(led.current);
        console.log(led.max);
        console.log("");

        led.current = 10;
    }
}
```

## Installation

Clone the repository and run:
```
./install.sh
```

This will build and install the QML module for use in your projects globally at `/usr/lib/qt6/qml/HardwareControls`


## Uninstalling
run `./uninstall.sh` or simply delete the `/usr/lib/qt6/qml/HardwareControls` directory.
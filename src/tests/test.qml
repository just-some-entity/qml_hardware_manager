import QtQuick
import QtQuick.Layouts

import HardwareManager

Window
{
    id: win
    width:  400
    height: 400

    visible: true

    CpuDataSampler
    {
        id: cpuData

        onFrequencyChanged: {
            for (let key in cpuData) {
                if (cpuData.hasOwnProperty(key)) {
                    console.log(key + ": " + cpuData[key]);
                }
            }

            console.log("--------------------------------------------");
        }
    }

    Text
    {
        text: "cpu-name: " + cpuData.name + " : " + cpuData.utilization
    }
}
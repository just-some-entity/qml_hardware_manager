import QtQuick

import HardwareControls

Window {
    id: window

    visible: true

    Component.onCompleted: {

        console.log("Hello world", BrightnessController.backlightAbsolute, BrightnessController.backlightPercent);
    }
}
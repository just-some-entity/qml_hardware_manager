#!/bin/bash
set -e  # Exit immediately if a command exits with a non-zero status

if [ -d /usr/lib/qt6/qml/HardwareControls ]; then
    echo "Removing HardwareControls QML module..."
    sudo rm -rf /usr/lib/qt6/qml/HardwareControls
    echo "Uninstallation complete."
else
    echo "HardwareControls module not found at /usr/lib/qt6/qml/HardwareControls"
fi
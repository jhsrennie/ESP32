@echo off
netsh interface set interface name=WiFi admin=disabled
netsh interface set interface name=WiFi admin=enabled
netsh wlan show networks
netsh wlan connect ssid=ESPWave name=ESPWave
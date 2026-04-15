# WiFiCounters

The ESP32 has a few undocumented Wi-Fi counters. This sketch shows how to use them. They are probably not that useful, and in any case using an undocumented feature in a production app is probably unwise, but the idly curious may find this interesting.

### g_hmac_cnt

There is a global structure called `g_hmac_cnt` that contains various counters. You simply need to define the structure then declare the variable as:

```
extern hmac_cnt_t g_hmac_cnt;
```

### rom_check_noise_floor

The Wi-Fi signal strength can be read using WiFi.RSSI() or the ESP-IDF equivalent, but this only tells you the signaal strength not the signal to noise ratio. To get the noise level there is a function:

```
int rom_check_noise_floor(void);
```

This returms a value in 1/4 dB so divide by 4 to get the noise in dB. The signal to noise is then:

```
int snr = WiFi.RSSI()) - rom_check_noise_floor()/4;
```

I saw this on Reddit [here](https://www.reddit.com/r/esp32/comments/1skxwxz/esp32_esp32s3_wifi_counters/) and [here](https://www.reddit.com/r/esp32/comments/1skxm1v/esp32s3_link_quality_estimation_undocumented_api/).
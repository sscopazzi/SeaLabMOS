# SeaLabMOS
Modular oceanographic systems using consumer-off-the-shelf parts with no prior experience and basic tools.

 This work will be presented at MTS Great Lakes in 2025. If curious please check out [my website's page](https://www.sophiescopazzi.com/projects/sealabmos) about it for some other words.

## Current Supported System Types

1. **Traditional CTD**: Salinity, temperature, depth sensors with three variants: 1.5in and 2.0in PVC (~140m) or 2.0in steel pipe (~350m)  
2. **Temperature logger**: Temperature only monitoring for a ~few days to ~two weeks, based on system configuration
3. **Bottom Pressure Recorder**: Pressure only for tide measuring, research ongoing for three used in conjuction for wave height and direction
4. **Surface Float**: All sensors optional. Salinity, water temperature, GNSS/location, LED beacon, atmospheric pressure/temperature, light, etc.

### Base Datalogger Hardware

| **Name** | **Adafruit Part** # | **What it does** | **Accuracy** | **Cost** |
|------|------------------|---------------|----------|------|
| [Adafruit Feather RP2040 Adalogger w/ microSD Card](https://www.adafruit.com/product/5980) | 5980 | Datalogger | — | \$15 |
| [SD/MicroSD Memory Card (512 MB)](https://www.adafruit.com/product/5252) | 5252 | Data storage | — | \$5 |
| [DS3231 Precision RTC FeatherWing](https://www.adafruit.com/product/3028) | 3028 | Real time clock | ±2 minutes per year | \$14 |
| [CR1220 12mm Diameter 3 V Lithium Coin Cell Battery](https://www.adafruit.com/product/380) | 380 | Battery for RTC | — | \$1 |
| [Lithium Ion Battery Pack 3.7 V 2.2 Ah (or 2.5–10 Ah)](https://www.adafruit.com/product/1781) | 1781 (2.2 Ah) | Battery | — | \$10 |
|      |      |      |      |      |
|      |      |      |      |      |
| **Display**     |      |      |      |      |
| [Adafruit FeatherWing 128×64 OLED Display (optional)](https://www.adafruit.com/product/4650) | 4650 | Screen, buttons | — | \$15 |
|      |      |      |      |      |
|      |      |      |      |      |
| **Location**      |      |      |      |      |
| [Mini GPS (PA1010D)](https://www.adafruit.com/product/4415) | 4415 | GPS/GNSS, Time | Testing 10 Hz | \$30 |
| [Ultimate GPS Breakout (PA1616D)](https://www.adafruit.com/product/5440) | 5440 | GPS/GNSS, Time | Testing, 10 Hz | \$30 |
| [External Active GPS Antenna](https://www.adafruit.com/product/960) | 960 | For Ultimate | Testing | \$21.5 |
| [High-strength 'rare earth' magnet](https://www.adafruit.com/product/9) | 9 | Hold antenna | — | \$2.5 |
|      |      |      |      |      |
|      |      |      |      |      |
| **Sensor Cables**      |      |      |      |      |
| [STEMMA QT / Qwiic JST SH 4-pin Cable](https://www.adafruit.com/product/5385) | 5385 | Sensor connection | — | \$1–1.5 |
| [STEMMA QT / Qwiic 5 Port Hub](https://www.adafruit.com/product/5625) | 5625 | Sensor connection | — | \$2.5 |

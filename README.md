This work will be presented at MTS Great Lakes in 2025. If curious, I can send you the paper in prep, or check out [my website's page](https://www.sophiescopazzi.com/projects/sealabmos) for some pictures.

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

### Static Sensors  

| **Type** | Part # | Variable | Depth Rating (m) | Response Time | Accuracy | Cost |
|---|---|---|---|---|---|---|
| [Atlas Scientific Mini Conductivity K 1.0 Kit](https://atlas-scientific.com/kits/mini-conductivity-k-1-0-kit/) | — | Conductivity | 353 | 90% in 1 s | 99% in 1 s | \$200 |
| [Waterproof 1-Wire DS18B20](https://www.adafruit.com/product/381) | 381 | Temperature | Not specified | 60% in 12 s, 90% in 31 s | ±0.5 °C | \$15 |
| [DS18B20 Extender](https://www.adafruit.com/product/5971) | 5971 | DS18B20 accessory board | — | — | — | \$— |
| [10K Precision Epoxy Thermistor](https://www.adafruit.com/product/372) | 372 | Temperature | Not specified | 60% in 3 s, 90% in 7 s | ±0.1 – 0.5 °C | \$4 |
| [Platinum RTD Sensor – PT100](https://www.adafruit.com/product/3290) | 3290 | Temperature | Not specified | — | — | \$— |
| [PT100 RTD Amplifier – MAX31865](https://www.adafruit.com/product/3328) | 3328 | Amplifier for PT100 | — | — | — | \$— |

> *Note:* For reduced costs, static deployments may not need a pressure sensor.  
> Depth rating for temperature sensors isn’t specified by manufacturers and depends on sealant/epoxy.

### Dynamic Sensors  

| **Type** | Part # | Variable | Depth Rating (m) | Response Time | Accuracy | Cost |
|---|---|---|---|---|---|---|
| [Atlas Scientific K 1.0 Kit](https://atlas-scientific.com/kits/mini-conductivity-k-1-0-kit/) | — | Conductivity | 353 | 90% in 1 s | 90% in 1 s | \$200 |
| [Blue Robotics Celsius Fast-Response (TSYS01)](https://bluerobotics.com/store/sensors-cameras/sensors/celsius-sensor-r1/) | — | Temperature | 975 | 3 s (90% in 2 s, measured) | ±0.1 °C | \$70 |
| [Blue Robotics Bar02 / Bar30 / Bar100 (pressure options)](https://bluerobotics.com/product-category/sensors-cameras/sensors/) | — | Pressure | 10m, 300, 1,000m | — | — | \$80, \$90, \$380 |

> *Note:* Response time and accuracy are from manufacturer specs and initial testing.

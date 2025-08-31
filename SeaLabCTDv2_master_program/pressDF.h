#pragma once
/*!
 * @file  SEN0257.ino
 * @brief  Water pressure sensor demo (Computer serial port)
 * @n      - Obtain the water pressure through the output voltage
 * @n        of the sensor.
 * @copyright  Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license  The MIT License (MIT)
 * @author  DFRobot
 * @version  V1.0
 * @date  2023-07-06
 */

/************************************************************
  Water Sensor Key Parameter
  - Parts No.:KY-3-5
  - Sensing range: 0 - 1 MPa
  - Input Voltage: 5VDC
  - Output Voltage: 0.5 - 4.5 VDC
    (Linearly corresponding to 0 - 1 MPa)
  - Accuracy: 0.5% - 1% FS
**************************************************************/

/************************************************************
  Water Sensor Calibration

  The output voltage offset of the sensor is 0.5V (nominal).
  However, due to the zero-drifting of the internal circuit, the
  no-load output voltage is not exactly 0.5V. Calibration needs to
  be carried out as follows.

  Calibration: connect the 3-pin wire to the Arduino UNO (VCC, GND, and Signal)
  without connecting the sensor to the water pipe and run the program
  once. Mark down the LOWEST voltage value through the serial
  monitor and revise the "OffSet" value to complete the calibration.

  After the calibration, the sensor is ready for measuring!
**************************************************************/

const float OffSet = 0.483;

float pressDF_V, pressDF_P;

inline void getPressureDF()
{
  // Connect sensor to Analog 0
  pressDF_V = analogRead(0) * 5.00 / 1024; // Sensor output voltage
  pressDF_P = (pressDF_V - OffSet) * 250;          // Calculate water pressure in KPa

  // Convert pressure to ocean depth
  pressDF_m = pressDF_P / 9.81;           // Depth in meters (1 KPa ≈ 0.10197 meters of seawater)
  pressDF_m = pressDF_m * 3.28084;  // Convert meters to feet
  pressDF_psi = pressDF_P * 0.145038;     // 1 KPa ≈ 0.145038 psi

}



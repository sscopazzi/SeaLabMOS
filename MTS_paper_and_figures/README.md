# SeaLabCTD MTS Paper Deployments and Figures

This folder contains plots, results, and analysis scripts from the series SeaLabCTD tests conducted April to July 2025 conducted immitely offshore the Valdez Marine Termina, in Port Valdez, Prince William Sound, Alaska. Each folder represents a different deployment.

---

### Tools Used
- Python (pandas, matplotlib, numpy, scipy, etc.)
- GSW (Gibbs SeaWater) Oceanographic Toolbox
- Arduino + Feather RP2040 Logger Firmware (see manual)
- Atlas Scientific + Blue Robotics sensors
---

### Notes
- All data is timestamped ISO 8601 format with "-" substituted for ":" for filename restrictions.
- `.csv` files in each directory contain "cleaned" data (only the relevent downcast). Sometimes there are two downcasts.
- The figures below are in the MTS paper.
---

### Contact

For questions, please ask Sophie Scopazzi. Contact information on [my website](https://www.sophiescopazzi.com/)

---

### Figures Overview

All figures used in MTS paper. All figures are located in their respective test folders. Alt text is provided for accessibility.

### Figures Overview

All figures used in the MTS paper. All figures are located in their respective test folders. Alt text is provided for accessibility.

### Figures Overview

All figures used in the MTS paper. All figures are located in their respective test folders. Alt text is provided for accessibility.

| Figure | Description |
|--------|-------------|
| **Hand Deployment**<br><img src="handDeploymentMethod.jpg" alt="Hand deployment with SeaLabCTD, shackle and bucket for rope" width="40%"> | Method used to lower the CTD. A shackle is used as weight; a bucket prevents line tangling. |
| **Battery Endurance**<br><img src="10testEndurance/testEndurance_battV_real_and_estimate.jpg" alt="Battery test results showing endurance vs clock speed" width="40%"> | Runtime comparison at 200 MHz vs 50 MHz. Estimation of endurance for different Adafruit batteries. |
| **Lowering Speed Comparison**<br><img src="loweringSpeed/loweringSpeed_temp.jpg" alt="Deployment speed effect on data resolution" width="40%"> | Slow (6.4 cm/s) vs fast (52 cm/s) lowering shows how descent rate affects data fidelity. |
| **Salinity Over Time**<br><img src="multiVMT/allTests_salt.jpg" alt="Salinity profiles from April to July, inset highlights surface layer" width="40%"> | Progressive surface freshening and salinity increase at depth across seasons. |
| **Temperature Over Time**<br><img src="multiVMT/allTests_temp.jpg" alt="Temperature profiles from April to July, warmer surface over time" width="40%"> | Strong surface warming trend towards summer. Slower casts resolve gradients better. |
| **Temperature–Salinity (T/S) Diagram**<br><img src="multiVMT/TS_combined_Test4_Test9.jpg" alt="T-S plot of two profiles, showing stratification increase" width="40%"> | July water is warmer and saltier. Broader curve shows stronger stratification. |
| **Stratification Summary**<br><img src="multiVMT/allTests_stratification.jpg" alt="Bar chart of surface and deep salinity averages over time" width="40%"> | Difference between surface and bottom salinity increases over time. |
| **Sensor Response Times**<br><img src="12tempResponse/testTwelveTempResponse_mainplot_skinny.png" alt="Thermal response test of three sensors" width="40%"> | Response curves from 40 °C then plunge to ice water. Fastest: Blue Robotics Fast Response TSYS01. |
| **Tide Rise Time Series**<br><img src="11durationTsunami/testElevenDuration_waves_.jpg" alt="Pressure depth trace with long-period trend from tide" width="40%"> | Static deployment shows incoming tide trend via pressure change. |
| **Power Spectral Density (PSD)**<br><img src="11durationTsunami/PSD_plot_skinny.png" alt="PSD analysis of detrended depth data showing wave energy" width="40%"> | Frequency-domain plot of wave-related motion captured with pressure sensor. |

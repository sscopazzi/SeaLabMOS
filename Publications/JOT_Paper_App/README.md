# Figures for "Conductivity Accuracy of Two Open Source SeaLabMOS CTDs"

Open `SeaLabMOS-JOT-app-paper.ipynb` and run all cells in order: the master CONFIG cell, the three loader sections, then any figure cell in any order. Figure cells never depend on each other. All paths are relative to the notebook, so this folder can be moved, zipped or cloned anywhere and still run.

To see photos of what the systems look like, [please see my website](https://www.sophiescopazzi.com/projects/sealabmos).

Figures are written to `plots/` as `figNN_*.png` at 600 dpi, numbered to match the paper. The list of required packages is fairly standard (there is an environment.yml if you need it).
  - python=3.12
  - numpy=2.4.4
  - pandas=3.0.2
  - scipy=1.17.1
  - matplotlib=3.10.8
  - xarray=2026.7.0
  - netcdf4=1.7.4 # conda package name is lowercase; imported as netCDF4
  - gsw=3.6.23 # TEOS-10 / PSS-78, provides gsw.SP_from_C

## Layout

Every data file sits directly in `data/`, prefixed by experiment. The two units MIS_1 and MIS_2 did not do the same things: MIS_1 was used on the workbench in the lab and MIS_2 was used for the stepped casts. Both took part in the bucket tests. In all cases, the ground truth was the RBR system.

```
SeaLabMOS-JOT-app-paper/
├── SeaLabMOS-JOT-app-paper.ipynb
├── README.md
├── environment.yml
├── requirements.txt
├── .gitignore
├── data/
│   ├── lab_MIS_1.csv                          lab standards, MIS_1
│   ├── lab_204386_20260703_1802.nc            lab standards, RBR          (+ .rsk)
│   ├── bucketA_MIS_1.csv                      Jul-3 bucket A, MIS_1
│   ├── bucketB_MIS_1.csv                      Jul-3 bucket B, MIS_1
│   ├── bucketA_MIS_2.csv                      Jul-3 bucket A, MIS_2
│   ├── bucketB_MIS_2.csv                      Jul-3 bucket B, MIS_2
│   ├── bucket_204386_20260703_1829.nc         Jul-3 buckets A and B, RBR  (+ .rsk)
│   ├── cast_MIS_2_2026-07-11T21-12-38.csv     Jul-11 stepped casts, MIS_2
│   ├── cast_204386_20260711_1639.nc           Jul-11 stepped casts, RBR   (+ .rsk)
│   └── static_frame/                          bottom-frame deployment, Jun 5-9
│       ├── long_originalCTD_2026-06-07T06-41-00.csv   co-deployment, originalCTD
│       ├── long_204386_20260609_2023_00008.nc         co-deployment, RBR
│       ├── profiles_originalCTD/              16 originalCTD casts
│       │   └── originalCTD_<stamp>.csv
│       └── profiles_rbr/                      17 RBR cast profiles
│           └── rbr_204386_<log>_<nnnnn>.nc
└── plots/                                     figNN_*.png at 600 dpi, tracked in git
```

## Figures

| Fig | Content |
|----|----|
| 1  | Experiment timeline |
| 2  | Lab setup photograph (not a plot, no cell in the notebook) |
| 3  | Temperature 1:1 vs RBR: pooled casts + static co-location |
| 4  | Implied EC gain, both units + MIS_1 pipeline |
| 5  | Depth vs time, both stepped casts |
| 6  | Firmware-reported salinity profiles |
| 7  | 1:1, firmware vs recalculated salinity at hold averages |
| 8  | EC_GAIN fit + two bucket-test insets |
| 9  | Four salinity pipelines vs RBR, MIS_1 workbench |
| 10 | Out-of-sample bucket validation for MIS_1 and MIS_2 |
| 11 | Salinity profiles: RBR, firmware and recalculated |
| 12 | GOFS 3.1 vertical level distribution |

## Corrections to Salinity Data

To sum up results of the paper in one sentence, MIS_1 and MIS_2 need a system-specific correction, applied to electrical conductivity before the PSS-78 call (never as an additive offset on salinity):

- **MIS_1**: quadratic, `EC_corr = 0.86331·C + 0.0149285·C²`, fit on the six lab
  calibration standards through the origin.
- **MIS_2**: scalar, `EC_corr = EC_GAIN_MIS2 · C`. MIS_2 never ran against the
  lab standards, so its gain comes from the thirteen equilibrated hold points of
  the 2026-07-11 stepped casts. It's `EC_GAIN_MIS2` = 0.9908. 

`C` used in is `gsw.SP_from_C` in mS/cm. The [Atlas Scientific salinity sensor (EZO)](https://atlas-scientific.com/kits/mini-conductivity-k-1-0-kit/) logs µS/cm, so every pipeline divides by 1000 beforehand. There is no single `EC_GAIN`. **Each correction is system-specific.**

`EC_GAIN_MIS2` is **fitted at run time**, not assumed. The hold points it is fit on carry whatever depths `DEPTH_ZERO_MODE` produces, so the gain and the depth
zero stay consistent. 

Firmware-reported salinity (`sal_fw`) is not corrected and **is not a data product**. It appears in Figures 6, 7 and 11 only to show why it is discarded. The EZO hard-codes 25 °C in its temperature compensation and applies no pressure term (that I can find information about).

## Data Notes

Every file is read by at least one figure, except the `.rsk` files, which are the RBR system originals kept for provenance. The notebook reads the `.nc` exports Ruskin produced from them.

| File | Used by |
|----|----|
| `lab_MIS_1.csv` | Figures 4, 9, 10 |
| `lab_204386_20260703_1802.nc` | Figures 4, 9, 10 |
| `bucketA_MIS_1.csv` | Figure 10 |
| `bucketB_MIS_1.csv` | Figure 10 |
| `bucketA_MIS_2.csv` | Figures 4, 8 inset, 10 |
| `bucketB_MIS_2.csv` | Figures 4, 8 inset, 10 |
| `bucket_204386_20260703_1829.nc` | Figures 8, 10 |
| `cast_204386_20260711_1639.nc` | Figures 5–8, 11 |
| `cast_MIS_2_2026-07-11T21-12-38.csv` | Figures 5–8, 11 |

Figures 1 and 12 require no data files. Figure 1 is built from hand-written timestamps in the loader CONFIG, Figure 12 from the transcribed GOFS [vertical level list](https://www.hycom.org/dataserver/gofs-3pt1/analysis).

The bottom of every RBR cast is trimmed by 0.1m where it contacted the muddy bottom, effecting the salinity value by sometimes up to 6.7 PSU (`RBR_BOTTOM_TRIM_M = 0.10`).

Figure 3 reads everything under `data/static_frame/`: the 17 `originalCTD_*.csv` casts, the 17 RBR cast profiles they pair against, and the long log. 16 are casts and 15 of those pair one-to-one with a profile; `originalCTD_2026-06-07T06-05-42.csv` has two casts in the same data file and thus is split into `_a` and `_b`. `long_originalCTD_2026-06-07T06-41-00.csv` is the long co-deployment and the loader selects it as `LONG`.

A keen viewer will see the numbers of the RBR casts are not sequential. Left out of `static_frame/profiles_rbr`: two RBR profiles `20260609_2023` numbers `_00002` and `_00020`, plus `20260606_0625_00001` where they do not have a paired cast.

Also missing are the two casts from the long deployment: `RBR_00007` is the descent into it and `RBR_00009` is the recovery. 

Nothing in the matcher looks at depth, so `MAX_DEPTH_MISMATCH_M = 1.0` in the pairing cell checks afterwards: a paired cast and profile should reach nearly
the same depth, and anything further apart is reported by name. It warns rather than drops, since a wide pair may still be legitimate. On the shipped data every pair passes, the worst being 0.38 m.

**For the stepped cast hold depth matching:** 15 holds are detected, but they become the 13 points in the Figure 8 because the RBR binned profile spans 0.22–9.80 m, and two MIS_2 surface holds sit above the RBRs shallowest bin with nothing to match against. Both are hold segments inside `cast_MIS_2_2026-07-11T21-12-38.csv` and are not separate files (two holds very 
close to the surface done with the MIS_2 and not RBR). These holds outside the binned RBR profile are dropped explicitly instead of silently taking an end-bin value.

## Font sizes

`FS_GLOBAL` in the master CONFIG controls every figure for consistency. 
Each figure cell opens with `fonts()`, which resets the six `FS_*` globals 
before drawing, so a size set inside one cell cannot leak into the next regardless 
of order ran. To change one figure only, pass the override to its own call, e.g. `fonts(FS_TICK=14)`, or just hardcode the line (as done in a few places for figure legibility and formatting).

## The SeaLabMOS Clocks and Time Issues

The absolute time from MIS_1 and MIS_2's clocks for the buckets was incorrect. On power-on the system initialised its clock to the firmware compile time, so the time record starts at the time that firmware was uploaded, regardless of true power-on time. This firmware bug has since been fixed. Elapsed time *within* one record is reliable and used for hold detection, transit rates, the Figure 5 time axis. 

The originalCTD and MIS_2's stepped cast (I found the firmware bug between the buckets and stepped cast) didn't have the time bug, but they carried a daylight-saving hour added to a UTC clock because the firmware was uploaded before DST happened (in the case of the originalCTD) or I put the wrong ZD in the firmware for DST (in the case of MIS_2).

- **Buckets are matched by which water.** Each instrument has its own window in its
  own clock. What ties them together is that both sampled the same bucket. The system's mean over its window is the reference salinity for the system in their respective buckets.
- **Casts are matched by time.** The Jul-11 stepped casts and the originalCTD casts carry a daylight-saving hour on an otherwise sound UTC clock; removing it (`MIS2_CAST_DST_FIX_H`, `APPLY_DST_FIX`) makes their timestamps directly comparable with the RBR, and cast-level pairing in the static-frame section is done on overlapping time interval, with maximum-depth agreement as an independent check.

## Atmospheric pressure and depth

Atmospheric pressure is **not** taken per cast from a barometer. `ATM_MBAR = 1013.25` is a nominal standard atmosphere, and the RBR independently uses its own default_atmospheric_pressure` of 10.1325 dbar, the same nominal value.

That constant enters results by two different routes.

**Through the PSS-78 pressure term.** `p_dbar` is passed to `gsw.SP_from_C`, where pressure barely matters: an error of 0.2 dbar moves salinity by 0.00005 PSU, and 2 dbar by 0.0005 PSU. This is why the bucket and lab work is unaffected regardless of the weather.

**Through the depth matching.** Each hold takes its reference by interpolating the RBR profile in depth, so a depth error maps straight onto a salinity error through the stratification.

### Zeroing the depth against in-air samples

`DEPTH_ZERO_MODE` in the master CONFIG selects what defines 0 m for MIS_2's stepped casts, using the samples where `ec` falls below `AIR_EC_MAX` (probe out of water). It changes Figures 5, 6, 7, 8 and 11; Figures 1, 4, 9, 10 and 12 are untouched.

| Mode | Offset subtracted | Fitted gain | Fit RMSE |
|----|----|----|----|
| `none` — firmware `brDepth` as-is | 0.0000 m | 0.9908 | 0.0629 PSU |
| `pre_cast_air` | 0.1488 m | 0.9898 | 0.0533 PSU |
| `post_cast_air` (default) | 0.0059 m | 0.9908 | 0.0626 PSU |

`post_cast_air` is the default: its in-air window sits after the cast, so it zeros the depth using the closest thermal state of the pressure sensor to the temperature of the water. 
 `none` gives the same 0.9908 the paper prints, because the firmware's own zero happens to land 0.0059 m from the post-cast in-air reading. This is luck rather than a reason to trust the firmware zero.

**The two in-air references disagree, and the reason is temperature, not weather.** The pressure reading moves with sensor temperature (roughly +2.9 mbar/°C within this record):

| In-air window | Sensor T | brPressure | brDepth |
|----|----|----|----|
| before the first cast (dry and hot from being in the sun, just powered on) | 26.1 °C | 1028.06 mbar | +0.1488 m |
| after the last cast (wet, 13 min later) | 21.1 °C | 1013.65 mbar | +0.0059 m |

Atmospheric pressure does not fall 14.5 mbar in 13 minutes. The sensor cooled 5 °C and the reading followed. During the casts the sensor sits at 20.3 °C (close to the post-cast in-air state), roughly 6 °C from the pre-cast one. So `pre_cast_air` derives its zero from both a thermal state the instrument was not in while measuring,  and from temperature warmer than the water, and `post_cast_air` derives it from very nearly the right one.

The loader prints all three offsets and the sensor temperature in each state on every run, and warns when the two in-air references differ by more than 0.05 m.

**Pre vs post-cast pressure is on our list for further study**. This should especially be compared to a weather station [like this one](https://github.com/sscopazzi/Portable_Weather_Station).
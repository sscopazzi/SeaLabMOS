// THESE ARE MY SYSTEM NAMES, YOUR NAMES WILL BE DIFFERENCE

// TEMP LOGGER SWITCH COLORS
#define GREEN  0
#define BLUE   1
#define ORANGE 2
#define WHITE  3

// OTHER SYSTEM VBAT NUMBERS
// CURRENTLY ONLY USED FOR VBAT CORRECTION FACTOR AND THERMISTOR RESISTOR VALUES
#define NALGENE 4       // surface float
#define STEEL   5       // extra-deep
#define TWOIN   6       // two-inch PVC
#define BPR     7       // pressure only sensor, larger battery, records first 20min of every hr by default (can use 3 in a triangle for calculating wave height/direction??)

// for UH
#define PRESS_ONLY 8    // 1.5" pvc continuous pressure 

// for Mokil - two floats and two Temp Loggers with Blue Robotics 
#define FLOAT_1 9
#define FLOAT_2 10
#define BRTL_1 11
#define BRTL_2 12

// for MIS - all CTDs
#define MIS_1 13
#define MIS_2 14
#define MIS_3 15

#define PRESS_ONLY_SCH80 16 // schedule 80 for deeper crush depth

//#############################
#define SYSTEM_NAME MIS_2
//#############################

// Map the numeric SYSTEM_NAME to a readable string for filenames/logs.
// Single source of truth: change only the #define SYSTEM_NAME line above and
// this follows automatically. Keep the case labels in sync with the #defines.
inline const char* systemNameStr() {
  switch (SYSTEM_NAME) {
    case GREEN:            return "GREEN";
    case BLUE:             return "BLUE";
    case ORANGE:           return "ORANGE";
    case WHITE:            return "WHITE";
    case NALGENE:          return "NALGENE";
    case STEEL:            return "STEEL";
    case TWOIN:            return "TWOIN";
    case BPR:              return "BPR";
    case PRESS_ONLY:       return "PRESS_ONLY";
    case FLOAT_1:          return "FLOAT_1";
    case FLOAT_2:          return "FLOAT_2";
    case BRTL_1:           return "BRTL_1";
    case BRTL_2:           return "BRTL_2";
    case MIS_1:            return "MIS_1";
    case MIS_2:            return "MIS_2";
    case MIS_3:            return "MIS_3";
    case PRESS_ONLY_SCH80: return "PRESS_ONLY_SCH80";
    default:               return "UNKNOWN";
  }
}
#include <map>

#include <Arduino.h>

#include <Wire.h>
#include <INA3221.h>
#include <alfalog.h>

#include "pinDefs.h"

typedef std::pair<int, int> outPinDef_t;

typedef enum {
    GATE_IN,
    GATE_1,
    GATE_2,
    GATE_3,
    GATE_R,
    GATE_G,
    GATE_B
} gate_t;

#define PWR_ON true
#define PWR_OFF false

//from https://github.com/Aircoookie/WLED/issues/2767
static const uint16_t cie1931[256] = {
	0, 2, 4, 5, 7, 9, 11, 13, 15, 16, 
	18, 20, 22, 24, 26, 27, 29, 31, 33, 35, 
	34, 36, 37, 39, 41, 43, 45, 47, 49, 52, 
	54, 56, 59, 61, 64, 67, 69, 72, 75, 78, 
	81, 84, 87, 90, 94, 97, 100, 104, 108, 111, 
	115, 119, 123, 127, 131, 136, 140, 144, 149, 154, 
	158, 163, 168, 173, 178, 183, 189, 194, 200, 205, 
	211, 217, 223, 229, 235, 241, 247, 254, 261, 267, 
	274, 281, 288, 295, 302, 310, 317, 325, 333, 341, 
	349, 357, 365, 373, 382, 391, 399, 408, 417, 426, 
	436, 445, 455, 464, 474, 484, 494, 505, 515, 526, 
	536, 547, 558, 569, 580, 592, 603, 615, 627, 639, 
	651, 663, 676, 689, 701, 714, 727, 741, 754, 768, 
	781, 795, 809, 824, 838, 853, 867, 882, 897, 913, 
	928, 943, 959, 975, 991, 1008, 1024, 1041, 1058, 1075, 
	1092, 1109, 1127, 1144, 1162, 1180, 1199, 1217, 1236, 1255, 
	1274, 1293, 1312, 1332, 1352, 1372, 1392, 1412, 1433, 1454, 
	1475, 1496, 1517, 1539, 1561, 1583, 1605, 1628, 1650, 1673, 
	1696, 1719, 1743, 1767, 1791, 1815, 1839, 1864, 1888, 1913, 
	1939, 1964, 1990, 2016, 2042, 2068, 2095, 2121, 2148, 2176, 
	2203, 2231, 2259, 2287, 2315, 2344, 2373, 2402, 2431, 2461, 
	2491, 2521, 2551, 2581, 2612, 2643, 2675, 2706, 2738, 2770, 
	2802, 2835, 2867, 2900, 2934, 2967, 3001, 3035, 3069, 3104, 
	3138, 3174, 3209, 3244, 3280, 3316, 3353, 3389, 3426, 3463, 
	3501, 3539, 3576, 3615, 3653, 3692, 3731, 3770, 3810, 3850, 
	3890, 3930, 3971, 4012, 4053, 4095
};

class powerPathHandler
{
public:
    powerPathHandler(): ina(INA3221_ADDR40_GND){};

    void init(TwoWire* wire){
        ina.begin(wire);
        ina.reset();
        ina.setShuntRes(
            SHUNT_IN_RES_MR,
            SHUNT_1_RES_MR,
            SHUNT_2_RES_MR
        );

        for(auto &pm: pinMap){
            if (pm.second != -1){
                pinMode(pm.second, OUTPUT);
                digitalWrite(pm.second, 0);
                ALOGV("pin {} set to output", pm.second)
            }
        }
        analogWriteResolution(12);
    }

    void setMainSwitch(bool state){
        if (pinMap[GATE_IN] != -1){
            digitalWrite(pinMap[GATE_IN], state);
        } else {
            ALOGW("GATE_IN not set");
        }
    }

    void setPower(gate_t gate, bool state){
        if (pinMap[gate] != -1){
            digitalWrite(pinMap[gate], state);
        } else {
            ALOGW("pin not set");
        }
    }

    //todo: add raw PWM and CIE-adjusted methods
    void setPWM(gate_t gate, uint8_t value){
        if (pinMap[gate] != -1){
            analogWrite(pinMap[gate], cie1931[value]);
        } else {
            ALOGW("pin not set");
        }
    }
    
    void logCurrent(){
        ALOGR("CH_IN:  {:7.3f}mA @ {:7.3f}V",
            ina.getCurrent(INA3221_CH1) * 1000,
            ina.getVoltage(INA3221_CH1));
        ALOGR("CH_1:   {:7.3f}mA @ {:7.3f}V", 
            ina.getCurrent(INA3221_CH2) * 1000,
            ina.getVoltage(INA3221_CH2));
        ALOGR("CH_2:   {:7.3f}mA @ {:7.3f}V",
            ina.getCurrent(INA3221_CH3) * 1000,
            ina.getVoltage(INA3221_CH3));
    }

    std::map<int,int> pinMap = {
        {GATE_IN, IO7_GATEIN},
        {GATE_1, -1},
        {GATE_2, -1},
        {GATE_3, -1},
        {GATE_R, -1},
        {GATE_G, -1},
        {GATE_B, -1}
    };

private:
    INA3221 ina;
};
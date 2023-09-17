#include <Arduino.h>
#include <Wire.h>

#include <Button2.h>
#include <INA3221.h>

#include <alfalog.h>
#include "advancedOledLogger.h"

#include <commonFwUtils.h>

// #include <main.h>
#include "pinDefs.h"

#define FW_REV "0.1.0"

TwoWire i2c = TwoWire(0);

void uartHandle(const char* str){
  Serial.println(str);
}

AdvancedOledLogger aOledLogger = 
  AdvancedOledLogger(i2c, LOG_INFO, OLED_128x32, OLED_UPSIDE_DOWN);
SerialLogger serialLogger = SerialLogger(uartHandle, LOG_DEBUG);
Button2 userButton = Button2();
INA3221 ina_0(INA3221_ADDR40_GND);

void current_measure_init() {
    ina_0.begin(&i2c);
    ina_0.reset();
    ina_0.setShuntRes(10, 10, 10);
}

void enablePowerOutputs(){
  pinMode(IO7_GATEIN, OUTPUT);
  digitalWrite(IO7_GATEIN, HIGH);

  pinMode(IO0, OUTPUT);
  digitalWrite(IO0, HIGH);

  pinMode(IO1, OUTPUT);
  digitalWrite(IO1, HIGH);

  pinMode(IO2, OUTPUT);
  digitalWrite(IO2, HIGH);
}

void setup() {
  Serial.begin(115200);
  // Serial.setTxTimeoutMs(0); // prevent logger slowdown when no usb connected
  Serial.println("begin...");
  // delay(3000);
  // bootloopOnButtonPress(PIN_B2);
  i2c.begin(IO6_SDA, IO5_SCL);
      
  AlfaLogger.addBackend(&aOledLogger);
  AlfaLogger.addBackend(&serialLogger);
  AlfaLogger.begin();
  ALOGI("display started");

  ALOGI("i2c device(s) found at:\n0x{:02x}",
        fmt::join(scan_i2c(i2c), ", 0x"));

  userButton.begin(IO9_BUT1, INPUT_PULLUP, false);
  userButton.setTapHandler([](Button2 & b){
      ALOGI("button tapped");    
    }
  );

  aOledLogger.setTopBarText(BAR_WIFI_IP, "MOSNode r. " FW_REV);

  enablePowerOutputs();
}

void loop() {
    handle_io_pattern(IO4_LED, PATTERN_HBEAT);
    userButton.loop();
    vTaskDelay(50/portTICK_PERIOD_MS);

    static int counter = 0;
    if (counter++ % 8 == 0){
      aOledLogger.redraw();
      ALOGV("dupa");
      ALOGV("current: {}", ina_0.getCurrent(INA3221_CH1) * 1000);
      // Serial.printf(
      //   "A1%3.0fma %1.1fV A2%3.0fma %1.1fV\r\n",
      //   ina_0.getCurrent(INA3221_CH1) * 1000, ina_0.getVoltage(INA3221_CH1),
      //   ina_0.getCurrent(INA3221_CH2) * 1000, ina_0.getVoltage(INA3221_CH2));
    }
}

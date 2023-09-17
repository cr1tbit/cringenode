#include <cmath>

#include <Arduino.h>
#include <Wire.h>

#include <Button2.h>
#include <INA3221.h>

#include <alfalog.h>
#include "advancedOledLogger.h"

#include <commonFwUtils.h>
#include <powerPathHandler.h>
#include "pinDefs.h"

#define FW_REV "0.1.0"

TwoWire i2c = TwoWire(0);

void uartHandle(const char* str){
  Serial.println(str);
}

AdvancedOledLogger aOledLogger = 
  AdvancedOledLogger(i2c, LOG_INFO, OLED_128x32, OLED_UPSIDE_DOWN);
SerialLogger serialLogger = SerialLogger(uartHandle, LOG_DEBUG, ALOG_FANCY);
Button2 userButton = Button2();

powerPathHandler pph = powerPathHandler();

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

  ALOG_I2CLS(i2c);

  userButton.begin(IO9_BUT1, INPUT_PULLUP, false);
  userButton.setTapHandler([](Button2 & b){
      ALOGI("button tapped");    
    }
  );

  aOledLogger.setTopBarText(BAR_WIFI_IP, "MOSNode r. " FW_REV);

  pph.pinMap[GATE_1] = IO2;
  pph.pinMap[GATE_2] = IO1;
  pph.pinMap[GATE_3] = IO0;

  pph.init(&i2c);
  pph.setMainSwitch(PWR_ON);
}

void loop() {
  handle_io_pattern(IO4_LED, PATTERN_HBEAT);
  userButton.loop();
  vTaskDelay(50/portTICK_PERIOD_MS);

  static int counter = 0;
  pph.setPWM(GATE_1, sin(counter/16.0)*128 + 128);

  if (counter++ % 8 == 0){
    aOledLogger.redraw();
    pph.logCurrent();
  }
}

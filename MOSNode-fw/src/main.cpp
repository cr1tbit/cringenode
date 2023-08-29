#include <Arduino.h>
#include <Wire.h>

#include <Button2.h>

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

AdvancedOledLogger aOledLogger = AdvancedOledLogger(i2c, LOG_INFO, OLED_128x32);
SerialLogger serialLogger = SerialLogger(uartHandle, LOG_DEBUG);
Button2 userButton = Button2();

void setup() {
  Serial.begin(115200);
  // Serial.setTxTimeoutMs(0); // prevent logger slowdown when no usb connected
  Serial.println("begin...");
  // delay(3000);
  // bootloopOnButtonPress(PIN_B2);
  i2c.begin(IO6_SDA, IO5_SCL);
      
  // AlfaLogger.addBackend(&aOledLogger);
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
}

void loop() {
    handle_io_pattern(IO4_LED, PATTERN_HBEAT);
    userButton.loop();
    vTaskDelay(50/portTICK_PERIOD_MS);

    static int counter = 0;
    if (counter++ % 8 == 0){
      ALOGI("dupa");
      // aOledLogger.redraw();
    }
}

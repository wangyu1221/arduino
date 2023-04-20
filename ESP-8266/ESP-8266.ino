#include <U8g2lib.h>
#include <ESP8266WiFi.h>

static const uint8_t PROGMEM su[] = {
  /*--  文字:  速  --*/
  /*--  宋体9;  此字体下对应的点阵为：宽x高=12x12   --*/
  /*--  高度不是8的倍数，现调整为：宽度x高度=12x16  --*/
  0x00, 0x02, 0x04, 0x02, 0xE8, 0x3F, 0x08, 0x02, 0xC0, 0x1F, 0x40, 0x12, 0x4F, 0x12, 0xC8, 0x1F,
  0x08, 0x07, 0x88, 0x0A, 0x48, 0x12, 0x28, 0x22, 0x08, 0x02, 0x14, 0x00, 0xE2, 0x7F, 0x00, 0x00
};

static const uint8_t PROGMEM du[] = {
  /*--  文字:  度  --*/
  /*--  宋体9;  此字体下对应的点阵为：宽x高=12x12   --*/
  /*--  高度不是8的倍数，现调整为：宽度x高度=12x16  --*/
  0x80, 0x00, 0x00, 0x01, 0xFC, 0x7F, 0x44, 0x04, 0x44, 0x04, 0xFC, 0x3F, 0x44, 0x04, 0x44, 0x04,
  0xC4, 0x07, 0x04, 0x00, 0xF4, 0x0F, 0x24, 0x08, 0x42, 0x04, 0x82, 0x03, 0x61, 0x0C, 0x1C, 0x70
};

static const uint8_t PROGMEM li[] = {
  /*--  文字:  里  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  0x00, 0x00, 0xFC, 0x1F, 0x84, 0x10, 0x84, 0x10, 0xFC, 0x1F, 0x84, 0x10, 0x84, 0x10, 0xFC, 0x1F,
  0x80, 0x00, 0x80, 0x00, 0xFC, 0x1F, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0xFF, 0x7F, 0x00, 0x00
};

static const uint8_t PROGMEM cheng[] = {
  /*--  文字:  程  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  0x10, 0x00, 0xB8, 0x3F, 0x8F, 0x20, 0x88, 0x20, 0x88, 0x20, 0xBF, 0x3F, 0x08, 0x00, 0x0C, 0x00,
  0x9C, 0x7F, 0x2A, 0x04, 0x2A, 0x04, 0x89, 0x3F, 0x08, 0x04, 0x08, 0x04, 0xC8, 0x7F, 0x08, 0x00
};

static const uint8_t PROGMEM qi[] = {
  /*--  文字:  骑  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  0x00, 0x04, 0x1F, 0x04, 0x90, 0x3F, 0x12, 0x0A, 0x12, 0x11, 0x92, 0x20, 0xD2, 0x7F, 0x3E, 0x10,
  0xA0, 0x17, 0xA0, 0x14, 0xB8, 0x14, 0xA7, 0x17, 0xA2, 0x14, 0x20, 0x10, 0x14, 0x14, 0x08, 0x08
};

static const uint8_t PROGMEM shi[] = {
  /*--  文字:  时  --*/
  /*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
  0x00, 0x10, 0x00, 0x10, 0x3E, 0x10, 0x22, 0x10, 0xA2, 0x7F, 0x22, 0x10, 0x22, 0x10, 0x3E, 0x10,
  0x22, 0x11, 0x22, 0x12, 0x22, 0x12, 0x22, 0x10, 0x3E, 0x10, 0x22, 0x10, 0x00, 0x14, 0x00, 0x08
};

// *rotation, clock, data, cs, dc, reset
// 老8266 + 老屏幕
// U8G2_UC1701_MINI12864_F_4W_SW_SPI u8g(U8G2_R0, 16, 15, 13, 14, 12);
// 新8266 + OLED
// U8G2_SSD1306_128X64_NONAME_2_4W_HW_SPI u8g(U8G2_R0, 4, 5, 3);
U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g(U8G2_R0, 4, 5, 3);

const int cycleDistance = 2065;  // 轮子一圈长度，毫米
const int interruptPin = 15;     // 按键位置
const int oledVccPin = 12;


unsigned long lastLoopTime = 0;
unsigned long lastInterruptTime = 0;  // 上次按键触发时间
unsigned long totalDistance = 0;      // 总里程，毫米
char speedValue[10] = "0.0";

void setup() {
  Serial.begin(9600);
  Serial.println("setup...");
  u8g.begin();
  u8g.setFont(u8g2_font_ncenB24_tr);
  attachInterrupt(digitalPinToInterrupt(interruptPin), interrupt, RISING);
  Serial.println("setup finish");
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  pinMode(oledVccPin, OUTPUT);
  digitalWrite(oledVccPin, HIGH);  
}

void loop(void) {
  lastLoopTime = millis();
  if (lastLoopTime - lastInterruptTime > 5000) {
    dtostrf(0, 3, 1, speedValue);
  }
  if (lastLoopTime - lastInterruptTime > 30000) {
    digitalWrite(oledVccPin, LOW);
  }
  if (lastLoopTime - lastInterruptTime > 60000) {
    sleep();
  }
  second();
  delay(1000 - (lastLoopTime % 1000));
}

void second(void) {
  draw();
}

void sleep(void) {
  u8g.clearBuffer();
  u8g.sendBuffer();
  digitalWrite(oledVccPin, LOW);
  ESP.deepSleep(0);
}

void draw(void) {
  u8g.clearBuffer();  // clear the internal memory
  // 第一行文本
  u8g.drawXBMP(0, 0, 16, 16, su);
  u8g.drawXBMP(16, 0, 16, 16, du);
  // // 速度值
  u8g.drawStr(34, 14, speedValue);
  // // 单位
  u8g.setFont(u8g_font_unifont);
  u8g.drawStr(96, 14, "km/h");

  // // 第二行文本
  u8g.drawXBMP(0, 16, 16, 16, li);
  u8g.drawXBMP(16, 16, 16, 16, cheng);
  // // 里程值
  char tmp[10] = "";
  dtostrf(((double)totalDistance / 1000000), 3, 2, tmp);
  u8g.drawStr(34, 30, tmp);
  u8g.setFont(u8g_font_unifont);
  // // 单位
  u8g.drawStr(96, 30, "km");

  // // 第三行文本
  u8g.drawXBMP(0, 32, 16, 16, qi);
  u8g.drawXBMP(16, 32, 16, 16, shi);
  // // 骑时
  long a1 = lastLoopTime / 1000;
  int second = a1 % 3600 % 60;
  int minute = a1 % 3600 / 60;
  int hour = a1 / 3600;
  char buffer[40];
  sprintf(buffer, "%02d:%02d:%02d", hour, minute, second);
  u8g.drawStr(34, 46, buffer);
  u8g.sendBuffer();
}

const unsigned long debounceDelay = 200;
byte timestampIndex = 0;
unsigned long timeArray[] = { 0, 0, 0, 0, 0 };
// 使用长整型变量
unsigned long lastDebounceTime = 0;  // 上次按键触发时间

IRAM_ATTR void interrupt() {
  digitalWrite(oledVccPin, HIGH);
  lastInterruptTime = millis();
  if (lastInterruptTime - lastDebounceTime > debounceDelay) {
    totalDistance = totalDistance + cycleDistance;
    lastDebounceTime = lastInterruptTime;
    timeArray[timestampIndex % 5] = lastInterruptTime;
    // mm / ms
    long lastTimeInArray = timeArray[(timestampIndex + 1) % 5];
    if (lastTimeInArray > 0) {
      double speed = ((double)cycleDistance * 4) / (lastInterruptTime - lastTimeInArray);
      dtostrf((speed * 36 / 10), 3, 1, speedValue);
    }
    timestampIndex++;
  }
  Serial.println(speedValue);
}
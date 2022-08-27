
#include "BluetoothSerial.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

#define inputPIN 12

BluetoothSerial SerialBT;

float temperature;
volatile bool DataReady = false;
volatile uint32_t time1 = 0, time2 = 0, timer1;
volatile uint16_t digitalData = 0;
volatile uint8_t i = 0;

IRAM_ATTR void isr1() { //Длительноность высокого уровня

  time1 = micros();
  detachInterrupt(inputPIN);
  timer1 = time1 - time2;

  if (timer1 > 150) {
    DataReady = false;
    i = 0;
    digitalData = 0;
  }
  else if (timer1 < 30) {
    digitalData = digitalData << 1;
    i++;
  }
  else if ((timer1 > 60) && (timer1 < 100) ) {
    digitalData = digitalData << 1 | 1;
    i++;
  }
  if (i == 16 ) {
    DataReady = true;
  }
  attachInterrupt(inputPIN, isr2, RISING);
}
IRAM_ATTR void  isr2() { //Длительноность низкого уровня
  time2 = micros();
  detachInterrupt(inputPIN);
  attachInterrupt(inputPIN, isr1, FALLING);
}

void setup() {
  Serial.begin(115200);
  pinMode(inputPIN, INPUT);
  SerialBT.begin("ESP32thermometer"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  attachInterrupt(inputPIN, isr1, FALLING);
  delay(1000);
}

void loop() {
  if (DataReady) {
    DataReady = false;
    temperature = (float(digitalData) / 16383) * 70 - 10;
    Serial.println(temperature);
    //Serial.println(digitalData, BIN);
    SerialBT.println(temperature);
  }
}

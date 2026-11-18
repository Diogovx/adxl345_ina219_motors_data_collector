#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_INA219.h>

#define SDA_PIN 8
#define SCL_PIN 9
#define SAMPLING_PERIOD_US 10000UL   // 100 Hz

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
Adafruit_INA219 ina219;

uint32_t proximaAmostraUs = 0;

[[noreturn]] void falha(const char* msg) {
  while (true) {
    Serial.println(msg);
    delay(1000);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(SDA_PIN, SCL_PIN);     // 100 kHz, igual aos adapters do firmware de inferencia

  if (!accel.begin()) falha("[ERRO] ADXL345 nao detectado");
  accel.setRange(ADXL345_RANGE_16_G);

  if (!ina219.begin()) falha("[ERRO] INA219 nao detectado");
  ina219.setCalibration_32V_2A();

  // Nenhuma mensagem de texto aqui: o forwarder so aceita linhas numericas
  proximaAmostraUs = micros() + SAMPLING_PERIOD_US;
}

void loop() {
  const uint32_t agora = micros();
  if ((int32_t)(agora - proximaAmostraUs) < 0) return;

  // Proximo instante = anterior + 10 ms (sem deriva)
  proximaAmostraUs += SAMPLING_PERIOD_US;
  if ((int32_t)(agora - proximaAmostraUs) >= 0) {
    proximaAmostraUs = agora + SAMPLING_PERIOD_US;   // ficou atrasado mais de um periodo: ressincroniza
  }

  sensors_event_t ev;
  accel.getEvent(&ev);
  const float corrente_mA = ina219.getCurrent_mA();

  Serial.printf("%.3f,%.3f,%.3f,%.2f\n",
                ev.acceleration.x, ev.acceleration.y, ev.acceleration.z, corrente_mA);
}
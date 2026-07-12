#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C manHinhLcd(0x27, 16, 2);

const int CHAN_PHAT_1 = 2;
const int CHAN_THU_1  = 3;
const int CHAN_PHAT_2 = 9;
const int CHAN_THU_2  = 10;

Servo thanhChan1;
Servo thanhChan2;
const int CHAN_SERVO_1 = 4;
const int CHAN_SERVO_2 = 5;

const int CHAN_COI_1 = 11;
const int CHAN_COI_2 = 12;

const int LED_XANH_PIN = 11;
const int LED_VANG_PIN = 12;
const int LED_DO_PIN   = 13;
const int LED_PHU_PIN  = 6;

const int NGUONG_KHOANG_CACH = 5;
const int GOC_MO   = 0;
const int GOC_DONG = 90;

int gocHienTai = GOC_MO;

float docKhoangCach(int chanPhat, int chanThu) {
  digitalWrite(chanPhat, LOW);
  delayMicroseconds(2);
  digitalWrite(chanPhat, HIGH);
  delayMicroseconds(10);
  digitalWrite(chanPhat, LOW);

  long thoiGian = pulseIn(chanThu, HIGH, 30000);
  if (thoiGian == 0) return 999.0;

  float khoangCach = (thoiGian * 0.0343) / 2;
  if (khoangCach < 0 || khoangCach > 500) return 999.0;

  return khoangCach;
}

void datLed(bool xanh, bool vang, bool doLed, bool phu) {
  digitalWrite(LED_XANH_PIN, xanh ? HIGH : LOW);
  digitalWrite(LED_VANG_PIN, vang ? HIGH : LOW);
  digitalWrite(LED_DO_PIN, doLed ? HIGH : LOW);
  digitalWrite(LED_PHU_PIN, phu ? HIGH : LOW);
}

void hienThiLcd(const char* dong1, const char* dong2) {
  manHinhLcd.setCursor(0, 0);
  manHinhLcd.print(dong1);
  manHinhLcd.setCursor(0, 1);
  manHinhLcd.print(dong2);
}

void coiKeuBip(int chan, int tanSo, int soLan, int thoiGianKeu) {
  for (int i = 0; i < soLan; i++) {
    tone(chan, tanSo, thoiGianKeu);
    delay(thoiGianKeu + 50);
    noTone(chan);
    if (i < soLan - 1) delay(100);
  }
}

void setup() {
  Serial.begin(9600);
  delay(1000);

  manHinhLcd.init();
  manHinhLcd.backlight();
  hienThiLcd("   THANH CHAN", "    TAU HOA");
  delay(2000);
  manHinhLcd.clear();

  pinMode(CHAN_PHAT_1, OUTPUT);
  pinMode(CHAN_THU_1, INPUT);
  pinMode(CHAN_PHAT_2, OUTPUT);
  pinMode(CHAN_THU_2, INPUT);

  pinMode(CHAN_COI_1, OUTPUT);
  pinMode(CHAN_COI_2, OUTPUT);
  digitalWrite(CHAN_COI_1, LOW);
  digitalWrite(CHAN_COI_2, LOW);

  pinMode(LED_XANH_PIN, OUTPUT);
  pinMode(LED_VANG_PIN, OUTPUT);
  pinMode(LED_DO_PIN, OUTPUT);
  pinMode(LED_PHU_PIN, OUTPUT);
  datLed(false, false, false, false);

  thanhChan1.attach(CHAN_SERVO_1);
  thanhChan2.attach(CHAN_SERVO_2);
  thanhChan1.write(GOC_MO);
  thanhChan2.write(GOC_MO);

  datLed(true, false, false, false);
  delay(500);
  datLed(false, true, false, false);
  delay(500);
  datLed(false, false, true, false);
  delay(500);
  datLed(false, false, false, false);

  coiKeuBip(CHAN_COI_1, 2000, 3, 150);
  delay(500);
  coiKeuBip(CHAN_COI_2, 1500, 2, 150);
  delay(500);

  datLed(true, false, false, false);
  hienThiLcd("MOI DI  ", "MOI DI  ");

  delay(500);
  thanhChan1.write(GOC_DONG);
  thanhChan2.write(GOC_DONG);
  delay(1000);
  thanhChan1.write(GOC_MO);
  thanhChan2.write(GOC_MO);
}

void loop() {
  float khoangCach1 = docKhoangCach(CHAN_PHAT_1, CHAN_THU_1);
  float khoangCach2 = docKhoangCach(CHAN_PHAT_2, CHAN_THU_2);

  bool camBien1PhatHien = khoangCach1 > 0 && khoangCach1 < NGUONG_KHOANG_CACH;
  bool camBien2PhatHien = khoangCach2 > 0 && khoangCach2 < NGUONG_KHOANG_CACH;

  if (camBien2PhatHien) {
    if (gocHienTai != GOC_MO) {
      datLed(true, false, false, false);
      coiKeuBip(CHAN_COI_1, 2000, 1, 100);
      coiKeuBip(CHAN_COI_2, 1500, 1, 100);
      thanhChan1.write(GOC_MO);
      thanhChan2.write(GOC_MO);
      gocHienTai = GOC_MO;
      manHinhLcd.clear();
      hienThiLcd("Trang thai:", "   MOI DI");
      delay(500);
    } else {
      datLed(true, false, false, false);
    }
  } else if (camBien1PhatHien) {
    if (gocHienTai != GOC_DONG) {
      datLed(false, true, false, true);
      coiKeuBip(CHAN_COI_1, 2000, 2, 200);
      coiKeuBip(CHAN_COI_2, 1500, 2, 200);
      thanhChan1.write(GOC_DONG);
      thanhChan2.write(GOC_DONG);
      gocHienTai = GOC_DONG;
      datLed(false, false, true, true);
      manHinhLcd.clear();
      hienThiLcd("TAU HOA ", "DANG  TOI");
      delay(500);
    } else {
      datLed(false, false, true, true);
    }
  } else if (gocHienTai == GOC_MO) {
    datLed(true, false, false, false);
    hienThiLcd("MOI DI          ", "MOI DI          ");
  } else {
    datLed(false, false, true, true);
    hienThiLcd("TAU HOA         ", "DANG TOI        ");
  }

  delay(100);
}

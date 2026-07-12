
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Khởi tạo LCD với địa chỉ I2C 0x27, màn hình 16x2
// Nếu không hoạt động, thử địa chỉ 0x3F
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Khai báo cảm biến siêu âm 1
const int TRIG_PIN_1 = 2;
const int ECHO_PIN_1 = 3;

// Khai báo cảm biến siêu âm 2
const int TRIG_PIN_2 = 9;
const int ECHO_PIN_2 = 10;

// Khai báo servo
Servo servo1;
Servo servo2;
const int SERVO1_PIN = 4;
const int SERVO2_PIN = 5;

// Khai báo buzzer
const int BUZZER_PIN = 11;   // Buzzer 1 - Pin 7
const int BUZZER2_PIN = 12;  // Buzzer 2 - Pin 8

// Khai báo LED
const int LED_XANH_PIN = 11;   // LED xanh lá - Trạng thái an toàn (thanh chắn mở)
const int LED_VANG_PIN = 12;   // LED vàng - Cảnh báo (đang xử lý)
const int LED_DO_PIN = 13;     // LED đỏ - Nguy hiểm (thanh chắn đóng)
const int LED_BO_SUNG_PIN = 6; // LED bổ sung - Báo hiệu bổ sung

// Ngưỡng phát hiện vật thể (cm)
const int DISTANCE_THRESHOLD = 5;

// Góc quay thanh chắn
const int ANGLE_0 = 0;      // THANH CHẮN MỞ (nằm ngang - xe có thể đi qua)
const int ANGLE_90 = 90;    // THANH CHẮN ĐÓNG (hạ xuống - chặn đường)

// Biến lưu trạng thái servo
int currentAngle = 0;

// Hàm đọc khoảng cách từ cảm biến siêu âm
float readDistance(int trigPin, int echoPin) {
  // Gửi xung Trig
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Đọc thời gian Echo với timeout 30000 microseconds (khoảng 5m)
  long duration = pulseIn(echoPin, HIGH, 30000);
  
  // Nếu không nhận được tín hiệu (duration = 0), trả về giá trị lớn
  if (duration == 0) {
    return 999.0;  // Trả về giá trị lớn để không bị coi là phát hiện
  }
  
  // Tính khoảng cách (cm)
  // Vận tốc âm thanh = 343 m/s = 0.0343 cm/microsecond
  // Khoảng cách = (thời gian * 0.0343) / 2 (vì âm thanh đi 2 chiều)
  float distance = (duration * 0.0343) / 2;
  
  // Giới hạn khoảng cách hợp lý (0-500cm)
  if (distance < 0 || distance > 500) {
    return 999.0;  // Trả về giá trị lớn nếu không hợp lệ
  }
  
  return distance;
}

// Hàm kêu buzzer PASSIVE (dùng tone)
// frequency: Tần số âm thanh (Hz) - 1000-4000 Hz là tốt
void buzzerBeep(int times, int duration) {
  int frequency = 2000;  // Tần số 2000Hz (có thể điều chỉnh 1000-4000)
  for (int i = 0; i < times; i++) {
    tone(BUZZER_PIN, frequency, duration);  // Phát âm với tần số và thời gian
    delay(duration + 50);  // Đợi âm thanh kết thúc + thêm 50ms
    noTone(BUZZER_PIN);  // Tắt buzzer
    if (i < times - 1) delay(100);  // Nghỉ giữa các lần kêu
  }
}

// Hàm kêu buzzer 2 PASSIVE (dùng tone)
// frequency: Tần số âm thanh (Hz) - 1000-4000 Hz là tốt
void buzzer2Beep(int times, int duration) {
  int frequency = 1500;  // Tần số 1500Hz (khác với buzzer 1 để phân biệt)
  for (int i = 0; i < times; i++) {
    tone(BUZZER2_PIN, frequency, duration);  // Phát âm với tần số và thời gian
    delay(duration + 50);  // Đợi âm thanh kết thúc + thêm 50ms
    noTone(BUZZER2_PIN);  // Tắt buzzer
    if (i < times - 1) delay(100);  // Nghỉ giữa các lần kêu
  }
}

void setup() {
  // Khởi tạo Serial Monitor
  Serial.begin(9600);
  delay(1000);  // Đợi Serial Monitor kết nối
  Serial.println(F("========================================"));
  Serial.println(F("  HE THONG BAT DAU KHOI DONG..."));
  Serial.println(F("========================================"));
  
  // Khởi tạo LCD
  lcd.init();
  lcd.backlight();
  
  // Hiển thị thông báo khởi động trên LCD
  lcd.setCursor(0, 0);
  lcd.print(" THANH CHAN");
  lcd.setCursor(0, 1);
  lcd.print("  TAU HOA");
  delay(2000);
  lcd.clear();
  
  // Cấu hình chân cảm biến siêu âm 1
  pinMode(TRIG_PIN_1, OUTPUT);
  pinMode(ECHO_PIN_1, INPUT);
  
  // Cấu hình chân cảm biến siêu âm 2
  pinMode(TRIG_PIN_2, OUTPUT);
  pinMode(ECHO_PIN_2, INPUT);
  
  // Cấu hình buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  pinMode(BUZZER2_PIN, OUTPUT);
  digitalWrite(BUZZER2_PIN, LOW);
  
  // Cấu hình LED
  pinMode(LED_XANH_PIN, OUTPUT);
  pinMode(LED_VANG_PIN, OUTPUT);
  pinMode(LED_DO_PIN, OUTPUT);
  pinMode(LED_BO_SUNG_PIN, OUTPUT);
  
  // Tắt tất cả LED ban đầu
  digitalWrite(LED_XANH_PIN, LOW);
  digitalWrite(LED_VANG_PIN, LOW);
  digitalWrite(LED_DO_PIN, LOW);
  digitalWrite(LED_BO_SUNG_PIN, LOW);
  
  // Khởi tạo servo
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  
  // Đặt thanh chắn về vị trí ban đầu: MỞ (0 độ) - Nằm ngang
  servo1.write(ANGLE_0);
  servo2.write(ANGLE_0);
  currentAngle = ANGLE_0;
  
  // TEST LED VÀ BUZZER KHI KHỞI ĐỘNG
  Serial.println(F("Test LED va Buzzer..."));
  
  // Test LED xanh
  Serial.println(F("Test LED XANH (Pin 11)..."));
  digitalWrite(LED_XANH_PIN, HIGH);
  delay(500);
  digitalWrite(LED_XANH_PIN, LOW);
  delay(200);
  
  // Test LED vàng
  Serial.println(F("Test LED VANG (Pin 12)..."));
  digitalWrite(LED_VANG_PIN, HIGH);
  delay(500);
  digitalWrite(LED_VANG_PIN, LOW);
  delay(200);
  
  // Test LED đỏ
  Serial.println(F("Test LED DO (Pin 13)..."));
  digitalWrite(LED_DO_PIN, HIGH);
  delay(500);
  digitalWrite(LED_DO_PIN, LOW);
  delay(200);
  
  // Test buzzer 1
  Serial.println(F("Test BUZZER 1 (Pin 7)..."));
  buzzerBeep(3, 150);
  delay(500);
  
  // Test buzzer 2
  Serial.println(F("Test BUZZER 2 (Pin 8)..."));
  buzzer2Beep(2, 150);
  delay(500);
  
  // Test LED bổ sung
  Serial.println(F("Test LED BO SUNG (Pin 6)..."));
  digitalWrite(LED_BO_SUNG_PIN, HIGH);
  delay(500);
  digitalWrite(LED_BO_SUNG_PIN, LOW);
  delay(200);
  
  Serial.println(F("Test LED va Buzzer hoan thanh!"));
  
  // Bật LED xanh ban đầu (thanh chắn mở)
  digitalWrite(LED_XANH_PIN, HIGH);
  digitalWrite(LED_VANG_PIN, LOW);
  digitalWrite(LED_DO_PIN, LOW);
  digitalWrite(LED_BO_SUNG_PIN, LOW);
  
  // Hiển thị trạng thái ban đầu trên LCD (cửa mở - 0 độ)
  lcd.setCursor(0, 0);
  lcd.print("MOI DI");
  lcd.setCursor(0, 1);
  lcd.print("MOI DI");
  
  // Test servo quay (để kiểm tra thanh chắn hoạt động)
  Serial.println(F("Test thanh chan..."));
  delay(500);
  servo1.write(ANGLE_90);
  servo2.write(ANGLE_90);
  delay(1000);
  servo1.write(ANGLE_0);
  servo2.write(ANGLE_0);
  delay(500);
  Serial.println(F("Test thanh chan hoan thanh!"));
  
  Serial.println(F("========================================"));
  Serial.println(F("  HE THONG THANH CHAN TAU HOA TU DONG"));
  Serial.println(F("========================================"));
  Serial.println(F("Logic:"));
  Serial.println(F("  - Ban dau: THANH CHAN MO (0 do)"));
  Serial.println(F("  - Cam bien 1 phat hien -> DONG THANH CHAN (90 do) - GIU DONG"));
  Serial.println(F("  - Cam bien 2 phat hien -> MO THANH CHAN (0 do) - CHI KHI NAY MOI MO"));
  Serial.println(F("  - Ca 2 khong phat hien -> Giu nguyen trang thai"));
  Serial.println(F("========================================"));
  delay(1000);
}

void loop() {
  // Đọc khoảng cách từ cảm biến 1
  float distance1 = readDistance(TRIG_PIN_1, ECHO_PIN_1);
  
  // Đọc khoảng cách từ cảm biến 2
  float distance2 = readDistance(TRIG_PIN_2, ECHO_PIN_2);
  
  // Kiểm tra trạng thái cảm biến
  bool camBien1PhatHien = (distance1 < DISTANCE_THRESHOLD && distance1 > 0);
  bool camBien2PhatHien = (distance2 < DISTANCE_THRESHOLD && distance2 > 0);
  
  // Hiển thị trên Serial Monitor
  Serial.print(F("CB1: "));
  Serial.print(distance1, 1);
  Serial.print(F("cm ["));
  Serial.print(camBien1PhatHien ? F("PHAT HIEN") : F("KHONG"));
  Serial.print(F("] | CB2: "));
  Serial.print(distance2, 1);
  Serial.print(F("cm ["));
  Serial.print(camBien2PhatHien ? F("PHAT HIEN") : F("KHONG"));
  Serial.print(F("] | Goc: "));
  Serial.print(currentAngle);
  Serial.print(F(" do"));
  
  // LOGIC ĐIỀU KHIỂN THANH CHẮN TÀU HỎA:
  // 1. Cảm biến 2 phát hiện vật đi → MỞ THANH CHẮN (0 độ) - Nâng lên (ƯU TIÊN CAO)
  // Chỉ khi cảm biến 2 phát hiện thì mới mở cửa
  if (camBien2PhatHien) {
    if (currentAngle != ANGLE_0) {
      Serial.println(F(" >>> MO THANH CHAN (0 do) - Vat da di qua!"));
      
      // Bật LED xanh, tắt các LED khác
      digitalWrite(LED_XANH_PIN, HIGH);
      digitalWrite(LED_VANG_PIN, LOW);
      digitalWrite(LED_DO_PIN, LOW);
      digitalWrite(LED_BO_SUNG_PIN, LOW);
      
      // Kêu buzzer 1 lần (báo hiệu mở)
      buzzerBeep(1, 100);
      // Kêu buzzer 2 1 lần (báo hiệu bổ sung)
      buzzer2Beep(1, 100);
      
      servo1.write(ANGLE_0);
      servo2.write(ANGLE_0);
      currentAngle = ANGLE_0;
      
      // Hiển thị trên LCD: Mời đi (cửa mở)
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Trang thai:");
      lcd.setCursor(0, 1);
      lcd.print("   MOI DI");
      
      delay(500);  // Đợi servo quay xong
    } else {
      Serial.println(F(" (Thanh chan da mo)"));
      // Giữ LED xanh sáng
      digitalWrite(LED_XANH_PIN, HIGH);
      digitalWrite(LED_VANG_PIN, LOW);
      digitalWrite(LED_DO_PIN, LOW);
      digitalWrite(LED_BO_SUNG_PIN, LOW);
    }
  }
  // 2. Cảm biến 1 phát hiện vật đến → ĐÓNG THANH CHẮN (90 độ) - Hạ xuống và GIỮ ĐÓNG
  // Khi cảm biến 1 phát hiện, đóng cửa và GIỮ ĐÓNG (không quay lại 0 độ)
  else if (camBien1PhatHien) {
    if (currentAngle != ANGLE_90) {
      Serial.println(F(" >>> DONG THANH CHAN (90 do) - Co vat den!"));
      
      // Bật LED vàng khi đang xử lý (cảnh báo)
      digitalWrite(LED_XANH_PIN, LOW);
      digitalWrite(LED_VANG_PIN, HIGH);
      digitalWrite(LED_DO_PIN, LOW);
      digitalWrite(LED_BO_SUNG_PIN, HIGH);  // Bật LED bổ sung khi cảnh báo
      
      // Kêu buzzer 1 và buzzer 2 (cảnh báo)
      buzzerBeep(2, 200);
      buzzer2Beep(2, 200);
      
      servo1.write(ANGLE_90);
      servo2.write(ANGLE_90);
      currentAngle = ANGLE_90;
      
      // Bật LED đỏ khi cửa đã đóng (nguy hiểm)
      digitalWrite(LED_XANH_PIN, LOW);
      digitalWrite(LED_VANG_PIN, LOW);
      digitalWrite(LED_DO_PIN, HIGH);
      digitalWrite(LED_BO_SUNG_PIN, HIGH);  // Giữ LED bổ sung sáng khi đóng
      
      // Hiển thị trên LCD: Tàu hỏa đang tới (cửa đóng)
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("TAU HOA ");
      lcd.setCursor(0, 1);
      lcd.print("DANG  TOI");
  
      delay(500);  // Đợi servo quay xong
    } else {
      // Đã đóng rồi, giữ nguyên - LED đỏ sáng
      Serial.println(F(" (Thanh chan da dong - giu nguyen)"));
      digitalWrite(LED_XANH_PIN, LOW);
      digitalWrite(LED_VANG_PIN, LOW);
      digitalWrite(LED_DO_PIN, HIGH);
      digitalWrite(LED_BO_SUNG_PIN, HIGH);  // Giữ LED bổ sung sáng
    }
  }
  // 3. Cả 2 cảm biến không phát hiện → Giữ nguyên trạng thái hiện tại
  else {
    Serial.println(F(" (Giu nguyen trang thai)"));
    
    // Cập nhật LCD và LED theo trạng thái hiện tại
    if (currentAngle == ANGLE_0) {
      // 0 độ (cửa mở) → hiển thị "MOI DI", LED xanh
      digitalWrite(LED_XANH_PIN, HIGH);
      digitalWrite(LED_VANG_PIN, LOW);
      digitalWrite(LED_DO_PIN, LOW);
      digitalWrite(LED_BO_SUNG_PIN, LOW);
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("MOI DI:");
      lcd.setCursor(0, 1);
      lcd.print("MOI DI");
    } else if (currentAngle == ANGLE_90) {
      // 90 độ (cửa đóng) → hiển thị "TAU HOA DANG TOI", LED đỏ sáng
      digitalWrite(LED_XANH_PIN, LOW);
      digitalWrite(LED_VANG_PIN, LOW);
      digitalWrite(LED_DO_PIN, HIGH);
      digitalWrite(LED_BO_SUNG_PIN, HIGH);  // LED bổ sung sáng khi đóng
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("TAU HOA");
      lcd.setCursor(0, 1);
      lcd.print("DANG TOI");
    }
  }
  
  // Delay để tránh đọc quá nhanh
  delay(100);
}           


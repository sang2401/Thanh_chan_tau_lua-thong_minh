# Thanh chắn tàu lửa thông minh

Mô hình thanh chắn đường sắt tự động phát hiện tàu/vật thể và hạ/nâng thanh chắn kèm cảnh báo. Dùng 2 cảm biến siêu âm, 2 servo, còi và LED, hiển thị trạng thái trên LCD.

## Luồng hoạt động
1. 2 cảm biến siêu âm quét phía đường ray.
2. Phát hiện vật trong vùng nguy hiểm → bật còi + LED đỏ.
3. 2 servo hạ thanh chắn xuống (90°) chặn giao thông.
4. Vật rời vùng an toàn → nâng thanh chắn, tắt cảnh báo, bật LED xanh.

## Công nghệ sử dụng
- Phần cứng: Arduino, 2x HC-SR04, 2x Servo, LCD I2C, Buzzer, LED
- Ngôn ngữ: C++ (Arduino)
- IDE: Arduino IDE

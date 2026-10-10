/*
            2023-2 명지대학교 융합프로젝트

        드론을 이용한 산불 피해 산림 재생 시스템
            中 배터리 온도 유지 장치 코드

                 Using Arduino UNO

  60222438 한수민 |  60201875 박규현  |  60201892 진용욱
  
*/

#include <NewPing.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define ONE_WIRE_BUS A0 // DS18B20 센서가 연결된 핀 번호(~핀 수정)

// 온도센서 세팅
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// LCD 세팅
LiquidCrystal_I2C lcd(0x27, 16, 2);

// 모스펫 세팅
int Mosfet1 = 6;
const int echoPinTrig = 2; // HY-SRF05 초음파 센서의 Trig 핀으로 변경
const int maxDistance = 200; // 최대 감지 거리 설정 (센서에 따라 다름)

// RF 세팅
const byte address[6] = "team02";
RF24 radio(7, 8);

// NewPing 라이브러리를 사용한 초음파 센서 설정
NewPing sonar(echoPinTrig, echoPinTrig, maxDistance);

void setup() {
  // 통신 속도 세팅
  Serial.begin(9600);

  // LCD 세팅
  lcd.init();
  lcd.backlight();

  // RF 수신 세팅
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();

  // 모스펫 및 초음파 핀 세팅
  pinMode(Mosfet1, OUTPUT);

  // 온도 센서 사용
  sensors.begin();
}

void loop() {
  // RF 수신용 데이터 버퍼-정수형 배열 선언
  int Receive[10] = {};

  // int형 RF 수신 -> sprintf로 값 변환 -> char string BT 송신
  if (radio.available()) {
    radio.read(Receive, sizeof(Receive));
    char *BT = (char *)malloc(sizeof(char) * 10);
    if (BT == NULL) {
      Serial.print("error\n");
      exit(1);
    }
    for (int i = 0; i < 10; i++) {
      sprintf(BT, "%d", Receive[i]); // BT 전송을 위해 int형에서 char형으로 변환
      Serial.print(BT);               // BT 모듈로 값 전송
      if (i < 10 - 1)
        Serial.print("#"); // 어플에서 값 분리용 문자
    }
    Serial.print("\n");
    free(BT);                   // 동적 메모리 해제
    //delay(200);                 // Clock.Interval < delay Arduino
  }

  // 초음파 센서 사용
  delay(50);
  unsigned int distance = sonar.ping_cm();

  sensors.requestTemperatures(); // DS18B20 센서에서 온도 값을 요청
  float temperatureC = sensors.getTempCByIndex(0); // 섭씨 온도 값 읽기

  // LCD 사용
  lcd.clear(); // 화면 지우기
  lcd.setCursor(0, 0);

  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(temperatureC);
  lcd.print(" °C");


  // 거리에 따른 온도 제어
  if (distance < 10.0 && temperatureC <= 25.0) {
    digitalWrite(Mosfet1, HIGH);
    if(distance < 10.0){
      lcd.setCursor(1, 0);
      lcd.print("Battery IN");
    }
    else{
      lcd.setCursor(1, 0);
      lcd.print("Battery OUT");
    }
  } else {
    digitalWrite(Mosfet1, LOW);
    if(distance < 10.0){
      lcd.setCursor(1, 0);
      lcd.print("Battery IN");
    }
    else{
      lcd.setCursor(1, 0);
      lcd.print("Battery OUT");
    }
  }

  //delay(100); // 원하는 간격으로 딜레이를 조절하세요
}

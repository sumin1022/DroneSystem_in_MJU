/*
            2023-2 명지대학교 융합프로젝트

        드론을 이용한 산불 피해 산림 재생 시스템
            中 배터리 온도 유지 장치 코드

                 Using Arduino UNO

  60222438 한수민 |  60201875 박규현  |  60201892 진용욱
  
*/

#include <Servo.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//전송 속도 정의
#define BaudRate 9600
// 적외선 센서 핀 정의
#define IR_1pin A0
#define IR_2pin A1
#define IR_3pin A2
//PWM 기준 값 정의
#define pwmHigh 1800
#define pwmLow 1700
//LED 핀 정의
#define Gled_pin 2
#define Rled_pin 3
//PWM 핀 정의
#define pwm_pin  6
//서보 핀 정의
#define Servo_pin  9
//적외선 센서 기준 값 정의
#define IR_1Std 10 
#define IR_2Std 10
#define IR_3Std 10
//서보모터 회전 값 정의
#define Servo_1Rot 60
#define Servo_2Rot 0

RF24 radio(7, 8);

const byte address[6] = "team02";

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

// 서보모터와 PWM 값 관련 변수 정의
Servo servo;
int pwmValue;
int rotationCount = 0;

void setup()
{
  Serial.println("가속도 센서 초도 테스트 문구입니다.");

  if (!accel.begin())
  {
    Serial.println("가속도 센서 오류!");
    while (1)
      ;
  }

  Serial.begin(BaudRate);

  pinMode(pwm_pin, INPUT);

  // 적외선 센서 핀 지정
  pinMode(IR_1pin, INPUT); // 아날로그 0번 핀 이용
  pinMode(IR_2pin, INPUT); // 아날로그 1번 핀 이용
  pinMode(IR_3pin, INPUT); // 아날로그 2번 핀 이용

  // LED 핀 지정
  pinMode(Gled_pin, OUTPUT); // 디지털 2번 핀 이용 - 녹색 LED
  pinMode(Rled_pin, OUTPUT); // 디지털 3번 핀 이용 - 적색 LED

  // 서보모터 초기화
  servo.attach(Servo_pin);

  //RF
	radio.begin();
  radio.openWritingPipe(address);

  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

void loop(void)
{
  // ADXL345 가속도 센서 동작
  sensors_event_t event;
  accel.getEvent(&event);

  pwmValue = pulseIn(pwm_pin, HIGH);

  //적외선 센서 값 변수 정의
  int IRV1 = analogRead(IR_1pin);
  int IRV2 = analogRead(IR_2pin);
  int IRV3 = analogRead(IR_3pin);


  Serial.print(IRV1);
  Serial.print("  ");
  Serial.print(IRV2);
  Serial.print("  ");
  Serial.print(IRV3);
  Serial.print("  \n");
  Serial.print("X: ");
  Serial.print(event.acceleration.x);
  Serial.print("  ");
  Serial.print("Y: ");
  Serial.print(event.acceleration.y);
  Serial.print("  ");
  Serial.print("Z: ");
  Serial.print(event.acceleration.z);
  Serial.println("  ");
  Serial.println(rotationCount);
  Serial.println(pwmValue);

  // 기울기 검사 및 동작
  if ((event.acceleration.x > -1 && event.acceleration.x < 1) && (event.acceleration.y > 0 && event.acceleration.y < 2))
  {
    delay(1000);
  }
  else
  {
    if (pwmValue >= pwmHigh)
    {
    if (IRV1 < IR_1Std || IRV2 < IR_2Std || IRV3 < IR_3Std)
    {
      // 정상 투척 (적외선 센서 3개 중 1개 이상에서 검출) - 녹색 LED 점등
      digitalWrite(Gled_pin, HIGH);
      digitalWrite(Rled_pin, LOW);
      rotationCount++;
      radio.write(&rotationCount, sizeof(rotationCount));
      //PWM 신호가 1800이상일 때 서보 작동
      servo.write(Servo_1Rot);
      delay(1500);
      servo.write(Servo_2Rot);
      delay(1000);
    }
    else
    {
      // 투척 오류 (적외선 센서 미검출) - 적색 LED 점등 및 서보모터 움직임
      digitalWrite(Gled_pin, LOW);
      digitalWrite(Rled_pin, HIGH);
			//delay(1000);
    }
    }
    else if (pwmValue <= pwmLow)
    {
      //PWM 신호가 1700이하일 때 서보 작동
      servo.write(Servo_2Rot);
      delay(100);
    }
  }
}

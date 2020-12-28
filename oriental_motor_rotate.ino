#include <MsTimer2.h>
#include <EEPROM.h>

int m_positiveRotatePin = 3;
int m_negtiveRotatePin = 5;
bool m_rotateClockWiseFlg = false;
bool m_rotateAntiClockWiseFlg = false;
int m_idIndex = 0;
int m_id = 0;
float m_sleepTime = 0;  //ms
unsigned long m_velocity = 0;
int m_direction = 0;
unsigned char m_recv[3];
float m_rotateAccelaration = 1000;
float m_rotateDeceleration = 1000;



void setup() {
  Serial.begin(9600);
  MsTimer2::set(100, onTimer);
  MsTimer2::start();
  
  // put your setup code here, to run once:
  pinMode(m_positiveRotatePin, OUTPUT);
  pinMode(m_negtiveRotatePin, OUTPUT);
  m_id = EEPROM.read(m_idIndex);
//  EEPROM.write(m_idIndex, 1);
}

void onTimer(){
//  int available_num = Serial.available();
  if(Serial.available()>=6){
    Serial.readBytes(m_recv, 6);
    if(m_recv[0] == 0){
      decodeData();
    }else{
      if(m_recv[0] == m_id){
        decodeData();
      }
    }
  }
}

void transformNum(unsigned char* recv, int direction, float* sleepTime){
    unsigned long velocity = ((unsigned long)m_recv[2])*256*256*256 + ((unsigned long)m_recv[3])*256*256 + ((unsigned long)m_recv[4])*256 + (unsigned long)(m_recv[5]);     //pulse/s
    if(direction == 0){
      if(m_direction == 0){
        float timeAcceleration = (velocity - m_velocity)/m_rotateAccelaration;
      }else if(m_direction == 1){
        float timeDeceleration = m_velocity / m_rotateDeceleration;
        float timeAcceleration = velocity / m_rotateAccelaration;
      }
      *sleepTime =  (500000.0)/velocity - 8.75;
      m_velocity = velocity; 
    }else if(direction == 1){
      if(m_direction == 0){
        float timeAcceleration = (velocity - m_velocity)/m_rotateAccelaration;
      }else if(m_direction == 1){
        float timeDeceleration = m_velocity / m_rotateDeceleration;
        float timeAcceleration = velocity / m_rotateAccelaration;
      }
      *sleepTime =  (500000.0)/velocity - 8.75;
    }else if(direction == 5){
      m_rotateClockWiseFlg = false;
      m_rotateAntiClockWiseFlg = false;
      m_velocity = 0;
      delayMicroseconds(1000);
      Serial.println("stop");      
    }
}

void setID(unsigned char* recv, int* id){
  *id = recv[5];
  EEPROM.write(m_idIndex, *id);
}

void setAccelerationAndDeceleration(unsigned char*recv, float* acceleration, float* deceleration){ 
  *acceleration = ((unsigned long)recv[2])*256 + ((unsigned long)recv[3]);
  *deceleration = ((unsigned long)recv[4])*256 + ((unsigned long)recv[5]);
}

void decodeData(){
      int directionTmp = m_recv[1];
      if(directionTmp == 0){
      transformNum(m_recv, directionTmp, &m_sleepTime);
      m_rotateClockWiseFlg = true;
      m_rotateAntiClockWiseFlg = false;
    }else if(directionTmp == 1){
      transformNum(m_recv, directionTmp, &m_sleepTime);
      m_rotateClockWiseFlg = false;
      m_rotateAntiClockWiseFlg = true;
    }else if(directionTmp == 2){
      m_rotateClockWiseFlg = false;
      m_rotateAntiClockWiseFlg = false;
      m_velocity = 0;
    }else if(directionTmp == 3){
      setID(m_recv, &m_id);
    }else if(directionTmp == 4){
      setAccelerationAndDeceleration(m_recv, &m_rotateAccelaration, &m_rotateDeceleration);
    }else if(directionTmp == 5){
      transformNum(m_recv, directionTmp, &m_sleepTime);
    }
    m_direction = directionTmp;
}

void clockwise()
{
  while(m_rotateClockWiseFlg){
      delayMicroseconds(m_sleepTime);
      digitalWrite(m_positiveRotatePin, HIGH);
      delayMicroseconds(m_sleepTime);
      digitalWrite(m_positiveRotatePin, LOW);
  }
}

void anticlockwise()
{
  while(m_rotateAntiClockWiseFlg){
      delayMicroseconds(m_sleepTime);
      digitalWrite(m_negtiveRotatePin, HIGH);
      delayMicroseconds(m_sleepTime);
      digitalWrite(m_negtiveRotatePin, LOW);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  clockwise();
  anticlockwise();
  delayMicroseconds(10000);
//  Serial.print("m_id ");
//  Serial.println(m_id, DEC);
}

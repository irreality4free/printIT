#include <MultiStepper.h>
#include <AccelStepper.h>

#include <Stepper.h>



#define NEW_LINE '\n'
#define LED_PIN 13

#define SENSE_PUSHED 1

#define RAK_BACK 1
#define RAK_FRONT 0

#define LIFT_TOP 1
#define LIFT_BOT 0

#define RAK_STEP 2
#define RAK_DIR 5

#define LIFT_STEP 6
#define LIFT_DIR 7

#define BUNK1_STEP 8
#define BUNK1_DIR 9

#define BUNK2_STEP 10
#define BUNK2_DIR 11

#define RAK_SENSE_BACK 30
#define RAK_SENSE_FRONT 31

#define LIFT_SENSE_BOT 32
#define LIFT_SENSE_TOP 33


int sensors[4];
String input_string = "";
bool Break = false;

int ind1; // , locations
int ind2;
int ind3;
int ind4;

String arg1; //data String
String arg2;
String arg3;
String arg4;



#define MAX_SPEED 1200
#define SPEED 1200
#define MAX_ACCEL 12000


AccelStepper local_step;
AccelStepper stepper1(1, RAK_STEP, RAK_DIR);
AccelStepper stepper2(1, LIFT_STEP, LIFT_DIR);
AccelStepper stepper3(1, BUNK1_STEP, BUNK1_DIR);
AccelStepper stepper4(1, BUNK2_STEP, BUNK2_DIR);


/// ДЕЛАЕТ ЗАДАННОЕ КОЛИЧЕСТВО ШАГОВ МОТОРОМ
// stepper_pin(RAK_STEP, LIFT_STEP,BUNK1_STEP,BUNK2_STEP) dir_pin(RAK_DIR, LIFT_DIR, BUNK1_DIR, BUNK2_DIR)
// dir(LIFT_TOP,LIFT_BOT,RAK_BACK,RAK_FRONT) del(задержка между степ) step_num(количество шагов)

void Go(int stepper_pin, int dir_pin, int dir, int del, int step_num) {
  digitalWrite(dir_pin, dir);
  for (int i = 0; i < step_num; i++) {
    digitalWrite(stepper_pin, HIGH);
    if (del > 0) delay(del);
    digitalWrite(stepper_pin, LOW);
    if (del > 0) delay(del);
    Control();
  }
}


void move(int step_num , int pos) {
  if (step_num==1)local_step= stepper1;
  if (step_num==2)local_step= stepper2;
  if (step_num==3)local_step= stepper3;
  if (step_num==4)local_step= stepper4;
  local_step.runToNewPosition(local_step.currentPosition() + pos);
}


void move_to(int step_num , int pos) {
  if (step_num==1)local_step= stepper1;
  if (step_num==2)local_step= stepper2;
  if (step_num==3)local_step= stepper3;
  if (step_num==4)local_step= stepper4;
  local_step.runToNewPosition(pos);
}
//bool move_to_switch(AccelStepper *st, int speed, int sw) {
////  AccelStepper local_step = 
//  
//  st->setSpeed(speed);
//  while (! digitalRead(sw))
//  {
//    st->runSpeed();
//  }
//  st->setSpeed(0);
//  for (int i = 0; i < 10000; i++)
//  {
//    st->runSpeed();
//  }
//}



bool move_to_switch(int step_num, int speed, int sw) {
  if (step_num==1)local_step= stepper1;
  if (step_num==2)local_step= stepper2;
  if (step_num==3)local_step= stepper3;
  if (step_num==4)local_step= stepper4;
  
  
  local_step.setSpeed(speed);
  while (! digitalRead(sw))
  {
    local_step.runSpeed();
  }
  local_step.setSpeed(0);
  for (int i = 0; i < 10000; i++)
  {
    local_step.runSpeed();
  }
}


void GoLOOP(int stepper_pin, int dir_pin, int dir, int del) {
  digitalWrite(dir_pin, dir);
  while (Break == false) {
    digitalWrite(stepper_pin, HIGH);
    if (del > 0) delay(del);
    digitalWrite(stepper_pin, LOW);
    if (del > 0) delay(del);
    Control();
    if (Break == true) {
      Break = false;
      break;
    }
  }
}


///ВОЗВРАЩАЕТ ДОМОЙ РАКЕЛЬ .
//dir(RAK_BACK, RAK_FRONT) Speed( Задержка между степ)

void HomeRak(int dir, int Speed) {
  while (1) {
    if (dir == RAK_BACK && digitalRead(RAK_SENSE_BACK) == SENSE_PUSHED)break;
    if (dir == RAK_FRONT && digitalRead(RAK_SENSE_FRONT) == SENSE_PUSHED)break;
    if (dir == RAK_BACK)Go(RAK_STEP, RAK_DIR, RAK_BACK, Speed, 1);
    if (dir == RAK_FRONT)Go(RAK_STEP, RAK_DIR, RAK_FRONT, Speed, 1);
  }
}


///ОПУСКАЕТ ИЛИ ПОДНИМАЕТ ЛИФТ .
//dir(LIFT_BOT, LIFT_TOP) Speed( Задержка между степ)

void HomeLift(int dir, int Speed) {
  while (1) {
    if (dir == LIFT_BOT && digitalRead(LIFT_SENSE_BOT) == SENSE_PUSHED)break;
    if (dir == LIFT_TOP && digitalRead(LIFT_SENSE_TOP) == SENSE_PUSHED)break;
    if (dir == LIFT_BOT)Go(LIFT_STEP, LIFT_DIR, LIFT_BOT, Speed, 1);
    if (dir == LIFT_TOP)Go(LIFT_STEP, LIFT_DIR, LIFT_TOP, Speed, 1);
  }
}


//СЧИТЫВАЕТ СОСТОЯНИЕ УПОРОВ РАКЕЛЯ И ЛИФТА /global

void CheckSense() {
  sensors[0] = digitalRead(RAK_SENSE_BACK);
  sensors[1] = digitalRead(RAK_SENSE_FRONT);
  sensors[2] = digitalRead(LIFT_SENSE_BOT);
  sensors[3] = digitalRead(LIFT_SENSE_TOP);
}



//ПОЛУЧАЕТ КОМАНДЫ ИЗ SERIAL И ПАРСИТ ИХ

void Control() {
  while (Serial.available() > 0 ) {
    char a = Serial.read();
    Serial.println(a);
    if (a == NEW_LINE) { //считается, что команда завершена, если пришел символ перевода строки
      parseIt(input_string);
      input_string = "";
    }
    else {
      input_string += a;
    }

  }
}


void answer(int ok) {
  if (ok == 1)Serial.println(1);
  if (ok == 0)Serial.println(0);
}

//ФУНЦИЯ ДЛЯ ПАРСИГА И ОБРАБОТКИ КОММАНД

void parseIt(String input_string) {

  Serial.println(input_string);

  if (input_string.substring(0, 4) == "c01:") {
    input_string =  input_string.substring(4);
    ind1 = input_string.indexOf(':');
    arg1 = input_string.substring(0, ind1);   //captures first data String
    ind2 = input_string.indexOf(':', ind1 + 1 ); //finds location of second ,
    arg2 = input_string.substring(ind1 + 1, ind2 ); //captures second data String
    ind3 = input_string.indexOf(':', ind2 + 1 );
    arg3 = input_string.substring(ind2 + 1, ind3 );
    ind4 = input_string.indexOf(',', ind3 + 1 );
    arg4 = input_string.substring(ind3 + 1);
    
    Serial.print("arg1 = ");
    Serial.println(arg1);
    Serial.print("arg2 = ");
    Serial.println(arg2);
    Serial.print("arg3 = ");
    Serial.println(arg3);
    Serial.print("arg4 = ");
    Serial.println(arg4);
  }
  //    if (input_string.substring(4, 5) == "1")
  //    if (input_string.substring(4, 5) == "8")Break = true;
  //    if (input_string.substring(4, 5) == "7")GoLOOP(RAK_STEP, RAK_DIR, RAK_BACK, 1);

}







void setup() {
  Serial.begin(9600);
  Serial3.begin(9600);
  pinMode(RAK_STEP, OUTPUT);
  pinMode(RAK_DIR, OUTPUT);

  pinMode(LIFT_STEP, OUTPUT);
  pinMode(LIFT_DIR, OUTPUT);

  pinMode(BUNK1_STEP, OUTPUT);
  pinMode(BUNK1_DIR, OUTPUT);

  pinMode(BUNK2_STEP, OUTPUT);
  pinMode(BUNK2_DIR, OUTPUT);

  pinMode(LED_PIN, OUTPUT);

  stepper1.setMaxSpeed(MAX_SPEED);
  stepper1.setAcceleration(MAX_SPEED);

  stepper2.setMaxSpeed(MAX_SPEED);
  stepper2.setAcceleration(MAX_SPEED);
  
  stepper3.setMaxSpeed(MAX_SPEED);
  stepper3.setAcceleration(MAX_SPEED);
  
  stepper4.setMaxSpeed(MAX_SPEED);
  stepper4.setAcceleration(MAX_SPEED);
  
  

}

void loop() {
  //  Control();
  //  move(&stepper0,1000);
  //  delay(1000);
//    move_to_switch1(1, 1000, RAK_SENSE_BACK);
//  move_to(&stepper0, 1000);
}

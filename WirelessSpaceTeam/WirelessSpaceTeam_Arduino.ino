#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>

#define red 6
#define green 7
#define yellow 8
RF24 radio(9, 10);

//sequence of light colors output
char sequence [100];
//sequence of light colors input
char userInput [100];
//round number
int roundNum = 1;
//number of latest light input
int lightNum = 1;

//stores data about current state of game
struct data {
  char color =  ' ';
  bool pass = true;
};
data info;

//initialize radio
void initRadio() {
  radio.setChannel(18);
  radio.setPALevel(RF24_PA_MIN);
  radio.openReadingPipe(1, 0xc2c2c2c2c2);
  radio.openWritingPipe(0xe7e7e7e7e7);
  radio.setCRCLength(RF24_CRC_16);
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  radio.begin();
  printf_begin();
  initRadio();

  pinMode(red, INPUT);
  pinMode(yellow, INPUT);
  pinMode(green, INPUT);

  radio.startListening();
}

void loop() {
  //wait for Teensy to send data
  while (!radio.available());
  
  //read data from Teensy
  radio.read((char*) &info, sizeof(info));
  sequence[roundNum - 1] = info.color;
  //Serial.println(info.color);

  //reset number of latest light input
  lightNum = 1;

  //take in input
  while (lightNum <= roundNum) {
    if (digitalRead(red)) {
      userInput[lightNum - 1] = 'r';
      lightNum++;
      delay(500);
    }
    else if (digitalRead(green)) {
      userInput[lightNum - 1] = 'g';
      lightNum++;
      delay(500);
    }
    else if (digitalRead(yellow)) {
      userInput[lightNum - 1] = 'y';
      lightNum++;
      delay(500);
    }
  }

  //check if input sequence is correct
  for (int i = 0; i < roundNum; i++) {
    if (sequence[i] != userInput[i]) {
      info.pass = false;
    }
  }
  
  //if input correct, move on; if not, reset game
  if (info.pass) {
    roundNum++;
  } else {
    //roundNum = 1;
    //lightNum = 1;
    //memset(userInput, 0, sizeof(userInput));
    //memset(sequence, 0, sizeof(sequence));
  }
  
  radio.stopListening();
  radio.write((char*) &info, sizeof(info));
  radio.startListening();
  if (info.pass == false) while (1);
}

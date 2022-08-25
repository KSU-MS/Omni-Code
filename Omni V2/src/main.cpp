#include "KSULED.h" //would rename to KSULED.h to avoid conflicts
#include "KSUBuffer.h" //renamed to avoid conflicts
#include "Card.h" //would rename to "KSUCard.h" to avoid conflicts
#include "ErrorCodes.h "
#include "CanNetwork.h"
#include "Arduino.h"
//#include "Radio.h"

// Voltage reading pin (For power loss detection) (TODO)
#define V_SENSE 18 //TODO implement hardware capability and update pin #

// Can chip (UPDATE PINS)
#define CAN_CS 10 //TODO update with actual pin on board
#define CAN_IRQ 14
#define MAX_ATTEMPTS 69 
// Radio chip (UPDATE & REPLACE)
//#define RADIO_CS 31
//#define RADIO_CE 36
//#define RADIO_INT 35
//#define RADIO_ADDRESS "00001" //TODO replace with xbee stuff
LED led = LED();
Buffer buffer = Buffer();
Card card = Card();
CanNetwork can = CanNetwork(CAN_CS);
//Radio radio = Radio(RADIO_CE, RADIO_CS);  //TODO xbees 
//Global Var Defines
volatile bool system_ready = false;
volatile bool power_down = false;
volatile int packets = 0;
bool act = false;
unsigned long last = 0;
//Debug mode
#define debug_mode true
#define SERIAL_TIMEOUT 5000
void recievePacket(); //have to define methods before they are called because vscode 
void powerDown();

void setup(){
  //pinMode(LED_BUILTIN,OUTPUT);
  //digitalWrite(LED_BUILTIN,HIGH);
  //digitalWrite(LED_BUILTIN,LOW);
  //cant call "nointerrupts" if _NO INTERRUPTS HAVE BEEN SET yet_
  //noInterrupts();
  led.startUp(50);
  delay(1000);
#if (debug_mode)
    Serial.begin(115200);
    // Wait for slow serial on the teensy, but only for 5 seconds max
    while (!(Serial) && millis()<SERIAL_TIMEOUT){}
    Serial.println("Setup begin...");
    Serial.println("Debug mode on.");
    Serial.print("RTC TIME: ");
    delay(100);
    buffer.debug();
    card.debug();
    can.debug();    
    Serial.println(Teensy3Clock.get()); //(UPDATE)
    delay(100);
#endif
  int initAttempts=1;
  // Init CAN system
  
  while(can.init(CAN_250KBPS)){
#if (debug_mode)
    led.blink(0,255,0,0);
    Serial.printf("CAN init failed! Reattempting try #: %d\n",initAttempts);

 #endif
    initAttempts++; delay(10); //
    if(initAttempts>=MAX_ATTEMPTS){
      Serial.println("Maxed out CAN attempts, breaking..."); //too lazy to put in debug #ifs, should make serial wrapper for debug mode
      led.setColor(0,255,0,0);
      break;
    }

  }; 
#if (debug_mode) //dont allow loopback if not at least in debug mode
  //    can.loopback(); /// NOTE DEBUGGING - NOT FOR PROD - WILL NOT LISTEN FOR MESSAGES!!!!!!!
#endif
  // Init SD Card
  if (!card.init(10)){
    Serial.println("Error initializing SD card controller");
    led.setColor(1,255,0,0);
    //led.errorTrap(SD_CARD_ERROR); //this will lock omni in fault state if no SD
  }

  // Create 1MB file (Will create new ones if we exceed this)
  card.openFile(10);
// Init Radio (xbee go here)
  delay(1000);
  // CAN Inturrupt registration
  pinMode(CAN_IRQ, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(CAN_IRQ), recievePacket, LOW); 
  noInterrupts();
  // CAN Inturrupt registration
  pinMode(V_SENSE, INPUT);
  attachInterrupt(digitalPinToInterrupt(V_SENSE), powerDown, FALLING); //falling edge on power rail sense pin will call powerDown() function
  noInterrupts();
  Serial.println("Setup complete.");
  led.setColor(0,0,255,0);
  interrupts();
}

void recievePacket()
{
  if(power_down) return;
  led.blink(0,0,0,255);
  CanPacket incoming = can.receive();
  if(!system_ready) return;
 if (incoming.timestamp != 0)
  {
    //radio.send(&incoming); //DISABLED
    buffer.push(incoming);
    packets++;
  }
}
void powerDown()
{
  noInterrupts();
  power_down = true;
  card.closeFile();
}

void loop(){
// if(power_down) {
//   led.setColor(0, 255, 0); //commission when ready
//   return;
// }
  system_ready = true;
  recievePacket();
  while (buffer.blockReady())
  {
    led.blink(1,0,255,0);
    card.writeBlock(buffer.peek());
    buffer.pop();
  }
  if (millis() - last > 1000)
  {
    Serial.println(packets);
    last = millis();
    packets = 0;
  }
 
}

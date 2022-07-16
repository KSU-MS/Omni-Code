#include "LED.h"
#include "Buffer.h"
#include "Card.h"
#include "ErrorCodes.h"
#include "CanNetwork.h"

#include "Radio.h"

// LED outputs (UNUSED)
#define LED_R 20
#define LED_G 21
#define LED_B 22

// Voltage reading pin (For power loss detection) (TO DO)
#define V_SENSE 18

// Can chip (UPDATE PINS)
#define CAN_CS 15
#define CAN_IRQ 14

// Radio chip (UPDATE & REPLACE)
#define RADIO_CS 31
#define RADIO_CE 36
#define RADIO_INT 35
#define RADIO_ADDRESS "00001"

Buffer buffer = Buffer();
Card card = Card();
LED led = LED(LED_R, LED_G, LED_B); //(UPDATE DISPLAY STATES)
CanNetwork can = CanNetwork(CAN_CS);
Radio radio = Radio(RADIO_CE, RADIO_CS);  //(UPDAE & REPLACE)

#define debug_mode true

void setup()
{

  noInterrupts();
  led.setColor(255, 100, 0);  //UPDATE TO LED

  if (debug_mode)
  {

    Serial.begin(1000000);

    // Wait for slow serial on the teensy, but only for 5 seconds max
    unsigned long timeout = millis();
    while (!Serial && millis() - timeout < 5000)
      delay(1);

    Serial.println("Setup begin...");
    Serial.println("Debug mode on.");

    Serial.print("RTC TIME: ");
    Serial.println(Teensy3Clock.get()); //(UPDATE)

    //    buffer.debug();
    //    card.debug();
//        can.debug();
    radio.debug();  //(DISABLE)
  }

  // Init CAN system
  can.init(CAN_250KBPS);
//    can.loopback(); /// NOTE DEBUGGING - NOT FOR PROD - WILL NOT LISTEN FOR MESSAGES!!!!!!!

  // Init SD Card
  if (!card.init(BUILTIN_SDCARD)){
    Serial.println("Error initializing SD card controller");
    led.errorTrap(SD_CARD_ERROR); //INVESTAGATE
  }

  // Create 1MB file (Will create new ones if we exceed this)
  card.openFile(10);



  // Init Radio

  /*
  if (!radio.init(RADIO_ADDRESS, &SPI1))
    Serial.println("Error initializing radio transmitter");
  else
    Serial.println("Radio init OK");


  delay(1000);
  */


  // CAN Inturrupt registration
  pinMode(CAN_IRQ, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(CAN_IRQ), recievePacket, LOW);
  noInterrupts();
  // CAN Inturrupt registration
  pinMode(V_SENSE, INPUT);
  attachInterrupt(digitalPinToInterrupt(V_SENSE), powerDown, FALLING);
  noInterrupts();

  Serial.println("Setup complete.");
  led.clear();  //(UPDATE)

  interrupts();
}
volatile bool system_ready = false;
volatile bool power_down = false;
volatile int packets = 0;
void recievePacket()
{
if(power_down) return;
  led.setColor(0, 0, 255); //(UPDATE)
  CanPacket incoming = can.receive();
  if(!system_ready) return;
  if (incoming.timestamp != 0)
  {
    radio.send(&incoming); //DISABLE
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
bool act = false;
unsigned long last = 0;
void loop()
{
if(power_down) {
    led.setColor(0, 255, 0); //UPDATE
    return;
}
  system_ready = true;

  while (buffer.blockReady())
  {

    led.setColor(0, 255, 0); //UPDATE
    card.writeBlock(buffer.peek());
    buffer.pop();
  }
  if (millis() - last > 1000)
  {
    Serial.println(packets);
    last = millis();
    packets = 0;
  }
  led.clear(); //UPDATE
  //  led.tick(millis());
}

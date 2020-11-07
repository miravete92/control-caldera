#include <Stepper.h>
#include <EtherCard.h>
 
// Esto es el número de pasos por revolución
#define STEPS 2038 
// Número de pasos que queremos que de
#define NUMSTEPS_ON -50
#define NUMSTEPS_OFF 50
#define CORRECTION_OFFSET 200

#define STATUS_TURN_ON 1
#define STATUS_TURN_OFF 0

#define SENSOR_PIN 8
#define DEBUG_BUTTON_PIN 9

static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
static byte myip[] = { 192,168,1,203 };

byte Ethernet::buffer[500];
BufferFiller bfill;

// Constructor, pasamos STEPS y los pines donde tengamos conectado el motor
Stepper stepper(STEPS, 4, 6, 5, 7);
int lastStatus = STATUS_TURN_OFF;

void setup() {
  // Asignamos la velocidad en RPM (Revoluciones por Minuto)
  Serial.begin(57600);
  stepper.setSpeed(5);
  pinMode(SENSOR_PIN, INPUT);
  //pinMode(DEBUG_BUTTON_PIN, INPUT);

  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0)
    Serial.println(F("Failed to access Ethernet controller"));
  ether.staticSetup(myip);
}
 
void loop() {
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);

  if (pos > 0){  // check if valid tcp data is received
    char* chArray = (char *) Ethernet::buffer + pos;        // this is how to read a request header
    String dataString = String(chArray);
    
    /*if(strstr(chArray, "GET /?action=turnon") != 0) {
      Serial.println("Turning on");
      turnOn();
      lastStatus = STATUS_TURN_ON;
      Serial.println("Turning on done");
    }*/

    /*if(strstr(chArray, "GET /?action=turnoff") != 0) {
      //Serial.println("Turning off");
      turnOff();
      lastStatus = STATUS_TURN_OFF;
      //Serial.println("Turning off done");
    }*/
    delay(5000);
    Serial.println("Replying");
    ether.httpServerReply(homePage());
  }
  
  /*if(isButtonPressed()) {
    if(lastStatus == STATUS_TURN_OFF) {
      turnOn();
      lastStatus = STATUS_TURN_ON;
    } else if(lastStatus == STATUS_TURN_ON) {
      turnOff();
      lastStatus = STATUS_TURN_OFF;
    }
  }*/
  
}

void turnOn() {
  //Serial.println("Turning on start");
  while(isInWhiteZone()) {
    stepper.step(NUMSTEPS_ON);
  }
  //Serial.println("Turning on correction");
  while(!isInWhiteZone()) {
    stepper.step(NUMSTEPS_OFF);
  }
  //Serial.println("Turning on ended");
}

void turnOff() {
  //Serial.println("Turning off start");
  while(isInWhiteZone()) {
    stepper.step(NUMSTEPS_OFF);
  }
  //Serial.println("Turning off correction");
  while(!isInWhiteZone()) {
    stepper.step(NUMSTEPS_ON);
  }
  //Serial.println("Turning off ended");
}

bool isInWhiteZone() {
  return digitalRead(SENSOR_PIN) == HIGH;
}

bool isButtonPressed(){
  return digitalRead(DEBUG_BUTTON_PIN) == HIGH;
}

static word homePage() {
  long t = millis() / 1000;
  word h = t / 3600;
  byte m = (t / 60) % 60;
  byte s = t % 60;
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
    "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Pragma: no-cache\r\n"
    "\r\n"
    "<head>"
    "<title>RBBB server</title><link rel=\"icon\" href=\"data:,\"></head>" 
    "<body>"
    "<h1>$D$D:$D$D:$D$D</h1>"
    "<p>Hello</p>"
    "</body>"),
      h/10, h%10, m/10, m%10, s/10, s%10);
  return bfill.position();
}

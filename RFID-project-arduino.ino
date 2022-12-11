#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// RFID Module
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance.

// RFID read
int readsuccess;
byte readcard[4];
char str[32] = "";
String StrUID;

//LED
int ledRed = 2;
int ledBlue = 3;
//int ledGreen = 4;

//Door status
int doorState = 1;

//Button
// int button = 5;
// int ButtonValue = 0;

//Servo
Servo servo;
int posServo = 0;

//reset
void(* resetFunc) (void) = 0;

void setup() {
  Serial.begin(9600); // Initialize serial communications with the PC
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max); // Increase to max frequency

  // RFID Module checker
  if(!mfrc522.PCD_PerformSelfTest()){
    Serial.println("RFiD reader failed to initialized");
    Serial.println("Restarting...");
    delay(500);
    resetFunc();
  }

  else{
    Serial.println("RFiD reader success initialized");
  }

  //Set pin mode
  pinMode(ledRed, OUTPUT);
  pinMode(ledBlue, OUTPUT);
  servo.attach(6);

  // Initial servo postion
  servo.write(posServo);
  digitalWrite(ledRed, HIGH);

  Serial.println("Please Scan your card\n");

}

void loop() {
  readsuccess = getid();

  if(readsuccess){
    Serial.println(StrUID);
    Serial.println("");   
    
    if (StrUID=="80910D7C") { // Registered card
      doorControl(); // Open/close door
    }
    else {  //Not registered card
      Serial.println("Status : Unknown\n");
      servo.write(0); // reset to lock postion
      digitalWrite(ledBlue, LOW); // turn off red led
      delay(500);
      for (int i = 0; i < 3; i++){ // Blinking red led
        digitalWrite(ledRed, HIGH);
        delay(500);
        digitalWrite(ledRed, LOW);
        delay(500);
      }
      digitalWrite(ledRed, HIGH); // Reset to default red led
      
    }
    //digitalWrite(ledRed, LOW);
    //digitalWrite(ledBlue, LOW);


  }
}

int getid(){  
  if(!mfrc522.PICC_IsNewCardPresent()){
    return 0;
  }
  if(!mfrc522.PICC_ReadCardSerial()){
    return 0;
  }
 
Serial.println("Please Scan your card\n");
  Serial.print("UID:");
  
  for(int i=0;i<4;i++){
    readcard[i]=mfrc522.uid.uidByte[i]; //storing the UID of the tag in readcard
    array_to_string(readcard, 4, str);
    StrUID = str;
  }
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}

void array_to_string(byte array[], unsigned int len, char buffer[])
{
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}

void doorControl(){

  // Opening Door
  if (doorState == 1){
    Serial.println("Status : Registered"); 
    digitalWrite(ledBlue, HIGH); // Blue On
    digitalWrite(ledRed, LOW); // Red off
    Serial.println("Door: Opened");
    posServo = 90;
  }

  // Closing Door
  else{
    digitalWrite(ledBlue, LOW); // Blue off
    digitalWrite(ledRed, HIGH); // Red on
    Serial.println("Door: Closed");
    posServo = 0;
  }

  servo.write(posServo);  // Change the servo postion
  doorState = -doorState; // Flip door State

  //digitalWrite(ledGreen, HIGH);
  //delay(1000);
  //digitalWrite(ledGreen, LOW);
}

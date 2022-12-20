#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h> 
LiquidCrystal_I2C lcd(0x27, 16, 2);
 
#define SS_PIN 10
#define RST_PIN 9
#define LED_G 3                     // Define green LED as pin 3
#define LED_R 2                     // Define red LED as pin 2
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
int button = 4;                     // Set button on pin 4
Servo servo;                        // Declare servo

String registered[4] = {"80 91 0D 7C", "F0 59 FE 73"};

void unlock() {

  Serial.println("Access Granted");
  Serial.println();
  delay(500);
  digitalWrite(LED_G, HIGH);
  lcd.setCursor(0,1);
  lcd.print("  Door Unlocked ");
  servo.write(90);

  // Automatically locks door after 3 seconds
  delay(3000);
  servo.write(15);
  delay(100);
  digitalWrite(LED_G, LOW);
  lcd.setCursor(0,1);
  lcd.print("   Door Locked   ");  
}

void setup() {

  Serial.begin(9600);     // Initiate a serial communication
  SPI.begin();            // Initiate  SPI bus
  mfrc522.PCD_Init();     // Initiate MFRC522
  pinMode(LED_G, OUTPUT); // Set green LED as OUTPUT
  pinMode(LED_R, OUTPUT); // Set red LED as OUTPUT
  pinMode(button, INPUT); // Set button as INPUT
  servo.attach(6);        // Set servo on pin 6
  servo.write(15);        // Set servo angle at 15 degrees
  
  Serial.println("Place your card on reader...");
  Serial.println();
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0); // Column, Row
  lcd.print(" Scan Your RFID "); 
  lcd.setCursor(0,1); // Column, Row
  lcd.print("   Door Locked   ");
}

void loop() {

  bool flag = false;

  // Unlocks door if button is pressed
  if(digitalRead(button) == HIGH){
  
   unlock();
  }

  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }

  // Show ID on serial monitor
  Serial.print("ID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
     
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();

  // Displays ID
  lcd.setCursor(0,1);
  lcd.print("ID: " + content.substring(1) + " ");
  delay(1500);
  
  for (int i = 0; i < 4; i ++) {

    if (content.substring(1) == registered[i]) {

      flag = true;
      unlock();
      break;
    }
  }
  
  // Denies access if unregistered ID is presented
  if (flag == false) {
    
    lcd.setCursor(0,1);
    lcd.print("Invalid RFID Tag");
    Serial.println("Access denied");
    digitalWrite(LED_R, HIGH);
    delay(500);
    digitalWrite(LED_R, LOW);
    delay(100);
    digitalWrite(LED_R, HIGH);
    delay(500);
    digitalWrite(LED_R, LOW);
    delay(100);
    digitalWrite(LED_R, HIGH);
    delay(500);
    digitalWrite(LED_R, LOW);
    lcd.setCursor(0,1);
    lcd.print("   Door Locked   ");
  }
}

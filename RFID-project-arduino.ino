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

int state = 0;                      // State of button
int startTime = 0;                  // Time of button press
int finishTime = 0;                 // Time of button release
int holdTime = 0;                   // Time spent holding button
int exists = 0;                     // Flag for checking registered IDs

String registered[10];  // Array of pre-registered IDs

// Unlocks door
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

  // Flag for checking unregistered IDs
  bool flag = false;
  
  // Checks whether button is pressed
  if ((digitalRead(button) == HIGH) && (state == 0)) {
    
    // Gets time of button press
    startTime = millis();
    state = 1;
    Serial.println("Button pressed");
  } 

  // Button has been just released
  if ((digitalRead(button) == LOW) && (state == 1))  {
    
    // Gets time of button release
    finishTime = millis();

    // Calculates time spent holding button
    holdTime = finishTime - startTime;

    // Unlocks door if button was held for less than 2 seconds
    if (holdTime < 2000) {

      unlock();
      state = 0;
    }

    // Goes into ID registration mode of button was held for more than 2 seconds
    if (holdTime >= 2000) {
        
      lcd.setCursor(0,1);
      lcd.print("   To Register  ");

      // Looks for new cards
      if ( ! mfrc522.PICC_IsNewCardPresent()) {
        
        return;
      }
      
      // Selects one of the cards
      if ( ! mfrc522.PICC_ReadCardSerial()) {
        
        return;
      }

      // Show ID on serial monitor
      Serial.print("ID tag :");
      String content = "";
      byte letter;
        
      for (byte i = 0; i < mfrc522.uid.size; i++) {
     
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
      }

      content.toUpperCase();

      // Displays ID
      lcd.setCursor(0,1);
      lcd.print("ID: " + content.substring(1) + " ");
      delay(1500);

      // Checks array for whether ID already exists
      for (int i = 0; i < 10; i ++) {

        if (registered[i] == content.substring(1)) {

          exists = 1;
          lcd.setCursor(0,0);
          lcd.print("   ID Already   ");
          lcd.setCursor(0,1);
          lcd.print("     Exists!    ");
          delay(1500);
          lcd.setCursor(0,0);
          lcd.print(" Scan Your RFID "); 
          lcd.setCursor(0,1);
          lcd.print("   Door Locked   ");
          break;
        }
      }

      // If ID does not already exist
      if (exists == 0) {

        // Finds an empty index in array to place ID
        for (int i = 0; i < 10; i ++) {
          
          if (registered[i] == "") {
            
            registered[i] = content.substring(1);
            lcd.setCursor(0,1);
            lcd.print(" ID Registered! ");
            delay(1500);
            lcd.setCursor(0,0);
            lcd.print(" Scan Your RFID "); 
            lcd.setCursor(0,1);
            lcd.print("   Door Locked   ");
            break;
          }
        }            
      }

      // Resets flags
      exists = 0; 
      state = 0;
    }
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
  String content = "";
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
  
  for (int i = 0; i < 10; i ++) {

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

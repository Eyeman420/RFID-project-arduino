#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <PinButton.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define SS_PIN 10
#define RST_PIN 9
#define LED_G 3                    // Define green LED as pin 3
#define LED_R 2                    // Define red LED as pin 2
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
const int buzzer = 7;              // buzzer 

//int button = 4;                   // Set button on pin 4
Servo servo;  // Declare servo

PinButton button(4);    // Button to lock and register card
String registered[10];  // Array of pre-registered IDs
String UIDcard;

//reset
void (*resetFunc)(void) = 0;

void setup() {
  Serial.begin(9600);                              // Initialize serial communications with the PC
  SPI.begin();                                     // Init SPI bus
  mfrc522.PCD_Init();                              // Init MFRC522 card
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);  // Increase to max frequency
  pinMode(buzzer, OUTPUT);                         // Set buzzer - pin 9 as an output

  // RFID Module checker
  if (!mfrc522.PCD_PerformSelfTest()) {
    Serial.println("RFiD reader failed to initialized");  // If encountered this issues, please reconnect the arduino
    Serial.println("Restarting...");
    delay(500);
    resetFunc();
  }

  else {
    Serial.println("RFiD reader success initialized");
  }

  pinMode(LED_G, OUTPUT);  // Set green LED as OUTPUT
  pinMode(LED_R, OUTPUT);  // Set red LED as OUTPUT
  servo.attach(6);         // Set servo on pin 6
  servo.write(15);         // Set servo angle at 15 degrees

  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);  // Column, Row
  lcd.print(" Scan Your RFID ");
  Serial.println("Scan Your RFID");
  lcd.setCursor(0, 1);  // Column, Row
  lcd.print("   Door Locked   ");
  Serial.println("Door Locked");
}

void loop() {
  
  button.update();
  
  // Read card
  if (ReadCard() == true) {
    
    if (RegisteredCard() == true) {
      
      Serial.println("Access Granted");
      //Serial.println();
      // Unlock();
      
    } else {
      
      lcd.setCursor(0, 0);
      lcd.print("Invalid RFID Tag");
      Serial.println("Invalid RFID Tag");
      lcd.setCursor(0, 1);
      lcd.print("   Door Locked   ");
      Serial.println("Door Locked");
      Lock();

      for (int i = 0; i < 5; i++) {
        
        digitalWrite(LED_R, HIGH);
        tone(buzzer, 250); // 250 Hz sound signal
        delay(250);    
        digitalWrite(LED_R, LOW);
        noTone(buzzer);
        delay(250);
      }
    }
  }
  
  // Read button
  if (button.isSingleClick()) {
    
    //Serial.println("Entering registeration mode");
    //NewRegisterationCard();
    Unlock();
  }

  if (button.isLongClick()) {  //Registering new card mode
    
    Serial.println("Entering registeration mode");
    NewRegisterationCard();
  }

  lcd.setCursor(0, 0);  // Column, Row
  lcd.print(" Scan Your RFID ");
  lcd.setCursor(0, 1);  // Column, Row
  lcd.print("   Door Locked   ");
}

// Read card UID
bool ReadCard() {

  if (!mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return false;
  }

  String content = "";
  byte letter;

  for (byte i = 0; i < mfrc522.uid.size; i++) {

    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  
  mfrc522.PICC_HaltA();  // Stop reading card

  content.toUpperCase();
  UIDcard = content.substring(1);
  Serial.println("\nReadCardUIDCard: " + UIDcard);
  return true;
}

bool RegisteredCard() {
  
  bool statRegister = false;

  for (int i = 0; i < 10; i++) {
    
    if (registered[i] == UIDcard) {

      statRegister = true;
      Serial.println("Card registered");
      Unlock();   // Unlock the door
      
      break;
    }
  }

  return statRegister;
}

// Unlocks door
void Unlock() {
  
  servo.write(90);
  digitalWrite(LED_G, HIGH);
  lcd.setCursor(0, 1);
  lcd.print("  Door Unlocked ");
  Serial.println("Door Unlocked");

  tone(buzzer, 1000); // Buzzer for unlock
  delay(1000);
  noTone(buzzer);
  Lock();  // Lock the door
}

// Locks door
void Lock() {
  
  servo.write(15);
  delay(100);
  digitalWrite(LED_G, LOW);
  lcd.setCursor(0, 1);
  lcd.print("   Door Locked   ");
  Serial.println("Door Locked");
}

// Register new Card
void NewRegisterationCard() {
  
  bool findCard = false;
  // Waiting to read card
  
  while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    
    Serial.println("Scan Your Card To Register");
    lcd.setCursor(0, 1);
    lcd.print("   To Register  ");
    digitalWrite(LED_R, HIGH);
    delay(50);
    digitalWrite(LED_R, LOW);
    delay(50);
    digitalWrite(LED_G, HIGH);
    delay(50);
    digitalWrite(LED_G, LOW);
    delay(50);
  }

  String content = "";
  byte letter;

  for (byte i = 0; i < mfrc522.uid.size; i++) {

    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  mfrc522.PICC_HaltA();  // Stop reading card

  content.toUpperCase();
  UIDcard = content.substring(1);
  Serial.println("\nUIDCard: " + UIDcard);


  for (int i = 0; i < 10; i++) {

    // If ID already exists
    if (registered[i] == UIDcard) {  // Deleting ID

      findCard = true;  // UIDcard registered in array

      registered[i] = "";
      lcd.setCursor(0, 0);
      lcd.print(" Same ID Found! ");
      Serial.println("Same ID Found!");
      lcd.setCursor(0, 1);
      lcd.print("   ID Deleted!  ");
      Serial.println("ID Deleted!");
      digitalWrite(LED_R, HIGH);  
      delay(2000);
      digitalWrite(LED_R, LOW);  
      break;
    }
  }
  
  // If not found card in array, thus the card not yet registered.
  if (findCard == false) {
    
    for (int i = 0; i < 10; i++) {

      // If an empty index is found
      if (registered[i] == "") {
        registered[i] = UIDcard;
        lcd.setCursor(0, 1);
        lcd.print(" ID Registered! ");
        Serial.println("ID Registered!");
        digitalWrite(LED_G, HIGH);
        delay(2000);
        digitalWrite(LED_G, LOW);
        break;
      }
    }
  }

  for (int i = 0; i < 10; i++) {
    Serial.println(registered[i]);
  }
}
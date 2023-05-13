#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>
#define VRY_PIN A0
#define VRX_PIN A1
#define RST_PIN 9
#define SS_PIN 10

void keuze2();
void keuze1();
void keuze3();

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;  
MFRC522::StatusCode status;

byte buffer[18];
byte block;
byte waarde[64][16];

LiquidCrystal lcd(8, 7, 6, 5, 4, 3, 2);

int xValue = 0;
int yValue = 0;



#define NR_KNOWN_KEYS   8
byte knownKeys[NR_KNOWN_KEYS][MFRC522::MF_KEY_SIZE] =  {
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, // FF FF FF FF FF FF 
    {0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5}, // A0 A1 A2 A3 A4 A5
    {0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5}, // B0 B1 B2 B3 B4 B5
    {0x4d, 0x3a, 0x99, 0xc3, 0x51, 0xdd}, // 4D 3A 99 C3 51 DD
    {0x1a, 0x98, 0x2c, 0x7e, 0x45, 0x9a}, // 1A 98 2C 7E 45 9A
    {0xd3, 0xf7, 0xd3, 0xf7, 0xd3, 0xf7}, // D3 F7 D3 F7 D3 F7
    {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}, // AA BB CC DD EE FF
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // 00 00 00 00 00 00
};

byte upArrow[8] = {
  0b00100,
  0b01110,
  0b11111,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100
};

byte downArrow[8] = {
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b11111,
  0b01110,
  0b00100
};

byte rightArrow[8] = {
  0b00000,
  0b00100,
  0b00110,
  0b11111,
  0b11111,
  0b00110,
  0b00100,
  0b00000
};

void setup() {
    lcd.begin(16, 2);  
    lcd.clear();  
    Serial.begin(9600);

    while (!Serial);            
    SPI.begin();               
    mfrc522.PCD_Init(); 
    delay(4);       
    mfrc522.PCD_DumpVersionToSerial();
    Serial.println(F("Try the most used default keys to print block 0 to 63 of a MIFARE PICC."));
    Serial.println("1.Read card \n2.Write to card");

    lcd.createChar(0, upArrow);
    lcd.createChar(1, downArrow);
    lcd.createChar(2, rightArrow);

    lcd.setCursor(0,0);    
    lcd.print("RFID Card Cloner");
    lcd.setCursor(0,1);
    lcd.print("By M. Baldzhiev"); 
    delay(5000);
    lcd.clear();

    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }
}

int choice = 1;
 
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

void dump_byte_array1(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.write(buffer[i]);
  }
}


bool try_key(MFRC522::MIFARE_Key *key)
{
    bool result = false;
    
    for(byte block = 0; block < 64; block++){
      
    // Serial.println(F("Authenticating using key A..."));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Failed!");
        delay(1000);
        return false;
    }

    byte byteCount = sizeof(buffer);
    status = mfrc522.MIFARE_Read(block, buffer, &byteCount);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Failed!");
        delay(1000);
    }

    else {
        result = true;
        Serial.print(F("Success with key:"));
        dump_byte_array((*key).keyByte, MFRC522::MF_KEY_SIZE);
        Serial.println();
        
        Serial.print(F("Block ")); Serial.print(block); Serial.print(F(":"));
        dump_byte_array1(buffer, 16);
        Serial.println();
        
        for (int p = 0; p < 16; p++)
        {
          waarde [block][p] = buffer[p];
          Serial.print(waarde[block][p]);
          Serial.print(" ");
        }  
      }
    }

    mfrc522.PICC_HaltA();       
    mfrc522.PCD_StopCrypto1();  
    return result;
    start();
}

void loop() {
  start();  
}

void start(){
  lcd.clear();
  lcd.setCursor(0,0);           
  lcd.write(byte(0));
  lcd.print("/");
  lcd.write(byte(1));
  lcd.print(",");
  lcd.write(byte(2));
  lcd.print(" to select:");     
  lcd.setCursor(0,1);  
  while (true){
    Serial.println(choice);

    if(choice == 1)
    {
      lcd.setCursor(0,1);  
      lcd.print("1. Read card"); 
    }

    else if(choice == 2)
    {
      lcd.setCursor(0,1);
      lcd.print("2. Write to card");
    }

    delay(500);
    xValue = analogRead(VRX_PIN);
    yValue = analogRead(VRY_PIN);

    if (yValue >= 800) 
        choice++;

    else if (yValue <= 300)
      choice--;

    if (choice <= 0)
          choice = 2;

    if (choice >= 3)
          choice = 1;

    else if (xValue >= 800) 
    {
      delay(3000);
      if(choice == 1)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Hold card");
        Serial.println("Read the card");
        readFromCard();
      
      }
      
      else if(choice == 2)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("3 chosen");
        Serial.println("Copying the data on to the new card");
        writeToCard();
      }
    }
  }         
}

void readFromCard(){ 
  Serial.println("Insert card...");

    while ( ! mfrc522.PICC_IsNewCardPresent())
     Serial.println("waiting for card...");

    if ( ! mfrc522.PICC_ReadCardSerial())
        return;

    lcd.setCursor(0,1);
    lcd.print("Wait! Reading!");
    Serial.print(F("Card UID:"));
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println();
    Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    Serial.println(mfrc522.PICC_GetTypeName(piccType));
    
    MFRC522::MIFARE_Key key;
    for (byte k = 0; k < NR_KNOWN_KEYS; k++) {
        for (byte i = 0; i < MFRC522::MF_KEY_SIZE; i++) {
            key.keyByte[i] = knownKeys[k][i];
        }

        if (try_key(&key)) {
            break;
        }
    }

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Done!");
    lcd.setCursor(0,1);
    lcd.print("Read card!");
    delay(2000);
    start();
}

void writeToCard(){ 
  lcd.setCursor(0,0);
  lcd.print("Hold card");
  Serial.println("Insert new card...");

  while( ! mfrc522.PICC_IsNewCardPresent())
        Serial.println("waiting for card...");

  if ( ! mfrc522.PICC_ReadCardSerial())
        return;

  lcd.setCursor(0,1);
  lcd.print("Wait! Writing!");
  Serial.print(F("Card UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));
    
  for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
  }

  for(int i = 4; i <= 62; i++){ 
    if(i == 7 || i == 11 || i == 15 || i == 19 || i == 23 || i == 27 || i == 31 || i == 35 || i == 39 || i == 43 || i == 47 || i == 51 || i == 55 || i == 59){
      i++;
    }

    block = i;
    
    Serial.println(F("Authenticating using key A..."));
    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }
    
    Serial.println(F("Authenticating again using key B..."));
    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, block, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }
    
    Serial.print(F("Writing data into block ")); 
    Serial.print(block);
    Serial.println("\n");
          
    dump_byte_array(waarde[block], 16); 
        
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(block, waarde[block], 16);
    if (status != MFRC522::STATUS_OK) {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Write failed!");
        delay(2000);
      } 
     Serial.println("\n");
  }

  mfrc522.PICC_HaltA(); 
  mfrc522.PCD_StopCrypto1(); 
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Done!");
  lcd.setCursor(0,1);
  lcd.print("Wrote to card!");
  delay(2000);
  start();
}
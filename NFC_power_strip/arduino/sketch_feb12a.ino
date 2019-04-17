#include <HTTPClient.h>
#include <MFRC522.h> //library responsible for communicating with the module RFID-RC522
#include <SPI.h> //library responsible for communicating of SPI bus
#include <WiFi.h>

#define SS_PIN    21
#define RST_PIN   22
#define SIZE_BUFFER     18
#define MASTER_KEY "098A4A73"

MFRC522::MIFARE_Key key; //used in authentication
MFRC522::StatusCode status; //authentication return status code
MFRC522 mfrc522(SS_PIN, RST_PIN); // Defined pins to module RC522

int httpResponseCode;
const char* ssid = "poc";
const char* password = "pocpocpoc";

void setup() 
{
    pinMode(32, OUTPUT);
    pinMode(33, OUTPUT);
    pinMode(25, OUTPUT);
    pinMode(2, OUTPUT);
    Serial.begin(9600); 
    WiFi.begin(ssid, password); 
    SPI.begin();
    mfrc522.PCD_Init();
    Serial.println("initialised!");
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

void loop()
{
    if (WiFi.status() != WL_CONNECTED) {
        digitalWrite(25, HIGH);
    } else {
        digitalWrite(25, LOW);
        if (!mfrc522.PICC_IsNewCardPresent()) 
            return;
        if (!mfrc522.PICC_ReadCardSerial()) 
            return;
        HTTPClient http;
        http.begin("http://192.168.0.130:5000/");

        for (int a = 0; a <= mfrc522.uid.size; a += 1)
            Serial.print(mfrc522.uid.uidByte[a]);
        httpResponseCode = http.POST((char *) mfrc522.uid.uidByte);
        char buffer[mfrc522.uid.size];
        array_to_string(mfrc522.uid.uidByte, mfrc522.uid.size, buffer);

        
        Serial.print("\nthis key equals ");
        Serial.print(buffer);
        Serial.print("\nmaster key equals ");
        Serial.print(MASTER_KEY);
        Serial.print("\n\n");
        
        if (strcmp(buffer, "098A4A73") == 0) {
            Serial.print("this is the master key\n");
            getState(200);
        } else
          Serial.print("this is not the master key\n");
        if (httpResponseCode > 0){
            String response = http.getString();
            getState(httpResponseCode);
        } else {
            Serial.print("Error on sending POST: ");
            Serial.println(httpResponseCode);
        }
        for (int a = 0; a <= mfrc522.uid.size; a += 1)
            mfrc522.uid.uidByte[a] = 0;
        http.end();
    }    
}

void getState(int code)
{
    Serial.println(code);
    if (code == 200) {
        digitalWrite(32, HIGH);
        digitalWrite(2, HIGH);
        Serial.println("open\n");
    } else if (code == 404) {
        digitalWrite(33, HIGH);
    }
    delay(1000);
    digitalWrite(26, LOW);
    digitalWrite(32, LOW);
    digitalWrite(33, LOW);
}


/*******************************************************************
    Simple test to verify the ESP32 can talk to the NFC Tag

    It will read the contents of the tag and print them to the
    serial monitor

    Parts:
    ESP32 D1 Mini stlye Dev board
        - Aliexpress*: http://s.click.aliexpress.com/e/C6ds4my
        - Amazon.com*: https://amzn.to/3gArkAY

    PN532 NFC Module
        - Aliexpress*: https://s.click.aliexpress.com/e/_d7p8MoK
        - Amazon.co.uk*: https://amzn.to/2DnfvzY


    Written by Brian Lough
 *******************************************************************/
// ----------------------------
// Standard Libraries for ESP32
// ----------------------------

#include <SPI.h>

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <NfcAdapter.h>
#include <PN532/PN532/PN532.h>
#include <PN532/PN532_SPI/PN532_SPI.h>
// Libraries for interfacing with the NFC Reader

// I modified the library to so it wouldn't lock if the NFC
// reader wasn't present

// Install from Github (one install covers the 3 includes)
// https://github.com/witnessmenow/Seeed_Arduino_NFC

// -------------------------------------
// -------   NFC Pin Config   ------
// -------------------------------------

//These should be the default SPI pins, but we'll call them out specficially
#define NFC_SCLK 18
#define NFC_MISO 19
#define NFC_MOSI 23
#define NFC_SS 5


// -------------------------------------
// -------   NFC Config   ------
// -------------------------------------

SPIClass spi = SPIClass(HSPI);
PN532_SPI pn532spi(SPI, NFC_SS);
NfcAdapter nfc = NfcAdapter(pn532spi);

unsigned long delayBetweenNfcReads = 200; // Time between NFC reads (.2 seconds)
unsigned long nfcDueTime;               //time when NFC read is due


void setup(void) {
  Serial.begin(115200);
  Serial.println("Simple NFC Test");
  nfc.begin();
  if (nfc.fail) {
    Serial.println("NFC reader not found");
    while (true)
    {
      // Loop here, there is no NFC reader
      delay(1);
    }
  } else {
    Serial.println("NFC reader found!");
  }
}

bool processNfcStringData(String stringData) {
  int firstDelim = stringData.indexOf("|");
  if (firstDelim > -1) {
    int secondDelim = stringData.indexOf("|", firstDelim +1);
    if (secondDelim > -1) {
      String name = stringData.substring(firstDelim + 1, secondDelim);
      String role = stringData.substring(secondDelim + 1);

      Serial.println("----------------");
      Serial.print("Name: ");
      Serial.println(name);
      Serial.print("Role: ");
      Serial.println(role);
      Serial.println("----------------");
      return true;
    }
    
  }

  Serial.println("String not in expected format: |Name|Role");
  return false;

}

bool processNfcTag() {

  // Check if there is an NFC Tag
  if (nfc.tagPresent()) {
    NfcTag tag = nfc.read();
    if (tag.isFormatted && tag.hasNdefMessage()) {

      NdefMessage message = tag.getNdefMessage();
      int recordCount = message.getRecordCount();

      Serial.print("\nThis NFC Tag contains an NDEF Message with ");
      Serial.print(message.getRecordCount());
      Serial.print(" NDEF Record");
      if (recordCount != 1) {
        Serial.print("s");
      }
      Serial.println(".");


      if (recordCount > 0) {
        for (int i = 0; i < recordCount; i++) {
          Serial.print("\nNDEF Record "); Serial.println(i + 1);
          NdefRecord record = message.getRecord(i);

          Serial.print("  TNF: "); Serial.println(record.getTnf());
          Serial.print("  Type: "); Serial.println(record.getType()); // will be "" for TNF_EMPTY

          // The TNF and Type should be used to determine how your application processes the payload
          // There's no generic processing for the payload, it's returned as a byte[]
          int payloadLength = record.getPayloadLength();
          if (payloadLength > 0) {
            byte payload[payloadLength];
            record.getPayload(payload);

            // Print the Hex and Printable Characters
            Serial.print("  Payload (HEX): ");
            PrintHexChar(payload, payloadLength);

            // id is probably blank and will return ""
            String uid = record.getId();
            if (uid != "") {
              Serial.print("  ID: "); Serial.println(uid);
            }

            // Force the data into a String (might work depending on the content)
            // Real code should use smarter processing
            char payloadAsString[payloadLength + 1];
            int numChars = 0;
            for (int c = 0; c < payloadLength; c++) {
              if ((char)payload[c] != '\0') {
                payloadAsString[numChars] = (char)payload[c];
                numChars++;
              }
            }

            payloadAsString[numChars] = '\0';
            Serial.print("  Payload (String): ");
            Serial.println(payloadAsString);

            
            return processNfcStringData(payloadAsString);
          }
        }
      }
    }
  }
  return false;
}

void loop(void) {
  if (millis() > nfcDueTime)
  {
    if (processNfcTag()) {
      Serial.println("Succesful Read - Back to loop:");
      nfcDueTime = millis() + 5000; // 5 second cool down on NFC tag if succesful
    } else {
      nfcDueTime = millis() + delayBetweenNfcReads;
      //Serial.println("Failed - Back to loop:");
    }
  }
}

#include <PN532_HSU.h>
#include <PN532.h>
#include <PN532Extended.h>
#include <Desfire.h>

// Use Serial2 of ESP32
HardwareSerial PN532Serial(2);

// Serial interface
PN532_HSU pn532hsu(PN532Serial);

// Original PN532 library for initialization
PN532 nfc(pn532hsu);

// Extended library which allows card extensions
PN532Extended nfcext(pn532hsu);

// Maximum RFID targets to be detected at once (PN532 limit is 2)
#define MAX_RFID_TARGETS 1

// Desfire key for authentication
const DesfireKey key = CreateDesfireKeyAES({ 0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xA0, 0xB0, 0xB0, 0xA0, 0x90, 0x80 });

// Set to 1, to change key after authentication
#define CHANGE_KEY 0
DesfireKey key_new = CreateDesfireKeyAES({ 0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xA0, 0xB0, 0xB0, 0xA0, 0x90, 0x80 });

void setup() {
  Serial.begin(115200);
  Serial.println("Hello!");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0xFF);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
    
  Serial.println("Waiting for an ISO14443A card");
}

// Helper function to print hex array
void PrintBin(const BinaryData& in)
{
  for (auto data : in)
  {
    Serial.print(data, HEX);
    Serial.print(' ');
  }
  Serial.print('\n');
}

void loop() {
  // Finds nearby ISO14443 Type A tags
  InListPassiveTargetResponse resp = nfcext.InListPassiveTarget(1, BRTY_106KBPS_TYPE_A);

  // For parsing response data
  ByteBuffer buf(resp.TgData);

  for (int i = 0; i < resp.NbTg; ++i)
  {
    // Parse as ISO14443 Type A target
    PN532Packets::TargetDataTypeA tgdata;
    buf >> tgdata;

    Serial.print("ATQA: ");
    Serial.print(tgdata.ATQA[0], HEX);
    Serial.print(" ");
    Serial.println(tgdata.ATQA[1], HEX);

    Serial.print("SAK: ");
    Serial.println(tgdata.SAK, HEX);

    Serial.print("UID length: ");
    Serial.println(tgdata.UID.size());
    Serial.print("UID: ");
    PrintBin(tgdata.UID);

    Serial.print("ATS length: ");
    Serial.println(tgdata.ATS.size());
    Serial.print("ATS: ");
    PrintBin(tgdata.ATS);

    // Identify card type
    CardType_t type = PN532Extended::IdentifyTypeACard(tgdata);
    Serial.print("Type: ");
    Serial.println(type);

    if (type == CARD_TYPE_MIFARE_DESFIRE)
    {
      // Creates a direct tag interface for Desfire library
      TagInterface tif = nfcext.CreateTagInterface(tgdata.Tg);
      Desfire desfire(tif);

      // Connects card with ISO7816 standard.
      // Desfire EV1 cards work without this, but ISO standards require it.
      if (!desfire.Connect())
      {
        Serial.println("Desfire connect failed.");
        continue;
      }
      else
        Serial.println("Desfire connect successful!");

      // Authenticates key 0 (master key)
      if (desfire.Authenticate(0, key))
        Serial.println("Desfire Auth SUCCESS!");
      else
        Serial.println("Desfire Auth FAILED!");

      if (CHANGE_KEY)
      {
        if (desfire.ChangeKey(0, key_new))
          Serial.println("ChangeKey SUCCESS!");
        else
          Serial.println("ChangeKey FAILED!");
      }
    }
  }
}
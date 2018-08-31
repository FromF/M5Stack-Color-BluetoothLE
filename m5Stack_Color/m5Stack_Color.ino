#include <M5Stack.h>
// Bluetooth LE
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

///////////////
bool updateColor;
String lastColor;

// the setup routine runs once when M5Stack starts up
void setup(){

  // Initialize the M5Stack object
  M5.begin();

  M5.Lcd.print("Setup....");
  
  initBLE();
  initLCDcolor();
  
  M5.Lcd.println("Done");
}

// the loop routine runs over and over again forever
void loop() {
  M5.update();
  loopBLE();
  loopLCDcolor();
}

///////////////////
// Bluetooth LE  //
///////////////////
BLEServer *pServer = NULL;
BLECharacteristic * pNotifyCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

#define LOCAL_NAME                  "M5Stack-Color"
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID                "e5a1c9a8-ab93-11e8-98d0-529269fb1459"
#define CHARACTERISTIC_UUID_RX      "e5a1cda4-ab93-11e8-98d0-529269fb1459"
#define CHARACTERISTIC_UUID_NOTIFY  "e5a1d146-ab93-11e8-98d0-529269fb1459"

// Bluetooth LE Change Connect State
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

// Bluetooth LE Recive
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0) {
        String cmd = String(rxValue.c_str());
        Serial.print("Received Value: ");
        Serial.println(cmd);
        if (cmd == "RED")
        {
          // RED
          lastColor = "RED";
          updateColor = true;
        }
        if (cmd == "YELLOW")
        {
          // YELLOW
          lastColor = "YELLOW";
          updateColor = true;
        }
        if (cmd == "BLUE")
        {
          // BLUE
          lastColor = "BLUE";
          updateColor = true;
        }
      }
    }
};

// Bluetooth LE initialize
void initBLE() {
  // Create the BLE Device
  BLEDevice::init(LOCAL_NAME);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pNotifyCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_NOTIFY,
                        BLECharacteristic::PROPERTY_NOTIFY
                        );
  
  pNotifyCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                       CHARACTERISTIC_UUID_RX,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
}

// Bluetooth LE loop
void loopBLE() {
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("startAdvertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}

///////////////
// LCD Color //
///////////////
void initLCDcolor() {
  lastColor = "NONE";
}

void loopLCDcolor() {
  if (M5.BtnA.wasPressed())
  {
    lastColor = "RED";
    updateColor = true;
  }
  if (M5.BtnB.wasPressed())
  {
    lastColor = "YELLOW";
    updateColor = true;
  }
  if (M5.BtnC.wasPressed())
  {
    lastColor = "BLUE";
    updateColor = true;
  }
  
  if (updateColor) {
    if (lastColor == "RED")
    {
      // RED
      M5.Lcd.fillScreen(RED);
    }
    if (lastColor == "YELLOW")
    {
      // YELLOW
      M5.Lcd.fillScreen(YELLOW);
    }
    if (lastColor == "BLUE")
    {
      // BLUE
      M5.Lcd.fillScreen(BLUE);
    }
    if (deviceConnected) {
      char sendMessage[10];
      lastColor.toCharArray(sendMessage, 10);
      pNotifyCharacteristic->setValue(sendMessage);
      pNotifyCharacteristic->notify();
    }
    updateColor = false;
  }
}




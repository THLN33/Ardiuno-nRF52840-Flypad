
// Dependent on Bluefruit52Lib library from Adafruit. This is included in the nRFMicro-like-Boards BCP (by pdcook)
#include <Arduino.h>
#include <bluefruit.h>



#define _LED_AUTO_



// Service et Caractéristique Parrot 

// UUID Service (16 octets)
uint8_t const parrot_service_uuid_rev[] = { 0x8B, 0x87, 0x46, 0x60, 0x7F, 0x0C, 0xE2, 0xA2, 0xD4, 0x44, 0x44, 0x43, 0x00, 0xFA, 0x35, 0x9E };
// UUID Caractéristique (on change juste le 0x00 en 0x01)
uint8_t const parrot_char_uuid_rev[]    = { 0x8B, 0x87, 0x46, 0x60, 0x7F, 0x0C, 0xE2, 0xA2, 0xD4, 0x44, 0x44, 0x43, 0x01, 0xFA, 0x35, 0x9E };

BLEUuid const service_uuid(parrot_service_uuid_rev);
BLEUuid const char_uuid(parrot_char_uuid_rev);

BLEClientBas  clientBas;  // battery client
BLEClientDis  clientDis;  // device information client
BLEClientService parrotService(service_uuid);
BLEClientCharacteristic parrotData(char_uuid);

// --- Fonctions de rappel (Callbacks) ---


// --- Fonction de réception des données ---
void data_callback(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len) {
  Serial.print("Axes/Boutons : ");
  for(int i=0; i<len; i++) {
    Serial.printf("%02X ", data[i]);
  }
  Serial.println();
}

void list_services(uint16_t conn_handle) {
  Serial.println("\n--- LISTE DES SERVICES DISPONIBLES ---");
  
  // On utilise une boucle pour parcourir les services
  // 0x0001 est le handle de départ habituel
  uint16_t current_handle = 0x0001;
  
  Serial.println("Exploration de la table GATT...");  
  Bluefruit.Discovery.begin(); 


  Serial.println("Exploration du service...");
  Bluefruit.printInfo(); 

  BLEClientService gap(0x1800);
  if(gap.discover(conn_handle)) {
    Serial.println("Le service 0x1800 est visible.");
  }

  Serial.print("Dicovering Battery ... ");
  if ( clientBas.discover(conn_handle) ) {
    Serial.println("Found it");
    Serial.print("Battery level: ");
    Serial.print(clientBas.read());
    Serial.println("%");
  } else {
    Serial.println("Found NONE");
  }

  Serial.print("Dicovering Device Information ... ");
  if ( clientDis.discover(conn_handle) ) {
    Serial.println("Found it");
    char buffer[32+1];
    
    // read and print out Manufacturer
    memset(buffer, 0, sizeof(buffer));
    if ( clientDis.getManufacturer(buffer, sizeof(buffer)) ) {
      Serial.print("Manufacturer: ");
      Serial.println(buffer);
    }

    // read and print out Model Number
    memset(buffer, 0, sizeof(buffer));
    if ( clientDis.getModel(buffer, sizeof(buffer)) ) {
      Serial.print("Model: ");
      Serial.println(buffer);
    }
  } else {
    Serial.println("Found NONE");
  }

  Serial.println("Recherche du service Parrot...");
  if ( parrotService.discover(conn_handle) ) {
    Serial.println("Service OK ! Recherche Data...");

    parrotData.begin();

    if ( parrotData.discover() ) {
      parrotData.setNotifyCallback(data_callback);
      Serial.println("Caracteristique OK ! Activation des joysticks...");
      parrotData.enableNotify();      
    }
  } else {
    Serial.println("Service non trouve.");
  }    
  Serial.println();
}

void connect_callback(uint16_t conn_handle) {
  Serial.println(">>> CONNECTÉ AU FLYPAD ! <<<");
  
  BLEConnection* conn = Bluefruit.Connection(conn_handle);
  char peer_name[32] = { 0 };
  if (conn) {
    conn->getPeerName(peer_name, sizeof(peer_name));
    Serial.printf("Nom : %s\n", peer_name);
  }
  list_services(conn_handle);
}




void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  Serial.printf("Déconnecté (Raison: 0x%02X). Relance du scan...\n", reason);
  Bluefruit.Scanner.start(0);
}


void scan_callback(ble_gap_evt_adv_report_t* report) {
  if ( Bluefruit.Scanner.checkReportForUuid(report, parrot_service_uuid_rev) ) {
    Serial.println("Flypad identifié ! Connexion en cours...");
    Bluefruit.Scanner.stop();
    Bluefruit.Central.connect(report);
  } else {
    // For Softdevice v6: after received a report, scanner will be paused
    // We need to call Scanner resume() to continue scanning
    Bluefruit.Scanner.resume();
  }
}





void setup_BLE(void) {

  Serial.println("--------------------------------------------------");
  Serial.println("Lien Direct vers Flypad");
  Serial.println("--------------------------------------------------");

  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central = 1
  // SRAM usage required by SoftDevice will increase dramatically with number of connections
  Bluefruit.begin(0, 1);

  Bluefruit.setTxPower(4);
  
  // Configure Battery client
  clientBas.begin();    
  // Configure DIS client
  clientDis.begin();  
  // Configure Parrot Service client
  parrotService.begin();

  // Configuration des fonctions de rappel
  Bluefruit.Central.setConnectCallback(connect_callback);
  Bluefruit.Central.setDisconnectCallback(disconnect_callback);  
#ifdef _LED_AUTO_
  /* Set the LED interval for blinky pattern on RED LED */
  Bluefruit.setConnLedInterval(100);
#else
  // 1. Désactive le clignotement automatique sur la pin Power
  Bluefruit.autoConnLed(false);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
#endif

  // 2. Initialise P0.13 comme sortie et active le courant
  //pinMode(PIN_EXT_VCC, OUTPUT);
  //digitalWrite(PIN_EXT_VCC, LOW);

  /* Start Central Scanning
   * - Enable auto scan if disconnected
   * - Filter out packet with a min rssi
   * - Interval = 100 ms, window = 80 ms
   * - Use active scan (used to retrieve the optional scan response adv packet)
   * - Start(0) = will scan forever since no timeout is given
   */  
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);    
  Bluefruit.Scanner.filterRssi(-100);
  Bluefruit.Scanner.setInterval(160, 128);      // Valeurs en unités de 0.625ms (160 = 100ms)
  Bluefruit.Scanner.useActiveScan(true);        // Request scan response data

  Bluefruit.Scanner.start(0);

  Serial.println("Scan en cours...");;

}


void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  // Bootloader version
  Serial.printf("Bootloader version: %s\n", getBootloaderVersion());

  setup_BLE();   
}

void loop() {

  if ( Bluefruit.Central.connected() ) {
    // Not discovered yet
    if ( parrotService.discovered() ) {
      // Discovered means in working state
      //Serial.printf("Parrot Service discovered\n");
      delay(500);
    }
  }
}

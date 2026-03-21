#include <SPI.h>
#include <LoRa.h>
#include <HardwareSerial.h>

String receivedData = "";

// Circular buffer size
const int tam_cir_buffer = 15;

// Pins
const int pwrsim7020 = 13;              // Power control pin for SIM7020E
const int pin_cam_signal_sleep = 36;    // Interrupt pin: CAM notifies it entered deep sleep
const int pin_pedir_foto = 23;          // Pin to wake the ESP32-CAM and request a picture

// States
bool flag_data = false;                 // True when a LoRa message has been received
bool cam_sleep = false;                 // True when CAM triggers sleep interrupt

// Deep sleep timing for the camera
long sleep_time = 1000 * 60 * 60;       // 1 hour
long t_inicio_sleep = 0;                // Timestamp of last wake-up

// Circular buffer to store last received values
int circularBuffer[tam_cir_buffer];
int pos_buffer = 0;

// Hardware serial for SIM7020E (UART2)
HardwareSerial sim7020(2);

// SIM7020E UART pins
#define SIM7020_RX_PIN 16
#define SIM7020_TX_PIN 17

// LoRa transceiver pins (SX1276)
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

void setup(){
  delay(1000);

  Serial.begin(115200);  
  sim7020.begin(115200, SERIAL_8N1, SIM7020_RX_PIN, SIM7020_TX_PIN); // Start SIM7020 UART

  // Configure pins
  pinMode(pwrsim7020, OUTPUT);
  pinMode(pin_pedir_foto, OUTPUT);
  pinMode(pin_cam_signal_sleep, INPUT);

  // Interrupt: CAM notifies when it enters deep sleep
  attachInterrupt(digitalPinToInterrupt(pin_cam_signal_sleep), cam_a_sleep, RISING);

  // Initialize SPI for LoRa
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);

  // Start LoRa at 868 MHz
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa system Initializing OK!");

  // Initialize circular buffer with -1 (invalid value)
  for (int i = 0; i < tam_cir_buffer; ++i) {
    circularBuffer[i] = -1;
  }

  // Set LoRa to receive mode
  Serial.println("Setting LoRa to receive mode");
  LoRa.onReceive(onReceive);
  LoRa.receive();
}

void loop(){

  // If a LoRa message was received
  if (flag_data){
    Serial.println("Message received!");
    power_on_sim7020();     // Turn on SIM7020E
    delay(800);
    flag_data = false;
    processReceivedData();  // Convert, store and upload the data
  }

  // Wake the CAM every hour to take a picture
  if (millis() - t_inicio_sleep > sleep_time){
    digitalWrite(pin_pedir_foto, HIGH);   // Wake-up pulse
    delay(350);
    digitalWrite(pin_pedir_foto, LOW);
    Serial.println("Waking up the CAM to take a picture!");
    t_inicio_sleep = millis();
  }

  // If CAM triggered sleep interrupt
  if (cam_sleep){
    Serial.println("The CAM has gone to sleep!");
    cam_sleep = false;
  }
}

/*
****************************************************************************************************************************************
  FUNCTION: Upload data to ThingSpeak using SIM7020E HTTP commands
****************************************************************************************************************************************
*/

void upload_data_thingspeak(int dato_){
  sendATCommand("AT");  // Basic check
  sendATCommand("AT+CHTTPCREATE=\"http://api.thingspeak.com/update.json\""); // Create HTTP session
  sendATCommand("AT+CHTTPCON=0"); // Connect session

  String dato_hex = int2hex(dato_);  // Convert number to ASCII HEX
  Serial.println("Received number in hex: " + dato_hex);

  // Send JSON payload encoded in HEX
  sendATCommand("AT+CHTTPSEND=0,1,\"/\",436f6e6e656374696f6e3a204b6565702d416c6976650a557365722d4167656e743a2053494d434f4d5f4d4f44554c450d0a,\"application/json\",7B226170695F6B6579223A222A222C226669656C6431223A"+ dato_hex +"7d");

  sendATCommand("AT+CHTTPDISCON=0");  // Disconnect
  sendATCommand("AT+CHTTPDESTROY=0"); // Destroy session
}

/*
****************************************************************************************************************************************
  FUNCTION: Update circular buffer index
****************************************************************************************************************************************
*/

void actualizar_pos_cir_buffer(){
  pos_buffer++;
  if (pos_buffer >= tam_cir_buffer)
    pos_buffer = 0;
}

/*
****************************************************************************************************************************************
  CALLBACK: Executed when a LoRa packet is received
****************************************************************************************************************************************
*/

void onReceive(int packetSize) {
  receivedData = "";

  // Read all bytes from LoRa buffer
  while (LoRa.available()) {
    receivedData += (char)LoRa.read();
  }

  flag_data = true;  // Notify main loop
}

/*
****************************************************************************************************************************************
  FUNCTION: Process received LoRa data
****************************************************************************************************************************************
*/

void processReceivedData(){
  int dato_int;
  String myString = "";

  // Copy received string
  for (char c : receivedData) {
    myString += c;
  }

  Serial.println("Received number: " + myString);

  // Convert to integer
  dato_int = myString.toInt();
  myString = "";

  // Store in circular buffer
  circularBuffer[pos_buffer] = dato_int;

  // Upload to ThingSpeak
  upload_data_thingspeak(dato_int);
  
  actualizar_pos_cir_buffer();  // Move buffer index
  power_off_sim7020();          // Turn off SIM7020E to save power
}

/*
****************************************************************************************************************************************
  FUNCTION: Send AT command to SIM7020E and wait for "OK"
****************************************************************************************************************************************
*/

void sendATCommand(String command) {
  delay(10);
  sim7020.println(command);  // Send command to SIM7020E

  int t = millis();
  int t_max = 10000;
  String response = "";

  // Wait for response or timeout
  while (millis() - t < t_max){
    delay(20);
    
    if (sim7020.available()) {
      char c = sim7020.read();
      response += c;
      delay(10);
    }

    if (response.indexOf("OK") != -1) {
      break;  // Stop when OK is received
    }
  }

  Serial.println(response);  // Print module response
}

/*
****************************************************************************************************************************************
  INTERRUPT: Called when CAM enters deep sleep
****************************************************************************************************************************************
*/

void cam_a_sleep(){
  cam_sleep = true;
  t_inicio_sleep = millis();  // Reset sleep timer
}

/*
****************************************************************************************************************************************
  FUNCTION: Power ON SIM7020E using pulse sequence
****************************************************************************************************************************************
*/

void power_on_sim7020(){
  digitalWrite(pwrsim7020, LOW);
  delay(100);
  digitalWrite(pwrsim7020, HIGH);
  Serial.println("Powering on SIM7020");
  delay(500);
  digitalWrite(pwrsim7020, LOW);
  delay(1000);
}

/*
****************************************************************************************************************************************
  FUNCTION: Power OFF SIM7020E using pulse sequence
****************************************************************************************************************************************
*/

void power_off_sim7020(){
  digitalWrite(pwrsim7020, LOW);
  delay(100);
  digitalWrite(pwrsim7020, HIGH);
  Serial.println("Powering off SIM7020");
  delay(1000);
  digitalWrite(pwrsim7020, LOW);
}

/*
****************************************************************************************************************************************
  FUNCTION: Convert integer to ASCII HEX for ThingSpeak
****************************************************************************************************************************************
*/

String int2hex(int n){
  String str = String(n);
  String str_valido = ""; // Must start with 3 (ASCII hex prefix)

  for (int i = 0; i < str.length(); i++) {
    if (str.charAt(i) == '.') {
      str_valido += "2e";  // ASCII hex for '.'
    } else {
      str_valido += 3;     // Prefix for ASCII hex digits
      str_valido += str.charAt(i);
    }
  }

  return str_valido;
}

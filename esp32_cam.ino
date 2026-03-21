#include "WiFi.h"
#include "esp_camera.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"
#include <LittleFS.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include "esp_sleep.h"

#include "cam_config.h"
#include "firebase_datos.h"

const char* ssid = "******";
const char* password = "*******";

int intentos_subir_foto = 0, ledpin = 4, go_sleep = 12;
String filePath = "";
bool foto_subida = false;
bool debug = true;

void setup() {
  delay(500);

  if (debug){
    Serial.begin(115200);
  }

  pinMode(ledpin, OUTPUT);
  pinMode(go_sleep, OUTPUT);

  Serial.println("Hola");

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 1);

  if(!LittleFS.begin()){
    Serial.println("LittleFS Mount Failed");
  }

  LittleFS.format(); // Limpio LittleFS

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  initCamera();
}

void loop(){
  // Toma una foto y guárdala
  capturePhotoSaveLittleFS();
  Serial.println(String("Nombre del archivo: ") + String("/data") + filePath);

  int n = 0;
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED && n < 15){
    n++;
    Serial.print("*");
    delay(1000);
  }
  Serial.println("");

  if (WiFi.status() == WL_CONNECTED){
    firebase();
  }

  while (intentos_subir_foto < 3 && !foto_subida && WiFi.status() == WL_CONNECTED){
    if (Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID, filePath, mem_storage_type_flash, "/data"+filePath, "image/jpeg")){
      Serial.println("Foto Subida a Firebase!");
      foto_subida = true;
    }
    else intentos_subir_foto++;
  }

  if (intentos_subir_foto == 3 && !foto_subida){
    Serial.println("No se ha podido subir la foto!!!!");
  }

  Serial.println("Vamos al modo sleep!");

  digitalWrite(go_sleep, HIGH);
  delay(400);
  digitalWrite(go_sleep, LOW);

  digitalWrite(ledpin, HIGH);
  delay(300);
  digitalWrite(ledpin, LOW);
  delay(200);
  digitalWrite(ledpin, HIGH);
  delay(300);
  digitalWrite(ledpin, LOW);
  delay(200);
  digitalWrite(ledpin, HIGH);
  delay(300);
  digitalWrite(ledpin, LOW);
  delay(200);

  esp_deep_sleep_start();
}

//Firebase
void firebase(){
  // Api Key
  configF.api_key = API_KEY;

  // Credenciales de incio de sesión
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Función callback para el token de generación
  //configF.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&configF, &auth);
  //Firebase.reconnectWiFi(true);
  Serial.println("Firebase configurado!");
}

// Capture Photo and Save it to LittleFS
void capturePhotoSaveLittleFS( void ) {
  // Descartar primeras imágenes debido a la mala calidad
  camera_fb_t *fb = NULL;
  // Omitir los primeros 3 fotogramas (aumentar o disminuir según sea necesario).

  for (int i = 0; i < 4; i++) {
    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);
    fb = NULL;
  }
  
  // Tomar una nueva foto
  fb = NULL;  
  digitalWrite(ledpin, HIGH);
  fb = esp_camera_fb_get();
  delay(300);
  digitalWrite(ledpin, LOW);
  if(!fb) {
    Serial.println("Fallo al capturar la imagen");
    delay(1000);
  }

  else Serial.println("Imagen capturada correctamente!");

  // Antes de crear un nuevo archivo, compruebo si existe ya uno con el path: /foto1.jpeg . Si existe entonces /foto +1 .jpeg, y vuelvo a comprobar, y así
  // Con el LittleFS.open() se crea el archivo nuevo, en la ruta que especifique, en el modo que desee (write / read)
  // Construct the file path for the photo
  filePath = generateRandomFilepath();
  Serial.println(filePath);

  delay(40);

  // Once the name is assigned, we'll create a LittleFS file where saving it
  File file_littlefs = LittleFS.open(filePath, "w");

  // Insertar los datos en el archivo de la foto
  if (!file_littlefs) {
    Serial.println("Error al abrir el archivo en modo escritura");
  }
  else {
    file_littlefs.write(fb->buf, fb->len); // Carga útil (imagen), longitud de la carga útil
    Serial.println("Imagen guardada en LittleFS!");
  }

  // Cerrar el archivo
  file_littlefs.close();

  esp_camera_fb_return(fb);
}

String generateRandomFilepath() {
    String filepath_ = "/foto_";
    filepath_ += String(random(1000));  // Añade un número aleatorio de 0 a 999
    filepath_ += "_";
    filepath_ += String(millis());  // Añade el tiempo actual en milisegundos
    filepath_ += ".jpeg";  // Extensión de archivo para una foto JPEG
    return filepath_;
}


//Define Firebase Data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;

// Insert Firebase project API Key
#define API_KEY "******************************"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "*************"
#define USER_PASSWORD "******************"

// Insert Firebase storage bucket ID e.g bucket-name.appspot.com
#define STORAGE_BUCKET_ID "****************"
// For example:
//#define STORAGE_BUCKET_ID "esp-iot-app.appspot.com"

// Photo File Name to save in LittleFS
#define BUCKET_PHOTO String("/data/") + String("foto_") + String(n) + String(".jpeg")

// https://firebasestorage.googleapis.com/v0/b/new-esp32-cam.appspot.com/o/data%2Fejemplo.jpg

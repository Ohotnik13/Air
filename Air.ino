//**************************************//
//     Погода и качество воздуха        //
const char ver[] = "V0.2"; //16.02.2020 //
//          Roman Kotelnikov            //
//**************************************//

#define O true     // Режим отладки

// Libraries for SD card
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#define SD_CS 5
#define DATALOG "/datalog.txt"
bool sdflag = true; //Режим работы с sd
bool fmsd;  //Режим мигания
int sdled = 128;
int totalKBytes;
int usedKBytes;
 

// CJMCU-811V1
//https://components101.com/sensors/ccs811-air-quality-gas-sensor-module
#include <Wire.h>    // I2C library
#include "ccs811.h"  // CCS811 library
CCS811 ccs811(17); // nWAKE

uint16_t eco2=0;
uint16_t  etvoc=0;
uint16_t  peco2=0;
uint16_t  petvoc=0;
//Суммарное значение (для расчета среднего)
unsigned long seco2 = 0; 
unsigned long setvoc = 0;
//Количество измерений
unsigned long n = 0;
//Максимальное значение (для расчета процента)
uint16_t meco2 = 8192;
uint16_t metvoc = 1187;
char str[20];

#include "BMP280.h"
#include "Wire.h"
BMP280 bmp;
double T =0;
float H =0;
double P =0;

int Pn[] ={725,725,725,725,725,725,725,725,725,725,725,725,725,725,725,725,725,725,725,725,725,725};
unsigned long delayTime;
unsigned long  zamer = 0;                         //Запомненный момент времени

#include "Adafruit_SHT31.h"
Adafruit_SHT31 sht31 = Adafruit_SHT31();

#include <EEPROM.h>
//Используемые адреса
// 0,1 - p[0], 42-43 - p[21] (предыдущие замеры памяти 
#define SDFLAG_EM   45 // Режим записи sd
#define IN_EM       47 // Режим вывода на индикатор

#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Время
#include <TimeLib.h>
time_t t;
byte ri = 0; //Режим индикатора
int sn = 0; //Следующий срок наблюдения (час наблюдения)

////////////////////////////////////////////// 
//        RemoteXY include library          // 
////////////////////////////////////////////// 

// определение режима соединения и подключение библиотеки RemoteXY  
#define REMOTEXY_MODE__ESP32CORE_WIFI_CLOUD
#include <WiFi.h> 
#include <RemoteXY.h> 

// настройки соединения  
const char* REMOTEXY_WIFI_SSID=""; 
const char* REMOTEXY_WIFI_PASSWORD ="";
char ssidStr[30];
char passwStr[30];

#define REMOTEXY_CLOUD_SERVER "cloud.remotexy.com" 
#define REMOTEXY_CLOUD_PORT 6376 
#define REMOTEXY_CLOUD_TOKEN "8397e9701fd338f019c7006b3ea11380" 


// конфигурация интерфейса   
#pragma pack(push, 1) 
uint8_t RemoteXY_CONF[] =
{ 255,2,0,105,0,206,4,8,227,6,
66,132,1,12,46,36,8,8,45,35,
1,23,24,129,0,43,49,5,3,52,
42,5,3,1,2,43,53,48,0,129,
0,8,5,32,5,15,1,32,5,1,
17,208,162,208,181,208,188,208,191,208,
181,209,128,208,176,209,130,209,131,209,
128,208,176,0,129,0,61,5,27,5,
17,44,27,5,1,17,208,146,208,187,
208,176,208,182,208,189,208,190,209,129,
209,130,209,140,0,66,132,52,12,46,
36,8,50,45,35,1,23,24,67,1,
62,44,25,6,18,80,24,6,1,23,
26,6,129,0,94,49,5,3,52,83,
5,3,1,2,49,48,48,0,129,0,
23,22,2,3,30,18,3,3,1,2,
48,0,129,0,1,49,4,3,5,39,
4,3,1,2,45,53,48,0,129,0,
20,1,60,5,2,2,61,5,2,17,
208,144,209,130,208,188,208,190,209,129,
209,132,208,181,209,128,208,189,208,190,
208,181,32,208,180,208,176,208,178,208,
187,208,181,208,189,208,184,208,181,0,
67,1,27,47,46,9,20,88,22,7,
2,23,26,6,129,0,61,53,37,3,
23,86,39,3,5,7,194,169,32,82,
111,109,97,110,32,75,111,116,101,108,
110,105,107,111,118,44,32,50,48,50,
48,0,131,1,1,58,20,5,0,90,
20,5,1,2,31,84,32,208,184,32,
72,0,131,0,80,58,20,5,43,90,
20,5,2,2,31,80,0,66,1,6,
7,4,40,9,10,4,37,2,13,227,
66,1,10,7,4,40,13,10,4,37,
2,13,227,66,1,14,7,4,40,17,
10,4,37,2,13,227,66,1,18,7,
4,40,21,10,4,37,2,13,227,66,
1,22,7,4,40,25,10,4,37,2,
13,227,66,1,26,7,4,40,29,10,
4,37,2,13,227,66,1,30,7,4,
40,33,10,4,37,2,13,227,66,1,
34,7,4,40,37,10,4,37,2,13,
227,66,1,38,7,4,40,41,10,4,
37,2,13,227,66,1,42,7,4,40,
45,10,4,37,2,13,227,66,1,46,
7,4,40,49,10,4,37,2,13,227,
66,1,50,7,4,40,9,49,4,37,
2,13,227,66,1,54,7,4,40,13,
49,4,37,2,13,227,66,1,58,7,
4,40,17,49,4,37,2,13,227,66,
1,62,7,4,40,21,49,4,37,2,
13,227,66,1,66,7,4,40,25,49,
4,37,2,13,227,66,1,70,7,4,
40,29,49,4,37,2,13,227,66,1,
74,7,4,40,33,49,4,37,2,13,
227,66,1,78,7,4,40,37,49,4,
37,2,13,227,66,1,82,7,4,40,
41,49,4,37,2,13,227,66,1,86,
7,4,40,45,49,4,37,2,13,227,
66,1,90,7,4,40,49,49,4,37,
2,13,227,129,0,94,45,5,3,2,
84,5,3,2,2,55,50,53,0,129,
0,1,45,5,3,2,44,5,3,2,
2,55,50,53,0,129,0,94,6,5,
3,2,50,5,3,2,2,55,54,53,
0,129,0,1,6,5,3,2,11,5,
3,2,2,55,54,53,0,129,0,94,
45,5,3,56,84,5,3,2,2,55,
50,53,0,129,0,1,45,5,3,56,
44,5,3,2,2,55,50,53,0,129,
0,1,6,5,3,56,50,5,3,2,
2,55,54,53,0,129,0,94,6,5,
3,57,11,5,3,2,2,55,54,53,
0,67,2,54,53,6,3,10,86,12,
3,5,240,7,5,65,12,46,6,5,
5,28,8,5,5,2,131,0,60,58,
20,5,43,95,20,5,3,2,31,208,
147,209,128,208,176,209,132,208,184,208,
186,0,68,53,0,0,100,58,0,0,
63,90,3,8,1,76,12,204,95,101,
67,79,50,44,112,112,109,0,101,84,
79,86,83,44,112,112,98,0,84,44,
194,176,208,161,0,72,44,37,0,80,
44,109,109,32,72,103,0,131,0,20,
58,20,5,0,95,20,5,4,2,31,
208,146,208,190,208,183,208,180,209,131,
209,133,0,66,132,1,12,46,36,7,
7,46,36,4,23,24,129,0,2,49,
4,3,6,42,4,3,4,2,48,37,
0,129,0,41,49,8,3,51,42,5,
3,4,2,49,48,48,37,0,129,0,
31,0,35,5,15,0,32,5,4,17,
208,154,208,190,208,189,209,134,208,181,
208,189,209,130,209,128,208,176,209,134,
208,184,209,143,0,66,132,52,12,46,
36,8,47,46,36,4,23,24,129,0,
92,49,8,3,52,82,5,3,4,2,
49,48,48,37,0,129,0,70,7,11,
5,1,50,11,5,4,17,208,155,208,
158,208,146,0,67,1,11,44,25,6,
17,38,28,6,1,23,26,8,67,1,
11,44,25,6,17,38,25,6,4,23,
26,10,67,1,62,44,25,6,18,79,
25,6,4,23,26,10,129,0,20,7,
10,5,1,8,11,5,4,17,208,161,
208,158,50,0,129,0,54,49,4,3,
5,82,4,3,4,2,48,37,0,65,
18,95,0,5,5,58,0,5,5,1,
131,0,40,58,20,5,22,95,20,5,
5,2,31,208,157,208,176,209,129,209,
130,209,128,208,190,208,185,208,186,208,
184,0,2,1,10,1,12,5,9,1,
13,5,5,2,26,31,31,79,78,0,
79,70,70,0,66,129,23,1,76,5,
23,1,39,5,5,2,26,129,0,2,
1,7,5,1,1,7,5,5,17,83,
68,0,129,0,2,8,43,4,2,7,
52,5,5,17,208,146,209,139,208,178,
208,190,208,180,32,208,189,208,176,32,
208,184,208,189,208,180,208,184,208,186,
208,176,209,130,208,190,209,128,58,0,
3,133,73,7,26,6,2,13,26,6,
5,2,26,67,0,45,8,22,4,30,
12,22,7,5,2,26,11 };

// структура определяет все переменные вашего интерфейса управления  
struct {

    // input variable
    uint8_t switch_sd; // =1 если переключатель включен и =0 если отключен 
    uint8_t select_1; // =0 если переключатель в положении A, =1 если в положении B, =2 если в положении C, ... 

      // output variable
    int8_t level_1; // =0..100 положение уровня 
    int8_t level_2; // =0..100 положение уровня 
    char text_2[6];  // =строка UTF8 оканчивающаяся нулем 
    char text_5[6];  // =строка UTF8 оканчивающаяся нулем 
    int8_t level00; // =0..100 положение уровня 
    int8_t level01; // =0..100 положение уровня 
    int8_t level02; // =0..100 положение уровня 
    int8_t level03; // =0..100 положение уровня 
    int8_t level04; // =0..100 положение уровня 
    int8_t level05; // =0..100 положение уровня 
    int8_t level06; // =0..100 положение уровня 
    int8_t level07; // =0..100 положение уровня 
    int8_t level08; // =0..100 положение уровня 
    int8_t level09; // =0..100 положение уровня 
    int8_t level10; // =0..100 положение уровня 
    int8_t level11; // =0..100 положение уровня 
    int8_t level12; // =0..100 положение уровня 
    int8_t level13; // =0..100 положение уровня 
    int8_t level14; // =0..100 положение уровня 
    int8_t level15; // =0..100 положение уровня 
    int8_t level16; // =0..100 положение уровня 
    int8_t level17; // =0..100 положение уровня 
    int8_t level18; // =0..100 положение уровня 
    int8_t level19; // =0..100 положение уровня 
    int8_t level20; // =0..100 положение уровня 
    int8_t level21; // =0..100 положение уровня 
    char text_0[5];  // =строка UTF8 оканчивающаяся нулем 
    uint8_t led_1_r; // =0..255 яркость красного цвета индикатора 
    float onlineGraph_1_var1;
    float onlineGraph_1_var2;
    float onlineGraph_1_var3;
    float onlineGraph_1_var4;
    float onlineGraph_1_var5;
    int8_t level_3; // =0..100 положение уровня 
    int8_t level_4; // =0..100 положение уровня 
    char text_1[8];  // =строка UTF8 оканчивающаяся нулем 
    char text_3[10];  // =строка UTF8 оканчивающаяся нулем 
    char text_4[10];  // =строка UTF8 оканчивающаяся нулем 
    uint8_t led_sd_g; // =0..255 яркость зеленого цвета индикатора 
    int8_t level_sd; // =0..100 положение уровня 
    char text_6[11];  // =строка UTF8 оканчивающаяся нулем 

      // other variable
    uint8_t connect_flag;  // =1 if wire connected, else =0 

} RemoteXY;
#pragma pack(pop) 

///////////////////////////////////////////// 
//           END RemoteXY include          // 
///////////////////////////////////////////// 



int Xold = 11; //Старое значение шкалы
int Si[] = {13, 12, 14, 27, 26, 25, 33, 32, 2, 15};

//Декларирование процедуры записи в файл
void writeFile(fs::FS& fs, const char* path);



///////////////////////////////////////////// 
//     Настройка при первом запуске        // 
///////////////////////////////////////////// 

void setup()  
{ 
   
//Код контроллера
#if O
    Serial.begin(115200);
    uint64_t chipid = ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).  
    Serial.printf("ESP32 Chip ID = %04X",(uint16_t)(chipid>>32));//print High 2 bytes
    Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.
#endif

 //EEPROM
    EEPROM.begin(512);
    for (int i = 1; i < 22; i++) {
        Pn[i] = EEPROM.readShort(i * 2);
#if O
        Serial.print("Pn[");
        Serial.print(i);
        Serial.print("]=");
        Serial.println(Pn[i]);
#endif
}



 // Тестирование контроллера флешкарты  
if (!SD.begin(SD_CS)) {
#if O
  Serial.println("Card Mount Failed");
#endif
  sdflag = false;  
  }
    
if (sdflag) {
            totalKBytes = (int)(SD.totalBytes() / 1024);
            usedKBytes = (int)(SD.usedBytes() / 1024);

#if O            
            uint8_t cardType = SD.cardType();
            if (cardType == CARD_NONE) {
                Serial.println("No SD card attached");
                sdflag = false;
            }
            Serial.print("SD Card Type: ");
            if (cardType == CARD_MMC) {
                Serial.println("MMC");
            }
            else if (cardType == CARD_SD) {
                Serial.println("SDSC");
            }
            else if (cardType == CARD_SDHC) {
                Serial.println("SDHC");
            }
            else {
                Serial.println("UNKNOWN");
                sdflag = false;
            }
#endif

            // Считывание настроек
            File inifile = SD.open("/wifi.txt");
            if (!inifile) {
                #if O 
                    Serial.println("File wifi.txt doens't exist");
                #endif
                REMOTEXY_WIFI_SSID = "Center of the forest pyrology";
                REMOTEXY_WIFI_PASSWORD = "3AACEEDD";
            }
            else {
                #if O 
                    Serial.println("File wifi.txt already exists");
                #endif
                int i = 0;
                int j = -1;
                while (inifile.available()) {
                char buff = inifile.read();
                if (buff == ';') { i++; ssidStr[j + 1] = 0; j = -1; }
                else {
                    ++j;
                    if (i == 0) ssidStr[j] = buff;
                        else passwStr[j] = buff;
                    }
                }
                passwStr[j + 1] = 0;
                #if O 
                    Serial.printf("ssidStr= %s;\t", ssidStr);
                    Serial.printf("passwStr= %s\n", passwStr);
                #endif
                REMOTEXY_WIFI_SSID = ssidStr;
                REMOTEXY_WIFI_PASSWORD = passwStr;
                }
            inifile.close();
            }

    remotexy = new CRemoteXY (RemoteXY_CONF_PROGMEM, &RemoteXY, 
    REMOTEXY_ACCESS_PASSWORD, 
    REMOTEXY_WIFI_SSID, REMOTEXY_WIFI_PASSWORD, REMOTEXY_CLOUD_SERVER, 
    REMOTEXY_CLOUD_PORT, REMOTEXY_CLOUD_TOKEN);  
    #if O
      Serial.println(F("RemoteXY init success!"));
   #endif  
   strcpy(RemoteXY.text_0, ver);     

   if (sdflag) {
       sdflag = EEPROM.readBool(SDFLAG_EM); // Режим записи на SD
       RemoteXY.switch_sd = sdflag;
   }
   else {
       RemoteXY.switch_sd = false;
   }
   ri = (int)EEPROM.readByte(IN_EM); // Режим отображения на индикаторе   
   RemoteXY.select_1 =ri;

#if O 
   Serial.printf("ri=%d;\n", RemoteXY.select_1);
   Serial.printf("sdflag=%d;\n", sdflag);
   Serial.printf("TotalKBytes=%d\t", (int)totalKBytes);
   Serial.printf("UsedKBytes=%d\n", (int)usedKBytes);   
#endif    

  
  // Enable I2C
  Wire.begin(); 
  
 // Enable CCS811
  ccs811.set_i2cdelay(50); // Needed for ESP8266 because it doesn't handle I2C clock stretch correctly
  bool ok= ccs811.begin();
 #if O
     if( !ok ) Serial.println("setup: CCS811 begin FAILED");
     Serial.print("setup: ccs811 lib  version: "); Serial.println(CCS811_VERSION);
  
    // Print CCS811 versions
  
    Serial.print("setup: hardware    version: "); Serial.println(ccs811.hardware_version(),HEX);
    Serial.print("setup: bootloader  version: "); Serial.println(ccs811.bootloader_version(),HEX);
    Serial.print("setup: application version: "); Serial.println(ccs811.application_version(),HEX);
  #endif
 
  // Start measuring
  ok= ccs811.start(CCS811_MODE_1SEC);
  if( !ok ) 
  {
   #if O
   Serial.println("setup: CCS811 start FAILED");
   #endif
  }
  else {
  delay (5000);
   #if O
    Serial.println("CCS811 init success!");
   #endif    
  }
   
// BMP280
  if(!bmp.begin()){
   #if O
    Serial.println("BMP init failed!");
   #endif
  }
  else
  {
   #if O
    Serial.println("BMP init success!");
   #endif
  }
  bmp.setOversampling(4);
  
//SHT31
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
   #if O    
    Serial.println("SHT31 init failed!");
   #endif
  }
  else 
  {
   #if O    
    Serial.println("SHT31 init success!");
   #endif    
  }

  timeClient.begin();  
  timeClient.setTimeOffset(25200);
   #if O    
    Serial.println("TimeClient begin");
   #endif    

 


 for (int i = 0; i <10 ; i++) {
  pinMode(Si[i], OUTPUT);   
 } 
 getTimeStamp();    //Настроили время
 t = now();
 sn = hour(t);      //Установили срок наблюдения
} 


/////////////////////////////////////////////
//                Работа                   //
/////////////////////////////////////////////
void loop()  
{  
  RemoteXY_Handler (); 
    // Read
  uint16_t errstat, raw; 
  t = now();
#if O    
  Serial.printf("Time: %d:%d\n", hour(t), minute(t));
#endif  

  //ccs811v1
  ccs811.read(&eco2,&etvoc,&errstat,&raw); 
  n++;
  seco2 += eco2;
  setvoc += etvoc;
#if O    
  if( errstat==CCS811_ERRSTAT_OK ) {
    
    Serial.printf("eco2 = %dppm; ", eco2);
    Serial.printf("etvoc = %dppb; ", etvoc);
  } else if( errstat==CCS811_ERRSTAT_OK_NODATA ) {
    Serial.println("CCS811: waiting for (new) data");
  } else if( errstat & CCS811_ERRSTAT_I2CFAIL ) { 
    Serial.println("CCS811: I2C error");
  } else {
    Serial.print("CCS811: errstat="); Serial.print(errstat,HEX); 
    Serial.print("="); Serial.println( ccs811.errstat_str(errstat) ); 
  }
#endif

   
//BMP280

  char result = bmp.startMeasurment();
 
  if(result!=0){
    delay(result);
    result = bmp.getTemperatureAndPressure(T,P);    
      if(result!=0) P = P / 1.3332239F;
      else {
        #if O            
          Serial.println(" BMP280 Error.");
        #endif
      }
  }
  else {
   #if O            
        Serial.println(" BMP280 Error.");
   #endif
  }

  //sht31
        H = sht31.readHumidity();
   #if O
        Serial.printf("T = %f°С; ", T);    
        Serial.printf("H = %f%s; ", H, "%");    
        Serial.printf("P = %fmmHg\n", P);    
   #endif
   
//Проверка необходимости замера       
   if (hour(t) == sn)
        {
            zamerP(); //Плановый замер давления
            sn = sn + 4;
            if (sn == 24) sn = 0;
        }
       else delay(2000);

//Если есть подключение
   if (RemoteXY.connect_flag) {

       //Если изменился режим работы с SD
       if (sdflag != RemoteXY.switch_sd)
       {
           insd();
           EEPROM.writeBool(SDFLAG_EM, sdflag);
           EEPROM.commit();
          #if O
             Serial.printf("save eeprom: SDFLAG=%d;\n", sdflag);
          #endif
       }
       // Мигание
       if (fmsd && sdflag) {
           if (sdled == 128) sdled = 10;
           else sdled = 128;
           RemoteXY.led_sd_g = sdled;
       }

      RemoteXY.level_1 = (int)(T)+50;
      dtostrf(T, 2, 1, str);
      sprintf(str, "%s°С", str);
      strcpy(RemoteXY.text_1, str);
      RemoteXY.onlineGraph_1_var3= T;

      
      dtostrf(H, 2, 1, str);
      sprintf(str, "%s%s", str, "%");
      strcpy(RemoteXY.text_2, str);
      RemoteXY.level_2 = (int)(H);
      RemoteXY.onlineGraph_1_var4= H;

      dtostrf(P, 4, 0, str);
      strcpy(RemoteXY.text_5, str);
      RemoteXY.onlineGraph_1_var5= P;


// Воздух    
      dtostrf(eco2, 5, 0, str);
      sprintf(str, "%s ppm", str);
      strcpy(RemoteXY.text_3, str);      
      peco2=map(eco2, 400, meco2, 0, 100);
      RemoteXY.level_3 = peco2;       
      RemoteXY.onlineGraph_1_var1=eco2;
   
      dtostrf(etvoc, 5, 0, str);
      sprintf(str, "%s ppb", str);
      strcpy(RemoteXY.text_4, str);
      petvoc=map(etvoc, 0, metvoc, 0, 100);
      RemoteXY.level_4 = petvoc; 
      RemoteXY.onlineGraph_1_var2= etvoc;

      //Проверка настроек отображения индикатора
      if (RemoteXY.select_1 != (int)ri) { //Если изменения

          ri = (byte)RemoteXY.select_1;
          switch (ri) {
          case 1:
              strcpy(RemoteXY.text_6, "H");
              break;
          case 2:
              strcpy(RemoteXY.text_6, "P");
              break;
          case 3:
              strcpy(RemoteXY.text_6, "eCO2");
              break;
          case 4:
              strcpy(RemoteXY.text_6, "eTOVS");
              break;
          default:
              strcpy(RemoteXY.text_6, "t");
          }
          EEPROM.writeByte(IN_EM, ri);
          EEPROM.commit();
          #if O
            Serial.printf("save eeprom: ri=%d;\n", ri);
          #endif
      }
   }

    
  //Индикация
 switch (ri) {
 case 1:
     Scale(H, 100);
     break;
 case 2:
     Scale(map(P,725,765,0,10));
     break;
 case 3:
     Scale(eco2, meco2);
     break;
 case 4:
     Scale(etvoc, metvoc);
     break;
 default:
     Scale(map(T,0,40,0,10));
 }

    

 }

//Процедура замера давления
void zamerP() {

        char result = bmp.startMeasurment();

        if (result != 0) {
            delay(result);
            result = bmp.getTemperatureAndPressure(T, P);
            if (result != 0)
            {
                P = P / 1.3332239F;
#if O
                Serial.print(F("Замер P № "));
                Serial.println(zamer);
#endif    
                for (int i = 1; i < 22; i++) {
                    Pn[i - 1] = Pn[i];
                    EEPROM.writeShort((i - 1) * 2, Pn[i - 1]);
#if O
                    Serial.print("Pn[");
                    Serial.print(i - 1);
                    Serial.print("]=");
                    Serial.println(Pn[i - 1]);
#endif
                }
                Pn[21] = (int)P;
                EEPROM.writeShort(42, Pn[21]);
                EEPROM.commit();

#if O
                Serial.print("Pn[21]=");
                Serial.println(Pn[21]);
#endif
                if (abs(Pn[21] - Pn[20]) >= 3 || abs(744 - Pn[21]) >= 7) {
                    RemoteXY.led_1_r = 225;
                }
                else RemoteXY.led_1_r = 5;
                RemoteXY.level00 = map(Pn[0], 725, 765, 0, 100);
                RemoteXY.level01 = map(Pn[1], 725, 765, 0, 100);
                RemoteXY.level02 = map(Pn[2], 725, 765, 0, 100);
                RemoteXY.level03 = map(Pn[3], 725, 765, 0, 100);
                RemoteXY.level04 = map(Pn[4], 725, 765, 0, 100);
                RemoteXY.level05 = map(Pn[5], 725, 765, 0, 100);
                RemoteXY.level06 = map(Pn[6], 725, 765, 0, 100);
                RemoteXY.level07 = map(Pn[7], 725, 765, 0, 100);
                RemoteXY.level08 = map(Pn[8], 725, 765, 0, 100);
                RemoteXY.level09 = map(Pn[9], 725, 765, 0, 100);
                RemoteXY.level10 = map(Pn[10], 725, 765, 0, 100);
                RemoteXY.level11 = map(Pn[11], 725, 765, 0, 100);
                RemoteXY.level12 = map(Pn[12], 725, 765, 0, 100);
                RemoteXY.level13 = map(Pn[13], 725, 765, 0, 100);
                RemoteXY.level14 = map(Pn[14], 725, 765, 0, 100);
                RemoteXY.level15 = map(Pn[15], 725, 765, 0, 100);
                RemoteXY.level16 = map(Pn[16], 725, 765, 0, 100);
                RemoteXY.level17 = map(Pn[17], 725, 765, 0, 100);
                RemoteXY.level18 = map(Pn[18], 725, 765, 0, 100);
                RemoteXY.level19 = map(Pn[19], 725, 765, 0, 100);
                RemoteXY.level20 = map(Pn[20], 725, 765, 0, 100);
                RemoteXY.level21 = map(Pn[21], 725, 765, 0, 100);
                zamer++;


                //Запись       
                if (sdflag) writeFile(SD, DATALOG);

            }
        }

}




void writeFile(fs::FS &fs, const char * path){
    char message [200];
    uint16_t feco2 = round(seco2/n);
    uint16_t fetvoc =round(setvoc/n);
    n=0;
    
    sprintf(message, "%f;%f;%f;%d;%d%;%d;%d;%d;%d;%d;%d", T, P, H, feco2, fetvoc, hour(), minute(), second(), day(),month(),year() );
    eco2 = 0;
    etvoc = 0;
#if O
      Serial.printf("Writing file: %s\n", path);
      Serial.printf("Message: %s\n", message);
    #endif
    File file = fs.open(path, FILE_APPEND);
    if(!file){
      #if O
        Serial.println("Failed to open file for appending");
      #endif
        return;
    }
    else 
    {      
      if(file.println(message)){
        #if O
          Serial.println("Append");
        #endif
      } 
      else {
        #if O
          Serial.println("Write failed");
        #endif
      }
      file.close();
    }
    insd();
}

//Отобразить натсройки индикатора SD
void insd() {
    sdflag = RemoteXY.switch_sd;
    if (sdflag) {
        File file = SD.open(DATALOG, FILE_READ);
        if (!file) {
            sdflag = false;
            #if O
                Serial.println("Failed to open file for size");
            #endif
        }
        file.close();
        int bsd  = map(usedKBytes, 0, totalKBytes, 10, 100);
        RemoteXY.level_sd = bsd;
        //Проверка на критический остатка памяти
        if (bsd>90) fmsd = true;
           else fmsd = false;
        sdled = 128;
    }
    else {
        RemoteXY.level_sd = 0;
        sdled = 0;
        fmsd = false;
    }
    RemoteXY.led_sd_g = sdled;
}

// Function to get date and time from NTPClient
void getTimeStamp() {
    int hr,min,sec,day,month,yr;  
    while (!timeClient.update()) {
        timeClient.forceUpdate();
    }
    // The formattedDate comes with the following format:
    // 2018-05-28T16:00:13Z
    // We need to extract date and time
    sscanf(timeClient.getFormattedDate().c_str(),"%d-%d-%dT%d:%d:%dZ", &yr,&month,&day,&hr,&min,&sec);
    setTime(hr,min,sec,day,month,yr);
    
        #if O
//        Serial.printf("formattedDate  %s\n", formattedDate);
//        Serial.println(formattedDate);
        #endif

 }


//Шкала
void Scale (int X) {
  if (X == Xold) return;
  for (int i = 0; i <10 ; i++) {
      digitalWrite (Si[i], X>i);      
 }
  Xold = X;    
}

void Scale (int Xi, int Xm) {  
  int X = map (Xi, 0, Xm, 0, 10);
      #if O
//        Serial.printf("Scale : %d\n", X);
      #endif
  if (X == Xold) return;
  for (int i = 0; i <10 ; i++) {
      digitalWrite (Si[i], X>i);            
 }
 Xold = X;
}

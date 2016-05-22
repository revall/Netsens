#ifndef NS_EEPROM_h
#define NS_EEPROM_h
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <EEPROM.h>

extern struct NS_Config EC_Config;
/*
extern int Hum;
extern int Temp;
extern int Avalue;
extern uint32_t tm;
extern bool flag_light;
extern bool flag_hum;
extern bool flag_btn;
extern uint16_t timer;
*/
struct NS_Config{
// Наименование в режиме точки доступа  
   char ESP_NAME[32];
   char ESP_PASS[32];
// Параметры подключения в режиме клиента
   char AP_SSID[32];
   char AP_PASS[32];
   IPAddress IP;
   IPAddress MASK;
   IPAddress GW;
// Параметры NTP сервера
   char NTP_SERVER1[32];
   char NTP_SERVER2[32];
   char NTP_SERVER3[32];
   short int  TZ;
// Интервал опроса NTP
   uint32_t TIMEOUT_NTP;
// Интервал отправки сообщений на сервер, мс
   uint32_t TIMEOUT_SEND1;
// Сервер куда отправлять статистику
   char     MQTT_SERVER[48];   
// Строка отправки параметров на сервер
//   char     HTTP_REQUEST[128];   
// Контрольная сумма   
   uint16_t CRC;   
};


void     EC_begin(void);
void     EC_read(void);
void     EC_save(void);
uint16_t EC_CRC(void);
void     EC_default(void);




#endif

#ifndef WS_HTTP_h
#define WS_HTTP_h
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>

extern ESP8266WebServer httpServer;
extern bool isAP;
extern bool flag_btn;



extern char Tmsg[50];
extern char Hmsg[50];

void HTTP_begin(void);
void HTTP_handleRoot(void);
void HTTP_handleConfig(void);
void HTTP_handleConfig2(void);
void HTTP_handleSave(void);
void HTTP_handleDefault(void);
void HTTP_handleReboot(void);
void HTTP_handleButton(void);
void HTTP_loop();

bool SetParamHTTP();


#endif

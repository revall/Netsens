#include "NS_HTTP.h"
#include "NS_EEPROM.h"
//#include "NS_NTP.h"
#include "netsens.h"

ESP8266WebServer httpServer(80);
bool isAP = false;
String authPass = "";


/**
 * Старт WEB сервера
 */
void HTTP_begin(void){
// Поднимаем WEB-сервер

//server.on("/", [](){
//  if(!server.authenticate("admin", "admin"))
//    return server.requestAuthentication();
//  server.send(200, "text/plain", "Login OK");
//});

   httpServer.on ( "/", HTTP_handleRoot );
   httpServer.on ( "/config", HTTP_handleConfig );
   httpServer.on ( "/save", HTTP_handleSave );
   httpServer.on ( "/reboot", HTTP_handleReboot );
   httpServer.on ( "/default", HTTP_handleDefault );
   httpServer.on ( "/button", HTTP_handleButton );
   httpServer.onNotFound ( HTTP_handleRoot );
   httpServer.begin();
   Serial.printf( "HTTP server started ...\n" );

}

/**
 * Обработчик событий WEB-сервера
 */
void HTTP_loop(void){
  httpServer.handleClient();
}

/*
 * Оработчик главной страницы сервера
 */
void HTTP_handleRoot(void) {
  String out = "";

  out =
"<html>\
  <head>\
    <meta charset=\"utf-8\" />\
    <meta http-equiv='refresh' content='5'/>\
    <title>NetSens</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
   <h1>Имя контроллера NetSens : ";
      out += EC_Config.ESP_NAME;
   out +="</h2>\n";
    out +="Температура : ";
    out += Tmsg;
    out +="&degC Влажность: ";
    out += Hmsg;
    out +="% \n";
    out +="Время: ";
    out += CurentTime;
   if( isAP ){
      out += "<p>Режим точки доступа: ";
      out += EC_Config.ESP_NAME;
      out+= " </p>";
   }
   else {
      out += "<p>Подключено к ";
      out += EC_Config.AP_SSID;
      out += " </p>";
   }
 char str[50];
// out +="<h2>";
 //sprintf(str,"Время: %02d:%02d</br>",( tm/3600 )%24,( tm/60 )%60);
 // out +=str;
 //out +="</h2>\n";

   out +="<h2><a href='/button'>КНОПКА</a></h2>\n";
   out +="\
     <p><a href=\"/config\">Настройка параметров</a>\
     </body>\
     </html>";
   httpServer.send ( 200, "text/html", out );

}



/*
 * Оработчик страницы настройки сервера
 */
void HTTP_handleConfig(void) {
  String out = "";
  char str[10];
  out =
"<html>\
  <head>\
    <meta charset=\"utf-8\" />\
    <title>NetSense Controller Config</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Настройка параметров</h1>\
     <ul>\
     <li><a href=\"/\">Главная</a>\
     <li><a href=\"/reboot\">Перезагрузка</a>\
     <li><a href=\"/default\">Сброс настроек</a>\
     </ul>\n";

  out +=
"     <form method='get' action='/save'>\
         <p><b>Параметры ESP модуля</b></p>\
         <table border=0><tr><td>Наименование:</td><td><input name='esp_name' length=32 value='";
  out += EC_Config.ESP_NAME;
  out += "'></td></tr>\
         <tr><td>Пароль admin:</td><td><input name='esp_pass' length=32 value='";
  out += EC_Config.ESP_PASS;
  out += "'></td></tr></table><br>\
         <p><b>Параметры WiFi подключения</b></p>\
         <table border=0><tr><td>Имя сети: </td><td><input name='ap_ssid' length=32 value='";
  out += EC_Config.AP_SSID;
  out += "'></td></tr>\
         <tr><td>Пароль:</td><td><input name='ap_pass' length=32 value='";
  out += EC_Config.AP_PASS;
  out += "'></td></tr>\
        <tr><td>IP-адрес:</td><td><input name='ip1' length=4 size=4 value='";
  sprintf(str,"%d",EC_Config.IP[0]);
  out += str;
  out += "'>.<input name='ip2' length=4 size=4 value='";
  sprintf(str,"%d",EC_Config.IP[1]);
  out += str;
  out += "'>.<input name='ip3' length=4 size=4 value='";
  sprintf(str,"%d",EC_Config.IP[2]);
  out += str;
  out += "'>.<input name='ip4' length=4 size=4 value='";
  sprintf(str,"%d",EC_Config.IP[3]);
  out += str;
  out += "'></td></tr>\
        <tr><td>IP-маска:</td><td><input name='mask1' length=4 size=4 value='";
  sprintf(str,"%d",EC_Config.MASK[0]);
  out += str;
  out += "'>.<input name='mask2' length=4 size=4 value='";
  sprintf(str,"%d",EC_Config.MASK[1]);
  out += str;
  out += "'>.<input name='mask3' length=4 size=4 value='";
  sprintf(str,"%d",EC_Config.MASK[2]);
  out += str;
  out += "'>.<input name='mask4' length=4 size=4 value='";
  sprintf(str,"%d",EC_Config.MASK[3]);
  out += str;
  out += "'></td></tr>\
        <tr><td>IP-шлюз:</td><td><input name='gw1' length=4 size=4 value='";
  sprintf(str,"%d",EC_Config.GW[0]);
  out += str;
  out += "'>.<input name='gw2' length=4 size=4 value='";
  sprintf(str,"%d",EC_Config.GW[1]);
  out += str;
  out += "'>.<input name='gw3' length=4 size=4 value='";
  sprintf(str,"%d",EC_Config.GW[2]);
  out += str;
  out += "'>.<input name='gw4' length=4 size=4 value='";
  sprintf(str,"%d",EC_Config.GW[3]);
  out += str;
  out += "'></td></tr></table>\n\
         <p><b>Параметры сервера времени</b></p>\
         <table border=0><tr><td>NTP сервер 1:</td><td><input name='ntp_server1' length=32 value='";
  out += EC_Config.NTP_SERVER1;
  out += "'></td></tr>\
         <tr><td>NTP сервер 2:</td><td><input name='ntp_server2' length=32 value='";
  out += EC_Config.NTP_SERVER2;
  out += "'></td></tr>\
         <tr><td>NTP сервер 3:</td><td><input name='ntp_server3' length=32 value='";
  out += EC_Config.NTP_SERVER3;
  out += "'></td></tr>\
         <tr><td>Таймзона:</td><td><input name='tz' length=4 size 4 value='";
  sprintf(str,"%d",EC_Config.TZ);
  out += str;
  out += "'></td></tr>\
         <tr><td>Интервал опроса NTP, сек:</td><td><input name='tm_ntp' length=32 value='";
  sprintf(str,"%d",EC_Config.TIMEOUT_NTP/1000);
  out += str;
  out += "'></td></tr>\
         <tr><td>MQTT сервер:</td><td><input name='mqtt_serv' length=32 value='";
  out += EC_Config.MQTT_SERVER;
  out += "'></td></tr></table>\
     <input type='submit' value='Сохранить'>\
     </form>\
  </body>\
</html>";
   httpServer.send ( 200, "text/html", out );
}



/*
 * Оработчик сохранения в базу
 */
void HTTP_handleSave(void) {

  if( httpServer.hasArg("esp_name")     )strcpy(EC_Config.ESP_NAME,httpServer.arg("esp_name").c_str());
  if( httpServer.hasArg("esp_pass")     )strcpy(EC_Config.ESP_PASS,httpServer.arg("esp_pass").c_str());
  if( httpServer.hasArg("ap_ssid")      )strcpy(EC_Config.AP_SSID,httpServer.arg("ap_ssid").c_str());
  if( httpServer.hasArg("ap_pass")      )strcpy(EC_Config.AP_PASS,httpServer.arg("ap_pass").c_str());
  if( httpServer.hasArg("ip1")          )EC_Config.IP[0] = atoi(httpServer.arg("ip1").c_str());
  if( httpServer.hasArg("ip2")          )EC_Config.IP[1] = atoi(httpServer.arg("ip2").c_str());
  if( httpServer.hasArg("ip3")          )EC_Config.IP[2] = atoi(httpServer.arg("ip3").c_str());
  if( httpServer.hasArg("ip4")          )EC_Config.IP[3] = atoi(httpServer.arg("ip4").c_str());
  if( httpServer.hasArg("mask1")        )EC_Config.MASK[0] = atoi(httpServer.arg("mask1").c_str());
  if( httpServer.hasArg("mask2")        )EC_Config.MASK[1] = atoi(httpServer.arg("mask2").c_str());
  if( httpServer.hasArg("mask3")        )EC_Config.MASK[2] = atoi(httpServer.arg("mask3").c_str());
  if( httpServer.hasArg("mask4")        )EC_Config.MASK[3] = atoi(httpServer.arg("mask4").c_str());
  if( httpServer.hasArg("gw1")          )EC_Config.GW[0] = atoi(httpServer.arg("gw1").c_str());
  if( httpServer.hasArg("gw2")          )EC_Config.GW[1] = atoi(httpServer.arg("gw2").c_str());
  if( httpServer.hasArg("gw3")          )EC_Config.GW[2] = atoi(httpServer.arg("gw3").c_str());
  if( httpServer.hasArg("gw4")          )EC_Config.GW[3] = atoi(httpServer.arg("gw4").c_str());
  if( httpServer.hasArg("ntp_server1")  )strcpy(EC_Config.NTP_SERVER1,httpServer.arg("ntp_server1").c_str());
  if( httpServer.hasArg("ntp_server2")  )strcpy(EC_Config.NTP_SERVER2,httpServer.arg("ntp_server2").c_str());
  if( httpServer.hasArg("ntp_server3")  )strcpy(EC_Config.NTP_SERVER3,httpServer.arg("ntp_server3").c_str());
  if( httpServer.hasArg("tz")           )EC_Config.TZ = atoi(httpServer.arg("tz").c_str());
  if( httpServer.hasArg("tm_ntp")       )EC_Config.TIMEOUT_NTP  = atoi(httpServer.arg("tm_ntp").c_str())*1000;
   if( httpServer.hasArg("tm_send1")      )EC_Config.TIMEOUT_SEND1  = atoi(httpServer.arg("tm_send1").c_str())*1000;
  if( httpServer.hasArg("mqtt_serv")  )strcpy(EC_Config.MQTT_SERVER,httpServer.arg("mqtt_serv").c_str());

  EC_save();

  String out = "";

  out =
"<html>\
  <head>\
    <meta charset='utf-8' />\
    <title>NetSens</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>WiFi контроллер</h1>\
     <h2>Конфигурация сохранена</h2>\
     <ul>\
     <li><a href=\"/reboot\">Перезагрузка</a>\
     <li><a href=\"/config\">Настройка параметров </a>\
     <li><a href=\"/\">Главная</a>\
     </ul>\
  </body>\
</html>";
   httpServer.send ( 200, "text/html", out );

}

/*
 * Сброс настроек
 */
void HTTP_handleDefault(void) {
  EC_default();
  HTTP_handleConfig();
}

/*
 * Перезагрузка часов
 */
void HTTP_handleReboot(void) {

  String out = "";

  out =
"<html>\
  <head>\
    <meta charset='utf-8' />\
    <meta http-equiv='refresh' content='30;URL=/'>\
    <title>NetSens</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Перезагрузка контроллера</h1>\
     <p><a href=\"/\">Через 30 сек будет переадресация на главную</a>\
   </body>\
</html>";
   httpServer.send ( 200, "text/html", out );
   ESP.reset();

}

/*
 * Нажатие кнопки
 */
void HTTP_handleButton(void) {
//  flag_btn = true;
  String out = "";

  out =
"<html>\
  <head>\
    <meta charset='utf-8' />\
    <meta http-equiv='refresh' content='3;URL=/'>\
    <title>NetSens</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Нажата кнопка</h1>\
     <p><a href=\"/\">Через 3 сек будет переадресация на главную</a>\
   </body>\
</html>";
   httpServer.send ( 200, "text/html", out );

}


/**
* Сохранить параметры на HTTP сервере
*/
bool SetParamHTTP(){

   WiFiClient client;
   IPAddress ip1;
   WiFi.hostByName(EC_Config.MQTT_SERVER, ip1);
   Serial.print("IP=");
   Serial.println(ip1);

   String out = "";
   char str[256];
   if (!client.connect(ip1, 80)) {
       Serial.printf("Connection %s failed",EC_Config.MQTT_SERVER);
       return false;
   }
   out += "GET ";
   out += "http://";
   out += EC_Config.MQTT_SERVER;
// Формируем строку запроса

//   sprintf(str,"/save3.php?id=%s&h=%d&t=%d&a=%d&tm1=%d&tm2=%d&uptime=%ld",EC_Config.ESP_NAME,Hum,Temp,Avalue,Count1/2,Count2/2,(tm-first_tm));
   out += str;
   out += " HTTP/1.0\r\n\r\n";
   Serial.print(out);
   client.print(out);
   delay(100);
   return true;


}

#include "NS_EEPROM.h"

struct NS_Config EC_Config;

/**
 * Инициализация EEPROM
 */
void EC_begin(void){
   size_t sz1 = sizeof(EC_Config);
   EEPROM.begin(sz1);
   Serial.printf("EEPROM init. Size = %d\n",(int)sz1);

}

/**
 * Читаем конфигурацию из EEPROM в память
 */
void EC_read(void){
   size_t sz1 = sizeof(EC_Config);
   for( int i=0; i<sz1; i++ ){
       uint8_t c = EEPROM.read(i);
       *((uint8_t*)&EC_Config + i) = c;
    }
    uint16_t crc = EC_CRC();
    if( EC_Config.CRC == crc ){
       Serial.printf("EEPROM Config is correct\n");
    }
    else {
       Serial.printf("EEPROM CRC is not valid: %d %d\n",crc,EC_Config.CRC);
       EC_default();
       EC_save();
    }
}

/**
 * Устанавливаем значения конфигурации по-умолчанию
 */
void EC_default(void){
   size_t sz1 = sizeof(EC_Config);
   memset( &EC_Config, '\0',sz1);
   strcpy(EC_Config.ESP_NAME,"netsens");
   strcpy(EC_Config.ESP_PASS,"admin");
   strcpy(EC_Config.AP_SSID, "none");
   strcpy(EC_Config.AP_PASS, "");
   EC_Config.IP[0]      = 0;
   EC_Config.IP[1]      = 0;
   EC_Config.IP[2]      = 0;
   EC_Config.IP[3]      = 0;
   EC_Config.MASK[0]    = 0;
   EC_Config.MASK[1]    = 0;
   EC_Config.MASK[2]    = 0;
   EC_Config.MASK[3]    = 0;
   EC_Config.GW[0]      = 0;
   EC_Config.GW[1]      = 0;
   EC_Config.GW[2]      = 0;
   EC_Config.GW[3]      = 0;
   strcpy(EC_Config.NTP_SERVER1, "0.ru.pool.ntp.org");
   strcpy(EC_Config.NTP_SERVER2, "1.ru.pool.ntp.org");
   strcpy(EC_Config.NTP_SERVER3, "2.ru.pool.ntp.org");
   EC_Config.TZ         = 3;
   EC_Config.TIMEOUT_NTP          = 600000;
   EC_Config.TIMEOUT_SEND1        = 900000;
   strcpy(EC_Config.MQTT_SERVER,"192.168.111.1");
}

/**
 * Сохраняем значение конфигурации в EEPROM
 */
void EC_save(void){
   EC_Config.CRC = EC_CRC();
   size_t sz1 = sizeof(EC_Config);
   for( int i=0; i<sz1; i++)
      EEPROM.write(i, *((uint8_t*)&EC_Config + i));
      EEPROM.commit();
   Serial.printf("Save Config to EEPROM. CRC=%d\n",EC_Config.CRC);
}

/**
 * Расчет CRC
 */
uint16_t EC_CRC(void){
   uint16_t crc = 0;
   size_t sz1 = sizeof(EC_Config);
   uint16_t crc_save = EC_Config.CRC;
   EC_Config.CRC = 0;
   for( int i=0; i<sz1; i++)crc +=*((uint8_t*)&EC_Config + i);
   Serial.printf("CRC=%d\n",crc);
   EC_Config.CRC = crc_save;
   return crc;
}

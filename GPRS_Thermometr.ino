    // библиотека для работы с GPRS устройством
    #include <GPRS_Shield_Arduino.h>
     
    // библиотека для эмуляции Serial порта
    // она нужна для работы библиотеки GPRS_Shield_Arduino
    #include <SoftwareSerial.h>
     
    // длина сообщения
    #define MESSAGE_LENGTH 160
    //текст сообщения запроса 
    #define MESSAGE_TEMP  "temp" 
    //аналоговый вход, к которому подключен датчик температуры
    #define TMP36_PIN 0
    //телефон владельца устройства
    #define MASTER_PHONE "+7**********"
     
    // текст сообщения
    char message[MESSAGE_LENGTH];
    // номер, с которого пришло сообщение
    char phone[16];
    // дата отправки сообщения
    char datetime[24];
    //интервал отправки сообщений в мс
    const unsigned long PERIOD = 3600000;
    //последнее время отправки температуры
    unsigned long lastTime = 0;
     
    // создаём объект класса GPRS и передаём в него объект Serial1 
    GPRS gprs(Serial1);

    void getTemp(char f_phone[], char f_message[])
    {
      if (strcmp(f_message, MESSAGE_TEMP) == 0) 
        {
        // если сообщение — с текстом «Temp»,
        // измеряем температуру
        float voltage = analogRead(TMP36_PIN)*5;
        voltage /= 1024;
        float temperature = (voltage-0.5)*100;
        char temp_sms[3];
        itoa(int(temperature), temp_sms, 10);
        // на номер, с которого пришёл запрос,
        // отправляем смс со значением температуры
        gprs.sendSMS(f_phone, temp_sms);
        }  
      else 
        {
        // если сообщение содержит неизвестный текст,
        // отправляем сообщение с текстом об ошибке
        gprs.sendSMS(f_phone, "Error...unknown command!");
        }
    }

    //отправка сообщения на номер владельца
    void timerMessage()
    {
      getTemp(MASTER_PHONE, MESSAGE_TEMP);
      }
     
    void setup()
    {
      Serial.begin(9600);
      while(!Serial){}
      // включаем GPRS шилд
      gprs.powerOn();
      Serial.println("GPRS ON");
      // открываем Serial-соединение с GPRS Shield
      Serial1.begin(9600);
      // проверяем есть ли связь с GPRS устройством
      while (!gprs.init()) {
        Serial.println("Initialization...");
        delay(1000);
        }
      Serial.println("Init success!");
    }

    void loop()
    {
      // если пришло новое сообщение
      if (gprs.ifSMSNow()) {
        // читаем его
         gprs.readSMS(message, phone, datetime);
         Serial.println("Message inbox!");
        //обрабатываем
         getTemp (phone, message);
      }
      
      if (millis() - lastTime > PERIOD)
         {
          Serial.println(lastTime);
          Serial.println(millis());
          Serial.println(millis() - lastTime);
          Serial.println("SENDING SMS...");
          //timerMessage();
          Serial.println("SENDING DONE!");
          lastTime = millis();
          }
    }
     
   

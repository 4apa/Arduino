//библиотека для управления питанием
#include <avr/power.h>

// библиотека для работы с GPRS устройством
#include <GPRS_Shield_Arduino.h>

// библиотека для эмуляции Serial порта
// она нужна для работы библиотеки GPRS_Shield_Arduino
#include <SoftwareSerial.h>

// длина сообщения
#define MESSAGE_LENGTH 160
//текст сообщения запроса
#define MESSAGE_TEMP  "temp"
//команда на отключение модуля
#define MESSAGE_OFF "off"
//аналоговый вход, к которому подключен датчик температуры
#define TMP36_PIN A0
//выход, к которому подключено питание датчика
#define TMP36_POWER 8
//телефон владельца устройства
#define MASTER_PHONE "+7**********"

// текст сообщения
char message[MESSAGE_LENGTH];
// номер, с которого пришло сообщение
char phone[16];
// дата отправки сообщения
char datetime[24];
//интервал отправки сообщений в мс
const unsigned long PERIOD = 86400000; //86400000 = 24 часа
//последнее время отправки температуры
unsigned long lastTime = 0;

// создаём объект класса GPRS и передаём в него объект Serial1
GPRS gprs(Serial1);

float getVoltage ()
{
  //число измерений
  const int NUMBER_OF_MEASURES = 10;

  //подаем питание на дачтик TMP36
  digitalWrite(TMP36_POWER, HIGH);
  delay(500);

  //сумма показаний аналогового входа за NUMBER_OF_MEASURES измерений 
  float sum = 0;
  for (int i = 0; i < NUMBER_OF_MEASURES; ++i)
  {
    sum += analogRead(TMP36_PIN);
    delay(500);
  }

  //выключаем питание датчика
  digitalWrite(TMP36_POWER, LOW);

  //определяем среднее за цикл измерений напряжение на аналговом входе
  return (sum / NUMBER_OF_MEASURES) * 5 / 1024;
}

int getTemperature ()
{
  //пересчитываем показания датчика в температуру (градусы C)
  float temperature = (getVoltage() - 0.5)*100;
  return (int) temperature;
}

void processMessage(char f_phone[], char f_message[])
{
  if (strcmp(f_message, MESSAGE_TEMP) == 0)
  {
    // если сообщение — с текстом MESSAGE_TEMP,
    // измеряем температуру

    //массив для значения температуры
    char temp_value[17];

    //массив для текстовой части смс
    char text_message[] = "Temperature (C): ";

    //переводим int в массив char
    itoa(getTemperature(), temp_value, 10);
    
    // на номер, с которого пришёл запрос,
    // отправляем смс со значением температуры
    gprs.sendSMS(f_phone, strcat(text_message, temp_value));
  }
  
  else if (strcmp(f_message, MESSAGE_OFF) == 0)
  {
    //если пришла команда на выключение устройства

    //сообщаем, что приняли команду
    gprs.sendSMS(f_phone, "Turned off");
    delay(10000);
    //отключаем питание gprs-модуля
    gprs.powerOff();
  }

  else
  {
    // если сообщение содержит неизвестный текст,
    // отправляем сообщение с текстом об ошибке
    gprs.sendSMS(MASTER_PHONE, "Error...unknown command!");
  }
}

//отправка сообщения на номер владельца
void timerMessage()
{
  processMessage(MASTER_PHONE, MESSAGE_TEMP);
}

void setup()
{
  // включаем GPRS шилд
  gprs.powerOn();
  // определяем как выход пин питания датчика
  pinMode(TMP36_POWER, OUTPUT);
  // открываем Serial-соединение с GPRS Shield
  Serial1.begin(9600);
  // проверяем есть ли связь с GPRS устройством
  while (!gprs.init()) {}
  gprs.sendSMS(MASTER_PHONE, "Temperature monitoring started!");
}

void loop()
{
  // если пришло новое сообщение
  if (gprs.ifSMSNow()) {
    // читаем его
    gprs.readSMS(message, phone, datetime);
    //обрабатываем
    processMessage (phone, message);
  }

  //если произошло переполнение millis()
  if (millis() < lastTime)
  {
    lastTime = 0;
    gprs.sendSMS(MASTER_PHONE, "Time reset!");
  }

  //если прошло больше заданного PERIOD с последней передачи
  if (millis() - lastTime > PERIOD)
  {
    timerMessage();
    lastTime = millis();
  }
}


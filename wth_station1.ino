// WeatherStation new
// ------------------
// V0.01 12.02.2016
// V1.10 11.03.2022
//

#include <OneWire.h>
#include <Wire.h>
#include <SHT2x.h>
#include <SPI.h>

#include <Adafruit_SSD1306.h>
#include <splash.h>

#include <Adafruit_GFX.h>
#include <gfxfont.h>

#include <Adafruit_BMP085.h>

#include <DHT.h>

#define LED_PIN 5
#define DHTTYPE DHT22 
#define DHTPIN 3 
#define DHT2PIN 6
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels

#define OLED_RESET     4    // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define SENS_UPDATE_INTERVAL    30000    // 30sec
#define DISPLAY_UPDATE_INTERVAL  5000     // display update interval
/*
#define SENS1_UPDATE_INTERVAL    30000    // gy-21 30 30sec
#define SENS2_UPDATE_INTERVAL    40000    // bmp-180 
#define SENS3_UPDATE_INTERVAL    50000    // ds18b20
#define SENS4_UPDATE_INTERVAL    60000    // dht21

#define SEND_DATA_INTERVAL       60000    // send data interval
*/
#define MAX_SCREEN  9

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

SHT2x sht;

char buf[80], text[30];
boolean screenUpdate = true, sensUpdate = false;
boolean isButton1 = true, isDisplay = true;

int next_screen = 0;
int curSens = 0;

// gy-21
float sens1Temp = 0, sens1Hmdt = 0;

// BMP 180
Adafruit_BMP085 bmp;
float bmpPressure = 0, bmpTemp = 0;

// DS18B20
OneWire ds(2);
float sens3Temp = 0;

String readstr;

float readDS();

// dht 1 out
DHT dht(DHTPIN, DHT22);
float dhtTemp, dhtHmdt;

// dht 2 in
DHT dht2(DHT2PIN, DHT22);
float dht2Temp, dht2Hmdt;

// -------------------------------

void setup()
{
  Serial.begin(9600);
  //Serial.println("start...");

  pinMode(LED_PIN, OUTPUT);
  ledOn();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println("SSD1306 allocation failed");
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();

  display.setTextWrap(false);
  display.setTextColor(SSD1306_WHITE);        // Draw white text

// get initail values

  sht.begin();
  sht.read();

  sens1Temp = sht.getTemperature();
  sens1Hmdt = sht.getHumidity();

  delay(500);

  bmp.begin();
  bmpPressure = bmp.readPressure() / 133.3;
  bmpTemp = bmp.readTemperature();

  sens3Temp = readDS();
  delay(500);

  dhtTemp = dht.readTemperature();
  dhtHmdt = dht.readHumidity();
 
  delay(100);

  dht2Temp = dht2.readTemperature();
  dht2Hmdt = dht2.readHumidity();
 
  delay(100);

  ledOff();
}

// -------------------------

void loop()
{
  static unsigned long scr_millis = 0;
//  static unsigned long sens1_millis = 0;
//  static unsigned long sens2_millis = 0;
//  static unsigned long sens3_millis = 0;
//  static unsigned long sens4_millis = 0;
  static unsigned long sens_millis = 0;

  unsigned long cmillis = millis();

// get command 

  if (Serial.available() > 0)
  {
    readstr = Serial.readString();
    //Serial.println(readstr);

    if ( readstr.indexOf("get") >= 0 )
    {
      Serial.print("###");
      Serial.print(";");
      Serial.print(sens1Temp);
      Serial.print(";");
      Serial.print(sens1Hmdt);
      Serial.print(";");
      Serial.print(bmpPressure);
      Serial.print(";");
      Serial.print(bmpTemp);
      Serial.print(";");
      Serial.print(sens3Temp);
      Serial.print(";");
      Serial.print(dhtTemp);
      Serial.print(";");
      Serial.print(dhtHmdt);
      Serial.print(";");
      Serial.print(dht2Temp);
      Serial.print(";");
      Serial.print(dht2Hmdt);
      Serial.println(";");
      blinkXT(5, 50);
    }

// display off

    if ( readstr.indexOf("d0") >= 0 )
    {
      display.clearDisplay();
      display.display();
      isDisplay = false;
      screenUpdate = false;
    }

// display on
    
    if ( readstr.indexOf("d1") >= 0 )
    {
      isDisplay = true;
    }
    
  }

// update sensors --------

  if (cmillis - sens_millis >= SENS_UPDATE_INTERVAL)
  {
    //Serial.println(cmillis);
    //Serial.println(sens_millis);
    //Serial.println(curSens);
    
    curSens++;
    sensUpdate = true;
    sens_millis = cmillis;
    blinkXT(curSens, 100);
  }      

  if (curSens == 1 && sensUpdate)
  {
    //Serial.println("1");
    sens1Temp = sht.getTemperature();
    sens1Hmdt = sht.getHumidity();
  }

  if (curSens == 2 && sensUpdate)
  {
    //Serial.println("2");
    bmpPressure = bmp.readPressure() / 133.3;
    bmpTemp = bmp.readTemperature();
  }

  if (curSens == 3 && sensUpdate)
  {
    //Serial.println("3");
    sens3Temp = readDS();
  }

  if (curSens == 4 && sensUpdate)
  {
    //Serial.println("4");
    dhtTemp = dht.readTemperature();
    dhtHmdt = dht.readHumidity();
  }
  
  if (curSens == 5 && sensUpdate)
  {
    //Serial.println("4");
    dht2Temp = dht2.readTemperature();
    dht2Hmdt = dht2.readHumidity();
  }

// ---

  if (cmillis - scr_millis > DISPLAY_UPDATE_INTERVAL)
  {
    //Serial.println(cmillis);
    scr_millis = cmillis;
    next_screen++;

    screenUpdate = isDisplay;
    //blinkXT(3, 50);
  }      

  // --- Display 

    if (screenUpdate)
    {
      display.clearDisplay();
      display.setCursor(0,0);
      display.setTextSize(1);
      display.println(text);
      display.setCursor(0,10);
      display.setTextSize(3);
      display.println(buf);
      display.display();
    }

// sens1 GY-21 temp

    if (next_screen == 1 && screenUpdate)
    {
        dtostrf(sens1Temp,6,2, buf);
        strcpy(text, "GY-21 temp, C");
    }

// sens1 gy-21 humidity

    if (next_screen == 2 && screenUpdate)
    {
        dtostrf(sens1Hmdt,5,2, buf);
        strcpy(text, "GY-21 humidity,%");
    }

// sens2 BMP-180 pressure

    if (next_screen == 3 && screenUpdate)
    {
        dtostrf(bmpPressure,6,2, buf);
        strcpy(text, "BMP-180 pressure, mmHG");
    }

// sens2 BMP-180 temp

    if (next_screen == 4 && screenUpdate)
    {
        dtostrf(bmpTemp,6,2, buf);
        strcpy(text, "BMP-180 temp, C");
    }

// sens3 DS18B20 temp

    if (next_screen == 5 && screenUpdate)
    {
        dtostrf(sens3Temp,6,2, buf);
        strcpy(text, "DS18B20 temp, C");
    }    

// sens4 DHT temp

    if (next_screen == 6 && screenUpdate)
    {
        dtostrf(dhtTemp,6,2, buf);
        strcpy(text, "dht21 temp, C");
    }  

// sens4 DHT humidity

    if (next_screen == 7 && screenUpdate)
    {
        dtostrf(dhtHmdt,6,2, buf);
        strcpy(text, "dht21, humidity,%");
    }  

// sens5 DHT temp

    if (next_screen == 8 && screenUpdate)
    {
        dtostrf(dht2Temp,6,2, buf);
        strcpy(text, "dht21in temp, C");
    }  

// sens5 DHT humidity

    if (next_screen == 8 && screenUpdate)
    {
        dtostrf(dht2Hmdt,6,2, buf);
        strcpy(text, "dht21in hmdt,%");
    }  
     
  if (next_screen > MAX_SCREEN) next_screen = 0;
  if (curSens >= 5) curSens = 0;

  screenUpdate = false;
  sensUpdate = false;

}

// -----------------------

float readDS()
{
// Определяем температуру от датчика DS18b20
  byte data[2]; // Место для значения температуры
  
  ds.reset(); // Начинаем взаимодействие со сброса всех предыдущих команд и параметров
  ds.write(0xCC); // Даем датчику DS18b20 команду пропустить поиск по адресу. В нашем случае только одно устрйоство 
  ds.write(0x44); // Даем датчику DS18b20 команду измерить температуру. Само значение температуры мы еще не получаем - датчик его положит во внутреннюю память
  
  delay(1000); // Микросхема измеряет температуру, а мы ждем.  
  
  ds.reset(); // Теперь готовимся получить значение измеренной температуры
  ds.write(0xCC); 
  ds.write(0xBE); // Просим передать нам значение регистров со значением температуры

  // Получаем и считываем ответ
  data[0] = ds.read(); // Читаем младший байт значения температуры
  data[1] = ds.read(); // А теперь старший

  // Формируем итоговое значение: 
  //    - сперва "склеиваем" значение, 
  //    - затем умножаем его на коэффициент, соответсвующий разрешающей способности (для 12 бит по умолчанию - это 0,0625)
  float temperature =  ((data[1] << 8) | data[0]) * 0.0625;
  
  // Выводим полученное значение температуры в монитор порта
  //Serial.println(temperature);

  return temperature;
}

void ledOn()
{
  digitalWrite(LED_PIN, HIGH);
}

void ledOff()
{
  digitalWrite(LED_PIN, LOW);
}

// blink 1 sec

void blink1s()
{
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
}

// turn led on t millisec

void blinkT(int t)
{
  int l;

  if (!t || t > 10000) l = 1000;
  
  digitalWrite(LED_PIN, HIGH);
  delay(l);
  digitalWrite(LED_PIN, LOW);
}  

// blink X times

void blinkX(int x)
{
  for(int i = 0; i < x; i++)
  {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
}

void blinkXT(int x, int t)
{
  for(int i = 0; i < x; i++)
  {
    digitalWrite(LED_PIN, HIGH);
    delay(t);
    digitalWrite(LED_PIN, LOW);
    delay(t);
  }
}

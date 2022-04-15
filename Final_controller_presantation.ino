
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <DHT.h>
#include "Wire.h"
#include <OneWire.h>
#include <DallasTemperature.h>


/*--------------------temp sensor-------------------------*/
#define ONE_WIRE_BUS 17
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
/*------------------------WATER LEVEL-----------------------------------------*/
 #define DHTPIN 17


  #define DHTPIN2 16
  #define DHTTYPE DHT11   
  DHT dht(DHTPIN2, DHTTYPE);
int lowerThreshold = 250;
int upperThreshold = 420;

// Sensor pins
#define sensorPower 10
#define sensorPin A6
int val = 0; // Value for storing water level
/*----------------------Timer-------------------------------------------------*/
#define DS3231_I2C_ADDRESS 0x68
// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val){
  return( (val/10*16) + (val%10) );
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val){
  return( (val/16*10) + (val%16) );
}



//setting buttons
#define buttonSet A8
#define buttonUp  A9
#define buttonDown A10
#define buttonOK A11
/*----------------------------------------------------------------------------*/
//buttons states for debouncing
  int menu = 0;
  int oldmenu = 0;
  bool curr_oldState = HIGH;
  bool curr_btSetState = HIGH;
  bool curr_btUpState = HIGH;
  bool curr_btDownState = HIGH;
  bool curr_btOkState = HIGH;

  
  bool prev_btSetState = HIGH;
  bool prev_btUpState = HIGH;
  bool prev_btDownState = HIGH;
  bool prev_btOkState = HIGH; 

unsigned long prevTime_Set = 0;
unsigned long prevTime_Up = 0;
unsigned long prevTime_Dn = 0;
unsigned long prevTime_Ok = 0;

unsigned long waitTime_Set = 5;
unsigned long waitTime_Up = 5;
unsigned long waitTime_Dn = 5;
unsigned long waitTime_Ok = 5;

bool homeSreen = true;

/*-----------------------------------------------------------------------------*/
//Variables for temperiture and humidity
float hum;  //Stores humidity value
float temp; //Stores temperature value

float minTemp;
float minHum;
float maxTemp;
float maxHum;

#define buzz 15
long previousVal = millis();
long currentVal = 4000;
/*--------------Countters for counting min and max--------------------*/

int buttonPushCounter = EEPROM.read(3);   // counter for the number of button presses
int buttonPushCounter1 =  EEPROM.read(1);//min temp
int buttonPushCounter2 = EEPROM.read(5); //minimum humid
int buttonPushCounter3 = EEPROM.read(7); //max humd

int up_buttonState = 0;         // current state of the up button

int up_lastButtonState = 0;     // previous state of the up button


int down_buttonState = 0;         // current state of the up button

int down_lastButtonState = 0;     // previous state of the up button

bool bPress = false;

String message;

/*-------------------------------------*/
const int rs = 23, en = 25, d4 = 28, d5 = 26, d6 = 24, d7 = 22; // ar
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  sensors.begin();
  dht.begin();

  Serial1.println("AT+CNMI=2,2,0,0,0"); // AT Command to receive a live SMS
  // Print a message to the LCD.
  
     pinMode(buttonSet,INPUT_PULLUP);
     pinMode(buttonUp,INPUT_PULLUP);
     pinMode(buttonDown,INPUT_PULLUP);
     pinMode(buttonOK,INPUT_PULLUP);
    /*------------------------------------------------------------------------------*/
    //setting initial state of the buttons to Low

     //digitalWrite(sensorPower, LOW);
      pinMode(buzz, OUTPUT);
      pinMode(A0,OUTPUT);
       pinMode(A1,OUTPUT);
       pinMode(2,OUTPUT);
       pinMode(48,OUTPUT);
       pinMode(40,OUTPUT);
       pinMode(3,OUTPUT);
      pinMode(A5, OUTPUT);
       pinMode(5,OUTPUT);
       pinMode(A2,OUTPUT);
       pinMode(A3,OUTPUT);
       pinMode(A4,OUTPUT);
       pinMode(A3,OUTPUT);
       pinMode(A4,OUTPUT);
       pinMode(42,OUTPUT);
       pinMode(46,OUTPUT);
       pinMode(sensorPower, OUTPUT);

       digitalWrite(42,HIGH);
       digitalWrite(46,HIGH);
       Wire.begin();

     digitalWrite(A1,LOW);
     digitalWrite(A2,LOW);
     //digitalWrite(A5,LOW);
}
/*===================================================================================================================================*/
/*======================================================================================================================================*/

/*-------------------COde for timer with Turn Up and Turn down-------------------*/
void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte
dayOfMonth, byte month, byte year){
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}
void readDS3231time(byte *second,
byte *minute,
byte *hour,
byte *dayOfWeek,
byte *dayOfMonth,
byte *month,
byte *year){
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}
void displayTime(){
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
  &year);
  // send it to the serial monitor
  Serial.print(hour, DEC);
  // convert the byte variable to a decimal number when displayed
  Serial.print(":");
  if (minute<10){
    Serial.print("0");
  }
  Serial.print(minute, DEC);
  Serial.print(":");
  if (second<10){
    Serial.print("0");
  }
  Serial.print(second, DEC);
  Serial.print(" ");
  Serial.print(dayOfMonth, DEC);
  Serial.print("/");
  Serial.print(month, DEC);
  Serial.print("/");
  Serial.print(year, DEC);
  Serial.print(" Day of week: ");
  switch(dayOfWeek){
  case 1:
    Serial.println("Sunday");
    break;
  case 2:
    Serial.println("Monday");
    break;
  case 3:
    Serial.println("Tuesday");
    break;
  case 4:
    Serial.println("Wednesday");
    break;
  case 5:
    Serial.println("Thursday");
    break;
  case 6:
    Serial.println("Friday");
    break;
  case 7:
    Serial.println("Saturday");
    break;
  }
}

void manageTurner(){
    byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
  &year);
/*--------------------------------------------------------Turn up (turn right)--------------------------------------------------------------------------*/
  if(hour == 1 || hour == 3 || hour == 5 || hour == 7 || hour == 9 || hour == 11 || hour == 13 || hour == 15 || hour == 17 || hour == 19 || hour == 21 || hour == 23){
      digitalWrite(A4,HIGH);
      digitalWrite(42,LOW);
        
      if(minute>=10){
      digitalWrite(42,HIGH);
      digitalWrite(A4,LOW);
      
        }
    }
/*--------------------------------------------------------Turn down (turn left)--------------------------------------------------------------------------*/
  if(hour == 2 || hour == 4 || hour == 6 || hour == 8 || hour == 10 || hour == 12 || hour == 14 || hour == 16 || hour == 18 || hour == 20 || hour == 22 || hour == 24){
      digitalWrite(A3,HIGH);
      digitalWrite(46,LOW);
        
      if(minute>=10){
      digitalWrite(A3,LOW);
      digitalWrite(46,HIGH);
        }
    }

  }





/*======================================================================================================================================*/

/*======================================================================================================================================*/
void loop() {
 //Serial1.println("AT+CNMI=2,2,0,0,0"); // AT Command to receive a live SMS
 int menu = 0;
 int oldMenu =0;

  
bool b = digitalRead(buttonOK);
bool curr_btSetRead = digitalRead(buttonSet);
bool curr_btUpRead = digitalRead(buttonUp);
bool curr_btDownRead = digitalRead(buttonDown);
bool curr_btOkRead = digitalRead(buttonOK);



  /*---------------changing state of the screen--------------*/
  if(curr_btSetRead != prev_btSetState){
    waitTime_Set = millis();
  }
  if(curr_btUpRead != prev_btUpState){
  prev_btUpState = millis();
  }
  
if(curr_btDownRead != prev_btDownState){
  prev_btDownState = millis();
  }

if(curr_btOkRead != prev_btOkState){
  prev_btOkState = millis();
  }
  //if( (millis()- prevTime_Set)> waitTime_Set){
    if(curr_btSetRead != curr_btSetState){
        curr_btSetState = curr_btSetRead;
        if(curr_btSetState == HIGH ){
              homeSreen = false;
              menu = oldmenu++;
              if(curr_btSetState == HIGH ){
              homeSreen = false;
              menu = oldmenu;
              Serial.println(menu);
              EEPROMWriteMenu(10, menu);
              if(menu % 5 == 0){
                lcd.clear();
                homeSreen = true;
                
                }else
                if(menu % 5 == 1){
                  homeSreen = false;
                  menu = 1;
                  
                  } else

                  if(menu == 8 ||menu == 13 || menu == 18 || menu == 23 || menu == 28 || menu == 33 || menu == 38 || menu == 43 || menu == 48 ||menu == 53 || menu == 58 ||menu == 63 ||menu == 73 ||menu == 78 ||menu == 83 ||menu == 88 ||menu == 93 ||menu == 98){
                    menu = 3;
                    
                    
                    }

                    else if(menu == 2 ||menu == 7 || menu == 12 || menu == 17 || menu == 22 || menu == 27 || menu == 32 || menu == 37 || menu == 42 ||menu == 47 || menu == 52 ||menu == 57 ||menu == 62 ||menu == 67 ||menu == 72 ||menu == 77 ||menu == 82 ||menu == 87 ||menu == 92 ||menu == 97){
                      menu = 2;
                      
                      }
                      
                     else if(menu == 4 || menu == 9 || menu == 14 || menu == 19 || menu == 24 || menu == 29 || menu == 34 || menu == 39 ||menu == 44 || menu == 49 || menu == 54 || menu == 59 ||menu == 64 ||menu == 69 ||menu == 74 ||menu == 79 ||menu == 84 ||menu == 89  ||menu == 94 ||menu == 99){
                      menu = 4;


                      }
    
             }
          }
      
      }
 // }

 
/*---Commenting te following line will help to make it dificult to enter into settings mode using one button*/
prev_btSetState = curr_btSetRead;
  prev_btUpState = curr_btUpRead;
  prev_btDownState = curr_btDownRead;
prev_btOkState = curr_btOkRead; 
  
  /*--------------------returning to main screen if ok is pressed------*/


  if(curr_btSetRead != curr_btSetState && curr_btOkRead != curr_btOkState){
   curr_btSetState = curr_btSetRead;
   curr_btOkState = curr_btOkRead;
  if( curr_btSetRead ==LOW && curr_btOkRead == LOW ){
    //digitalWrite(A2,HIGH);
    homeSreen = false;
    menu = 1;
    //Serial.print(menu);

    }
      delay(50);
  }

  /*==============Manual turning up================*/
    if(curr_btUpRead != curr_btUpState){
   curr_btUpState = curr_btUpRead;
  if( curr_btUpRead ==LOW){
       digitalWrite(42,HIGH);
       digitalWrite(A4,HIGH);
    }
      delay(50);
  }else{}



/*-----------------------switch cases-----------------------*/
int maxTemp = EEPROMReadMaxTemp(3);
    switch(menu){

        case 1:

        if(menu == 1){  
        lcd.clear();
        lcd.setCursor(0, 0); 
        lcd.print("  Set Min Temp  "); 
        lcd.setCursor(0, 1);
        lcd.print("      ");
        lcd.print(EEPROMReadMaxTemp(1));
        lcd.print(char(223));
        lcd.print("C");
       Serial.print(menu);
            
           }
       
   oldMenu = menu;
        break;

        case 2:
        lcd.clear();
        lcd.setCursor(0, 0); 
        lcd.print("  Set Max Temp  "); 
        lcd.setCursor(0, 1);
        lcd.print("      ");
        lcd.print(EEPROMReadMaxTemp(3));
        lcd.print(char(223));
        lcd.print("C");
        
        oldMenu = menu;
        break;

        case 3:
        lcd.clear();
        lcd.setCursor(0, 0); 
        lcd.print("  Set Min Humid  "); 
        lcd.setCursor(0, 1);
        lcd.print("      ");
        lcd.print(EEPROMReadMaxTemp(5));
        lcd.print("%");
        oldMenu = menu;
        break;


        case 4:
        
        lcd.clear();
        lcd.setCursor(0, 0); 
        lcd.print("  Set Max Humid  "); 
        lcd.setCursor(0, 1);
        lcd.print("      ");
        lcd.print(EEPROMReadMaxTemp(7));
        lcd.print("%");
        oldMenu = menu;

        break;

        default:

        break;
      }
 /*-------calling other functions in main loop*/
gsmCommands();
autoReadWater();
manageTurner();
startBuzz1();
relayLED();
upDown3(); // for maximum Humidity
upDown2(); // for minimum Humidity
upDown1(); //for min Temp up and down    
upDown();  //max temp up and down 
startBuzz();   
startBuzz3();
//digitalWrite(A1,HIGH);
float currentTemp = displayTemp();
float humiD = takeHumidity();

      if(homeSreen == true){ 
                    
               lcd.setCursor(0, 0);           
               lcd.print("TM: "); 
               lcd.print(currentTemp);
               lcd.print(char(223));
              //lcd.print("C ");

               lcd.print(" MN:"); 
               lcd.print(EEPROMReadMaxTemp(1));
                              
               lcd.setCursor(0, 1);
               lcd.print("HM: ");
               lcd.print(humiD);
               lcd.print("% ");

               lcd.print("MN:");
               lcd.print(EEPROMReadMaxTemp(5));
               
      }else
      if(homeSreen == false){
        menu = 1;
        }

}

/*========================Function for increamenting Minimum temp when==============*/

 void checkUp1()

{
  if (up_buttonState != up_lastButtonState) {

    if(buttonPushCounter1 < (buttonPushCounter-1)){
    if (up_buttonState == LOW ) {
    
      bPress = true;
      buttonPushCounter1++;
      EEPROMWriteMaxTemp(1, buttonPushCounter1); //updating the value of max temp on address 3
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print("  Set Min Temp  "); 
      lcd.setCursor(0,1);
      lcd.print("      ");
      lcd.print(EEPROMReadMaxTemp(1));
      lcd.print(char(223));
      lcd.print("C");    

    } 
    }else{
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print("  Warning!: 1 "); 
      }


    delay(50);

  }
 up_lastButtonState = up_buttonState;


}

/*==============Function for decreamenting maximum temperature===================*/
 void checkDown1()

{
  if (down_buttonState != down_lastButtonState) {

    if(buttonPushCounter1 > 20){
    if (down_buttonState == LOW ) {

      bPress = true;

      buttonPushCounter1--;
      EEPROMWriteMaxTemp(1, buttonPushCounter1); //updating the value of max temp on address 3
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print("  Set Min Temp  "); 
      lcd.setCursor(0,1);
      lcd.print("      ");
      lcd.print(EEPROMReadMaxTemp(1));
      lcd.print(char(223));
      lcd.print("C");
    

    }
    }else{
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print("  Warning: 2"); 
      }


    delay(50);

  }
down_lastButtonState = down_buttonState;


}

/*========================================================================================*/

/*========================Function for increamenting max temp when==============*/

 void checkUp()

{
  if (up_buttonState != up_lastButtonState) {

    if( buttonPushCounter <= 45){
      
    if (up_buttonState == LOW) {

        bPress = true;

      buttonPushCounter++;
      EEPROMWriteMaxTemp(3, buttonPushCounter); //updating the value of max temp on address 3
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print("  Set Max Temp  "); 
      lcd.setCursor(0,1);
      lcd.print("      ");
      lcd.print(EEPROMReadMaxTemp(3));
      lcd.print(char(223));
      lcd.print("C");

    }
    }else{
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print("  Warning: 3");
      }

    delay(50);

  }
 up_lastButtonState = up_buttonState;


}

/*==============Function for decreamenting maximum temperature===================*/
 void checkDown()

{
  if (down_buttonState != down_lastButtonState) {

    if(buttonPushCounter >= buttonPushCounter1){
    if (down_buttonState == LOW) {

        bPress = true;

      buttonPushCounter--;
      EEPROMWriteMaxTemp(3, buttonPushCounter); //updating the value of max temp on address 3
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print("  Set Max Temp  "); 
      lcd.setCursor(0,1);
      lcd.print("      ");
      lcd.print(EEPROMReadMaxTemp(3));
      lcd.print(char(223));
      lcd.print("C");

    }
    }else{
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print("  Warning: 4"); 
      }

    delay(50);

  }
down_lastButtonState = down_buttonState;


}


/*========================Function for increamenting Minimum Humidity when==============*/

 void checkUp2()

{
  if (up_buttonState != up_lastButtonState) {

    if(buttonPushCounter2 <  (buttonPushCounter3-1)){
    if (up_buttonState == LOW) {

        bPress = true;

      buttonPushCounter2++;
      EEPROMWriteMaxTemp(5, buttonPushCounter2); //updating the value of max temp on address 3
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print("  Set Min Humid"); 
      lcd.setCursor(0,1);
      lcd.print("      ");
      lcd.print(EEPROMReadMaxTemp(5));
      lcd.print("%");

    }
    }else{
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print("  Warning!: 5");
      }

    delay(50);

  }
 up_lastButtonState = up_buttonState;


}

/*==============Function for decreamenting minmum Humidity===================*/
 void checkDown2()

{
  if (down_buttonState != down_lastButtonState) {

  if( buttonPushCounter2 >20){
    if (down_buttonState == LOW) {

        bPress = true;

      buttonPushCounter2--;
      EEPROMWriteMaxTemp(5, buttonPushCounter2); //updating the value of max temp on address 3
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print("  Set Min Humid  "); 
      lcd.setCursor(0,1);
      lcd.print("      ");
      lcd.print(EEPROMReadMaxTemp(5));
     
      lcd.print("%");

    }
  }else{
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print(" Warning!: 6");
    }

    delay(50);

  }
down_lastButtonState = down_buttonState;


}

/*========================================================================================*/

 void checkUp3()

{
  if (up_buttonState != up_lastButtonState) {

if(buttonPushCounter3 < 90){
    if (up_buttonState == LOW) {

        bPress = true;

      buttonPushCounter3++;
      EEPROMWriteMaxTemp(7, buttonPushCounter3); //updating the value of max temp on address 3
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print("  Set Max Humid"); 
      lcd.setCursor(0,1);
      lcd.print("      ");
      lcd.print(EEPROMReadMaxTemp(7));
      lcd.print("%");

    }
}else{
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print("  Warning!: 7"); 
  }

    delay(50);

  }
 up_lastButtonState = up_buttonState;


}

/*==============Function for decreamenting minmum Humidity===================*/
 void checkDown3()

{
  if (down_buttonState != down_lastButtonState) {

if(buttonPushCounter3 > (buttonPushCounter2+1)){
    if (down_buttonState == LOW) {

        bPress = true;

      buttonPushCounter3--;
      EEPROMWriteMaxTemp(7, buttonPushCounter3); //updating the value of max temp on address 3
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print("  Set Min Humid  "); 
      lcd.setCursor(0,1);
      lcd.print("      ");
      lcd.print(EEPROMReadMaxTemp(7));
     
      lcd.print("%");

    }
}else{
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print("  Warning!: 8  ");
  }

    delay(50);

  }
down_lastButtonState = down_buttonState;


}

/*========================================================================================*/

/*================Function for manipulating up and down button====================*/
void upDown(){
  //Reading current menu from address 10
  int currentMenu = EEPROMReadMenu(10);
  down_buttonState = digitalRead(buttonDown);
  up_buttonState = digitalRead(buttonUp);
    if(currentMenu == 2 ||currentMenu == 7 || currentMenu == 12 || currentMenu == 17 || currentMenu == 22 || currentMenu == 27 || currentMenu == 32 || currentMenu == 37 || currentMenu == 42 ||currentMenu == 47 || currentMenu == 52 ||currentMenu == 57 ||currentMenu == 62 ||currentMenu == 67 ||currentMenu == 72 ||currentMenu == 77 ||currentMenu == 82 ||currentMenu == 87 ||currentMenu == 92 ||currentMenu == 97){
        
        checkDown();
        checkUp();
      }
  
  }

  /*================Function for manipulating up and down button====================*/
void upDown1(){
  //Reading current menu from address 10
  int currentMenu = EEPROMReadMenu(10);
  down_buttonState = digitalRead(buttonDown);
  up_buttonState = digitalRead(buttonUp);
    if(currentMenu == 1 || currentMenu % 5 == 1){
        
        checkDown1();
        checkUp1();
      }
  
  }


/*================Function for manipulating up and down button for minimum humidity====================*/
void upDown2(){
  //Reading current menu from address 10
  int currentMenu = EEPROMReadMenu(10);
  down_buttonState = digitalRead(buttonDown);
  up_buttonState = digitalRead(buttonUp);
    if(currentMenu == 3 || currentMenu == 8 || currentMenu == 13 || currentMenu == 18 || currentMenu == 23 || currentMenu == 28 || currentMenu == 33 || currentMenu == 38 || currentMenu == 43 || currentMenu == 48 ||currentMenu == 53 || currentMenu == 58 ||currentMenu == 63 ||currentMenu== 73 ||currentMenu == 78 ||currentMenu == 83 ||currentMenu == 88 ||currentMenu == 93 ||currentMenu == 98){
        
        checkDown2();
        checkUp2();
      }
  
  }

  /*================Function for manipulating up and down button for maximum humidity====================*/
void upDown3(){
  //Reading current menu from address 10
  int currentMenu = EEPROMReadMenu(10);
  down_buttonState = digitalRead(buttonDown);
  up_buttonState = digitalRead(buttonUp);
    if(currentMenu == 4 || currentMenu == 9 || currentMenu == 14 || currentMenu == 19 || currentMenu == 24 || currentMenu == 29 || currentMenu == 34 || currentMenu == 39 ||currentMenu == 44 || currentMenu == 49 || currentMenu == 54 || currentMenu == 59 ||currentMenu == 64 ||currentMenu == 69 ||currentMenu == 74 ||currentMenu == 79 ||currentMenu == 84 ||currentMenu == 89  ||currentMenu == 94 ||currentMenu == 99){
        
        checkDown3();
        checkUp3();
      }
  
  }

/*----------storing menu status into eeprome--------------*/

void EEPROMWriteMenu(int address, int value)
{
 byte two = (value & 0xFF);
 byte one = ((value >> 8) & 0xFF);

 EEPROM.update(address, two);
 EEPROM.update(address + 1, one);
}
int EEPROMReadMenu(int address)
{
 long two = EEPROM.read(address);
 long one = EEPROM.read(address + 1);
 return ((two << 0) & 0xFFFFFF) + ((one << 8) & 0xFFFFFFFF);
}


/*--------Writting and reading minmum temperature-------*/
void EEPROMWriteMinTemp(int address, int value)
{
 byte two = (value & 0xFF);
 byte one = ((value >> 8) & 0xFF);

 EEPROM.update(address, two);
 EEPROM.update(address + 1, one);
}
int EEPROMReadMinTemp(int address)
{
 long two = EEPROM.read(address);
 long one = EEPROM.read(address + 1);
 return ((two << 0) & 0xFFFFFF) + ((one << 8) & 0xFFFFFFFF);
}

/*-------Writting maximum temperatur-----------------------*/

void EEPROMWriteMaxTemp(int address, int value)
{
 byte two = (value & 0xFF);
 byte one = ((value >> 8) & 0xFF);

 EEPROM.update(address, two);
 EEPROM.update(address + 1, one);
}
int EEPROMReadMaxTemp(int address)
{
 long two = EEPROM.read(address);
 long one = EEPROM.read(address + 1);
 return ((two << 0) & 0xFFFFFF) + ((one << 8) & 0xFFFFFFFF);
}

/*=================================================================================================================================*/
/*==================================================================================================================================*/
float displayTemp(){
sensors.requestTemperatures();
 float temp = sensors.getTempCByIndex(0);
   if(temp != DEVICE_DISCONNECTED_C) 
  {
    Serial.print("Temperature for the device 1 (index 0) is: ");
    Serial.println(temp);
  } 
  else
  {
    Serial.println("Error: Could not read temperature data");
  }

  return temp;
} 
/*=========buzzer for high temperatures===================*/
void startBuzz(){
   float b = displayTemp();
   if(b >= EEPROM.read(3)){
    digitalWrite(A0,HIGH);
if (millis() - previousVal > currentVal)
{
   for(int i =1; i < 2; i++){

tone(buzz,1000);
 delay(500);
   noTone(buzz);
  delay(500);
    }
  for(int i =1; i < 2; i++){

  tone(buzz,1000);
  delay(500);
  noTone(buzz);
  delay(500);
    }
previousVal = millis();
}

    }else{digitalWrite(A0,LOW);}
  }

/*==========================Buzzer for low temperature============*/
void startBuzz1(){
   float b = displayTemp();
   if(b <= EEPROM.read(1)){
    digitalWrite(A0,HIGH);
if (millis() - previousVal > currentVal)
{
   for(int i =1; i < 2; i++){

tone(buzz,5);
 delay(500);
   noTone(buzz);
  delay(500);
    }
  for(int i =1; i < 2; i++){

  tone(buzz,5);
  delay(500);
  noTone(buzz);
  delay(500);
    }
previousVal = millis();
}

    }else{digitalWrite(A0,LOW);}
  }

  
  /*==========================Buzzer for low Humidity============*/
void startBuzz2(){
   float b = takeHumidity();
   if(b <= EEPROM.read(5)){
    digitalWrite(A0,HIGH);
if (millis() - previousVal > currentVal)
{
   for(int i =1; i < 2; i++){

tone(buzz,300);
 delay(500);
   noTone(buzz);
  delay(500);
    }
  for(int i =1; i < 2; i++){

  tone(buzz,300);
  delay(500);
  noTone(buzz);
  delay(500);
    }
previousVal = millis();
}

    }else{digitalWrite(A0,LOW);}
  }


    /*==========================Buzzer for low Humidity============*/
void startBuzz3(){
   float b = takeHumidity();
   if(b >= EEPROM.read(7)){
    digitalWrite(A0,HIGH);
if (millis() - previousVal > currentVal)
{
   for(int i =1; i < 2; i++){

tone(buzz,500);
 delay(500);
   noTone(buzz);
  delay(500);
    }
  for(int i =1; i < 2; i++){

  tone(buzz,300);
  delay(500);
  noTone(buzz);
  delay(500);
    }
previousVal = millis();
}

    }else{digitalWrite(A0,LOW);}
  }

  float takeHumidity(){

     float humid; 
    
    humid = dht.readHumidity();

    return humid;
    }

    void relayLED(){
      float catchTemp = displayTemp();
      float catchHum = takeHumidity();

      if (catchTemp < EEPROM.read(1)+0.5){
          digitalWrite(2,HIGH); //heat1 relay
          digitalWrite(A1,HIGH);

          if(catchTemp < EEPROM.read(1)-1){
            sendSms2();
            }
        }else if(catchTemp > (EEPROM.read(3)+EEPROM.read(1))/2){
          digitalWrite(2,LOW);
          digitalWrite(A1,LOW);
          }

          
       if(catchTemp > EEPROM.read(3)){
          digitalWrite(48,HIGH); //EXHAUSE LELAY
          digitalWrite(40,HIGH);
          
          if(catchTemp > EEPROM.read(3)+1){ //sending sms message for high temp
            sendSms1();
            }else{}
            
        }else{
           digitalWrite(48,LOW); //
          digitalWrite(40,LOW);   
          }

        if(catchHum <= EEPROM.read(5)+2){
          digitalWrite(3,HIGH); //WET LELAY
          digitalWrite(A5,HIGH);
          }else{
          digitalWrite(3,LOW); 
          digitalWrite(A5,LOW);
            }
         if(catchHum <= EEPROM.read(5)){//sending message if humidity is too low
          sendSms4();
          }   

          
          if(catchHum > EEPROM.read(7)){
          digitalWrite(48,HIGH); //EXHAUSE LELAY IF HUMIDITY IS TOO HIGH
          digitalWrite(40,HIGH);
            }else{
          digitalWrite(48,LOW); 
          digitalWrite(40,LOW); 
              }

          if(catchHum >= EEPROM.read(7)){//sending message if humidity is too low
            sendSms3();
          } 

          if(catchTemp < EEPROM.read(1)+0.5){
          digitalWrite(5,HIGH); //HEATER 2
          digitalWrite(A2,HIGH);
          }else if(catchTemp > (EEPROM.read(3)+EEPROM.read(1))/2){
          digitalWrite(5,LOW); 
          digitalWrite(A2,LOW);
            }

                     
          if(catchHum == -127.00){
                noSensorSMS();
            }else{

              }
      }

      
/*===================Water level reading and manipulation==============*/
 void waterLevel(){
int level = readSensor();
Serial.print(level);
  if (level == 0) {
    Serial.println("Water Level: Empty");
    sendSms8();
  }
  else if (level > 0 && level <= lowerThreshold) {
    Serial.println("Water Level: Low");
    sendSms8();
    
  }
  else if (level > lowerThreshold && level <= upperThreshold) {
    Serial.println("Water Level: Medium");

  }
  else if (level > upperThreshold) {
    Serial.println("Water Level: High");
    
  }
        }
        
 int readSensor() {
   byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
  &year);
  if(minute ==1 ||minute == 30){
  digitalWrite(sensorPower, HIGH);
  val = analogRead(sensorPin);
  digitalWrite(sensorPower, LOW);
  
  } else{}
  return val;
}

/*=====================================GSM recieving and executing SMS=====================*/
void gsmCommands(){
  if (Serial1.available()>0){
  message = Serial1.readString();
  }

  if(message.indexOf("HEATER ON") > -1){
     /*tURNING ON HEATER 1 AND 2 AND THEIR RESPECTIVE LEDs*/
      digitalWrite(A2,LOW);
      digitalWrite(A1,LOW);
      digitalWrite(2,LOW);
      digitalWrite(5,LOW);
     sendSms11(); //notification sms
     }
    else if(message.indexOf("HEATER OFF") > -1){
       /*tURNING ON HEATER 1 AND 2 AND THEIR RESPECTIVE LEDs*/
      digitalWrite(A2,LOW);
      digitalWrite(A1,LOW);
      digitalWrite(2,HIGH);
      digitalWrite(5,HIGH);
      sendSms12(); //notification sms

  }

      else if(message.indexOf("DELETE") > -1){
      DeleteAllMessages();
      sendSms14(); //notification sms

  }

     else if(message.indexOf("TURNER ON") > -1){

      digitalWrite(42,LOW);
      digitalWrite(A4,HIGH);
      sendSms13();
      

  }
       else if(message.indexOf("TURNER OFF") > -1){

      digitalWrite(42,HIGH);
      digitalWrite(A4,LOW);
      sendSms16();//Notification sms

  }

       else if(message.indexOf("TURNEL ON") > -1){

      digitalWrite(A3,HIGH);
      digitalWrite(46,LOW);
      sendSms17();////notification sms
      
  }
      else if(message.indexOf("TURNEL OFF") > -1){

      digitalWrite(A3,LOW);
      digitalWrite(46,HIGH);
      sendSms18();//notification sms

  }


      else if(message.indexOf("DEFAULT ST") > -1){
      sendSms21();
      defaultConfig();

  }
      else if(message.indexOf("EXHAUSE ON") > -1){

      digitalWrite(48,LOW);
      digitalWrite(40,HIGH);
      sendSms19(); // nitification sms
      

  }
      else if(message.indexOf("EXHAUSE OFF") > -1){
      digitalWrite(48,LOW);
      digitalWrite(40,HIGH);
      sendSms20();  // nitification 
      

  }
  delay(10);
 
  }

/*============================Reading water level every 30 minutes =========================*/
   void autoReadWater(){
   byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
  &year);
  if(minute ==1 ||minute == 30){
  digitalWrite(sensorPower, HIGH);
  val = analogRead(sensorPin);
  digitalWrite(sensorPower, LOW);
  
  } else{}
  return val;
}



void sendSms1(){
  Serial1.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(500);  // Delay of 1000 milli seconds or 1 second
  Serial1.println("AT+CMGS=\"+265881533682\"\r"); // Replace x with mobile number
  delay(500);
  Serial1.println("Temperature is extremely high. Please ensure the exhaust fan is working properly.");// The SMS text you want to send
  delay(100);
  Serial1.println((char)26);// ASCII code of CTRL+Z
  delay(500);


}
  void sendSms2(){
  Serial1.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(500);  // Delay of 1000 milli seconds or 1 second
  Serial1.println("AT+CMGS=\"+265881533682\"\r"); // Replace x with mobile number
  delay(500);
  Serial1.println("Temperature is too low. Please make sure heaters are working properly.");// The SMS text you want to send
  delay(100);
  Serial1.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
  
  }

    void sendSms3(){
  Serial1.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(500);  // Delay of 1000 milli seconds or 1 second
  Serial1.println("AT+CMGS=\"+265881533682\"\r"); // Replace x with mobile number
  delay(500);
  Serial1.println("Humidity is too high, please make sure there are enough ventilation holes and heaters are working properly.");// The SMS text you want to send
  delay(100);
  Serial1.println((char)26);// ASCII code of CTRL+Z
  delay(500);
  
  }

    void sendSms4(){
  Serial1.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(500);  // Delay of 1000 milli seconds or 1 second
  Serial1.println("AT+CMGS=\"+265881533682\"\r"); // Replace x with mobile number
  delay(500);
  Serial1.println("Humidity is too low. Please make sure there is enough water and humidifier is working properly.");// The SMS text you want to send
  delay(100);
  Serial1.println((char)26);// ASCII code of CTRL+Z
  delay(500);
  
  }

    void sendSms5(){
   Serial1.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(500);  // Delay of 1000 milli seconds or 1 second
  Serial1.println("AT+CMGS=\"+265881533682\"\r"); // Replace x with mobile number
  delay(500);
  Serial1.println("Turner is not working. Please inspect the motor and its connections.");// The SMS text you want to send
  delay(100);
  Serial1.println((char)26);// ASCII code of CTRL+Z
  delay(500);
  while(1);
  }

    void sendSms6(){
   Serial1.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(500);  // Delay of 1000 milli seconds or 1 second
  Serial1.println("AT+CMGS=\"+265881533682\"\r"); // Replace x with mobile number
  delay(500);
  Serial1.println("Circulation fan is not working, please inspect the fan and if the switch is on.");// The SMS text you want to send
  delay(100);
  Serial1.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
  
  }
    void sendSms7(){
   Serial1.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  Serial1.println("AT+CMGS=\"+265881533682\"\r"); // Replace x with mobile number
  delay(1000);
  Serial1.println("7. Exhaust fan has stopped working, please inspect the fan and its connections.");// The SMS text you want to send
  delay(100);
  Serial1.println((char)26);// ASCII code of CTRL+Z
  delay(500);
  
  }

    void sendSms8(){
  Serial1.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(500);  // Delay of 1000 milli seconds or 1 second
  Serial1.println("AT+CMGS=\"+265881533682\"\r"); // Replace x with mobile number
  delay(1000);
  Serial1.println("Water level is too low, please add more water.");// The SMS text you want to send
  delay(100);
  Serial1.println((char)26);// ASCII code of CTRL+Z
  delay(500);
  
  }

    void sendSms9(){
   Serial1.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  Serial1.println("AT+CMGS=\"+265881533682\"\r"); // Replace x with mobile number
  delay(1000);
  Serial1.println("Current controller configuration settings A. Min temperature: ");// The SMS text you want to send

  }

    void help(){
   Serial1.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  Serial1.println("AT+CMGS=\"+265881533682\"\r"); // Replace x with mobile number
  delay(1000);
  Serial1.println("1.DELETE: Delete all messages from GSM 2.HEATER ON: Turning on Heater 3.HEATER OFF: Turning off heaters 4.WET ON: Turning humidifier on 5.WET OFF");

  delay(100);
  Serial1.println((char)26);// ASCII code of CTRL+Z
 delay(1000);

  }

  void DeleteAllMessages()
{
   Serial.println("Setting the GSM in text mode");
   Serial1.println("AT+CMGF=1\r");
   Serial1.println("AT+CMGDA=\"DEL ALL\"\r");
   delay(1000);

}


/*========Waiting function to assist gsm not to send multiple messages at the same time=======*/
  void waitFunction(){
  Serial1.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  Serial1.println("AT+CMGS=\"+265881533682\"\r"); // Replace x with mobile number
  delay(1000);
  Serial1.println("");// The SMS text you want to send
  delay(100);
  Serial1.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
  while(1);
  }

    void sendSms11(){
   Serial1.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  Serial1.println("AT+CMGS=\"+265881533682\"\r"); // Replace x with mobile number
  delay(1000);
  Serial1.println("Heaters are turned on");// The SMS text you want to send
  delay(100);
  Serial1.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
  }

    void sendSms12(){
   Serial1.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  Serial1.println("AT+CMGS=\"+265881533682\"\r"); // Replace x with mobile number
  delay(1000);
  Serial1.println("heaters are turned OFF");// The SMS text you want to send
  delay(100);
  Serial1.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
  }

    void sendSms13(){
   Serial1.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  Serial1.println("AT+CMGS=\"+265881533682\"\r"); // Replace x with mobile number
  delay(1000);
  Serial1.println("Turner R is turned ON");// The SMS text you want to send
  delay(100);
  Serial1.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
  }

    void sendSms14(){
   Serial1.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  Serial1.println("AT+CMGS=\"+265881533682\"\r"); // Replace x with mobile number
  delay(1000);
  Serial1.println("Messages have been deleted successfully");// The SMS text you want to send
  delay(100);
  Serial1.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
  }

  void noSensorSMS(){
  Serial1.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  Serial1.println("AT+CMGS=\"+265881533682\"\r"); // Replace x with mobile number
  delay(1000);
  Serial1.println("Temperature sensor is not well connected");// The SMS text you want to send
  delay(100);
  Serial1.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
    }
    void sendSms16(){
  Serial1.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  Serial1.println("AT+CMGS=\"+265881533682\"\r"); // Replace x with mobile number
  delay(1000);
  Serial1.println("Turn R is Turned OFF");// The SMS text you want to send
  delay(100);
  Serial1.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
      }

  void sendSms17(){
  Serial1.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  Serial1.println("AT+CMGS=\"+265881533682\"\r"); // Replace x with mobile number
  delay(1000);
  Serial1.println("Turn L is Turned ON");// The SMS text you want to send
  delay(100);
  Serial1.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
    }

 void sendSms18(){
  Serial1.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  Serial1.println("AT+CMGS=\"+265881533682\"\r"); // Replace x with mobile number
  delay(1000);
  Serial1.println("Turn L is Turned OFF");// The SMS text you want to send
  delay(100);
  Serial1.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
  }

   void sendSms19(){
  Serial1.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  Serial1.println("AT+CMGS=\"+265881533682\"\r"); // Replace x with mobile number
  delay(1000);
  Serial1.println("Exhause fun is turned ON");// The SMS text you want to send
  delay(100);
  Serial1.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
  }

   void sendSms20(){
  Serial1.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  Serial1.println("AT+CMGS=\"+265881533682\"\r"); // Replace x with mobile number
  delay(1000);
  Serial1.println("Exhause fun is turned OFF");// The SMS text you want to send
  delay(100);
  Serial1.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
  }

     void sendSms21(){
  Serial1.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  Serial1.println("AT+CMGS=\"+265881533682\"\r"); // Replace x with mobile number
  delay(1000);
  Serial1.println("Default configuration are set successfully");// The SMS text you want to send
  delay(100);
  Serial1.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
  }


  void defaultConfig(){
  EEPROM.update(1,35);
  EEPROM.update(3,37);
  EEPROM.update(5,60);
  EEPROM.update(7,80);
  }


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

  #define DHTPIN2 16
  #define DHTTYPE DHT11   
  DHT dht(DHTPIN2, DHTTYPE);

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

#define buttonLoad A12
#define buttonChick A13
#define buttonQuil A14
#define buttonDuck A15
#define buttonTurk 49
#define buttonGu 47
//loading buttons

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
long currentVal = 50;
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


  
     pinMode(buttonSet,INPUT_PULLUP);
     pinMode(buttonUp,INPUT_PULLUP);
     pinMode(buttonDown,INPUT_PULLUP);
     pinMode(buttonOK,INPUT_PULLUP);


 
     pinMode(buttonLoad,INPUT_PULLUP);
     pinMode(buttonChick,INPUT_PULLUP);
     pinMode(buttonQuil,INPUT_PULLUP);
     pinMode(buttonDuck,INPUT_PULLUP);
     pinMode(buttonTurk,INPUT_PULLUP);
     pinMode(buttonGu,INPUT_PULLUP);




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

       //relay pins initial state
       digitalWrite(48,HIGH);
       //digitalWrite(46,HIGH);
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
  if(minute == 1 || minute == 6 || minute == 12 || minute == 18 || minute == 24 || minute == 30 || minute == 36 || minute == 42 || minute == 48 || minute == 54){
      digitalWrite(A4,HIGH);
      digitalWrite(42,LOW);
        

    } else{
      digitalWrite(42,HIGH);
      digitalWrite(A4,LOW);
      }
/*--------------------------------------------------------Turn down (turn left)--------------------------------------------------------------------------*/
  if(minute == 3 || minute == 9 || minute == 15 || minute == 21 || minute == 27 || minute == 33 || minute == 39 || minute == 45 || minute == 51 || minute == 57){
      digitalWrite(A3,HIGH);
      digitalWrite(46,LOW);
        
    }else{
      digitalWrite(A3,LOW);
      digitalWrite(46,HIGH);
      }

  }





/*======================================================================================================================================*/

/*======================================================================================================================================*/
void loop() {
 //Serial1.println("AT+CNMI=2,2,0,0,0"); // AT Command to receive a live SMS
 int menu = 0;
 int oldMenu =0;



bool curr_btLoadRead = digitalRead(buttonLoad);
bool curr_btChickRead = digitalRead(buttonChick);
bool curr_btQuilRead = digitalRead(buttonQuil);
bool curr_btDuckRead = digitalRead(buttonDuck);
bool curr_btTurkRead = digitalRead(buttonTurk);
bool curr_btGuRead = digitalRead(buttonGu);


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
      //delay(50);
  }

  
  if(curr_btLoadRead != curr_btSetState && curr_btOkRead != curr_btOkState){
   curr_btSetState = curr_btSetRead;
   curr_btOkState = curr_btOkRead;
  if( curr_btSetRead ==LOW && curr_btOkRead == LOW ){
    //digitalWrite(A2,HIGH);
    homeSreen = false;
    menu = 1;
    //Serial.print(menu);

    }
      //delay(50);
  }


//Loding

     pinMode(buttonLoad,INPUT_PULLUP);
     pinMode(buttonChick,INPUT_PULLUP);
     pinMode(buttonQuil,INPUT_PULLUP);
     pinMode(buttonDuck,INPUT_PULLUP);
     pinMode(buttonTurk,INPUT_PULLUP);
     pinMode(buttonGu,INPUT_PULLUP);


     if(digitalRead(buttonLoad) == HIGH && digitalRead(buttonChick) == HIGH){
        digitalWrite(39,HIGH);
      }else{
        digitalWrite(39,LOW);
        }

        
     if(digitalRead(buttonLoad) == HIGH && digitalRead(buttonTurk) == HIGH){
        digitalWrite(39,HIGH);
      }else{
        digitalWrite(39,LOW);
        }

         if(digitalRead(buttonLoad) == HIGH && digitalRead(buttonQuil) == HIGH){
        digitalWrite(39,HIGH);
      }else{
        digitalWrite(39,LOW);
        }

 if(digitalRead(buttonLoad) == HIGH && digitalRead(buttonDuck) == HIGH){
        digitalWrite(39,HIGH);
      }else{
        digitalWrite(39,LOW);
        }
 if(digitalRead(buttonLoad) == HIGH && digitalRead(buttonGu) == HIGH){
        digitalWrite(39,HIGH);
      }else{
        digitalWrite(39,LOW);
        }




  /*==============Manual turning up================*/
  if(curr_btUpRead != curr_btUpState){
  curr_btUpState = curr_btUpRead;
  if( curr_btUpRead ==HIGH){
       digitalWrite(42,HIGH);
       digitalWrite(A4,HIGH);
    }
      //delay(50);
  }else{}

  /*==============Manual turning down================*/
  if(curr_btDownRead != curr_btDownState){
  curr_btDownState = curr_btDownRead;
  if( curr_btDownRead == HIGH){
      digitalWrite(A3,HIGH);
      digitalWrite(46,LOW);
    }
      //delay(50);
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


   // delay(50);

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


    //delay(50);

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

    //delay(50);

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

   // delay(50);

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

   // delay(50);

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

    //delay(50);

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

   // delay(50);

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

   // delay(50);

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
alarm();
    }else{digitalWrite(A0,LOW);}
  }

/*==========================Buzzer for low temperature============*/
void startBuzz1(){
   float b = displayTemp();
   if(b <= EEPROM.read(1)){
    digitalWrite(A0,HIGH);
alarm();

    }else{digitalWrite(A0,LOW);}
  }

  
  /*==========================Buzzer for low Humidity============*/
void startBuzz2(){
   float b = takeHumidity();
   if(b <= EEPROM.read(5)){
    digitalWrite(A0,HIGH);
alarm();

    }else{digitalWrite(A0,LOW);}
  }


    /*==========================Buzzer for low Humidity============*/
void startBuzz3(){
   float b = takeHumidity();
   if(b >= EEPROM.read(7)){
    digitalWrite(A0,HIGH);
    alarm();

    }else{digitalWrite(A0,LOW);}
  }

  void alarm(){
    if (millis() - previousVal > currentVal)
{
   for(int i =1; i < 2; i++){

tone(buzz,500);
 delay(500);
   noTone(buzz);
  delay(500);
    }
  for(int i =1; i < 2; i++){

  tone(buzz,500);
  delay(500);
  noTone(buzz);
  delay(500);
    }
previousVal = millis();
}
    }

  float takeHumidity(){

     float humid; 
    
    humid = dht.readHumidity();

    return humid;
    }

   void relayLED(){
      float catchTemp = displayTemp();
      float catchHum = takeHumidity();

      if (catchTemp <= EEPROM.read(1)+0.5){
          digitalWrite(2,LOW); //heat1 relay
          digitalWrite(A1,HIGH);
        }else{
          digitalWrite(2,HIGH);
          digitalWrite(A1,LOW);
          }
       if(catchTemp >= EEPROM.read(3)-0.5){
          digitalWrite(48,LOW); //EXHAUSE LELAY
          digitalWrite(40,HIGH);
        }else{
           digitalWrite(48,HIGH); //
          digitalWrite(40,LOW);   
          }

        if(catchHum <= EEPROM.read(5)*1){
          digitalWrite(3,LOW); //WET LELAY
          digitalWrite(A5,HIGH);
          }else{
          digitalWrite(3,HIGH); //EXHAUSE LELAY
          digitalWrite(A5,LOW);
            }

          if(catchTemp < EEPROM.read(1)+0.5){
          digitalWrite(5,LOW); //HEATER 2
          digitalWrite(A2,HIGH);
          }else{
          digitalWrite(5,HIGH); 
          digitalWrite(A2,LOW);
            }
      }

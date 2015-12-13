//************************************************************************************/
//LIBRARIES
#include <SPI.h>
#include <Adafruit_GFX.h>   //Screen Graphics library
#include <Adafruit_ILI9341.h>  //Screen Library
#include <Adafruit_FT6206.h>   //Touch Screen Library
#include <Time.h>			// Time functions Library
#include <Wire.h>      // this is needed for FT6206
#include <DS1307.h>	// RTC Library
#include <EEPROM.h>  // EEPROM Library
#include <EEPROMAnything.h> //EEPROM Writer Library
#include <OneWire.h>  //OneWire Sensor Bus Library
#include <DallasTemperature.h> //Dallas Temp Library

//Arduino MEGA pinout
//Digital Pins
#define FEEDER_PWR 5 
#define WATER_FILTER_PWR 6    
#define LIGHTS_PWR 7
#define TFT_DC 9	//Touch Screen
#define TFT_CS 10 	//Touch Screen
#define ONE_WIRE_BUS 49  //Temp Sensor

//Anaglog Pins
#define HEATER_PWR 40
#define RTC_GND A10
#define RTC_PWR A11
#define TEMP_SENSOR_PWR A14 
#define OUTLET_BOX_PWR A8


//Display Screen Variables
#define VER 1.00
#define ROTATION 3

//Menu Screens
#define MAINSCREEN 0
#define MENUSCREEN_ONE 1
#define CLOCKSETUP 2
#define TEMPCONTROL 3
#define LIGHTS  6 
#define GENERAL_SETTINGS 12
#define AUTOMATIC_FEEDER 13
#define FEEDER_TIMER 14
#define MAINTENANCE 15

//Misc defines
#define TEMERATURE_PRECISION 9
#define NUM_LIGHTS 2

//Declare which fonts to be utilized
#define XLARGE 4
#define LARGE 3
#define MEDIUM 2
#define SMALL 1
#define CENTER 9998

#define ILI9341_GRAY  0x4208

//TOUCH PANEL & Adafruit ILI9341 
// The FT6206 uses hardware I2C (SCL/SDA)
// Use hardware SPI for CS/DC
Adafruit_FT6206 ctp = Adafruit_FT6206();

//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// screen size  w = 320 h = 240
// For the Adafruit shield, these are the default.
 //touch coordinates
int x, y; 
int x1, y1;                       

unsigned long previousMillisFive = 0; //Used in the Main Loop (Checks Time,Temp,LEDs,Screen)
unsigned long previousMillisAlarm = 0; //Used in the Alarm
unsigned long previousFeedingMillis;  //Used in feeding time display on main screen
// const int tempHeatPin = 36; //Heater on/off (set thermostat on heater to highest desired level)

int setScreensaver = 1; //ON=1 || OFF=2 (change in prog)
int screenSaverTimer = 0; //counter for Screen Saver
int setScreenSaverTimer = ( 5 ) * 12; //how long in (minutes) before Screensaver comes on
boolean SCREEN_RETURN = true; //Auto Return to mainScreen() after so long of inactivity
boolean screenSaverRunning = false;
int returnTimer = 0; //counter for Screen Return
int setReturnTimer = setScreenSaverTimer * .75; //Will return to main screen after 75% of the amount of
																//time it takes before the screensaver turns on
int dispScreen = 0;  //0-Main Screen, 1-Menu, 2-Clock Setup, 3-Temp Control,
							//12-General Settings, 13-Automatic Feeder,
							// 14-Set Feeder Timers, 15-About
int filterOffMinutes = 5; //start water filter off display minutes
int filterOffSeconds = 0; //start water filter off display seconds

//RTC data
tmElements_t tm;

int countcycle; 

int FEEDTime1, FEEDTime2, FEEDTime3, FEEDTime4, FEEDTimeNow;
int feedTime;
int feedFish1H, feedFish1M,          //Times to feed the fish
    feedFish2H, feedFish2M, feedFish3H, feedFish3M, feedFish4H, feedFish4M;
int FEEDTime;

int lightTime1H, lightTime1M, lightTime1S, 
    lightTime2H, lightTime2M, lightTime2S;
int lightTime;


boolean Lights_On_Flag = false;
boolean checkTemp = true;

// DS18B20 Temperture Sensor plugged into pin 51
OneWire oneWire ( ONE_WIRE_BUS );  
// Pass outr oneWire reference to Dallas Temperature.
DallasTemperature sensors ( &oneWire );
// Assign the addresses of temperature sensors.  Add/Change addresses as needed.
DeviceAddress waterThermometer = { 0x28, 0xB7, 0xDE, 0x5F, 0x06, 0x00, 0x00, 0xF3 };

float tempW = 0;                     //Water temperature values

int setCalendarFormat = 1; //DD/MM/YYYY=0 || Month DD, YYYY=1 (change in prog)
char otherData [19]; // other printed data
							//char formattetTime12[9]; //07:48 PM
							//char formattetTime24[6]; //19:48
							//byte data [56];

// Maintenance itme reminder names. for Maintenance Screen and main display.
char* maintName [ ] = {"Water Change","Charcoal Filter","NH3 Filter","Sponge Filter"};

char time [11], date [16]; //day [16];
char  time1 [11];
int timeDispH, timeDispM, xTimeH, xTimeM10, xTimeM1, xTimeAMPM, xColon;
int timeDispH1, timeDispM1, xTimeH1;
int rtc [7], rtcSet [7], rtcSet2 [7]; //Clock arrays
int AM_PM, yTime, yTime1; //Setting clock stuff
int setTimeFormat = 1; //24HR=0 || 12HR=1 (change in prog)
int setTempScale = 0; //Celsius=0 || Fahrenheit=1 (change in prog)
int setAutoStop = 2; //ON=1 || OFF=2 (change in prog)

int light_num = 0;
int light_on_time[3] = {0,0,0};
int lighttimes[NUM_LIGHTS][3] = {{ lightTime1H, lightTime1M, lightTime1S}, 
                                 { lightTime2H, lightTime2M, lightTime2S}};
int doyC = 0;
int daym, monthm, yearm;
int doy1C, doy2C, doy3C, doy4C;
int maint1D, maint2D, maint3D, maint4D;


boolean waterfilterStopped = false; 
boolean feederMotorRunning = false;
boolean WaterFilerCtrl_1 = false;
boolean WaterFilerCtrl_2 = false;
boolean WaterFilerCtrl_3 = false;
boolean WaterFilerCtrl_4 = false;
boolean WaterFilerCtrl_Now = false;

boolean alarm1 = false;				
boolean alarm2 = false;
boolean alarm3 = false;
boolean alarm4 = false;
boolean tempAlarmflag = false;
boolean CLOCK_SCREENSAVER = true; //For a Clock Screensaver "true" / Blank Screen "false"
											 //You can turn the Screensaver ON/OFF in the pogram

int fiveTillBackOn1, fiveTillBackOn2;
int tenSecTimer;

float temp2beS; //Temporary Temperature Values
float temp2beO; //Temporary Temperature Values
float temp2beA; //Temporary Temperature Values
char degC_F [2]; //Used in the Conversion of Celsius to Fahrenheit
float setTempC = 0.0; //Desired Water Temperature (User input in program)
float setTempF = 0.0; 
float offTempC = 0.0; //Desired Water Temp. Offsets for Heater & Chiller (User input in program)
float offTempF = 0.0;
float alarmTempC = 0.0; //Temperature the Alarm will sound (User input in program)
float alarmTempF = 0.0; 
float setTempCF = 0.0; //Desired Water Temperature (User input in program)
float offTempCF = 0.0; //Desired Water Temp. Offsets for Heater & Chiller (User input in program)
float alarmTempCF = 0.0; //Temperature the Alarm will sound (User input in program)
// boolean tempCoolflag = 0; //1 if cooling on
boolean tempHeatflag = false; //true if heating on


/**************************** CHOOSE OPTION MENU1 BUTTONS *****************************/
const int tanD [] = { 10, 30, 120, 35 }; //"TIME and DATE" settings
const int temC [] = { 10, 70, 120, 35 }; //"H2O TEMP CONTROL" settings
const int gSet [] = { 10, 110, 120, 35 }; //"GENERAL SETTINGS" page
const int aFeed [] = { 165, 30, 120, 35 }; //"AUTOMATIC FEEDER" menu
const int maint [] = { 165, 110, 120, 35 }; //"Maintenance Settings" program information
const int liteS [] = { 165, 70, 120, 35 }; //'LIGHT CONTROL" page
/**************************** TIME AND DATE SCREEN BUTTONS ***************************/
const int houU [] = { 110, 22, 25, 25 }; //hour up
const int minU [] = { 180, 22, 25, 25 }; //min up
const int ampmU [] = { 265, 22, 25, 25 }; //AM/PM up
const int houD [] = { 110, 78, 25, 25 }; //hour down
const int minD [] = { 180, 78, 25, 25 }; //min down
const int ampmD [] = { 265, 78, 25, 25 }; //AM/PM down
const int dayU [] = { 110, 112, 25, 25 }; //day up
const int monU [] = { 180, 112, 25, 25 }; //month up
const int yeaU [] = { 265, 112, 25, 25 }; //year up
const int dayD [] = { 110, 162, 25, 25 }; //day down
const int monD [] = { 180, 162, 25, 25 }; //month down
const int yeaD [] = { 265, 162, 25, 25 }; //year down
/*************************** H2O TEMP CONTROL SCREEN BUTTONS *************************/
const int temM [] = { 90, 49, 25, 25 }; //temp. minus
const int temP [] = { 205, 49, 25, 25 }; //temp. plus
const int offM [] = { 90, 99, 25, 25 }; //offset minus
const int offP [] = { 205, 99, 25, 25 }; //offset plus
const int almM [] = { 90, 149, 25, 25 }; //alarm minus
const int almP [] = { 205, 149, 25, 25 }; //alarm plus
/**************************** LIGHT CONTROL MENU BUTTONS *****************************/
const int hou1U [] = { 110, 25, 25, 25 }; //Light On hour up
const int min1U [] = { 180, 25, 25, 25 }; //Light On min up
// const int ampm1U [] = { 265, 25, 25, 25 }; //Light On AM/PM up
const int hou1D [] = { 110, 76, 25, 25 }; //Light On hour down
const int min1D [] = { 180, 76, 25, 25 }; //Light On min down
// const int ampm1D [] = { 265, 76, 25, 25 }; //Light On AM/PM down
const int hou2U [] = { 110, 112, 25, 25 }; //Light Off hour up
const int min2U [] = { 180, 112, 25, 25 }; //Light Off min up
// const int ampm2U [] = { 265, 112, 25, 25 }; //Light Off AM/PM up
const int hou2D [] = { 110, 162, 25, 25 }; //Light Off hour down
const int min2D [] = { 180, 162, 25, 25 }; //Light Off min down
// const int ampm2D [] = { 265, 162, 25, 25 }; //Light Off AM/PM down
/*************************** Maintenacne Menu Buttons ********************************/
const int day1D [] = {120, 39, 25, 25 }; 		
const int day2D [] = {120, 79, 25, 25 };
const int day3D [] = {120, 119, 25, 25 };
const int day4D [] = {120, 159, 25, 25 };
const int day1U [] = {205, 39, 25, 25 };
const int day2U [] = {205, 79, 25, 25 };
const int day3U [] = {205, 119, 25, 25 };
const int day4U [] = {205, 159, 25, 25 };
const int day1R [] = {250, 39, 40, 25 };
const int day2R [] = {250, 79, 40, 25 };
const int day3R [] = {250, 119, 40, 25 };
const int day4R [] = {250, 159, 40, 25 };
/*********************************** MISCELLANEOUS BUTTONS ***************************/
const int back [] = { 5, 200, 100, 25 }; //BACK
const int prSAVE [] = { 110, 200, 100, 25 }; //SAVE or NEXT
const int canC [] = {215 , 200, 100, 25 }; //CANCEL
/************************* AUTOMATIC FISH FEEDER BUTTONS *****************************/
//These Buttons are made within the function
/******************* SET AUTOMATIC FISH FEEDING TIMES BUTTONS ************************/
const int houP [] = { 110, 38, 25, 25 }; //hour up
const int minP [] = { 180, 38, 25, 25 }; //min up
// const int ampmP [] = { 265, 38, 25, 25 }; //AM/PM up
const int houM [] = { 110, 89, 25, 25 }; //hour down
const int minM [] = { 180, 89, 25, 25 }; //min down
// const int ampmM [] = { 265, 89, 25, 25 }; //AM/PM down
/*********************** Maint Item Display on Main Screen ***************************/
const int mdis1 [] = { 30, 150};
const int mdis2 [] = { 30, 160};
const int mdis3 [] = { 30, 170};
const int mdis4 [] = { 30, 180};
// const int maint [] = { 165, 110, 120, 35 }; //"Maintenance Settings" program information
/********************************* EEPROM FUNCTIONS **********************************/
struct config_t 
{
	int tempset;
	int tempFset;
	int tempoff;
	int tempFoff;
	int tempalarm;
	int tempFalarm;
} tempSettings;  //640 - 660

struct config_g 
{
	int calendarFormat;
	int timeFormat;
	int tempScale;
	int SCREENsaver;
	int autoStop;
} GENERALsettings;  //660 - 680

struct config_l 
{
  int lightTime1h;
  int lightTime1m;
 // int lightTime1s;
  int lightTime2h;
  int lightTime2m;
  // int lightTime2s;
} LIGHTsettings;  //16 - 31  Tank Light On/Off Times

struct config_f 
{
	int feedFish1h;
	int feedFish1m;
	int feedFish2h;
	int feedFish2m;
	int feedFish3h;
	int feedFish3m;
	int feedFish4h;
	int feedFish4m;
	int feedTime1;
	int feedTime2;
	int feedTime3;
	int feedTime4;
} FEEDERsettings;  //680 - 700

struct config_m
{
	int maint1d;
	int maint2d;
	int maint3d;
	int maint4d;
} MAINTsettings; //Maintenance task days 

struct config_doy //Day of Year Start Date for maint task
{
	int doy1c;
	int doy2c;
	int doy3c;
	int doy4c;
} DOYsettings; //Maintenance DOY settings

void SaveTempToEEPROM () 
{
	tempSettings.tempset = int ( setTempC * 10 );
	tempSettings.tempFset = int ( setTempF * 10 );
	tempSettings.tempoff = int ( offTempC * 10 );
	tempSettings.tempFoff = int ( offTempF * 10 );
	tempSettings.tempalarm = int ( alarmTempC * 10 );
	tempSettings.tempFalarm = int ( alarmTempF * 10 );
	EEPROM_writeAnything ( 640, tempSettings );
}

void SaveGenSetsToEEPROM () 
{
	GENERALsettings.calendarFormat = int ( setCalendarFormat );
	GENERALsettings.timeFormat = int ( setTimeFormat );
	GENERALsettings.tempScale = int ( setTempScale );
	GENERALsettings.SCREENsaver = int ( setScreensaver );
	GENERALsettings.autoStop = int ( setAutoStop );
	EEPROM_writeAnything ( 660, GENERALsettings );
}

void SaveLightTimesToEEPROM () 
{
  LIGHTsettings.lightTime1h = lightTime1H;
  LIGHTsettings.lightTime1m = lightTime1M;
  // LIGHTsettings.lightTime1s = lightTime1S;
  LIGHTsettings.lightTime2h = lightTime2H;
  LIGHTsettings.lightTime2m = lightTime2M;
  // LIGHTsettings.lightTime2s = lightTime2S;
  EEPROM_writeAnything ( 16, LIGHTsettings );
}

void SaveFeedTimesToEEPROM () 
{
	FEEDERsettings.feedFish1h = int ( feedFish1H );
	FEEDERsettings.feedFish1m = int ( feedFish1M );
	FEEDERsettings.feedFish2h = int ( feedFish2H );
	FEEDERsettings.feedFish2m = int ( feedFish2M );
	FEEDERsettings.feedFish3h = int ( feedFish3H );
	FEEDERsettings.feedFish3m = int ( feedFish3M );
	FEEDERsettings.feedFish4h = int ( feedFish4H );
	FEEDERsettings.feedFish4m = int ( feedFish4M );
	FEEDERsettings.feedTime1 = int ( FEEDTime1 );
	FEEDERsettings.feedTime2 = int ( FEEDTime2 );
	FEEDERsettings.feedTime3 = int ( FEEDTime3 );
	FEEDERsettings.feedTime4 = int ( FEEDTime4 );
	EEPROM_writeAnything ( 680, FEEDERsettings );
}

void SaveMaintDaysToEEPROM ()
{
	MAINTsettings.maint1d = int ( maint1D );
	MAINTsettings.maint2d = int ( maint2D );
	MAINTsettings.maint3d = int ( maint3D );
	MAINTsettings.maint4d = int ( maint4D );
	EEPROM_writeAnything ( 710, MAINTsettings );
}
void SaveDoyToEEPROM ()
{
	DOYsettings.doy1c = int ( doy1C );
	DOYsettings.doy2c = int ( doy2C );
	DOYsettings.doy3c = int ( doy3C );
	DOYsettings.doy4c = int ( doy4C );
	EEPROM_writeAnything ( 720, DOYsettings );
}
void ReadFromEEPROM () 
{
	int k = EEPROM.read ( 0 );
	char tempString [3];

EEPROM_readAnything ( 640, tempSettings );
	setTempC = tempSettings.tempset;
	setTempC /= 10;
	setTempF = tempSettings.tempFset;
	setTempF /= 10;
	offTempC = tempSettings.tempoff;
	offTempC /= 10;
	offTempF = tempSettings.tempFoff;
	offTempF /= 10;
	alarmTempC = tempSettings.tempalarm;
	alarmTempC /= 10;
	alarmTempF = tempSettings.tempFalarm;
	alarmTempF /= 10;

	EEPROM_readAnything ( 660, GENERALsettings );
	setCalendarFormat = GENERALsettings.calendarFormat;
	setTimeFormat = GENERALsettings.timeFormat;
	setTempScale = GENERALsettings.tempScale;
	setScreensaver = GENERALsettings.SCREENsaver;
	setAutoStop = GENERALsettings.autoStop;

	EEPROM_readAnything ( 16, LIGHTsettings );
  	lightTime1H = LIGHTsettings.lightTime1h;
  	lightTime1M = LIGHTsettings.lightTime1m;
  	// lightTime1S = LIGHTsettings.lightTime1s;
  	lightTime2H = LIGHTsettings.lightTime2h;
  	lightTime2M = LIGHTsettings.lightTime2m;
  	// lightTime2S = LIGHTsettings.lightTime2s;

	EEPROM_readAnything ( 680, FEEDERsettings );
	feedFish1H = FEEDERsettings.feedFish1h;
	feedFish1M = FEEDERsettings.feedFish1m;
	feedFish2H = FEEDERsettings.feedFish2h;
	feedFish2M = FEEDERsettings.feedFish2m;
	feedFish3H = FEEDERsettings.feedFish3h;
	feedFish3M = FEEDERsettings.feedFish3m;
	feedFish4H = FEEDERsettings.feedFish4h;
	feedFish4M = FEEDERsettings.feedFish4m;
	FEEDTime1 = FEEDERsettings.feedTime1;
	FEEDTime2 = FEEDERsettings.feedTime2;
	FEEDTime3 = FEEDERsettings.feedTime3;
	FEEDTime4 = FEEDERsettings.feedTime4;

	EEPROM_readAnything ( 710, MAINTsettings );
	maint1D = MAINTsettings.maint1d;
	maint2D = MAINTsettings.maint2d;
	maint3D = MAINTsettings.maint3d;
	maint4D = MAINTsettings.maint4d;

	EEPROM_readAnything ( 720, DOYsettings );
	doy1C = DOYsettings.doy1c;
	doy2C = DOYsettings.doy2c;
	doy3C = DOYsettings.doy3c;
	doy4C = DOYsettings.doy4c;
}

/********************************** RTC FUNCTIONS ************************************/
void SaveRTC () 
{
	int year = rtcSet [6] - 2000;
	RTC.stop (); //RTC clock setup
	RTC.set ( DS1307_SEC, 1 );
	RTC.set ( DS1307_MIN, rtcSet [1] );
	RTC.set ( DS1307_HR, rtcSet [2] );
	RTC.set ( DS1307_DATE, rtcSet [4] );
	RTC.set ( DS1307_MTH, rtcSet [5] );
	RTC.set ( DS1307_YR, year );
	delay ( 10 );
	RTC.start ();
	delay ( 10 );
}

/********************************** TIME AND DATE BAR ********************************/
void TimeDateBar ( boolean refreshAll = false ) 
{
	char oldVal [16], minute1 [3], hour1 [3], ampm [6], month1 [5];
	tft.setTextSize ( SMALL );
	if ( ( rtc [1] >= 0 ) && ( rtc [1] <= 9 ) ) 
	{
		sprintf ( minute1, "%i%i", 0, rtc [1] ); //adds 0 to minutes
	}              
	else 
	{
		sprintf ( minute1, "%i", rtc [1] );
	}
	if ( setTimeFormat == 1 ) 
	{
		if ( rtc [2] == 0 ) 
		{
			sprintf ( hour1, "%i", 12 );  //12 HR Format
		}              
		else 
		{
			if ( rtc [2] > 12 ) { sprintf ( hour1, "%i", rtc [2] - 12 ); }
			else { sprintf ( hour1, "%i", rtc [2] ); }
		}
	}
	if ( rtc [2] < 12 ) { sprintf ( ampm, " AM  " ); } //Adding the AM/PM sufffix            
	else { sprintf ( ampm, " PM  " ); } sprintf ( oldVal, "%i", time ); //refresh time if different
	if ( setTimeFormat == 1 ) { sprintf ( time, "%s:%s%s", hour1, minute1, ampm ); }
	else { sprintf ( time, " %i:%s      ", rtc [2], minute1 ); }
	if ( ( oldVal != time ) || refreshAll ) 
	{
		tft.fillRoundRect ( 215, 227, 60, 10, 10/8, ILI9341_BLACK);
		tft.setTextColor ( ILI9341_YELLOW ); tft.setCursor ( 215, 227 ); tft.print ( time ); //Display time
	}
	if ( rtc [5] == 1 ) { sprintf ( month1, "JAN " ); } //Convert the month to its name            
	if ( rtc [5] == 2 ) { sprintf ( month1, "FEB " ); }
	if ( rtc [5] == 3 ) { sprintf ( month1, "MAR " ); }
	if ( rtc [5] == 4 ) { sprintf ( month1, "APR " ); }
	if ( rtc [5] == 5 ) { sprintf ( month1, "MAY " ); }
	if ( rtc [5] == 6 ) { sprintf ( month1, "JUN " ); }
	if ( rtc [5] == 7 ) { sprintf ( month1, "JLY " ); }
	if ( rtc [5] == 8 ) { sprintf ( month1, "AUG " ); }
	if ( rtc [5] == 9 ) { sprintf ( month1, "SEP " ); }
	if ( rtc [5] == 10 ) { sprintf ( month1, "OCT " ); }
	if ( rtc [5] == 11 ) { sprintf ( month1, "NOV " );	}
	if ( rtc [5] == 12 ) { sprintf ( month1, "DEC " );	}
	sprintf ( oldVal, "%s", date ); //refresh date if different
	if ( setCalendarFormat == 0 ) { sprintf ( date, "  %i/%i/%i   ", rtc [4], rtc [5], rtc [6] ); 
	}
	else 
	{
		sprintf ( date, "  %s%i, %i", month1, rtc [4], rtc [6] );
	}
	if ( ( oldVal != date ) || refreshAll ) 
	{
		tft.fillRoundRect ( 20, 227, 80, 10, 10/8, ILI9341_BLACK);
		tft.setTextColor ( ILI9341_YELLOW ); tft.setCursor ( 20, 227 ); tft.print ( date ); //Display date
	}
}

/******************************** TEMPERATURE FUNCTIONS *******************************/
void checkTempC () 
{
	if ( checkTemp ) 
	{
		sensors.requestTemperatures (); // call sensors.requestTemperatures() to issue a global
											  	  // temperature request to all devices on the bus
		if ( setTempScale == 1 )   //F scale
		{	
			tempW = ( sensors.getTempF ( waterThermometer ) );  //read water temperature
			setTempCF = setTempF; offTempCF = offTempF; alarmTempCF = alarmTempF;
		}
		else
		{	
			tempW = ( sensors.getTempC ( waterThermometer ) );  //read water temperature
			setTempCF = setTempC; offTempCF = offTempC; alarmTempCF = alarmTempC;
		}	
		if ( tempW < ( setTempCF + offTempCF + alarmTempCF ) && tempW > ( setTempCF - offTempCF - alarmTempCF ) ) 
		{
			tempAlarmflag = false;
		}
		if ( tempW < ( setTempCF + offTempCF ) && tempW > ( setTempCF - offTempCF ) )  //turn off heater
		{
			tempHeatflag = false; digitalWrite ( HEATER_PWR, LOW );
		}
		if ( offTempCF > 0 ) 
		{
			if ( tempW <= ( setTempCF - offTempCF ) ) //turn an heater
			{
				tempHeatflag = true;	digitalWrite ( HEATER_PWR, HIGH );
			}
		}
		if ( alarmTempCF > 0 ) //turn on alarm
		{
			if ( ( tempW >= ( setTempCF + offTempCF + alarmTempCF ) ) || ( tempW <= ( setTempCF - offTempCF - alarmTempCF ) ) ) 
			{
				tempAlarmflag = true;
				unsigned long cMillis = millis ();
			}
		}
	}
}

/*********************** MAIN SCREEN ********** dispScreen = 0 ************************/
void mainScreen( boolean refreshAll = false ) 
{
	if ( dispScreen != 0) {}	
	if ( refreshAll ) //If refeshAll is true do the following
	{
		screenSaverRunning = false;
		tft.fillScreen(ILI9341_BLACK);
		tft.drawRect( 0, 0, 320, 240, ILI9341_BLUE); //Outside Border
		dispScreen = 0; tft.fillScreen(ILI9341_BLACK);
		tft.drawRect( 0, 0, 320, 240, ILI9341_BLUE); //Outside Border
		dispScreen = 0;
		tft.setTextColor(ILI9341_WHITE);  tft.setTextSize( SMALL );
		tft.drawRect ( 0, 125, 320, 12, ILI9341_BLUE); //Horizontal Divider
		tft.drawRect (158,137, 2, (240-137), ILI9341_BLUE ); //Vertical Divider
		tft.fillRect ( 0, 0, 319, 14, ILI9341_BLUE ); //Top Bar
		tft.setCursor(60, 4); 
		setFont ( SMALL, ILI9341_YELLOW );
		tft.print(F("Bill's Aquarium Controller v")); tft.print(VER);
		setFont ( SMALL, ILI9341_RED );
		printTxt ( "MONITORS & ALERTS", 110, 128 );
		tft.fillRoundRect ( 28, 58, 120, 14, 14/8, ILI9341_BLACK );
	}
	else
	{	
		doyC = calculateDayOfYear(rtc [4],rtc [5],rtc [6]); 
		setFont ( SMALL, ILI9341_YELLOW );
		if ( Lights_On_Flag == true ) { printTxt ( "Light Output On", 30, 60); }
		if (doy1C <= doyC) { alarm1 = true; } else { alarm1 = false; } //check for Maint Items due
		if (doy2C <= doyC) { alarm2 = true; } else { alarm2 = false; }
		if (doy3C <= doyC) { alarm3 = true; } else { alarm3 = false; }
		if (doy4C <= doyC) { alarm4 = true; } else { alarm4 = false; }
		if ( ( alarm1 == true ) && ( alarm2 == false ) && (alarm3 == false ) & ( alarm4 == false ) ) 
		{
			// alarm 1
			setFont ( SMALL, ILI9341_YELLOW ); 
			printTxt ( maintName [0], mdis1 [0], mdis1 [1] ); tft.print (" Due");
		}
		if ( ( alarm1 == true ) && ( alarm2 == true ) && (alarm3 == false ) & ( alarm4 == false ) ) 
		{
			// alarm 1 & 2
			setFont ( SMALL, ILI9341_YELLOW );
			printTxt ( maintName [0], mdis1 [0], mdis1 [1] ); tft.print (" Due");
			printTxt ( maintName [1], mdis2 [0], mdis2 [1] ); tft.print (" Due");
		}
		if ( ( alarm1 == true ) && ( alarm2 == true ) && (alarm3 == true ) & ( alarm4 == false ) ) 
		{
			// alarm 1, 2, & 3
			setFont ( SMALL, ILI9341_YELLOW );
			printTxt ( maintName [0], mdis1 [0], mdis1 [1] ); tft.print (" Due");
			printTxt ( maintName [1], mdis2 [0], mdis2 [1] ); tft.print (" Due");
			printTxt ( maintName [2], mdis3 [0], mdis3 [1] ); tft.print (" Due");
		}
		if ( ( alarm1 == true ) && ( alarm2 == true ) && (alarm3 == true ) & ( alarm4 == true ) ) 
		{
			// alarm 1, 2, 3, & 4
			setFont ( SMALL, ILI9341_YELLOW );
			printTxt ( maintName [0], mdis1 [0], mdis1 [1] ); tft.print (" Due");
			printTxt ( maintName [1], mdis2 [0], mdis2 [1] ); tft.print (" Due");
			printTxt ( maintName [2], mdis3 [0], mdis3 [1] ); tft.print (" Due");
			printTxt ( maintName [3], mdis4 [0], mdis4 [1] ); tft.print (" Due");
		}
		if ( ( alarm1 == true ) && ( alarm2 == false ) && (alarm3 == true ) & ( alarm4 == false ) ) 
		{
			// alerm 1 & 3
			setFont ( SMALL, ILI9341_YELLOW );
			printTxt ( maintName [0], mdis1 [0], mdis1 [1] ); tft.print (" Due");
			printTxt ( maintName [2], mdis2 [0], mdis2 [1] ); tft.print (" Due");
		}
		if ( ( alarm1 == true ) && ( alarm2 == false ) && (alarm3 == true ) & ( alarm4 == true ) ) 
		{
			// alarm 1, 3, & 4
			setFont ( SMALL, ILI9341_YELLOW );
			printTxt ( maintName [0], mdis1 [0], mdis1 [1] ); tft.print (" Due");
			printTxt ( maintName [2], mdis2 [0], mdis2 [1] ); tft.print (" Due");
			printTxt ( maintName [3], mdis3 [0], mdis3 [1] ); tft.print (" Due");
		}
		if ( ( alarm1 == true ) && ( alarm2 == false ) && (alarm3 == false ) & ( alarm4 == true ) ) 
		{
			// alarm 1 & 4
			setFont ( SMALL, ILI9341_YELLOW );
			printTxt ( maintName [0], mdis1 [0], mdis1 [1] ); tft.print (" Due");
			printTxt ( maintName [3], mdis2 [0], mdis2 [1] ); tft.print (" Due");
		}
		if ( ( alarm1 == false ) && ( alarm2 == true ) && (alarm3 == false ) & ( alarm4 == false ) ) 
		{
			// alarm 2 
			setFont ( SMALL, ILI9341_YELLOW );
			printTxt ( maintName [1],  mdis1 [0], mdis1 [1] ); tft.print (" Due");
		}
		if ( ( alarm1 == false ) && ( alarm2 == true ) && (alarm3 == true ) & ( alarm4 == false ) ) 
		{
			// alarm 2 & 3
			setFont ( SMALL, ILI9341_YELLOW );
			printTxt ( maintName [1], mdis1 [0], mdis1 [1] ); tft.print (" Due");
			printTxt ( maintName [2], mdis2 [0], mdis2 [1] ); tft.print (" Due");
		}
		if ( ( alarm1 == false ) && ( alarm2 == true ) && (alarm3 == true ) & ( alarm4 == true ) ) 
		{
			// alarm 2, 3 & 4
			setFont ( SMALL, ILI9341_YELLOW );
			tft.setCursor ( 40, 150 );
			printTxt ( maintName [1], mdis1 [0], mdis1 [1] ); tft.print (" Due");
			printTxt ( maintName [2], mdis2 [0], mdis2 [1] ); tft.print (" Due");
			printTxt ( maintName [3], mdis3 [0], mdis3 [1] ); tft.print (" Due");
		}
		if ( ( alarm1 == false ) && ( alarm2 == true ) && (alarm3 == false ) & ( alarm4 == true ) ) 
		{
			// alarm 2 & 4
			setFont ( SMALL, ILI9341_YELLOW );
			printTxt ( maintName [1], mdis1 [0], mdis1 [1] ); tft.print (" Due");
			printTxt ( maintName [3], mdis2 [0], mdis2 [1] ); tft.print (" Due");
		}
		if ( ( alarm1 == false ) && ( alarm2 == false ) && (alarm3 == true ) & ( alarm4 == false ) ) 
		{
			// alarm 3
			setFont ( SMALL, ILI9341_YELLOW );
			printTxt ( maintName [2], mdis1 [0], mdis1 [1] ); tft.print (" Due");
		}
		if ( ( alarm1 == false ) && ( alarm2 == false ) && (alarm3 == true ) & ( alarm4 == false ) ) 
		{
			// alarm 3 & 4
			setFont ( SMALL, ILI9341_YELLOW );
			printTxt ( maintName [2], mdis1 [0], mdis1 [1] ); tft.print (" Due");
			printTxt ( maintName [3], mdis2 [0], mdis2 [1] ); tft.print (" Due");
		}
		if ( ( alarm1 == false ) && ( alarm2 == false ) && (alarm3 == false ) & ( alarm4 == true ) ) 
		{
			// alarm 4
			setFont ( SMALL, ILI9341_YELLOW );
			printTxt ( maintName [3], mdis1 [0], mdis1 [1] ); tft.print (" Due");
		}
		if ( ( waterfilterStopped == true ) && ( dispScreen == 0 ) && ( screenSaverRunning == 0 ) ) //Display water filter countdown timer
		{
			tft.fillRect ( 170, 20, 142, 16, ILI9341_BLACK ); setFont ( MEDIUM, ILI9341_YELLOW );	printTxt ( "Water Filter", 170, 20 );
			tft.fillRect ( 180, 46, 120, 16, ILI9341_BLACK ); printTxt ( "Back On in", 180, 46 );
		}
		else
		{
			tft.fillRect ( 170, 20, 142, 16, ILI9341_BLACK ); tft.fillRect ( 180, 46, 120, 16, ILI9341_BLACK );
			tft.fillRect ( 216, 80, 16, 22, ILI9341_BLACK ); tft.fillRect ( 230, 80, 58, 22, ILI9341_BLACK );
		}
		if ( setTempScale == 1 )   //Print deg C or deg F, degC_F=deg;
		{
			sprintf ( degC_F, "F" ); //deg ="F"
			if ( ( tempW > 80 ) || ( tempW < 50 ) )
			{
				setFont ( SMALL, ILI9341_RED ); printTxt ( "Error", 260, 148 ); //range in deg F
			} 
		}               
		else 
		{
			sprintf ( degC_F, "C" ); //deg = "C"
			if ( ( tempW > 50 ) || ( tempW < 10 ) ) { setFont ( SMALL, ILI9341_RED ); printTxt ( "Error", 260, 148 ); }
		}
		setFont ( SMALL, ILI9341_GREEN ); printTxt ( "Water Temp:", 169, 148 );
		tft.drawCircle ( 304, 150, 1, ILI9341_GREEN ); printTxt ( degC_F, 309, 148 );
		tft.fillRoundRect ( 200, 189, 65, 14, 14/8, ILI9341_BLACK );
		// tft.fillRoundRect ( 182, 203, 123, 22, 18/8, ILI9341_RED );
		tft.fillRoundRect ( 260, 148, 30, 8, 12/8, ILI9341_BLACK );	
		if ( tempHeatflag == true ) //Water temperature too LOW
		{
			setFont ( SMALL, ILI9341_BLUE );
			tft.setCursor ( 260, 148 ); tft.print (tempW, 1);
			setFont ( SMALL, ILI9341_YELLOW ); printTxt ( "Heater ON", 203, 191 );
		}
		if ( tempAlarmflag == true )	//Water temperature too HIGH
		{
			setFont ( SMALL, ILI9341_RED );
			tft.setCursor ( 260, 148 ); tft.print (tempW, 1);
			setFont ( LARGE, ILI9341_GREEN ); printTxt ( "ALARM!!", 185, 204 );
		}
		else 
		{
			setFont ( SMALL, ILI9341_GREEN );
			tft.setCursor ( 260, 148 ); tft.print (tempW, 1);
			tft.fillRect ( 185, 204, 126, 22, ILI9341_BLACK );
		}
	}

}
void screenReturn () 		//Auto Return to MainScreen()
{                                   
	if ( SCREEN_RETURN == true ) 
	{
		if ( dispScreen != 0 ) 
		{
			if (ctp.touched () ) { processMyTouch (); }
			else { returnTimer++; }
			if ( returnTimer > setReturnTimer ) { returnTimer = 0; ReadFromEEPROM (); dispScreen = 0; clearScreen (); mainScreen ( true ); }
		}
	}
}

/********************************* MISC. FUNCTIONS ************************************/
void clearScreen () 
{
	tft.fillRect ( 1, 15, 318, 226, ILI9341_BLACK );
}

void printTxt ( char* text, int16_t x1, int16_t y1 ) 
{
	tft.setCursor ( x1, y1 );
	tft.print ( text );
}

void printVar (int16_t var, int16_t x1, int16_t y1) 
{
	tft.setCursor ( x1, y1 );
	tft.print ( var );
}

void printButton ( char* text, int x1, int y1, int x2, int y2, boolean fontsize = false, int buttonColor = 0 ) 
{
	int stl = strlen ( text );
	int fx, fy;

	tft.fillRoundRect ( x1, y1, x2, y2, y2 / 8, ILI9341_BLUE );
	tft.drawRoundRect ( x1, y1, x2, y2, y2 / 8, ILI9341_WHITE );
	tft.setTextColor(ILI9341_WHITE);

	if ( fontsize ) 
	{
		tft.setTextSize ( SMALL );
		fx = x1 + ( ( x2 / 2 ) - ( ( stl * 6 ) / 2) );
		fy = y1 + ( ( y2 / 2 ) - 3 );
		printTxt ( text , fx, fy );
	}
	else 
	{
		tft.setTextSize ( SMALL );
		fx = x1 + ( ( x2 / 2 ) - ( ( stl * 6 ) / 2) );
		fy = y1 + ( ( y2 / 2 ) - 3 );
		printTxt ( text , fx, fy );
	}
}
void printctr ( char* ctrtxt, int fy )
{
	int stl = strlen ( ctrtxt );
	int fx;
	fx = 1 + ( ( 318 / 2 ) - ( ( stl * 6 ) / 2) );
	tft.setCursor ( fx, fy );
}

void printHeader ( char* headline ) 
{
	int stl = strlen ( headline );
	int fx, fy;
	
	tft.fillRect ( 1, 1, 318, 14, ILI9341_YELLOW );
	fx = 1 + ( ( 318 / 2 ) - ( ( stl * 6 ) / 2) );
	fy = 3 ;
	tft.setCursor ( fx, fy);
	tft.setTextSize ( SMALL );
	tft.setTextColor(ILI9341_BLACK);
	tft.printf( headline );
	
}
void setFont ( boolean font,  int uint16_t) 
{
	tft.setTextColor (  uint16_t );                  
	if ( font == LARGE )	tft.setTextSize ( LARGE );  
	else if ( font == SMALL ) tft.setTextSize ( SMALL );
	else if ( font == MEDIUM ) tft.setTextSize ( MEDIUM );
	else if ( font == XLARGE ) tft.setTextSize ( XLARGE );
}
void waitForIt ( int x1, int y1, int x2, int y2 ) // Draw a red frame while a button is touched
{
	tft.drawRoundRect ( x1, y1, x2, y2, y2 / 8, ILI9341_RED );
	while ( ctp.touched () ) 
	{
		delay(125);
	}
	tft.drawRoundRect ( x1, y1, x2, y2, y2 / 8, ILI9341_WHITE );
}
void feedingTimeOnOff () 
{
	if ( feedTime == 1 ) { FEEDTime = FEEDTime1; }
	if ( feedTime == 2 ) { FEEDTime = FEEDTime2; }
	if ( feedTime == 3 ) { FEEDTime = FEEDTime3; }
	if ( feedTime == 4 ) { FEEDTime = FEEDTime4; }
	if ( ( FEEDTime == 1 ) ) 
	{
		tft.fillRoundRect ( 70, 150, 180, 20 , 20/8, ILI9341_GREEN );
		tft.setCursor( 94, 154 ); setFont ( SMALL, ILI9341_BLACK ); 
		tft.printf ( "Feeding Time %d ON", feedTime );
	}
	if ( (  FEEDTime == 0 ) ) 
	{
		tft.fillRoundRect ( 70, 150, 180, 20, 20/8, ILI9341_RED );
		tft.setCursor( 94, 154 ); setFont (SMALL, ILI9341_WHITE );
		tft.printf ( "Feeding Time %d OFF", feedTime );
	}
	tft.drawRoundRect ( 70, 150, 180, 20, 20/8, ILI9341_WHITE );
}	
void TimeSaver ( boolean refreshAll = false ) 
{
	if ( setTimeFormat == 0 )  //24HR Format
	{
		setFont ( XLARGE, ILI9341_BLUE );
		if ( ( rtc [2] >= 0 ) && ( rtc [2] <= 9 ) ) //Display HOUR
		{
			tft.fillRoundRect ( 80, 95, 55, 50, 20/8, ILI9341_BLACK );
			tft.setTextColor ( ILI9341_BLUE );
			tft.setCursor ( 112, 95 ); tft.print ( rtc [2] );
		}
		else 
		{
			tft.fillRoundRect ( 80, 95, 55, 50, 20/8, ILI9341_BLACK );
			tft.setCursor ( 80, 95 ); tft.print ( rtc [2] );
		}
	}

	if ( setTimeFormat == 1 ) //12HR Format
	{
		setFont ( XLARGE, ILI9341_BLUE );
		if ( rtc [2] == 0 ) //Display HOUR
		{
			tft.fillRoundRect ( 80, 95, 55, 50, 20/8, ILI9341_BLACK );
			printTxt ( "12", 80, 95 );
		}
		if ( ( rtc [2] >= 1 ) && ( rtc [2] <= 9 ) ) 
		{
			tft.fillRoundRect ( 80, 95, 55, 50, 20/8, ILI9341_BLACK );
			tft.setTextColor ( ILI9341_BLUE ); printVar ( rtc [2], 112, 95 );
		}
		if ( ( rtc [2] >= 10 ) && ( rtc [2] <= 12 ) ) 
		{
			tft.fillRoundRect ( 80, 95, 55, 50, 20/8, ILI9341_BLACK );
			printVar ( rtc[2], 80, 95 );
		}
		if ( ( rtc [2] >= 13 ) && ( rtc [2] <= 21 ) ) 
		{
			tft.fillRoundRect ( 80, 95, 55, 50, 20/8, ILI9341_BLACK );
			tft.setTextColor ( ILI9341_BLUE ); printVar ( ( rtc [2] - 12 ), 112, 95 );
		}
		if ( rtc [2] >= 22 ) 
		{
			tft.fillRoundRect ( 80, 95, 55, 50, 20/8, ILI9341_BLACK );
			printVar ( ( rtc [2] - 12 ), 80, 95 );
		}

		if ( ( rtc [2] >= 0 ) && ( rtc [2] <= 11 ) ) //Display AM/PM
		{
			setFont ( MEDIUM, ILI9341_BLUE ); printTxt ( "AM", 220, 108 );
		}
		else 
		{
			setFont ( MEDIUM, ILI9341_BLUE ); printTxt ( "PM", 220, 108 );
		}
	}
	setFont ( XLARGE, ILI9341_BLUE );
	tft.fillCircle ( 140, 100, 3, ILI9341_BLUE );
	tft.fillCircle ( 140, 115, 3, ILI9341_BLUE );
	
	if ( ( rtc [1] >= 0 ) && ( rtc [1] <= 9 ) ) //Display MINUTES
	{
		tft.fillRoundRect ( 156, 95, 63, 50, 20/8, ILI9341_BLACK );
		printTxt ( "0", 156, 95 ); printVar ( rtc [1], 188, 95 );
	}
	else {
		tft.fillRoundRect ( 156, 95, 63, 50, 20/8, ILI9341_BLACK );
		printVar ( rtc [1], 156, 95 );
	}
}

void screenSaver ()  //Make the Screen Go Blank after so long
{
	if ( ( setScreensaver == 1 ) && ( tempAlarmflag == false ) ) 
	{
		if (ctp.touched () ) { processMyTouch (); screenSaverRunning = false; screenSaverTimer = 0; } 
		else { screenSaverTimer++; }
		if ( screenSaverTimer == setScreenSaverTimer ) { dispScreen = 0;
			tft.fillScreen(ILI9341_BLACK); tft.setCursor(20, 120); screenSaverRunning = true; }
		if ( CLOCK_SCREENSAVER == true ) 
		{ 
			if ( screenSaverTimer > setScreenSaverTimer )
			{ 
			dispScreen = 0; TimeSaver ( true ); 
			 
			}
		}
	}	
	if ( ( setScreensaver == 1 ) && ( tempAlarmflag == true ) && ( dispScreen == 0) ) 
	{
		if ( screenSaverTimer > setScreenSaverTimer )
		{
			returnTimer = 0;
			screenSaverTimer = 0;
			screenSaverRunning = false;
			clearScreen();
			mainScreen( true );
		}
		else
		{
			returnTimer = 0;
			screenSaverTimer = 0;
			screenSaverRunning = false;
			mainScreen ();
		}
	}
}	

void genSetSelect () 
{
	if ( setCalendarFormat == 0 ) //Calendar Format Buttons
	{	tft.fillRoundRect ( 185, 19, 120, 20, 20/8, ILI9341_GREEN );
		setFont ( SMALL, ILI9341_BLACK ); printTxt ( "DD MM YYYY", 207, 23 );
		tft.fillRoundRect ( 185, 45, 120, 20, 20/8, ILI9341_BLUE );
		setFont (SMALL, ILI9341_WHITE); printTxt ( "MTH DD YYYY", 199, 49 );
	}
	else 
	{
		tft.fillRoundRect ( 185, 19, 120, 20, 20/8, ILI9341_BLUE );
		setFont ( SMALL, ILI9341_WHITE ); printTxt ( "DD MM YYYY", 207, 23 );
		tft.fillRoundRect ( 185, 45, 120, 20, 20/8, ILI9341_GREEN );
		setFont (SMALL, ILI9341_BLACK ); printTxt ( "MTH DD YYYY", 199, 49 );
	}
	if ( setTimeFormat == 0 ) //Time Format Buttons
	{
		tft.fillRoundRect ( 195, 76, 40, 20, 20/8, ILI9341_BLUE );
		setFont ( SMALL, ILI9341_WHITE ); printTxt ( "12HR", 201, 80);
		tft.fillRoundRect ( 255, 76, 40, 20, 20/8, ILI9341_GREEN );
		setFont ( SMALL, ILI9341_BLACK); printTxt ( "24HR", 261, 80);
	}
	else 
	{
		tft.fillRoundRect ( 195, 76, 40, 20, 20/8, ILI9341_GREEN  );
		setFont ( SMALL, ILI9341_BLACK ); printTxt ( "12HR", 201, 80 );
		tft.fillRoundRect ( 255, 76, 40, 20, 20/8, ILI9341_BLUE  );
		setFont ( SMALL, ILI9341_WHITE ); printTxt ( "24HR", 261, 80 );
	}
	if ( setTempScale == 0 )  //Temperature Scale Buttons
	{
		tft.fillRoundRect ( 195, 107, 40, 20, 20/8, ILI9341_GREEN  );
		setFont ( SMALL, ILI9341_BLACK ); printTxt ( "C", 215, 111 );
		tft.fillRoundRect ( 255, 107, 40, 20, 20/8, ILI9341_BLUE  );
		setFont ( SMALL, ILI9341_WHITE ); printTxt ( "F", 275, 111 );
		tft.drawCircle( 210, 113, 1, ILI9341_BLACK );
		tft.drawCircle ( 270, 113, 1, ILI9341_WHITE );
	}
	else 
	{
		tft.fillRoundRect ( 195, 107, 40, 20, 20/8, ILI9341_BLUE  );
		setFont (SMALL, ILI9341_WHITE ); printTxt ( "C", 215, 111 );
		tft.fillRoundRect ( 255, 107, 40, 20, 20/8, ILI9341_GREEN  );
		setFont (SMALL, ILI9341_BLACK ); printTxt ( "F", 275, 111 );
		tft.drawCircle( 210, 113, 1, ILI9341_WHITE );
		tft.drawCircle ( 270, 113, 1, ILI9341_BLACK );
	}	
	if ( setScreensaver == 1 )  //Screensaver Buttons
	{
		tft.fillRoundRect ( 195, 138, 40, 20, 20/8, ILI9341_GREEN  );
		setFont ( SMALL, ILI9341_BLACK ); printTxt ( "ON", 209, 142 );
		tft.fillRoundRect ( 255, 138, 40, 20, 20/8, ILI9341_BLUE  );
		setFont ( SMALL, ILI9341_WHITE ); printTxt ( "OFF", 265, 142 );
	}
	else 
	{
		tft.fillRoundRect ( 195, 138, 40, 20, 20/8, ILI9341_BLUE  );
		setFont ( SMALL, ILI9341_WHITE ); printTxt ( "ON", 209, 142 );
		tft.fillRoundRect ( 255, 138, 40, 20, 20/8, ILI9341_GREEN  );
		setFont ( SMALL, ILI9341_BLACK ); printTxt ( "OFF", 265, 142 );
	}
	if ( setAutoStop == 1 ) //Auto-Stop on Feed Buttons
	{
		tft.fillRoundRect ( 195, 169, 40, 20, 20/8, ILI9341_GREEN  );
		setFont ( SMALL, ILI9341_BLACK ); printTxt ( "ON", 209, 173 );
		tft.fillRoundRect ( 255, 169, 40, 20, 20/8, ILI9341_BLUE  );
		setFont ( SMALL, ILI9341_WHITE ); printTxt ( "OFF", 265, 173 );
	}
	else 
	{
		tft.fillRoundRect ( 195, 169, 40, 20, 20/8, ILI9341_BLUE  );
		setFont ( SMALL, ILI9341_WHITE ); printTxt ( "ON", 209, 173 );
		tft.fillRoundRect ( 255, 169, 40, 20, 20/8, ILI9341_GREEN  );
		setFont ( SMALL, ILI9341_BLACK ); printTxt ( "OFF", 265, 173 );
	}
	tft.drawRoundRect ( 185, 19, 120, 20, 20/8, ILI9341_WHITE );
	tft.drawRoundRect ( 185, 45, 120, 20, 20/8, ILI9341_WHITE );
	for ( int x = 0; x < 2; x++ ) 
	{
		for ( int y = 0; y < 4; y++ ) 
		{
			tft.drawRoundRect ( ( x * 60 ) + 195, ( y * 31 ) + 76, 40, 20 , 20/8, ILI9341_WHITE);
		}
	}	
}		

int calculateDayOfYear(int daym, int monthm, int yearm) 
{
  // Given a day, month, and year (4 digit), returns 
  // the day of year. Errors return 999.
  int daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  if (yearm < 1000) { return 999; } // Verify we got a 4-digit year
  
  if (yearm%4  == 0) { if (yearm%100 != 0) { daysInMonth[1] = 29; } // Check if it is a leap year
    else { if (yearm%400 == 0) { daysInMonth[1] = 29; } } }
  // Make sure we are on a valid day of the month
  if (daym < 1) { return 999; } else if (daym > daysInMonth[monthm-1]) { return 999; }
  int doy = 0;
  for (int i = 0; i < monthm - 1; i++) { doy += daysInMonth[i]; }
  doy += daym; return doy;
}

void waterFilterTimer ()
{
	 unsigned long currentFeedingMillis = millis ();
	if ( ( filterOffMinutes > 0 ) && ( filterOffSeconds >= 0) || ( filterOffMinutes == 0 ) && ( filterOffSeconds > 0 ))
	{
 		if ( currentFeedingMillis - previousFeedingMillis >= 1000 ) 
		{ 
			if ( waterfilterStopped == true )
			{
				if ( dispScreen == 0 )
				{
					if ( screenSaverRunning == false ) 
					{
						// tft.fillRect ( 170, 20, 142, 16, ILI9341_BLACK ); setFont ( MEDIUM, ILI9341_YELLOW );	printTxt ( "Water Filter", 170, 20 );
						// tft.fillRect ( 180, 46, 120, 16, ILI9341_BLACK ); printTxt ( "Back On in", 180, 46 );
						tft.fillRect ( 216, 80, 16, 22, ILI9341_BLACK ); setFont ( LARGE, ILI9341_YELLOW );
						printVar ( filterOffMinutes, 216, 80 ); printTxt (":", 230, 80 ); 
						tft.fillRect ( 242, 80, 34, 22, ILI9341_BLACK ); setFont ( LARGE, ILI9341_YELLOW );
						if (filterOffSeconds < 10) { setFont ( LARGE, ILI9341_YELLOW ); 
							printTxt ("0", 242, 80 ); printVar (filterOffSeconds, 260, 80 ); 
						}
						else 
						{ 
							printVar (filterOffSeconds, 242, 80 ); 
						}
					}
					if ( screenSaverRunning == true )
					{
						// tft.fillRect ( 170, 20, 142, 16, ILI9341_BLACK ); tft.fillRect ( 180, 46, 120, 16, ILI9341_BLACK );
						tft.fillRect ( 216, 80, 16, 22, ILI9341_BLACK ); tft.fillRect ( 230, 80, 58, 22, ILI9341_BLACK );
					}
				}					 
				stepDown(); 
				previousFeedingMillis = currentFeedingMillis;
			}
		}	
	}
}		
void stepDown() 
{
	if (filterOffSeconds > 0) { filterOffSeconds -= 1; }
 	else { if (filterOffMinutes > 0) { filterOffSeconds = 59; filterOffMinutes -= 1; } 
 		else {}
	}	
}

void fiveSecDelayToUpdateScreen ()
{
	unsigned long currentMillis = millis ();
	if ( currentMillis - previousMillisFive >= 5000 )   //check time, temp and LED levels every 5s
	{
		
		RTC.get (rtc, true ); //Serial.println("Get RTC Time");
		feedingTimeOutput (); //Serial.println("Check Feeding time");
		if ( screenSaverTimer < setScreenSaverTimer ) 
		{
			TimeDateBar (); //Serial.println("Print TimeDateBar");
		}
		checkTempC(); //Serial.println("Check Water Temp");
		lights();
		screenReturn ();
		screenSaver ();
		// cyclecount ();
	
		// printOutput();
		if ( ( dispScreen == 0 ) && ( screenSaverTimer < setScreenSaverTimer ) ) 
		{
			mainScreen (); //Serial.println("Run mainScreen routine");
  		}
  		previousMillisFive = currentMillis;
	}
}	
/******************************** MENU TEMPLATE SCREEN ********************************/
void menuTemplate () 
{
	if ( dispScreen == 1 ) { printHeader ( "Choose Option " ); }
	if ( dispScreen == 2 ) { printHeader ( "Time and Date Settings" ); }
	if ( dispScreen == 3 ) { printHeader ( "H2O Temperature Control Settings" ); }
	if ( dispScreen == 4 ) { printHeader ( "Light Contol Page"); }
	if ( dispScreen == 4 && lightTime == 1 ) { printHeader ( "Set Light On Time" ); }
	if ( dispScreen == 4 && lightTime == 2 ) { printHeader ( "Set Light Off Time" ); }
	if ( dispScreen == 12 ) { printHeader ( "View/Change General Settings" ); }
	if ( dispScreen == 13 ) { printHeader ( "Automatic Fish Feeder Page" ); }
	if ( dispScreen == 14 && feedTime == 1 ) { printHeader ( "Set Feeding Time 1" ); }
	if ( dispScreen == 14 && feedTime == 2 ) { printHeader ( "Set Feeding Time 2" ); }
	if ( dispScreen == 14 && feedTime == 3 ) { printHeader ( "Set Feeding Time 3" ); }
	if ( dispScreen == 14 && feedTime == 4 ) { printHeader ( "Set Feeding Time 4" ); }
	if ( dispScreen == 15 ) { printHeader ( "Maintenance Menu" ); }
	printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );
	tft.drawRect ( 0, 196, 319, 2,  ILI9341_GRAY );
	if (dispScreen != 1 ) { printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );
		if (dispScreen != 13) { printButton ( "SAVE", prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3], SMALL ); } }
}

/*********************** MENU SCREEN ********** dispScreen = 1 ************************/
void menuScreen () 
{
	// tft.fillScreen(ILI9341_BLACK);
	menuTemplate ();
	printButton ( "Time and Date", tanD [0], tanD [1], tanD [2], tanD [3] );
	printButton ( "H2O Temp Control", temC [0], temC [1], temC [2], temC [3] );
	printButton ( "General Settings", gSet [0], gSet [1], gSet [2], gSet [3] );
	printButton ( "Automatic Feeder", aFeed [0], aFeed [1], aFeed [2], aFeed [3] );
	printButton ( "Maintenace", maint [0], maint [1], maint [2], maint [3] );
	printButton ( "Light Control", liteS [0], liteS [1], liteS [2], liteS [3] );
}	

/************** TIME and DATE SCREEN ********** dispScreen = 2 ************************/
void clockScreen ( boolean refreshAll = true ) 
{
	if ( refreshAll ) 
	{
		for ( int i = 0; i < 7; i++ ) 
		{
			rtcSet [i] = rtc [i]; rtcSet2 [i] = rtc [i];
		}
		menuTemplate ();
		printButton ( "+", houU [0], houU [1], houU [2], houU [3], SMALL ); //hour up
		printButton ( "+", minU [0], minU [1], minU [2], minU [3], SMALL ); //min up
		printButton ( "-", houD [0], houD [1], houD [2], houD [3], SMALL ); //hour down
		printButton ( "-", minD [0], minD [1], minD [2], minD [3], SMALL ); //min down
		printButton ( "+", monU [0], monU [1], monU [2], monU [3], SMALL ); //month up
		printButton ( "+", dayU [0], dayU [1], dayU [2], dayU [3], SMALL ); //day up
		printButton ( "+", yeaU [0], yeaU [1], yeaU [2], yeaU [3], SMALL ); //year up
		printButton ( "-", monD [0], monD [1], monD [2], monD [3], SMALL ); //month down
		printButton ( "-", dayD [0], dayD [1], dayD [2], dayD [3], SMALL ); //day down
		printButton ( "-", yeaD [0], yeaD [1], yeaD [2], yeaD [3], SMALL ); //year down
	}
	
	ReadFromEEPROM ();
	timeDispH = rtcSet [2]; timeDispM = rtcSet [1];
	xTimeH = 107; yTime = 52;
	xColon = xTimeH + 72;
	xTimeM10 = xTimeH + 86;
	xTimeM1 = xTimeH + 102;
	xTimeAMPM = xTimeH + 155;
	timeChange ();
	setFont ( SMALL, ILI9341_WHITE );
	printTxt ( "Date", 20, 142 ); printTxt ( "/", 149, 142 ); printTxt ( "/", 219, 142 );
	if ( setCalendarFormat == 0 ) //DD/MM/YYYY Format
	{
		setFont ( SMALL, ILI9341_WHITE ); printTxt ( "(DD/MM/YYYY)", 5, 160 );
		setFont ( MEDIUM, ILI9341_WHITE );
		if ( ( rtcSet [4] >= 0 ) && ( rtcSet [4] <= 9 ) ) //Set DAY
		{
			printTxt ( "0", 107, 142 ); printVar ( rtcSet [4], 123, 142 );			
		}
		else 
		{
			printVar ( rtcSet [4], 107, 142 );			
		}
		if ( ( rtcSet [5] >= 0 ) && ( rtcSet [5] <= 9 ) ) //Set MONTH
		{
			printTxt ( "0", 177, 142 ); printVar ( rtcSet [5], 193, 142 );			
		}
		else 
		{
			printVar ( rtcSet [5], 177, 142 );			
		}
	}
	else if ( setCalendarFormat == 1 ) //MM/DD/YYYY Format
	{
		setFont ( SMALL, ILI9341_WHITE ); printTxt ( "(MM/DD/YYYY)", 5, 160 );
		setFont ( MEDIUM, ILI9341_WHITE ); 
		if ( ( rtcSet [5] >= 0 ) && ( rtcSet [5] <= 9 ) ) //Set MONTH
		{
			tft.fillRoundRect ( 107, 142 , 40, 20, 20/8 , ILI9341_BLACK);
			printTxt ( "0", 107, 142 ); printVar ( rtcSet [5], 123, 142 );
		}
		else 
		{
			tft.fillRoundRect ( 107, 142 , 40, 20, 20/8 , ILI9341_BLACK);
			printVar ( rtcSet [5], 107, 142 );
		}
		if ( ( rtcSet [4] >= 0 ) && ( rtcSet [4] <= 9 ) ) //Set DAY
		{
			tft.fillRoundRect ( 177, 142 , 40, 20, 20/8 , ILI9341_BLACK);
			printTxt ( "0", 177, 142 ); printVar ( rtcSet [4], 193, 142 );
		}
		else 
		{
			tft.fillRoundRect ( 177, 142 , 40, 20, 20/8 , ILI9341_BLACK);
			printVar ( rtcSet [4], 177, 142 );
		}
	}
	tft.fillRoundRect ( 247, 142 , 60, 20, 20/8 , ILI9341_BLACK);
	tft.setTextSize ( MEDIUM ); printVar ( rtcSet [6], 247, 142 );
}

void timeChange () 
{
	tft.setTextSize( SMALL ); printTxt ( "Time", 20, yTime );
	if ( setTimeFormat == 0 ) { tft.setTextSize ( SMALL ); printTxt ( "(24HR)", 20, ( yTime + 18 ) ); } //24HR Format
	if ( setTimeFormat == 1 ) { tft.setTextSize ( SMALL ); printTxt ( "(12HR)", 20, ( yTime + 18 ) ); } //12HR Format
	timeCorrectFormat ();
}

void buildCorrectTime () 
{
	char minute [3], hour1 [3], ampm [4];
	if ( ( timeDispM >= 0 ) && ( timeDispM <= 9 ) ) { sprintf ( minute, "%i%i", 0, timeDispM ); } //adds 0 to minutes
	else { sprintf ( minute, "%i", timeDispM ); } 
	if ( setTimeFormat == 1 ) { if ( timeDispH == 0 ) { sprintf ( hour1, "%i", 12 ); } //12 HR Format
		else 	{	if ( timeDispH > 12 ) { sprintf ( hour1, "%i", timeDispH - 12 ); } 
					else { sprintf ( hour1, "%i", timeDispH ); } } }	
	if ( timeDispH < 12 ) { sprintf ( ampm, " AM" ); AM_PM = 1; } //Adding the AM/PM sufffix
	else { sprintf ( ampm, " PM" ); AM_PM = 2; } 
	if ( setTimeFormat == 1 ) { sprintf ( time, "%s:%s%s", hour1, minute, ampm ); }
	else { sprintf ( time, "%i:%s", timeDispH, minute ); }

//  second time display for light setting screen
	if ( dispScreen == 4 )
	{
		char minute2 [3], hour11 [3], ampm1 [4];
		if ( ( timeDispM1 >= 0 ) && ( timeDispM1 <= 9 ) ) { sprintf ( minute2, "%i%i", 0, timeDispM1 ); } //adds 0 to minutes 
		else { sprintf ( minute2, "%i", timeDispM1 ); }
		if ( setTimeFormat == 1 ) 
		{
			if ( timeDispH1 == 0 ) { sprintf ( hour11, "%i", 12 ); } //12 HR Format
			else 
			{
				if ( timeDispH1 > 12 ) { sprintf ( hour11, "%i", timeDispH1 - 12 ); }
				else { sprintf ( hour11, "%i", timeDispH1 ); }
			}
			if ( timeDispH1 < 12 )  //Adding the AM/PM sufffix
			{
				sprintf ( ampm, " AM" ); AM_PM = 1;
			}              
			else { sprintf ( ampm, " PM" ); AM_PM = 2; }
			if ( setTimeFormat == 1 ) { sprintf ( time1, "%s:%s%s", hour11, minute2, ampm ); }
			else { sprintf ( time1, "%i:%s", timeDispH1, minute2 ); }
		}
	}	
}

void timeCorrectFormat () 
{
	setFont ( MEDIUM, ILI9341_WHITE ); buildCorrectTime ();
	tft.fillRoundRect ( xTimeH, yTime , (xTimeH + 40), 20, 20/8 , ILI9341_BLACK);
	tft.setTextColor ( ILI9341_WHITE ); printTxt ( time, xTimeH, yTime ); //Display time
	if ( dispScreen == 4 )
	{
		tft.fillRoundRect ( xTimeH, yTime1 , (xTimeH + 40), 20, 20/8 , ILI9341_BLACK);
		tft.setTextColor ( ILI9341_WHITE ); printTxt ( time1, xTimeH, yTime1 ); // 
	}	
}

/*********** H2O TEMP CONTROL SCREEN ********** dispScreen = 3 ************************/
void tempScreen ( boolean refreshAll = false ) 
{		
	menuTemplate ();
	if ( refreshAll ) 
	{
		if ( ( setTempC == 0 ) && ( setTempScale == 0 ) ) 
		{
			setTempC = 26.1; //change to 26.1 deg C
		}                         
		if ( ( ( setTempF == 0 ) || ( setTempF == setTempC ) ) && ( setTempScale == 1 ) ) 
		{
			setTempF = 79.0;  //change to 79.0 deg F
		}                        

		if ( setTempScale == 1 ) 
		{
			temp2beS = setTempF; temp2beO = offTempF; temp2beA = alarmTempF;
		}
		else 
		{
			temp2beS = setTempC; temp2beO = offTempC; temp2beA = alarmTempC;
		}

		if ( setTempScale == 1 )   //Print deg C or deg F, degC_F=deg;
		{
			sprintf ( degC_F, "F" ); //deg ="F";
		}               
		else 
		{
			sprintf ( degC_F, "C" ); //deg = "C";
		}
		setFont ( SMALL, ILI9341_WHITE );
		printTxt ( "Desired Temperature in", 94, 34 );
		tft.drawCircle ( 245, 36, 1, ILI9341_WHITE );
		printTxt ( degC_F, 250, 34 ); printTxt ( ":", 258, 34 );
		printTxt ( "Temperature Offset:", 103, 84 ); printTxt ( "Alarm Offset", 121, 134 );

		printButton ( "-", temM [0], temM [1], temM [2], temM [3], true );  //temp minus
		printButton ( "+", temP [0], temP [1], temP [2], temP [3], true );  //temp plus
		printButton ( "-", offM [0], offM [1], offM [2], offM [3], true );  //offset minus
		printButton ( "+", offP [0], offP [1], offP [2], offP [3], true );  //offset plus
		printButton ( "-", almM [0], almM [1], almM [2], almM [3], true );  //alarm minus
		printButton ( "+", almP [0], almP [1], almP [2], almP [3], true );  //alarm plus
	}

		setFont ( MEDIUM, ILI9341_WHITE );
		tft.fillRoundRect (148, 54, 47, 25, 20/8, ILI9341_BLACK );
		tft.setCursor ( 148, 54 ); tft.print ( temp2beS, 1 );
		tft.fillRoundRect (148, 104, 47, 25, 20/8, ILI9341_BLACK );
		tft.setCursor ( 148, 104 ); tft.print ( temp2beO, 1 );
		tft.fillRoundRect (148, 154, 47, 25, 20/8, ILI9341_BLACK );
		tft.setCursor ( 148, 154 ); tft.print ( temp2beA, 1 );
}

/********** LIGHT CONTROL SCREEN ******** dispScreen = 4 ************************/
void LightControlScreen ( boolean refreshAll = true ) 
{

	if ( refreshAll ) 
	{
		ReadFromEEPROM ();
		rtcSet [2] = lightTime1H; rtcSet [1] = lightTime1M;
		rtcSet2 [2] = lightTime2H; rtcSet2 [1] = lightTime2M;
	}	
	menuTemplate ();
	tft.setTextSize( SMALL );
	printTxt ( "Lights On", 25, 46 ); printTxt ( "Lights Off", 25, 136 );

	printButton ( "+", hou1U [0], hou1U [1], hou1U [2], hou1U [3], false );     //hour up
	printButton ( "+", min1U [0], min1U [1], min1U [2], min1U [3], false );     //min up
	printButton ( "-", hou1D [0], hou1D [1], hou1D [2], hou1D [3], false );     //hour down
	printButton ( "-", min1D [0], min1D [1], min1D [2], min1D [3], false );     //min down
	printButton ( "+", hou2U [0], hou2U [1], hou2U [2], hou2U [3], false );     //hour up
	printButton ( "+", min2U [0], min2U [1], min2U [2], min2U [3], false );     //min up
	printButton ( "-", hou2D [0], hou2D [1], hou2D [2], hou2D [3], false );     //hour down
	printButton ( "-", min2D [0], min2D [1], min2D [2], min2D [3], false );     //min down

	timeDispH = rtcSet [2]; timeDispM = rtcSet [1];
	timeDispH1 = rtcSet2 [2]; timeDispM1 = rtcSet2 [1];
	xTimeH = 118; yTime = 55; yTime1 = 142; xColon = xTimeH + 82;
	timeChange ();
}

void lights()
{
	if ( ( lightTime1H > rtc [2] ) || ( lightTime1H == rtc [2] ) && ( lightTime1M > rtc [1] )
	|| ( lightTime2H == rtc [2] ) && ( lightTime2M <= rtc [1] ) || ( lightTime2H < rtc [2] ) ) 
	{
 		digitalWrite(LIGHTS_PWR,HIGH); Lights_On_Flag = false; } // Turn Off Lights
  	else 
  	{ 
	   digitalWrite(LIGHTS_PWR,LOW); Lights_On_Flag = true; } //Turn On Lights
} 

/******** GENERAL SETTINGS SCREEN ************* dispScreen = 12 ***********************/
void generalSettingsScreen () 
{

	menuTemplate ();
	tft.setTextSize( SMALL );
	printTxt ( "Calendar Format", 25, 36 ); printTxt ( "Time Format", 25, 80 );
	printTxt ( "Temperature Scale", 25, 111 ); printTxt ( "Screensaver", 25, 142 );
	printTxt ( "Auto-Stop on feed", 25, 173 );
	genSetSelect ();
}

/******** AUTOMATIC FEEDER SCREEN ************* dispScreen = 13 ***********************/
void autoFeederScreen () 
{
	menuTemplate ();
	tft.drawRect ( 159, 120, 3, 76, 	ILI9341_GRAY );
	tft.drawRoundRect ( 78, 87, 164, 34, 20/8,	ILI9341_GRAY );
	tft.drawRoundRect ( 80, 89, 160, 30, 20/8,	ILI9341_GRAY );
	tft.drawRect ( 0, 103, 78, 2, 	ILI9341_GRAY );
	tft.drawRect ( 242, 103, 319, 2, 	ILI9341_GRAY );
	tft.drawLine ( 159, 87, 159, 14,	ILI9341_GRAY );
 	tft.drawLine ( 161, 87, 161, 14,	ILI9341_GRAY );
	tft.drawLine ( 160, 195, 160, 193, ILI9341_BLACK );
	tft.drawLine ( 160, 122, 160, 120, ILI9341_BLACK );
	tft.drawLine ( 77, 104, 79, 104, ILI9341_BLACK );
	tft.drawLine ( 241, 104, 243, 104, ILI9341_BLACK );
	tft.drawLine ( 160, 88, 160, 86, ILI9341_BLACK );
	tft.fillRoundRect ( 85, 94, 150, 20, 20/8, 0x980C ); //Feed Fish Now Button
	if ( FEEDTimeNow == 1 )
	{ 
		tft.fillRoundRect ( 85, 94, 150, 20, 20/8, ILI9341_GREEN );
		tft.drawRoundRect ( 85, 94, 150, 20, 20/8, ILI9341_WHITE );
		tft.setTextColor ( ILI9341_BLACK ); printTxt ( "Now Feeding", 120, 100 );
	}
	else
	{
		tft.fillRoundRect ( 85, 94, 150, 20, 20/8, 	0x980C );
		tft.drawRoundRect ( 85, 94, 150, 20, 20/8, 	0x980C );
		tft.setTextColor ( ILI9341_WHITE ); printTxt ( "Feed Fish Now!", 120, 100 );

	}
	// tft.setCursor ( 120, 100 ); tft.print (F( "Feed Fish Now!"));
	if ( FEEDTime1 == 0 ) //Feeding Time 1 Button
	{
		tft.fillRoundRect ( 5, 20, 150, 20, 20/8, ILI9341_RED );
		tft.setCursor ( 34, 24);
		setFont ( SMALL, ILI9341_WHITE ); tft.print (F( "Feeding Time 1" ));
		tft.setTextColor (ILI9341_RED); 
		tft.setCursor ( 22, 52 ); tft.print (F( "This time has not" ));
		tft.setCursor ( 34, 65 ); tft.print (F( "been scheduled" ));
	}
	else 
	{	
		tft.fillRoundRect ( 5, 20, 150, 20, 20/8, ILI9341_GREEN );
		setFont ( SMALL, ILI9341_BLACK );
		tft.setCursor ( 34, 24 ); tft.print (F( "Feeding Time 1" ));
		timeDispH = feedFish1H; timeDispM = feedFish1M;
		if ( setTimeFormat == 0 ) { xTimeH = 45; } 
		if ( setTimeFormat == 1 ) { xTimeH = 16; }
		ReadFromEEPROM ();
		if ( ( timeDispH >= 0 ) && ( timeDispH <= 11 ) ) { AM_PM = 1; } 
		else { AM_PM = 2; }
		yTime = 58; xColon = xTimeH + 42; xTimeM10 = xTimeH + 48; xTimeM1 = xTimeH + 64; xTimeAMPM = xTimeH + 96;
		timeCorrectFormat ();
	}	
	if ( FEEDTime2 == 0 ) //Feeding Time 2 Button
	{
		tft.fillRoundRect ( 165, 20, 150, 20, 20/8, ILI9341_RED );
		tft.setCursor ( 194, 24);
		setFont ( SMALL, ILI9341_WHITE ); tft.print (F( "Feeding Time 2" ));
		tft.setTextColor (ILI9341_RED);
		tft.setCursor ( 182, 52 ); tft.print (F( "This time has not" ));
		tft.setCursor ( 194, 65 ); tft.print (F( "been scheduled" ));
	}
	else 
	{	
		tft.fillRoundRect ( 165, 20, 150, 20, 20/8, ILI9341_GREEN );
		setFont ( SMALL, ILI9341_BLACK );
		tft.setCursor ( 194, 24 ); tft.print (F( "Feeding Time 2" ));
		timeDispH = feedFish2H;
		timeDispM = feedFish2M;

		if ( setTimeFormat == 0 ) { xTimeH = 200; }
		if ( setTimeFormat == 1 ) { xTimeH = 176; }
		ReadFromEEPROM ();
		if ( ( timeDispH >= 0 ) && ( timeDispH <= 11 ) ) { AM_PM = 1; } 
		else { AM_PM = 2; }	
		yTime = 56; xColon = xTimeH + 32; xTimeM10 = xTimeH + 48; xTimeM1 = xTimeH + 64; xTimeAMPM = xTimeH + 96;
		timeCorrectFormat ();
	}	
	if ( FEEDTime3 == 0 ) //Feeding Time 3 Button
	{
		tft.fillRoundRect ( 5, 168, 150, 20, 20/8,ILI9341_RED );
		tft.setCursor ( 34, 172 );
		setFont ( SMALL, ILI9341_WHITE ); tft.print (F( "Feeding Time 3" ));
		tft.setTextColor ( ILI9341_RED ); 
		tft.setCursor ( 22, 133 ); tft.print (F( "This time has not" ));
		tft.setCursor ( 34, 146 ); tft.print (F( "been scheduled" ));
	}
	else 
	{	
		tft.fillRoundRect ( 5, 168, 150, 20, 20/8, ILI9341_GREEN );
		setFont ( SMALL, ILI9341_BLACK );
		tft.setCursor ( 24, 172 ); tft.print (F( "Feeding Time 3" ));
		timeDispH = feedFish3H; timeDispM = feedFish3M;
		if ( setTimeFormat == 0 ) { xTimeH = 45; }
		if ( setTimeFormat == 1 ) { xTimeH = 16; }
		ReadFromEEPROM ();
		if ( ( timeDispH >= 0 ) && ( timeDispH <= 11 ) ) { AM_PM = 1; }
		else { AM_PM = 2; }	 
		yTime = 136; xColon = xTimeH + 42; xTimeM10 = xTimeH + 63; xTimeM1 = xTimeH + 64; xTimeAMPM = xTimeH + 96;
		timeCorrectFormat ();
	}	
	if ( FEEDTime4 == 0 ) //Feeding Time 4 Button
	{
		tft.fillRoundRect ( 165, 168, 150, 20, 20/8, ILI9341_RED );
		tft.setCursor ( 194, 172 );
		setFont ( SMALL, ILI9341_WHITE ); tft.print (F( "Feeding Time 4" ));
		tft.setTextColor ( ILI9341_RED );
		tft.setCursor ( 182, 133 ); tft.print (F( "This time has not" ));
		tft.setCursor ( 194, 146 ); tft.print (F( "been scheduled" ));
	}
	else 
	{	
		tft.fillRoundRect ( 165, 168, 150, 20, 20/8, ILI9341_GREEN );
		setFont ( SMALL, ILI9341_BLACK );
		tft.setCursor ( 194, 172 ); tft.print (F( "Feeding Time 4" ));
		timeDispH = feedFish4H; timeDispM = feedFish4M;
		if ( setTimeFormat == 0 ) { xTimeH = 200; }
		if ( setTimeFormat == 1 ) { xTimeH = 176; }
		ReadFromEEPROM ();
		if ( ( timeDispH >= 0 ) && ( timeDispH <= 11 ) ) { AM_PM = 1; }
		else { AM_PM = 2; }	
		yTime = 137; xColon = xTimeH + 32; xTimeM10 = xTimeH + 48; xTimeM1 = xTimeH + 64; xTimeAMPM = xTimeH + 96;
		timeCorrectFormat ();
	}	
	for ( int x = 0; x < 2; x++ ) 
	{
		for ( int y = 0; y < 1; y++ ) 
		{
			tft.drawRoundRect ( ( x * 160 ) + 5, ( y * 148 ) + 20, ( x * 1 ) + 150, ( y * 1 ) + 20, 20/8, ILI9341_WHITE );
		}
	}
}

void feedingTimeOutput () 
{
	if (feederMotorRunning == true ) 
	{
		tenSecTimer++;
		if ( tenSecTimer >= 3 ) 
		{
			feederMotorRunning = false;
			RTC.get (rtc, true );
			digitalWrite( FEEDER_PWR, HIGH ); //Turn off feeder
		}
	}
	if ( ( FEEDTimeNow == 1 )  && ( waterfilterStopped == false ) )
	{	
		Serial.println ( " Now Feeding ");
		if ( setAutoStop == 1 )
		{  
			filterOffMinutes = 5; filterOffSeconds = 0;
			WaterFilerCtrl_Now = true; waterfilterStopped = true;
			// screenSaverTimer = 0; mainScreen ( true ); dispScreen = 0;
			digitalWrite(WATER_FILTER_PWR,LOW); // Turn off water filter
		}
		tenSecTimer = 0; 	feederMotorRunning = true;	
		digitalWrite ( FEEDER_PWR, LOW ); // Turn on feeder
	}
	if ( WaterFilerCtrl_Now == true )
	{
		if ( ( filterOffMinutes == 0 ) && ( filterOffSeconds == 0 ) ) 
		{	
			FEEDTimeNow = 0; waterfilterStopped = false; 
			digitalWrite(WATER_FILTER_PWR,HIGH); // Turn oN water filter
		}
	}
	if ( ( FEEDTime1 == 1 ) && ( feedFish1H == rtc [2] ) && ( feedFish1M == rtc [1] ) && ( rtc [0] <= 5 ) ) 
	{	
		if ( setAutoStop == 1 )
		{  
			filterOffMinutes = 5; filterOffSeconds = 0;
			WaterFilerCtrl_1 = true; waterfilterStopped = true;
			// screenSaverTimer = 0; mainScreen ( true ); dispScreen = 0;
			digitalWrite(WATER_FILTER_PWR,LOW); // Turn off water filter
		}
		tenSecTimer = 0; 	feederMotorRunning = true;	
		digitalWrite ( FEEDER_PWR, LOW ); // Turn on feeder
	}
	if ( WaterFilerCtrl_1 == true )
	{
		if ( ( filterOffMinutes == 0 ) && ( filterOffSeconds == 0 ) ) 
		{	
			waterfilterStopped = false; 
			digitalWrite(WATER_FILTER_PWR,HIGH); // Turn oN water filter
		}
	}
	if ( ( FEEDTime2 == 1 ) && ( feedFish2H == rtc [2] ) && ( feedFish2M == rtc [1] ) && ( rtc [0] <= 4 ) ) 
	{
		if ( setAutoStop == 1 )
		{  
			filterOffMinutes = 5; filterOffSeconds = 0;
			WaterFilerCtrl_2 = true; waterfilterStopped = true; 
			digitalWrite(WATER_FILTER_PWR,LOW); // Turn off water filter
		}
		tenSecTimer = 0; 	feederMotorRunning = true;	
		digitalWrite ( FEEDER_PWR, LOW ); // Turn on feeder
	}
	if ( WaterFilerCtrl_2 == true )
	{
		if ( ( filterOffMinutes == 0 ) && ( filterOffSeconds == 0 ) )
		{	
			waterfilterStopped = false; 
			digitalWrite(WATER_FILTER_PWR,HIGH); // Turn oN water filter
		}
	}
	if ( ( FEEDTime3 == 1 ) && ( feedFish3H == rtc [2] ) && ( feedFish3M == rtc [1] ) && ( rtc [0] <= 4 ) ) 
	{
		if ( setAutoStop == 1 )
		{  
			filterOffMinutes = 5; filterOffSeconds = 0;
			WaterFilerCtrl_3 = true; waterfilterStopped = true; 
			digitalWrite(WATER_FILTER_PWR,LOW); // Turn off water filter
		}
		tenSecTimer = 0; 	feederMotorRunning = true;	
		digitalWrite ( FEEDER_PWR, LOW ); // Turn on feeder
	}
	if ( WaterFilerCtrl_3 == true )
	{
		if ( ( filterOffMinutes == 0 ) && ( filterOffSeconds == 0 ) ) 
		{	
			waterfilterStopped = false; 
			digitalWrite(WATER_FILTER_PWR,HIGH); // Turn oN water filter
		}
	}
	if ( ( FEEDTime4 == 1 ) && ( feedFish4H == rtc [2] ) && ( feedFish4M == rtc [1] ) && ( rtc [0] <= 4 ) ) 
	{
		if ( setAutoStop == 1 )
		{  
			filterOffMinutes = 5; filterOffSeconds = 0;
			WaterFilerCtrl_4 = true; waterfilterStopped = true; 
			digitalWrite(WATER_FILTER_PWR,LOW); // Turn off water filter
		}
		tenSecTimer = 0; 	feederMotorRunning = true;	
		digitalWrite ( FEEDER_PWR, LOW ); // Turn on feeder
	}
	if ( WaterFilerCtrl_4 == true )
	{
		if ( ( filterOffMinutes == 0 ) && ( filterOffSeconds == 0 ) )
		{	
			waterfilterStopped = false; 
			digitalWrite(WATER_FILTER_PWR,HIGH); // Turn oN water filter
		}
	}
}

/***** SET AUTOMATIC FEEDER TIMES SCREEN ********** dispScreen = 14 *******************/
void setFeederTimesScreen ( boolean refreshAll = true ) 
{

	menuTemplate();	
	if ( refreshAll ) 
	{
		for ( int i = 0; i < 7; i++ ) { rtcSet [i] = rtc [i]; rtcSet2 [i] = rtc [i]; }
		feedingTimeOnOff ();

		printButton ( "+", houP [0], houP [1], houP [2], houP [3], SMALL );     //hour up
		printButton ( "+", minP [0], minP [1], minP [2], minP [3], SMALL );     //min up
		printButton ( "-", houM [0], houM [1], houM [2], houM [3], SMALL );     //hour down
		printButton ( "-", minM [0], minM [1], minM [2], minM [3], SMALL );     //min down
	}
	timeDispH = rtcSet [2]; timeDispM = rtcSet [1]; 
	xTimeH = 118; yTime = 68; xColon = xTimeH + 82; xTimeM10 = xTimeH + 80; xTimeM1 = xTimeH + 96; xTimeAMPM = xTimeH + 175;
	timeChange ();
}

/************** MAINTENANCE SCREEN ****************** dispScreen = 15 ***********************/

//TODO: Add in routine to display "Days"	& to process Alarm reset.

void maintSettingScreen () 
{
	menuTemplate ();
	setFont ( SMALL, ILI9341_WHITE );
	tft.setCursor ( 25, 25 ); tft.print (F( " Item                  Days           Alarm" ) );
	tft.setCursor ( 20, ( day1D [1] + 8 ) ); tft.print ( maintName [0] );
	tft.setCursor ( 20, ( day2D [1] + 8 ) ); tft.print ( maintName [1] );
	tft.setCursor ( 20, ( day3D [1] + 8 ) ); tft.print ( maintName [2] );
	tft.setCursor ( 20, ( day4D [1] + 8 ) ); tft.print ( maintName [3] );;
	printButton ( "-", day1D [0], day1D [1], day1D [2], day1D [3], true );     
	printButton ( "+", day1U [0], day1U [1], day1U [2], day1U [3], true );     
	printButton ( "-", day2D [0], day2D [1], day2D [2], day2D [3], true );     
	printButton ( "+", day2U [0], day2U [1], day2U [2], day2U [3], true );      
	printButton ( "-", day3D [0], day3D [1], day3D [2], day3D [3], true );     
	printButton ( "+", day3U [0], day3U [1], day3U [2], day3U [3], true );     
	printButton ( "-", day4D [0], day4D [1], day4D [2], day4D [3], true );     
	printButton ( "+", day4U [0], day4U [1], day4U [2], day4U [3], true );   
	printButton ( "Reset", day1R [0], day1R [1], day1R [2], day1R [3], true);
	printButton ( "Reset", day2R [0], day2R [1], day2R [2], day2R [3], true);
	printButton ( "Reset", day3R [0], day3R [1], day3R [2], day3R [3], true);
	printButton ( "Reset", day4R [0], day4R [1], day4R [2], day4R [3], true);
	setFont ( MEDIUM, ILI9341_WHITE );
	tft.fillRoundRect ( 150, 45, 47, 25, 20/8, ILI9341_BLACK ); tft.setCursor ( 160, 45 ); tft.print ( maint1D, 1 ); 
	tft.fillRoundRect ( 150, 86, 47, 25, 20/8, ILI9341_BLACK ); tft.setCursor ( 160, 86 ); tft.print ( maint2D, 1 );
	tft.fillRoundRect ( 148, 125, 47, 25, 20/8, ILI9341_BLACK ); tft.setCursor ( 160, 125 ); tft.print ( maint3D, 1 );
	tft.fillRoundRect ( 148, 165, 47, 25, 20/8, ILI9341_BLACK ); tft.setCursor ( 160, 165 ); tft.print ( maint4D, 1 );
}

/************************************ TOUCH SCREEN ************************************/
void processMyTouch () 
{
	TS_Point p = ctp.getPoint();

	x1 = p.x; y1 = p.y; // flip it around to match the screen.
	p.x = map(y1, 0, 320, 0, 320); p.y = map(x1, 0, 240, 240, 0);
	x = p.x; y = p.y;

	
	if ( ( x >= canC [0] ) && ( x <= ( canC [0] + canC [2] ) ) && ( y >= canC [1] ) && ( y <= (canC [1] + canC [3] ) )   //press cancel
			&& ( dispScreen != 0 ) && ( dispScreen != 5 ) && ( dispScreen != 6 ) && ( dispScreen != 8 ) && ( dispScreen != 11 ) ) 
	{
		waitForIt ( canC [0], canC [1], canC [2], canC [3] );
		ReadFromEEPROM (); dispScreen = 0; clearScreen (); mainScreen ( true );
	}
	else if ( ( x >= back [0] ) && ( x <= ( back [0] + back [2] ) ) && ( y >= back [1] ) && ( y <= ( back [1] + back [3] ) )  //press back
			&& ( dispScreen != 0 ) && ( dispScreen != 1 ) && ( dispScreen != 5 ) && ( dispScreen != 6 ) && ( dispScreen != 8 ) && ( dispScreen != 11 )
			&& ( dispScreen != 14 ) && ( dispScreen != 18 ) && ( dispScreen != 19 ) ) 
	{
		waitForIt ( back [0], back [1], back [2], back [3] );
		ReadFromEEPROM (); dispScreen = MENUSCREEN_ONE; clearScreen (); menuScreen (); 		
	}
	else 
	{
		switch ( dispScreen ) 
		{
			case 0 :		 //--------------- MAIN SCREEN (Press Any Key) ---------------
				if ( screenSaverRunning == true )
				{
					screenSaverRunning = false; screenSaverTimer = 0;
					mainScreen ( true );
				}
				else 
				{
					dispScreen = MENUSCREEN_ONE;
					clearScreen (); 
					menuScreen ();  
				}
				break;
			case 1 :     //--------------------- MENU SCREEN -------------------------
				if ( ( x >= tanD [0] ) && ( x <= ( tanD [0] + tanD [2] ) ) ) //first column
				{
					if ( ( y >= tanD [1] ) && ( y <= ( tanD [1] + tanD [3] ) ) ) //press Date & Clock Screen
					{
						waitForIt ( tanD [0], tanD [1], tanD [2], tanD [3] );
						dispScreen = 2;
						clearScreen ();
						clockScreen ();
					}
					if ( ( y >= temC [1] ) && ( y <= ( temC [1] + temC [3] ) ) ) //press H2O Temp Control
					{
						waitForIt ( temC [0], temC [1], temC [2], temC [3] );
						ReadFromEEPROM ();
						dispScreen = 3;
						clearScreen ();
						tempScreen ( true );
					}
					if ( ( y >= gSet [1] ) && ( y <= (gSet [1] + gSet [3] ) ) )  //press General Settings
					{
						waitForIt ( gSet [0], gSet [1], gSet [2], gSet [3] );
						dispScreen = 12;
						clearScreen ();
						generalSettingsScreen ();
					}
				}

				if ( ( x >= aFeed [0] ) && ( x <= ( aFeed [0] + aFeed [2] ) ) ) //first column
				{
					if ( ( y >= aFeed [1] ) && ( y <= ( aFeed [1] + aFeed [3] ) ) ) //press Automatic Feeder screen
					{
						waitForIt ( aFeed [0], aFeed [1], aFeed [2], aFeed [3] );
						dispScreen = 13;
						clearScreen ();
						autoFeederScreen ();
					}
					if ( ( y >= maint [1] ) && ( y <= ( maint [1] + maint [3] ) ) ) //press Maintenance Settings
					{
						waitForIt ( maint [0], maint [1], maint [2], maint [3] );
						dispScreen = 15;
						clearScreen ();
						maintSettingScreen ();
					}
					if ( ( y >= liteS [1] ) && ( y <= ( liteS [1] + liteS [3] ) ) ) //press Lights Settings
					{
						waitForIt ( liteS [0], liteS [1], liteS [2], liteS [3] );
						dispScreen = 4;
						clearScreen ();
						LightControlScreen ();
					}
				}		
				break;
			case 2  :     //--------------- CLOCK & DATE SETUP SCREEN -----------------
				if ( ( x >= prSAVE [0] ) && ( x <= ( prSAVE [0] + prSAVE [2] ) ) && ( y >= prSAVE [1] ) && ( y <= (prSAVE [1] + prSAVE [3] ) ) )  //press SAVE
				{
					waitForIt ( prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3] );
					if ( setTimeFormat == 1 ) 
					{
						if ( ( rtcSet [2] == 0 ) && ( AM_PM == 2 ) ) 
						{
							rtcSet [2] += 12;
						}
						if ( ( ( rtcSet [2] >= 1 ) && ( rtcSet [2] <= 11 ) ) && ( AM_PM == 2 ) ) 
						{
							rtcSet [2] += 12;
						}
						if ( ( ( rtcSet [2] >= 12 ) && ( rtcSet [2] <= 23 ) ) && ( AM_PM == 1 ) ) 
						{
							rtcSet [2] -= 12;
						}
					}
					SaveRTC ();
					dispScreen = 0;
					clearScreen ();
					mainScreen ( true );
				}
				else 
				{
					if ( ( y >= houU [1] ) && ( y <= ( houU [1] + houU [3] ) ) ) //FIRST ROW
					{
						if ( ( x >= houU [0] ) && ( x <= ( houU [0] + houU [2] ) ) ) //press hour up
						{
							waitForIt ( houU [0], houU [1], houU [2], houU [3] );
							rtcSet [2]++;
							if ( rtcSet [2] >= 24 ) 
							{
								rtcSet [2] = 0;
							}
						}
						if ( ( x >= minU [0] ) && ( x <= (minU [0] + minU [2] ) ) ) //press min up
						{
							waitForIt ( minU [0], minU [1], minU [2], minU [3] );
							rtcSet [1]++;
							if ( rtcSet [1] >= 60 ) 
							{
								rtcSet [1] = 0;
							}
						}
					}
					if ( ( y >= houD [1] ) && ( y <= (houD [1] + houD [3] ) ) ) //SECOND ROW
					{
						if ( ( x >= houD [0] ) && ( x <= (houD [0] + houD [2] ) ) ) //press hour down
						{
							waitForIt ( houD [0], houD [1], houD [2], houD [3] );
							rtcSet [2]--;
							if ( rtcSet [2] < 0 ) 
							{
								rtcSet [2] = 23;
							}
						}
						if ( ( x >= minD [0] ) && ( x <= (minD [0] + minD [2] ) ) ) //press min down
						{
							waitForIt ( minD [0], minD [1], minD [2], minD [3] );
							rtcSet [1]--;
							if ( rtcSet [1] < 0 ) 
							{
								rtcSet [1] = 59;
							}
						}
					}
					if ( ( y >= dayU [1] ) && ( y <= ( dayU [1] + dayU [3] ) ) ) //THIRD ROW
					{
						if ( setCalendarFormat == 0 ) //DD/MM/YYYY Format
						{
							if ( ( x >= dayU [0] ) && ( x <= ( dayU [0] + dayU [2] ) ) ) //press day up
							{
								waitForIt ( dayU [0], dayU [1], dayU [2], dayU [3] );
								rtcSet [4]++;
								if ( rtcSet [4] > 31 ) 
								{
									rtcSet [4] = 1;
								}
							}
							if ( ( x >= monU [0] ) && ( x <= ( monU [0] + monU [2] ) ) ) //press month up
							{
								waitForIt ( monU [0], monU [1], monU [2], monU [3] );
								rtcSet [5]++;
								if ( rtcSet [5] > 12 ) 
								{
									rtcSet [5] = 1;
								}
							}
						}
						else 
						{
							if ( setCalendarFormat == 1 ) //MM/DD/YYYY Format
							{
								if ( ( x >= dayU [0] ) && ( x <= ( dayU [0] + dayU [2] ) ) ) //press month up
								{
									waitForIt ( dayU [0], dayU [1], dayU [2], dayU [3] );
									rtcSet [5]++;
									if ( rtcSet [5] > 12 ) 
									{
										rtcSet [5] = 1;
									}
								}
								if ( ( x >= monU [0] ) && ( x <= ( monU [0] + monU [2] ) ) ) //press day up
								{
									waitForIt ( monU [0], monU [1], monU [2], monU [3] );
									rtcSet [4]++;
									if ( rtcSet [4] > 31 ) 
									{
										rtcSet [4] = 1;
									}
								}
							}
						}
						if ( ( x >= yeaU [0] ) && ( x <= ( yeaU [0] + yeaU [2] ) ) ) //press year up
						{
							waitForIt ( yeaU [0], yeaU [1], yeaU [2], yeaU [3] );
							rtcSet [6]++;
							if ( rtcSet [6] > 2100 ) 
							{
								rtcSet [6] = 2000;
							}
						}
					}
					if ( ( y >= dayD [1] ) && ( y <= ( dayD [1] + dayD [3] ) ) ) //FOURTH ROW
					{
						if ( setCalendarFormat == 0 ) //DD/MM/YYYY Format
						{
							if ( ( x >= dayD [0] ) && ( x <= ( dayD [0] + dayD [2] ) ) ) //press day down
							{
								waitForIt ( dayD [0], dayD [1], dayD [2], dayD [3] );
								rtcSet [4]--;
								if ( rtcSet [4] < 1 ) 
								{
									rtcSet [4] = 31;
								}
							}
							if ( ( x >= monD [0] ) && ( x <= ( monD [0] + monD [2] ) ) ) //press month down
							{
								waitForIt ( monD [0], monD [1], monD [2], monD [3] );
								rtcSet [5]--;
								if ( rtcSet [5] < 1 ) 
								{
									rtcSet [5] = 12;
								}
							}
						}
						// else {
							if ( setCalendarFormat == 1 ) //MM/DD/YYYY Format
							{
								if ( ( x >= dayD [0] ) && ( x <= ( dayD [0] + dayD [2] ) ) ) //press month down
								{
									waitForIt ( dayD [0], dayD [1], dayD [2], dayD [3] );
									rtcSet [5]--;
									if ( rtcSet [5] < 1 ) 
									{
										rtcSet [5] = 12;
									}
								}
								if ( ( x >= monD [0] ) && ( x <= ( monD [0] + monD [2] ) ) ) //press day down
								{
									waitForIt ( monD [0], monD [1], monD [2], monD [3] );
									rtcSet [4]--;
									if ( rtcSet [4] < 1 ) 
									{
										rtcSet [4] = 31;
									}
								}
							}
						// }
						if ( ( x >= yeaD [0] ) && ( x <= (yeaD [0] + yeaD [2] ) ) ) //press year down
						{
							waitForIt ( yeaD [0], yeaD [1], yeaD [2], yeaD [3] );
							rtcSet [6]--;
							if ( rtcSet [6] < 2000 ) 
							{
								rtcSet [6] = 2100;
							}
						}
					}
					clockScreen ( false );
				}
				// break;

				break;
			case 3  :   	//------------------ H20 TEMPERATURE CONTROL ---------------
				if ( ( x >= prSAVE [0] ) && ( x <= ( prSAVE [0] + prSAVE [2] ) ) && ( y >= prSAVE [1] ) && ( y <= ( prSAVE [1] + prSAVE [3] ) ) )  //press SAVE
				{
					waitForIt ( prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3] );
					setTempC = temp2beS; setTempF = temp2beS;
					offTempC = temp2beO; offTempF = temp2beO;
					alarmTempC = temp2beA; alarmTempF = temp2beA;
					if ( setTempScale == 0 ) //Celsius to Farenheit (Consistency Conversion)
					{
						setTempF = ( ( 1.8 * setTempC ) + 32.05 );
						offTempF = ( ( 1.8 * offTempC ) + 0.05 );
						alarmTempF = ( ( 1.8 * alarmTempC ) + 0.05 );
					}
					if ( setTempScale == 1 ) //Farenheit to Celsius (Consistency Conversion)
					{
						setTempC = ( ( .55556 * ( setTempF - 32 ) ) + .05 );
						offTempC = ( .55556 ) * offTempF + .05;
						alarmTempC = ( .55556 ) * alarmTempF + .05;
					}
					dispScreen = 0;
					SaveTempToEEPROM ();
					clearScreen ();
					mainScreen ( true );
				}
				else
					tft.setTextColor ( ILI9341_WHITE );
				{
					if ( ( x >= temM [0] ) && ( x <= ( temM [0] + temM [2] ) ) ) //first column
					{
						if ( ( y >= temM [1] ) && ( y <= ( temM [1] + temM [3] ) ) ) //press temp minus
						{
							waitForIt ( temM [0], temM [1], temM [2], temM [3] );
							temp2beS -= 0.1;
							if ( ( setTempScale == 1 ) && ( temp2beS <= 50 ) ) 
							{
								temp2beS = 50;
							}
							if ( ( setTempScale == 0 ) && ( temp2beS <= 10 ) ) 
							{
								temp2beS = 10;
							}
							tempScreen ();
						}
						if ( ( y >= offM [1] ) && ( y <= ( offM [1] + offM [3] ) ) ) //press offset minus
						{
							waitForIt ( offM [0], offM [1], offM [2], offM [3] );
							temp2beO -= 0.1;
							if ( temp2beO < 0.1 ) 
							{
								temp2beO = 0.0;
							}
							tempScreen ();
						}
						if ( ( y >= almM [1] ) && ( y <= ( almM [1] + almM [3] ) ) ) //press alarm minus
						{
							waitForIt ( almM [0], almM [1], almM [2], almM [3] );
							temp2beA -= 0.1;
							if ( temp2beA < 0.1 ) 
							{
								temp2beA = 0.0;
							}
							tempScreen ();
						}
					}
					if ( ( x >= temP [0] ) && ( x <= ( temP [0] + temP [2] ) ) ) //second column
					{
						if ( ( y >= temP [1] ) && ( y <= ( temP [1] + temP [3] ) ) ) //press temp plus
						{
							waitForIt ( temP [0], temP [1], temP [2], temP [3] );
							temp2beS += 0.1;
							if ( ( setTempScale == 1 ) && ( temp2beS >= 104 ) ) 
							{
								temp2beS = 104;
							}
							if ( ( setTempScale == 0 ) && ( temp2beS >= 40 ) ) 
							{
								temp2beS = 40;
							}
							tempScreen ();
						}
						if ( ( y >= offP [1] ) && ( y <= ( offP [1] + offP [3] ) ) ) //press offset plus
						{
							waitForIt ( offP [0], offP [1], offP [2], offP [3] );
							temp2beO += 0.1;
							if ( temp2beO >= 10 ) 
							{
								temp2beO = 9.9;
							}
							tempScreen ();
						}
						if ( ( y >= almP [1] ) && ( y <= ( almP [1] + almP [3] ) ) ) //press alarm plus
						{
							waitForIt ( almP [0], almP [1], almP [2], almP [3] );
							temp2beA += 0.1;
							if ( temp2beA >= 10 ) 
							{
								temp2beA = 9.9;
							}
							tempScreen ();
						}
					}
				}
				break;
			case 4  :    //-------------------- LIGHT CONTROL -------------------
				if ( ( x >= prSAVE [0] ) && ( x <= ( prSAVE [0] + prSAVE [2] ) ) && ( y >= prSAVE [1] ) && ( y <= ( prSAVE [1] + prSAVE [3] ) ) )  //press SAVE
				{
					waitForIt ( prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3] );
					lightTime1H = timeDispH; lightTime1M = timeDispM;
					lightTime2H = timeDispH1; lightTime2M = timeDispM1;

					SaveLightTimesToEEPROM ();
					dispScreen = 1;
					clearScreen ();
					mainScreen ( true );
				}		
				else 
				{
					if ( ( y >= hou1U [1] ) && ( y <= ( hou1U [1] + hou1U [3] ) ) ) //FIRST ROW
					{                   
						if ( ( x >= hou1U [0] ) && ( x <= ( hou1U [0] + hou1U [2] ) ) ) //press hour up
						{             
							waitForIt ( hou1U [0], hou1U [1], hou1U [2], hou1U [3] );
							rtcSet [2]++;
							if ( rtcSet [2] >= 24 ) 
							{
								rtcSet [2] = 0;
							}
						}
						if ( ( x >= min1U [0] ) && ( x <= ( min1U [0] + min1U [2] ) ) ) //press min up
						{
							waitForIt ( min1U [0], min1U [1], min1U [2], min1U [3] );
							rtcSet [1]++;
							if ( rtcSet [1] >= 60 ) 
							{
								rtcSet [1] = 0;
							}
						}
					}	
					if ( ( y >= hou1D [1] ) && ( y <= ( hou1D [1] + hou1D [3] ) ) ) //SECOND ROW
					{
						if ( ( x >= hou1D [0] ) && ( x <= ( hou1D [0] + hou1D [2] ) ) ) //press hour down
						{
							waitForIt ( hou1D [0], hou1D [1], hou1D [2], hou1D [3] );
							rtcSet [2]--;
							if ( rtcSet [2] < 0 ) 
							{
								rtcSet [2] = 23;
							}
						}
						if ( ( x >= min1D [0] ) && ( x <= ( min1D [0] + min1D [2] ) ) ) //press min down
						{
							waitForIt ( min1D [0], min1D [1], min1D [2], min1D [3] );
							rtcSet [1]--;
							if ( rtcSet [1] < 0 ) 
							{
								rtcSet [1] = 59;
							}
						}
					}
					if ( ( y >= hou2U [1] ) && ( y <= ( hou2U [1] + hou2U [3] ) ) ) //FIRST ROW
					{
						if ( ( x >= hou2U [0] ) && ( x <= ( hou2U [0] + hou2U [2] ) ) ) //press hour up
						{
							waitForIt ( hou2U [0], hou2U [1], hou2U [2], hou2U [3] );
							rtcSet2 [2]++;
							if ( rtcSet2 [2] >= 24 ) 
							{
								rtcSet2 [2] = 0;
							}
						}
						if ( ( x >= min2U [0] ) && ( x <= ( min2U [0] + min2U [2] ) ) ) //press min up
						{
							waitForIt ( min2U [0], min2U [1], min2U [2], min1U [3] );
							rtcSet2 [1]++;
							if ( rtcSet2 [1] >= 60 ) 
							{
								rtcSet2 [1] = 0;
							}
						}
					}
					if ( ( y >= hou2D [1] ) && ( y <= ( hou2D [1] + hou2D [3] ) ) ) //SECOND ROW
					{
						if ( ( x >= hou2D [0] ) && ( x <= ( hou2D [0] + hou2D [2] ) ) ) //press hour down
						{
							waitForIt ( hou2D [0], hou2D [1], hou2D [2], hou1D [3] );
							rtcSet2 [2]--;
							if ( rtcSet2 [2] < 0 ) 
							{
								rtcSet2 [2] = 23;
							}
						}
						if ( ( x >= min2D [0] ) && ( x <= ( min2D [0] + min2D [2] ) ) ) //press min down
						{
							waitForIt ( min2D [0], min2D [1], min2D [2], min2D [3] );
							rtcSet2 [1]--;
							if ( rtcSet2 [1] < 0 ) 
							{
								rtcSet2 [1] = 59;
							}
						}
					}
					LightControlScreen ( false );
				}
				break;
			case 5  : //Not In Use
				break;
			case 6  : //Not In Use
				break;
			case 7  : //Not In Use
				break;
			case 8  : //Not In Use
				break;
			case 9  : //Not In Use
				break;
			case 10 : //Not In Use
				break;
			case 11 : //Not In Use
				break;
			case 12 :     //-------------------- GENERAL SETTINGS -------------------
				if ( ( x >= prSAVE [0] ) && ( x <= (prSAVE [0] + prSAVE [2] ) ) && ( y >= prSAVE [1] ) && ( y <= (prSAVE [1] + prSAVE [3])  ) )  //press SAVE
				{
					waitForIt ( prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3] );
					SaveGenSetsToEEPROM ();
					dispScreen = MENUSCREEN_ONE;
					clearScreen ();
					menuScreen ();
				}
				if ( ( x >= 185 ) && ( x <= 305 ) && ( y >= 19 ) && ( y <= 39 ) )   //press DD/MM/YYYY Button
				{
					waitForIt ( 185, 19, 120, 20 );
					setCalendarFormat = 0;
					genSetSelect ();
				}

				if ( ( x >= 185 ) && ( x <= 305 ) && ( y >= 45 ) && ( y <= 65 ) )   //press Month DD, YYYY Button
				{
					waitForIt ( 185, 45, 120, 20 );
					setCalendarFormat = 1;
					genSetSelect ();
				}

				if ( ( x >= 195 ) && ( x <= 235 ) ) //first column
				{
					if ( ( y >= 76 ) && ( y <= 96 ) ) //press 12HR Button
					{
						waitForIt ( 195, 76, 40, 20 );
						setTimeFormat = 1;
						genSetSelect ();
					}
					if ( ( y >= 107 ) && ( y <= 127 ) ) //press deg C
					{
						waitForIt ( 195, 107, 40, 20 );
						setTempScale = 0;
						genSetSelect ();
					}
					if ( ( y >= 138 ) && ( y <= 158 ) ) //press Screensaver ON
					{
						waitForIt ( 195, 138, 40, 20 );
						setScreensaver = 1;
						genSetSelect ();
					}
					if ( ( y >= 169 ) && ( y <= 189 ) ) //press Auto-Stop on Feed ON
					{
						waitForIt ( 195, 169, 40, 20 );
						setAutoStop = 1;
						genSetSelect ();
					}
				}
				if ( ( x >= 255 ) && ( x <= 295 ) ) //second col				
				{
					if ( ( y >= 76 ) && ( y <= 96 ) ) //press 24HR Button
					{
						waitForIt ( 255, 76, 40, 20 );
						setTimeFormat = 0;
						genSetSelect ();
					}
					if ( ( y >= 107 ) && ( y <= 127 ) ) //press deg F
					{
						waitForIt ( 255, 107, 40, 20 );
						setTempScale = 1;
						genSetSelect ();
					}
					if ( ( y >= 138 ) && ( y <= 158 ) ) //press Screensaver OFF
					{
						waitForIt ( 255, 138, 40, 20 );
						setScreensaver = 2;
						genSetSelect ();
					}
					if ( ( y >= 169 ) && ( y <= 189 ) ) //press Auto-Stop on Feed OFF
					{
						waitForIt ( 255, 169, 40, 20 );
						setAutoStop = 2;
						genSetSelect ();
					}
				}

				break;

			case 13 :     //--------------- AUTOMATIC FISH FEEDER PAGE --------------
				if ( ( x >= 5 ) && ( x <= 150 ) && ( y >= 20 ) && ( y <= 40 ) ) //press Feeding Time 1
				{
					waitForIt ( 5, 20, 150, 20 );
					feedTime = 1; dispScreen = 14;
					clearScreen ();
					setFeederTimesScreen ();
				}
				else if ( ( x >= 165 ) && ( x <= 315 ) && ( y >= 20 ) && ( y <= 40 ) ) //press Feeding Time 2
				{
					waitForIt ( 165, 20, 150, 20 );
					feedTime = 2; dispScreen = 14;
					clearScreen ();
					setFeederTimesScreen ();
				}
				else if ( ( x >= 5 ) && ( x <= 150 ) && ( y >= 168 ) && ( y <= 188 ) ) //press Feeding Time 3
				{
					waitForIt ( 5, 168, 150, 20 );
					feedTime = 3; dispScreen = 14;
					clearScreen ();
					setFeederTimesScreen ();
				}
				else if ( ( x >= 165 ) && ( x <= 315 ) && ( y >= 168 ) && ( y <= 188 ) ) //press Feeding Time 4
				{
					waitForIt ( 165, 168, 315, 20 );
					feedTime = 4; dispScreen = 14;
					clearScreen ();
					setFeederTimesScreen ();
				}
				else if ( ( x >= 85 ) && ( x <= 230 ) && ( y >= 94 ) && ( y <= 114 ) ) //press Feeding Fish Now!
				{
					waitForIt ( 85, 94, 150, 20 );
					tft.fillRoundRect ( 85, 94, 150, 20, 20/8, ILI9341_GREEN );
					tft.drawRoundRect ( 85, 94, 150, 20, 20/8, ILI9341_WHITE );
					tft.setTextColor ( ILI9341_BLACK ); printTxt ( "Now Feeding", 120, 100 );
					FEEDTimeNow = 1;
				}
				break;				

			case 14 :     //------------ SET AUTOMATIC FISH FEEDER TIMES ------------
				if ( ( x >= back [0] ) && ( x <= ( back [0] + back [2] ) ) && ( y >= back [1] ) && ( y <= ( back [1] + back [3] ) ) )
				{
					waitForIt ( back [0], back [1], back [2], back [3] );
					ReadFromEEPROM ();
					if ( ( timeDispH >= 0 ) && ( timeDispH <= 11 ) ) 
					{
						AM_PM = 1;
					}
					else 
					{
						AM_PM = 2;
					}
					dispScreen = 13;
					clearScreen ();
					autoFeederScreen ();
				}
				else if ( ( x >= prSAVE [0] ) && ( x <= ( prSAVE [0] + prSAVE [2] ) ) && ( y >= prSAVE [1] ) && ( y <= (prSAVE [1] + prSAVE [3] ) ) )  //press SAVE
				{
					waitForIt ( prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3] );
					if ( setTimeFormat == 1 ) 
					{
						if ( ( rtcSet [2] == 0 ) && ( AM_PM == 2 ) )	
						{
							rtcSet [2] += 12;
						}
						if ( ( ( rtcSet [2] >= 1 ) && ( rtcSet [2] <= 11 ) ) && ( AM_PM == 2 ) ) 
						{
							rtcSet [2] += 12;
						}
						if ( ( ( rtcSet [2] >= 12 ) && ( rtcSet [2] <= 23 ) ) && ( AM_PM == 1 ) ) 
						{
							rtcSet [2] -= 12;
						}
					}
					if ( feedTime == 1 ) 
					{
						feedFish1H = rtcSet [2];
						feedFish1M = rtcSet [1];
					}
					if ( feedTime == 2 ) 
					{
						feedFish2H = rtcSet [2];
						feedFish2M = rtcSet [1];
					}
					if ( feedTime == 3 ) 
					{
						feedFish3H = rtcSet [2];
						feedFish3M = rtcSet [1];
					}
					if ( feedTime == 4 ) 
					{
						feedFish4H = rtcSet [2];
						feedFish4M = rtcSet [1];
					}
					SaveFeedTimesToEEPROM ();
					dispScreen = 13;
					clearScreen ();
					autoFeederScreen ();				
				}	
				else if ( ( x >= 70 ) && ( x <= 250 ) && ( y >= 150 ) && ( y <= 170 ) ) //Feeding ON/OFF
				{
					waitForIt ( 70, 150, 180, 20 );
					if ( feedTime == 1 ) { if ( FEEDTime1 == 1 ) { FEEDTime1 = 0; } else { FEEDTime1 = 1; } }
					if ( feedTime == 2 ) { if ( FEEDTime2 == 1 ) { FEEDTime2 = 0; } else { FEEDTime2 = 1; } }
					if ( feedTime == 3 ) { if ( FEEDTime3 == 1 ) { FEEDTime3 = 0; } else { FEEDTime3 = 1; } }
					if ( feedTime == 4 ) { if ( FEEDTime4 == 1 ) { FEEDTime4 = 0; } else { FEEDTime4 = 1; } }
					feedingTimeOnOff ();
				}
				else 
				{
					if ( ( y >= houP [1] ) && ( y <= (houP [1] + houP [3] ) ) ) //FIRST ROW
					{
						if ( ( x >= houP [0] ) && ( x <= ( houP [0] + houP [2] ) ) ) //press hour up
						{
							waitForIt ( houP [0], houP [1], houP [2], houP [3] );
							rtcSet [2]++; if ( rtcSet [2] >= 24 ) { rtcSet [2] = 0; }
						}
						if ( ( x >= minP [0] ) && ( x <= ( minP [0] + minP [2] ) ) ) //press min up
						{
							waitForIt ( minP [0], minP [1], minP [2], minP [3] );
							rtcSet [1]++; if ( rtcSet [1] >= 60 ) { rtcSet [1] = 0; }
						}
						
					}
					if ( ( y >= houM [1] ) && ( y <= ( houM [1] + houM [3] ) ) ) //SECOND ROW
					{
						if ( ( x >= houM [0] ) && ( x <= ( houM [0] + houM [2] ) ) ) //press hour down
						{
							waitForIt ( houM [0], houM [1], houM [2], houM [3] );
							rtcSet [2]--; if ( rtcSet [2] < 0 ) { rtcSet [2] = 23; }
						}
						if ( ( x >= minM [0] ) && ( x <= ( minM [0] + minM [2] ) ) ) //press min down
						{
							waitForIt ( minM [0], minM [1], minM [2], minM [3] );
							rtcSet [1]--; if ( rtcSet [1] < 0 ) { rtcSet [1] = 59; }
						}
				
					}
					setFeederTimesScreen ( false );
				}
				break;
			case 15 :  //----------------Maintenance Menu -----------------------
				if ( ( x >= prSAVE [0] ) && ( x <= ( prSAVE [0] + prSAVE [2] ) ) && ( y >= prSAVE [1] ) && ( y <= ( prSAVE [1] + prSAVE [3] ) ) )  //press SAVE
				{
					waitForIt ( prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3] );
					SaveDoyToEEPROM(); SaveMaintDaysToEEPROM ();
					clearScreen (); mainScreen ( true );
				}
				else	
				{
					if ( ( x >= day1D [0] ) && ( x <= ( day1D [0] + day1D [2] ) ) ) //x axis of day Down Buttons
							{
						if ( ( y >= day1D [1] ) && ( y <= ( day1D [1] + day1D [3] ) ) ) //y axis of day 1 Down Button
								{
							waitForIt ( day1D [0], day1D [1], day1D [2], day1D [3] ); //wait for button press
							maint1D--; if ( maint1D < 0 ) { maint1D = 99; }
						}
						if ( ( y >= day2D [1] ) && ( y <= ( day2D [1] + day2D [3] ) ) ) //y axis of day 2 Down Button
								{
							waitForIt ( day2D [0], day2D [1], day2D [2], day2D [3] ); //wait for button press
							maint2D--; if ( maint2D < 0 ) { maint2D = 99; }
						}
						if ( ( y >= day3D [1] ) && ( y <= ( day3D [1] + day3D [3] ) ) ) //y axis of day 3 Down Button
								{
							waitForIt ( day3D [0], day3D [1], day3D [2], day3D [3] ); //wait for button press
							maint3D--; if ( maint3D < 0 ) { maint3D = 99; }						
						}
						if ( ( y >= day4D [1] ) && ( y <= ( day4D [1] + day4D [3] ) ) ) //y axis of day 4 Down Button
								{
							waitForIt ( day4D [0], day4D [1], day4D [2], day4D [3] ); //wait for button press
							maint4D--; if ( maint4D < 0 ) { maint4D = 99; }						
						}
					}	
					if ( ( x >= day1U [0] ) && ( x <= ( day1U [0] + day1U [2] ) ) ) //x axis of day Up Buttons
							{
						if ( ( y >= day1U [1] ) && ( y <= ( day1U [1] + day1U [3] ) ) ) //y axis of day 1 Up Button
								{
							waitForIt ( day1U [0], day1U [1], day1U [2], day1U [3] ); //wait for button press
							maint1D++; if ( maint1D > 99 ) { maint1D= 0; }						
						}
						if ( ( y >= day2U [1] ) && ( y <= ( day2U [1] + day2U [3] ) ) ) //y axis of day 1 Up Button
								{
							waitForIt ( day2U [0], day2U [1], day2U [2], day2U [3] ); //wait for button press
							maint2D++; if ( maint2D > 99 ) { maint2D = 0; }	
						}
						if ( ( y >= day3U [1] ) && ( y <= ( day3U [1] + day3U [3] ) ) ) //y axis of day 1 Up Button
								{
							waitForIt ( day3U [0], day3U [1], day3U [2], day3U [3] ); //wait for button press
							maint3D++; if ( maint3D > 99 ) { maint3D = 0; }	
						}
						if ( ( y >= day4U [1] ) && ( y <= ( day4U [1] + day4U [3] ) ) ) //y axis of day 1 Up Button
								{
							waitForIt ( day4U [0], day4U [1], day4U [2], day4U [3] ); //wait for button press
							maint4D++; if ( maint4D > 99 ) { maint4D = 0; }	
						}
					}	
					if ( ( x >= day1R [0] ) && ( x <= ( day1R [0] + day1R [2] ) ) ) //x axis of Reset Buttons
					{
						if ( ( y >= day1R [1] ) && ( y <= ( day1R [1] + day1R [3] ) ) ) //y axis of Day 1 Reset Button
								{
							waitForIt ( day1R [0], day1R [1], day1R [2], day1R [3] );
							doy1C = (doyC + maint1D); //update with new DOY
						}
						if ( ( y >= day2R [1] ) && ( y <= ( day2R [1] + day2R [3] ) ) ) //y axis of Day 2 Reset Button
								{
							waitForIt ( day2R [0], day2R [1], day2R [2], day2R [3] );	
							doy2C = (doyC + maint2D); //update with new DOY
						}
						if ( ( y >= day3R [1] ) && ( y <= ( day3R [1] + day3R [3] ) ) ) //y axis of Day 3 Reset Button
								{
							waitForIt ( day3R [0], day3R [1], day3R [2], day3R [3] );	
							doy3C = (doyC + maint3D); //update with new DOY
						}
						if ( ( y >= day4R [1] ) && ( y <= ( day4R [1] + day4R [3] ) ) ) //y axis of Day 4 Reset Button
								{
							waitForIt ( day4R [0], day4R [1], day4R [2], day4R [3] );	
							doy4C = (doyC + maint4D); //update with new DOY
						}

					}
					maintSettingScreen ();
				}
		}
	}	
}

/******************************* End of Touch Screen *********************************/

/************************************* Setup *****************************************/   
void setup() 
{
	Serial.begin(9600); 	Serial.println("System Boot...");
	pinMode(OUTLET_BOX_PWR, OUTPUT); digitalWrite(OUTLET_BOX_PWR, HIGH);
	pinMode (WATER_FILTER_PWR, OUTPUT); digitalWrite(WATER_FILTER_PWR, HIGH); // Water Filter Motor 
	pinMode (RTC_PWR, OUTPUT); digitalWrite(RTC_PWR, HIGH); //setup arduino to power RTC from analog pins (NEED TO MOVE POWER OFF OF THIS PIN)
   pinMode(RTC_GND, OUTPUT); digitalWrite(RTC_GND, LOW); //setup arduino to power RTC from analog pins (NEED TO MOVE POEW OFF OF THIS PIN)
   pinMode(FEEDER_PWR, OUTPUT); digitalWrite(FEEDER_PWR, HIGH);
   pinMode (LIGHTS_PWR, OUTPUT ); digitalWrite ( LIGHTS_PWR, HIGH ); Lights_On_Flag = false ;// Tank Lights Power
   pinMode ( TEMP_SENSOR_PWR, OUTPUT ); digitalWrite( TEMP_SENSOR_PWR, HIGH ); //Temp Sensor Power
 
	RTC.get(rtc, true); Serial.println( "Start RTC");
	tft.begin(); Serial.println ("Start TFT");		
	tft.setRotation( ROTATION ); Serial.println("Set Screen Orientation");

	if ( checkTemp ){
		sensors.begin(); Serial.println("Start Sensor Routine");    //start up temperature library
		sensors.setResolution ( waterThermometer, TEMERATURE_PRECISION );	// set the resolution to 9 bit
	}
	Serial.println("Check Touch Screen Sensitivity");
	if (! ctp.begin(40))   // pass in 'sensitivity' coefficient
   {
   	Serial.println(F(" T6206 ERROR"));
   	while (1);
  	
  }
  	ReadFromEEPROM (); Serial.println("Get data from EEPROM");
   dispScreen = 1;
	mainScreen( true );
}

/******************************** End of Setup ****************************************/

/********************************  Main Loop ******************************************/
void loop()
{
	if ( ( ctp.touched () ) && ( screenSaverTimer >= setScreenSaverTimer ) ) 
	{
		screenSaverTimer = 0;
		processMyTouch();
		clearScreen ();
		mainScreen ( true );
		dispScreen = 0;
	}
	else 
	{
		if ( ctp.touched() ) { processMyTouch(); }
	}
	waterFilterTimer ();
	fiveSecDelayToUpdateScreen ();
}	

/***************************  End of Main Loop ****************************************/
void printOutput()
{
	// serialOutput();
	Serial.print ( "Display Screen = " ); Serial.println ( dispScreen );
	Serial.print ( "Lights_On_Flag = "); Serial.println( Lights_On_Flag );
	if ( digitalRead ( A5 ) == 1 )
	{
		Serial.println ( "Feeder is On");
	}
	if ( digitalRead ( A5 ) == 0 )
	{
		Serial.println ( "Feeder is Off");
	}
}
void serialOutput()
{
  // digital clock display of the time
  Serial.print(rtc [2]);
  printDigits(rtc [1]);
  printDigits(rtc [0]);
  Serial.print(" ");
	
 }

void printDigits(int digits)
{
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void cyclecount ()
{
	// countcycle++;
	serialOutput (); Serial.print ( "waterfilterStopped = "); Serial.println (  waterfilterStopped );
	serialOutput (); Serial.print ( "Display = "); Serial.print (dispScreen); Serial.print ( " screenSaverRunning = "); Serial.println (screenSaverRunning);
	serialOutput (); Serial.print ( filterOffMinutes ); Serial.print (":"); Serial.println ( filterOffSeconds);
	serialOutput (); Serial.print ( "screenSaverTimer = "); Serial.println (screenSaverTimer);
	// serialOutput (); Serial.print ( "setScreenSaverTimer = "); Serial.println (setScreenSaverTimer);
	// Serial.print ( "")
}
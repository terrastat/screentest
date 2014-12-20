#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Time.h"
#include <DS3234RTC.h>
#include <EEPROM.h>
#include <EEPROMAnything.h>
#include <SPI.h>       // this is needed for display
#include <Wire.h>      // this is needed for FT6206
#include <Adafruit_FT6206.h>

// screen size  w = 320 h = 240
// For the Adafruit shield, these are the default.
//#define VER 1.00
#define VER 1.01
#define TFT_DC 9
#define TFT_CS 10
#define rotation 3

#define MAINSCREEN 0
#define MENUSCREEN_ONE 1
#define CLOCKSETUP 2
#define TEMPCONTROL 3
#define GENERAL_SETTINGS 12
#define AUTOMATIC_FEEDER 13
#define FEEDER_TIMER 14
#define ABOUT 15
#define MENUSCREEN_TWO 16

#define XLARGE 4
#define LARGE 3
#define MEDIUM 2
#define SMALL 1
#define BUTTONCOLOR_BLUE 0
#define BUTTONCOLOR_RED 1
#define BUTTONCOLOR_GREEN 2

#define ILI9341_GRAY  0x4208
int x, y;   
int x1, y1;                         //touch coordinates

unsigned long previousMillisFive = 0;         //Used in the Main Loop (Checks Time,Temp,LEDs,Screen)
unsigned long previousMillisAlarm = 0;        //Used in the Alarm

int setScreensaver = 1;              //ON=1 || OFF=2 (change in prog)
int screenSaverTimer = 0;            //counter for Screen Saver
int setScreenSaverTimer = ( 5 ) * 12;   //how long in (minutes) before Screensaver comes on
boolean SCREEN_RETURN = true;        //Auto Return to mainScreen() after so long of inactivity
int returnTimer = 0;                 //counter for Screen Return
int setReturnTimer = setScreenSaverTimer * .75;       //Will return to main screen after 75% of the amount of
//time it takes before the screensaver turns on

int dispScreen = 0;     
//0-Main Screen, 1-Menu, 2-Clock Setup, 3-Temp Control,
//4-LED Test Options, 5-Test LED Arrays, 6-Test Individual
//LED Colors, 7-Choose LED Color, 8-View Selected Color
//Values, 9-Change Selected Color Array Values
//10-Wavemaker, 11-Wavemaker Settings, 12-General
//Settings, 13-Automatic Feeder, 14-Set Feeder Timers,
//15-About, 16-Options menu 2

int FEEDTime1, FEEDTime2, FEEDTime3, FEEDTime4;
//int feedTime1, feedTime2, feedTime3, feedTime4;
int feedTime;
int feedFish1H, feedFish1M,          //Times to feed the fish
		feedFish2H, feedFish2M, feedFish3H, feedFish3M, feedFish4H, feedFish4M;

int setCalendarFormat = 1;           //DD/MM/YYYY=0 || Month DD, YYYY=1 (change in prog)
char otherData [19];  // other printed data
//char formattetTime12[9]; //07:48 PM
//char formattetTime24[6]; //19:48
//byte data [56];
char time [11], date [16]; //day [16];


int timeDispH = 8, timeDispM = 30, xTimeH, xTimeM10, xTimeM1, xTimeAMPM, xColon;
int rtc [7], rtcSet [7];               //Clock arrays
int rtcSet2, AM_PM, yTime;           //Setting clock stuff
int setTimeFormat = 1;               //24HR=0 || 12HR=1 (change in prog)
int setTempScale = 0;                //Celsius=0 || Fahrenheit=1 (change in prog)
int setAutoStop = 2;                 //ON=1 || OFF=2 (change in prog)

// The FT6206 uses hardware I2C (SCL/SDA)
Adafruit_FT6206 ctp = Adafruit_FT6206();
// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// If using the breakout, change pins as desired
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
boolean tempAlarmflag = 0;
boolean CLOCK_SCREENSAVER = true;    //For a Clock Screensaver "true" / Blank Screen "false"
//You can turn the Screensaver ON/OFF in the pogram

float temp2beS;                      //Temporary Temperature Values
float temp2beO;                      //Temporary Temperature Values
float temp2beA;                      //Temporary Temperature Values
char degC_F [2];                       //Used in the Conversion of Celsius to Fahrenheit
float setTempC = 0.0;                //Desired Water Temperature (User input in program)
float setTempF = 0.0;
float offTempC = 0.0;                //Desired Water Temp. Offsets for Heater & Chiller (User input in program)
float offTempF = 0.0;
float alarmTempC = 0.0;              //Temperature the Alarm will sound (User input in program)
float alarmTempF = 0.0;
boolean tempCoolflag = 0;            //1 if cooling on
boolean tempHeatflag = 0;            //1 if heating on
/**************************** CHOOSE OPTION MENU1 BUTTONS *****************************/
const int tanD [] = { 10, 30, 120, 35 };        //"TIME and DATE" settings
const int temC [] = { 10, 70, 120, 35 };        //"H2O TEMP CONTROL" settings
const int gSet [] = { 10, 110, 120, 35 };      //"GENERAL SETTINGS" page
const int aFeed [] = { 165, 30, 120, 35 };    //"AUTOMATIC FEEDER" menu
const int about [] = { 165, 70, 120, 35 };    //"ABOUT" program information
const int liteS [] = { 165, 110, 120, 35 };   //'LIGHT CONTROL" page
/**************************** TIME AND DATE SCREEN BUTTONS ***************************/
const int houU [] = { 110, 22, 25, 25 };       //hour up
const int minU [] = { 180, 22, 25, 25 };       //min up
const int ampmU [] = { 265, 22, 25, 25 };      //AM/PM up
const int houD [] = { 110, 78, 25, 25 };       //hour down
const int minD [] = { 180, 78, 25, 25 };       //min down
const int ampmD [] = { 265, 78, 25, 25 };      //AM/PM down
const int dayU [] = { 110, 112, 25, 25 };     //day up
const int monU [] = { 180, 112, 25, 25 };     //month up
const int yeaU [] = { 265, 112, 25, 25 };     //year up
const int dayD [] = { 110, 162, 25, 25 };     //day down
const int monD [] = { 180, 162, 25, 25 };     //month down
const int yeaD [] = { 265, 162, 25, 25 };     //year down
/*************************** H2O TEMP CONTROL SCREEN BUTTONS *************************/
const int temM [] = { 90, 49, 115, 74 };        //temp. minus
const int temP [] = { 205, 49, 230, 74 };       //temp. plus
const int offM [] = { 90, 99, 115, 124 };       //offset minus
const int offP [] = { 205, 99, 230, 124 };      //offset plus
const int almM [] = { 90, 149, 115, 174 };      //alarm minus
const int almP [] = { 205, 149, 230, 174 };     //alarm plus
/**************************** LIGHT CONTROL MENU BUTTONS *******************************/
const int hou1U [] = { 110, 25, 25, 25 };       //Light On hour up
const int min1U [] = { 180, 25, 25, 25 };       //Light On min up
const int ampm1U [] = { 265, 25, 25, 25 };      //Light On AM/PM up
const int hou1D [] = { 110, 76, 25, 25 };      //Light On hour down
const int min1D [] = { 180, 76, 25, 25 };      //Light On min down
const int ampm1D [] = { 265, 76, 25, 25 };     //Light On AM/PM down
const int hou2U [] = { 110, 112, 25, 25 };       //Light Off hour up
const int min2U [] = { 180, 112, 25, 25 };       //Light Off min up
const int ampm2U [] = { 265, 112, 25, 25 };      //Light Off AM/PM up
const int hou2D [] = { 110, 162, 25, 25 };      //Light Off hour down
const int min2D [] = { 180, 162, 25, 25 };      //Light Off min down
const int ampm2D [] = { 265, 162, 25, 25 };     //Light Off AM/PM down

/*********************************** MISCELLANEOUS BUTTONS *********************************/
const int back [] = { 5, 200, 100, 25 };       //BACK
const int prSAVE [] = { 110, 200, 100, 25 };   //SAVE or NEXT
const int canC [] = {215 , 200, 100, 25 };     //CANCEL
/************************* AUTOMATIC FISH FEEDER BUTTONS *****************************/
//These Buttons are made within the function
/******************* SET AUTOMATIC FISH FEEDING TIMES BUTTONS ************************/
const int houP [] = { 110, 38, 25, 25 };       //hour up
const int minP [] = { 180, 38, 25, 25 };       //min up
const int ampmP [] = { 265, 38, 25, 25 };      //AM/PM up
const int houM [] = { 110, 89, 25, 25 };      //hour down
const int minM [] = { 180, 89, 25, 25 };      //min down
const int ampmM [] = { 265, 89, 25, 25 };     //AM/PM down

/********************************* EEPROM FUNCTIONS ***********************************/
struct config_g {
	int calendarFormat;
	int timeFormat;
	int tempScale;
	int SCREENsaver;
	int autoStop;
} GENERALsettings;  //660 - 680

struct config_f {
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

void SaveGenSetsToEEPROM () {
	GENERALsettings.calendarFormat = int ( setCalendarFormat );
	GENERALsettings.timeFormat = int ( setTimeFormat );
	GENERALsettings.tempScale = int ( setTempScale );
	GENERALsettings.SCREENsaver = int ( setScreensaver );
	GENERALsettings.autoStop = int ( setAutoStop );
	EEPROM_writeAnything ( 660, GENERALsettings );
}

void SaveFeedTimesToEEPROM () {
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

void ReadFromEEPROM () {
	int k = EEPROM.read ( 0 );
	char tempString [3];

	EEPROM_readAnything ( 660, GENERALsettings );
	setCalendarFormat = GENERALsettings.calendarFormat;
	setTimeFormat = GENERALsettings.timeFormat;
	setTempScale = GENERALsettings.tempScale;
	setScreensaver = GENERALsettings.SCREENsaver;
	setAutoStop = GENERALsettings.autoStop;

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
}

/***************************** END OF EEPROM FUNCTIONS ********************************/

/********************************** RTC FUNCTIONS *************************************/
// Real Time Clock settings go here

/********************************* END OF RTC FUNCTIONS *******************************/

/********************************** TIME AND DATE BAR **********************************/
void TimeDateBar ( boolean refreshAll = false ) {
	char oldVal [16], minute [3], stunde [3], ampm [6], month [5];
//////////////////////////
// temp values for testing
rtc [1] = 5;
rtc [2] = 12;
//rtc [3] = 
rtc [4] = 17;
rtc [5] =12; 
rtc [6] = 2014;
//rtc [7] =
//////////////////////////
	tft.setTextSize ( SMALL );
	if ( ( rtc [1] >= 0 ) && ( rtc [1] <= 9 ) ) {
		sprintf ( minute, "%i%i", 0, rtc [1] );
	}               //adds 0 to minutes
	else {
		sprintf ( minute, "%i", rtc [1] );
	}

	if ( setTimeFormat == 1 ) {
		if ( rtc [2] == 0 ) {
			sprintf ( stunde, "%i", 12 );
		}                //12 HR Format
		else {
			if ( rtc [2] > 12 ) {
				sprintf ( stunde, "%i", rtc [2] - 12 );
			}
			else {
				sprintf ( stunde, "%i", rtc [2] );
			}
		}
	}

	if ( rtc [2] < 12 ) {
		sprintf ( ampm, " AM  " );
//    ampm=" AM  ";
	}              //Adding the AM/PM sufffix
	else {
		sprintf ( ampm, " PM  " );
//    ampm= " PM  ";
	}

	sprintf ( oldVal, "%i", time );                                 //refresh time if different
	if ( setTimeFormat == 1 ) {
		sprintf ( time, "%s:%s%s", stunde, minute, ampm );
	}
	else {
		sprintf ( time, " %i:%s      ", rtc [2], minute );
	}
	if ( ( oldVal != time ) || refreshAll ) {
		tft.setTextColor ( ILI9341_YELLOW );
		tft.setCursor ( 215, 227 );
		tft.print ( time );            //Display time
	}

	if ( rtc [5] == 1 ) {
		sprintf ( month, "JAN " );
	}             //Convert the month to its name
	if ( rtc [5] == 2 ) {
		sprintf ( month, "FEB " );
	}
	if ( rtc [5] == 3 ) {
		sprintf ( month, "MAR " );
	}
	if ( rtc [5] == 4 ) {
		sprintf ( month, "APR " );
	}
	if ( rtc [5] == 5 ) {
		sprintf ( month, "MAY " );
	}
	if ( rtc [5] == 6 ) {
		sprintf ( month, "JUN " );
	}
	if ( rtc [5] == 7 ) {
		sprintf ( month, "JLY " );
	}
	if ( rtc [5] == 8 ) {
		sprintf ( month, "AUG " );
	}
	if ( rtc [5] == 9 ) {
		sprintf ( month, "SEP " );
	}
	if ( rtc [5] == 10 ) {
		sprintf ( month, "OCT " );
	}
	if ( rtc [5] == 11 ) {
		sprintf ( month, "NOV " );
	}
	if ( rtc [5] == 12 ) {
		sprintf ( month, "DEC " );
	}

	sprintf ( oldVal, "%s", date );                                 //refresh date if different
//  date.reserve(24);
	if ( setCalendarFormat == 0 ) {
		sprintf ( date, "  %i/%i/%i   ", rtc [4], rtc [5], rtc [6] );
//    date= "  " + String(rtc[4]) + "/" + String(rtc[5]) + "/" + String(rtc[6]) + "   ";

	}
	else {
		sprintf ( date, "  %s%i, %i", month, rtc [4], rtc [6] );
//    date= "  " + month + String(rtc[4]) + ',' + ' ' + String(rtc[6]);
	}
	if ( ( oldVal != date ) || refreshAll ) {
//    char bufferD[15];
//    date.toCharArray(bufferD, 15);              //String to Char array
		tft.setTextColor ( ILI9341_YELLOW );
		tft.setCursor ( 20, 227 );
		tft.print ( date );             //Display date
	}
}
/****************************** END OF TIME AND DATE BAR ******************************/

/********************************* MISC. FUNCTIONS ************************************/
void clearScreen () {
	tft.fillRect ( 1, 15, 318, 226, ILI9341_BLACK );
}

void printButton ( char* text, int x1, int y1, int x2, int y2, boolean fontsize = false, int buttonColor = 0 ) {
	int stl = strlen ( text );
	int fx, fy;

	tft.fillRoundRect ( x1, y1, x2, y2, y2 / 8, ILI9341_BLUE );
	tft.drawRoundRect ( x1, y1, x2, y2, y2 / 8, ILI9341_WHITE );
	tft.setTextColor(ILI9341_WHITE);

	if ( fontsize ) {
		tft.setTextSize ( SMALL );
		fx = x1 + ( ( x2 / 2 ) - ( ( stl * 6 ) / 2) );
		fy = y1 + ( ( y2 / 2 ) - 3 );
		tft.setCursor(fx, fy);
		tft.printf ( text );
	}
	else {
		tft.setTextSize ( SMALL );
		fx = x1 + ( ( x2 / 2 ) - ( ( stl * 6 ) / 2) );
		fy = y1 + ( ( y2 / 2 ) - 3 );
		tft.setCursor(fx, fy);
		tft.printf ( text );
	}
}

void printHeader ( char* headline ) {
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

void waitForIt ( int x1, int y1, int x2, int y2 ){   // Draw a red frame while a button is touched
	
	tft.drawRoundRect ( x1, y1, x2, y2, y2 / 8, ILI9341_RED );
	while ( ctp.touched () ) {
		delay(500);
	}
	
	tft.drawRoundRect ( x1, y1, x2, y2, y2 / 8, ILI9341_BLUE );
}

void feedingTimeOnOff () {
	
	if ( ( feedTime == 1 ) && ( FEEDTime1 == 1 ) ) {
		tft.fillRoundRect ( 70, 150, 180, 20 , 20/8, ILI9341_GREEN );
		tft.setCursor( 94, 154 );
		tft.setTextSize ( SMALL );
		tft.setTextColor ( ILI9341_BLACK );
		tft.print ( "Feeding Time 1 ON" );
	}
	if ( ( feedTime == 1 ) && ( FEEDTime1 == 0 ) ) {
		tft.fillRoundRect ( 70, 150, 180, 20, 20/8, ILI9341_RED );
		tft.setCursor( 94, 154 );
		tft.setTextSize ( SMALL );
		tft.print ( "Feeding Time 1 OFF" );
	}
	if ( ( feedTime == 2 ) && ( FEEDTime2 == 1 ) ) {
		tft.fillRoundRect ( 70, 150, 180, 20 , 20/8, ILI9341_GREEN );
		tft.setCursor( 94, 154 );
		tft.setTextSize ( SMALL );
		tft.setTextColor ( ILI9341_BLACK );
		tft.print ( "Feeding Time 2 ON" );
	}
	if ( ( feedTime == 2 ) && ( FEEDTime2 == 0 ) ) {
		tft.fillRoundRect ( 70, 150, 180, 20, 20/8, ILI9341_RED );
		tft.setCursor( 94, 154 );
		tft.setTextSize ( SMALL );
		tft.print ( "Feeding Time 2 OFF" );
	}
	if ( ( feedTime == 3 ) && ( FEEDTime3 == 1 ) ) {
		tft.fillRoundRect ( 70, 150, 180, 20 , 20/8, ILI9341_GREEN );
		tft.setCursor( 94, 154 );
		tft.setTextColor ( ILI9341_BLACK );
		tft.setTextSize ( SMALL );
		tft.print ( "Feeding Time 3 ON" );
	}
	if ( ( feedTime == 3 ) && ( FEEDTime3 == 0 ) ) {
		tft.fillRoundRect ( 70, 150, 180, 20, 20/8, ILI9341_RED );
		tft.setCursor( 94, 154 );
		tft.setTextSize ( SMALL );
		tft.print ( "Feeding Time 3 OFF" );
	}
	if ( ( feedTime == 4 ) && ( FEEDTime4 == 1 ) ) {
		tft.fillRoundRect ( 70, 150, 180, 20 , 20/8, ILI9341_GREEN );
		tft.setCursor( 94, 154 );
		tft.setTextColor ( ILI9341_BLACK );
		tft.setTextSize ( SMALL );
		tft.print ( "Feeding Time 4 ON" );
	}
	if ( ( feedTime == 4 ) && ( FEEDTime4 == 0 ) ) {
		tft.fillRoundRect ( 70, 150, 180, 20, 20/8, ILI9341_RED );
		tft.setCursor( 94, 154 );
		tft.setTextSize ( SMALL );
		tft.print ( "Feeding Time 4 OFF" );
	}

	tft.drawRoundRect ( 70, 150, 180, 20, 20/8, ILI9341_WHITE );
}	

void TimeSaver ( boolean refreshAll = false ) {
	if ( setTimeFormat == 0 ) {                              //24HR Format
		tft.setTextSize ( XLARGE );
		tft.setTextColor ( ILI9341_BLUE );
		if ( ( rtc [2] >= 0 ) && ( rtc [2] <= 9 ) ) {                   //Display HOUR
			tft.fillRoundRect ( 80, 95, 31, 50, 20/8, ILI9341_BLACK );
			tft.setTextColor ( ILI9341_BLUE );
			tft.setCursor ( 112, 95 );
			tft.print ( rtc [2] );
		}
		else {
			tft.setCursor ( 80, 95 );
			tft.print ( rtc [2] );
		}
	}

	if ( setTimeFormat == 1 ) {                                //12HR Format
		tft.setTextColor ( ILI9341_BLUE );
		tft.setTextSize ( XLARGE );
		if ( rtc [2] == 0 ) {                                    //Display HOUR
			tft.setCursor ( 80, 95 )		;
			tft.print ( "12" );
		}
		if ( ( rtc [2] >= 1 ) && ( rtc [2] <= 9 ) ) {
			tft.fillRoundRect ( 80, 95, 31, 50, 20/8, ILI9341_BLACK );
			tft.setTextColor ( ILI9341_BLUE );
			tft.setCursor ( 112, 95 );
			tft.print ( rtc [2] );
		}
		if ( ( rtc [2] >= 10 ) && ( rtc [2] <= 12 ) ) {
			tft.setCursor ( 80, 95 );
			tft.print ( rtc [2] );
		}
		if ( ( rtc [2] >= 13 ) && ( rtc [2] <= 21 ) ) {
			tft.fillRoundRect ( 80, 95, 31, 50, 20/8, ILI9341_BLACK );
			tft.setTextColor ( ILI9341_BLUE );
			tft.setCursor ( 80, 95 );
			tft.print ( rtc [2] - 12 );
		}
		if ( rtc [2] >= 22 ) {
			tft.setCursor ( 80, 95 );
			tft.print ( rtc [2] - 12 );
		}

		if ( ( rtc [2] >= 0 ) && ( rtc [2] <= 11 ) ) {                  //Display AM/PM
			tft.setTextColor ( ILI9341_BLUE );
			tft.setTextSize ( MEDIUM );
			tft.setCursor ( 220, 108 );
			tft.print ( "AM" );
		}
		else {
			tft.setTextColor ( ILI9341_BLUE );
			tft.setTextSize ( MEDIUM );
			tft.setCursor ( 220, 108 );
			tft.print ( "PM" );
		}
	}

		tft.setTextColor ( ILI9341_BLUE );
		tft.setTextSize ( XLARGE );
		tft.fillCircle ( 140, 100, 3, ILI9341_BLUE );
		tft.fillCircle ( 140, 115, 3, ILI9341_BLUE );
	if ( ( rtc [1] >= 0 ) && ( rtc [1] <= 9 ) ) {                      //Display MINUTES
		tft.setCursor ( 156, 95 );
		tft.print ( "0" );
		tft.setCursor ( 188, 95 );
		tft.print ( rtc [1] );
	}
	else {
		tft.setCursor ( 188, 95 );
		tft.print ( rtc [1] );
	}
}

void screenSaver ()                               //Make the Screen Go Blank after so long
{
	if ( ( setScreensaver == 1 ) && ( tempAlarmflag == false ) ) {
		if (ctp.touched () ) {
			processMyTouch ();
		}
		else {
			screenSaverTimer++;
		}
		if ( screenSaverTimer == setScreenSaverTimer ) {
			dispScreen = 0;
			tft.fillScreen(ILI9341_BLACK);
			tft.setCursor(20, 120);
		}
		if ( CLOCK_SCREENSAVER == true ) {
			if ( screenSaverTimer > setScreenSaverTimer ) {
				dispScreen = 0;
				TimeSaver ( true );
			}
		}
	}	
}	

void genSetSelect () {
	if ( setCalendarFormat == 0 ) {                     //Calendar Format Buttons
		tft.fillRoundRect ( 185, 19, 120, 20, 20/8, ILI9341_GREEN );
		tft.setTextSize ( SMALL);
		tft.setTextColor ( ILI9341_BLACK );
		tft.setCursor ( 207, 23 );
		tft.print ( "DD MM YYYY" );
		tft.fillRoundRect ( 185, 45, 120, 20, 20/8, ILI9341_BLUE );
		tft.setTextSize ( SMALL);
		tft.setTextColor ( ILI9341_WHITE );
		tft.setCursor ( 199, 49 );
		tft.print ( "MTH DD YYYY" );
	}
	else {
		tft.fillRoundRect ( 185, 19, 120, 20, 20/8, ILI9341_BLUE );
		tft.setTextSize ( SMALL);
		tft.setTextColor ( ILI9341_WHITE );
		tft.setCursor ( 207, 23 );
		tft.print ( "DD MM YYYY" );
		tft.fillRoundRect ( 185, 45, 120, 20, 20/8, ILI9341_GREEN );
		tft.setTextSize ( SMALL);
		tft.setTextColor ( ILI9341_BLACK );
		tft.setCursor ( 199, 49 );
		tft.print ( "MTH DD YYYY" );
	}
	if ( setTimeFormat == 0 ) {                         //Time Format Buttons
		tft.fillRoundRect ( 195, 76, 40, 20, 20/8, ILI9341_BLUE );
		tft.setTextSize ( SMALL);
		tft.setTextColor ( ILI9341_WHITE );
		tft.setCursor ( 201, 80 );
		tft.print ( "12HR" );
		tft.fillRoundRect ( 255, 76, 40, 20, 20/8, ILI9341_GREEN );
		tft.setTextSize ( SMALL);
		tft.setTextColor ( ILI9341_BLACK );
		tft.setCursor ( 261, 80 );
		tft.print ( "24HR" );
	}
	else {
		tft.fillRoundRect ( 195, 76, 40, 20, 20/8, ILI9341_GREEN  );
		tft.setTextSize ( SMALL);
		tft.setTextColor ( ILI9341_BLACK );
		tft.setCursor ( 201, 80 );
		tft.print ( "12HR" );
		tft.fillRoundRect ( 255, 76, 40, 20, 20/8, ILI9341_BLUE  );
		tft.setTextSize ( SMALL);
		tft.setTextColor ( ILI9341_WHITE );
		tft.setCursor ( 261, 80 );
		tft.print ( "24HR" );
	}
	if ( setTempScale == 0 ) {                          //Temperature Scale Buttons
		tft.fillRoundRect ( 195, 107, 40, 20, 20/8, ILI9341_GREEN  );
		tft.setTextSize ( SMALL);
		tft.setTextColor ( ILI9341_BLACK );
		tft.setCursor ( 215, 111 );
		tft.print ( "C" );
		tft.fillRoundRect ( 255, 107, 40, 20, 20/8, ILI9341_BLUE  );
		tft.setTextSize ( SMALL);
		tft.setTextColor ( ILI9341_WHITE );
		tft.setCursor ( 275, 111 );
		tft.print ( "F" );
		tft.drawCircle( 210, 113, 1, ILI9341_BLACK );
		tft.drawCircle ( 270, 113, 1, ILI9341_WHITE );
	}
	else {
		tft.fillRoundRect ( 195, 107, 40, 20, 20/8, ILI9341_BLUE  );
		tft.setTextSize ( SMALL);
		tft.setTextColor ( ILI9341_WHITE );
		tft.setCursor ( 215, 111 );
		tft.print ( "C" );
		tft.fillRoundRect ( 255, 107, 40, 20, 20/8, ILI9341_GREEN  );
		tft.setTextSize ( SMALL);
		tft.setTextColor ( ILI9341_BLACK );
		tft.setCursor ( 275, 111 );
		tft.print ( "F" );
		tft.drawCircle( 210, 113, 1, ILI9341_WHITE );
		tft.drawCircle ( 270, 113, 1, ILI9341_BLACK );
	}	
}		

/*******************************END OF MISC. FUNCTIONS ************************************/


/*********************** MAIN SCREEN ********** dispScreen = 0 ************************/
void mainScreen( boolean refreshAll = false ){

	if ( dispScreen != 0){
		tft.fillScreen(ILI9341_BLACK);
		tft.drawRect( 0, 0, 320, 240, ILI9341_BLUE);            //Outside Border
		dispScreen = 0;
	}
		tft.setTextColor(ILI9341_WHITE);  tft.setTextSize( SMALL );
		tft.drawRect ( 0, 125, 319, 12, ILI9341_BLUE);        //Horizontal Divider
		tft.fillRect ( 0, 0, 319, 14, ILI9341_BLUE );             //Top Bar
		tft.setCursor(60, 4); 
		tft.setTextColor(ILI9341_YELLOW);    tft.setTextSize( SMALL );
		tft.print("Bill's Aquarium Controller v");
		tft.print(VER);
		tft.setTextColor(ILI9341_RED);
		tft.setCursor( 110, 128 );
		tft.print ( "MONITORS & ALERTS" );
		tft.setTextColor ( ILI9341_YELLOW );
		tft.setTextSize ( SMALL);
		tft.setCursor( 30, 60 );
		tft.print ( "Next Event ");
}

void screenReturn ()                                    //Auto Return to MainScreen()
{
	if ( SCREEN_RETURN == true ) {
		if ( dispScreen != 0 ) {
			if (ctp.touched () ) {
				processMyTouch ();
			}
			else {
				returnTimer++;
			}
			if ( returnTimer > setReturnTimer ) {
				returnTimer = 0;
//				LEDtestTick = false;
//				colorLEDtest = false;
//				ReadFromEEPROM ();
				dispScreen = 0;
				clearScreen ();
				mainScreen ( true );
			}
		}
	}
}
/******************************** END OF MAIN SCREEN **********************************/

/*********************** MENU SCREEN ********** dispScreen = 1 ************************/
void menuScreen () {
	tft.fillScreen(ILI9341_BLACK);
	printHeader ( "Choose Option " );
	printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );
	printButton ( "Time and Date", tanD [0], tanD [1], tanD [2], tanD [3] );
	printButton ( "H2O Temp Control", temC [0], temC [1], temC [2], temC [3] );
	printButton ( "General Settings", gSet [0], gSet [1], gSet [2], gSet [3] );
	printButton ( "Automatic Feeder", aFeed [0], aFeed [1], aFeed [2], aFeed [3] );
	printButton ( "Maintenace", about [0], about [1], about [2], about [3] );
	printButton ( "Light Control", liteS [0], liteS [1], liteS [2], liteS [3] );
}	
/********************************* END OF MENU1 SCREEN *********************************/
/*********************** MENU SCREEN ********** dispScreen = 15 ************************/
void menuScreen2 () {
	printHeader ( "Choose Option 2/2" );
	tft.drawRect ( 0, 196, 319, 194, ILI9341_BLUE);
	printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );
	printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );
}

/************** TIME and DATE SCREEN ********** dispScreen = 2 ************************/
void clockScreen ( boolean refreshAll = true ) {
/*
		printHeader ( "Time and Date Settings" );
		tft.setTextColor(ILI9341_YELLOW);
		printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );
		printButton ( "SAVE", prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3], SMALL );
		printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );


		printButton ( "+", houU [0], houU [1], houU [2], houU [3], SMALL );     //hour up
		printButton ( "+", minU [0], minU [1], minU [2], minU [3], SMALL );     //min up
		printButton ( "-", houD [0], houD [1], houD [2], houD [3], SMALL );     //hour down
		printButton ( "-", minD [0], minD [1], minD [2], minD [3], SMALL );     //min down
		if ( setTimeFormat == 1 ) {
			printButton ( "+", ampmU [0], ampmU [1], ampmU [2], ampmU [3], SMALL );  //AM/PM up
			printButton ( "-", ampmD [0], ampmD [1], ampmD [2], ampmD [3], SMALL );
		}  //AM/PM down

		printButton ( "+", monU [0], monU [1], monU [2], monU [3], SMALL );     //month up
		printButton ( "+", dayU [0], dayU [1], dayU [2], dayU [3], SMALL );     //day up
		printButton ( "+", yeaU [0], yeaU [1], yeaU [2], yeaU [3], SMALL );     //year up
		printButton ( "-", monD [0], monD [1], monD [2], monD [3], SMALL );     //month down
		printButton ( "-", dayD [0], dayD [1], dayD [2], dayD [3], SMALL );     //day down
		printButton ( "-", yeaD [0], yeaD [1], yeaD [2], yeaD [3], SMALL );     //year down
	

	ReadFromEEPROM ();
	timeDispH = rtcSet [2];
	timeDispM = rtcSet [1];
	xTimeH = 107;
	yTime = 52;
	xColon = xTimeH + 72;
	xTimeM10 = xTimeH + 86;
	xTimeM1 = xTimeH + 102;
	xTimeAMPM = xTimeH + 155;
	timeChange ();

	tft.setTextSize ( SMALL );
	tft.setTextColor (ILI9341_WHITE);
	tft.setCursor ( 20, 142 );
	tft.print ( "Date" );
	tft.setCursor ( 149, 142 );
	tft.print ( "/" );
	tft.setCursor(219, 142 );
	tft.print ( "/"  );
////////////////////////////////////////////////////////////////
////////////    variables added for testing only
//rtcSet [4] = 15;
//rtcSet [5] = 12;	
//rtcSet [6] = 2014;

	if ( setCalendarFormat == 0 )                             //DD/MM/YYYY Format
			{
		tft.setTextSize ( SMALL );
		tft.setTextColor ( ILI9341_WHITE );
		tft.setCursor ( 5, 160 );
		tft.print ( "(DD/MM/YYYY)" );
		tft.setTextSize ( MEDIUM );
		tft.setTextColor ( ILI9341_WHITE );
		

		if ( ( rtcSet [4] >= 0 ) && ( rtcSet [4] <= 9 ) )              //Set DAY
				{
			tft.setCursor ( 107 , 142 );
			tft.print ( "0" );
			tft.setCursor ( 123, 142 );
			tft.print ( rtcSet [4] );
		}
		else {
			tft.setCursor ( 107, 142);
			tft.print ( rtcSet [4] );
		}
		if ( ( rtcSet [5] >= 0 ) && ( rtcSet [5] <= 9 ) )              //Set MONTH
				{
			tft.setCursor ( 177, 142 );
			tft.print ( "0" );
			tft.setCursor ( 193, 142 );
			tft.print ( rtcSet [5] );
		}
		else {
			tft.setCursor ( 177, 142 );
			tft.print ( rtcSet [5] );
		}
	}
	
	else if ( setCalendarFormat == 1 )                             //MM/DD/YYYY Format
			{
			tft.setTextSize ( SMALL );
		tft.setTextColor ( ILI9341_WHITE );
		tft.setCursor ( 5, 160 );
		tft.print ( "(MM/DD/YYYY)" );
		tft.setTextSize ( MEDIUM );
		tft.setTextColor ( ILI9341_WHITE );
		
		if ( ( rtcSet [5] >= 0 ) && ( rtcSet [5] <= 9 ) )              //Set MONTH
				{
			tft.setCursor ( 107 , 142 );
			tft.print ( "0" );
			tft.setCursor ( 123, 142 );
			tft.print ( rtcSet [5] );
		}
		else {
			tft.setCursor ( 107, 142);
			tft.print ( rtcSet [5] );
		}
		if ( ( rtcSet [4] >= 0 ) && ( rtcSet [4] <= 9 ) )              //Set DAY
				{
				tft.setCursor ( 177, 142 );
			tft.print ( "0" );
			tft.setCursor ( 193, 142 );
			tft.print ( rtcSet [4] );
		}
		else {
			tft.setCursor ( 177, 142 );
			tft.print ( rtcSet [4] );
		}
	}
	tft.setCursor ( 247, 142 );
	tft.print ( rtcSet [6] );                //Set YEAR
*/}

void timeChange () {
	tft.setTextSize( SMALL );
	tft.setCursor ( 20, yTime );
	tft.print ( "Time" );

	if ( setTimeFormat == 0 )                                 //24HR Format
			{
		tft.setTextSize ( SMALL);
		tft.setCursor ( 20, (yTime + 18 ) );
		tft.print ( "(24HR)" );
	}

	if ( setTimeFormat == 1 )                                 //12HR Format
			{
		tft.setTextSize ( SMALL );
		tft.setCursor ( 20, (yTime + 18 ) );
		tft.print ( "(12HR)" );
	}

	timeCorrectFormat ();
}

void buildCorrectTime () {
	char minute [3], stunde [3], ampm [4];
	if ( ( timeDispM >= 0 ) && ( timeDispM <= 9 ) ) {
		sprintf ( minute, "%i%i", 0, timeDispM );
	}               //adds 0 to minutes
	else {
		sprintf ( minute, "%i", timeDispM );
	}
	if ( setTimeFormat == 1 ) {
		if ( timeDispH == 0 ) {
			sprintf ( stunde, "%i", 12 );
		}                //12 HR Format
		else {
			if ( timeDispH > 12 ) {
				sprintf ( stunde, "%i", timeDispH - 12 );
			}
			else {
				sprintf ( stunde, "%i", timeDispH );
			}
		}
	}

	if ( timeDispH < 12 ) {
		sprintf ( ampm, " AM" );
	}              //Adding the AM/PM sufffix
	else {
		sprintf ( ampm, " PM" );
	}

	if ( setTimeFormat == 1 ) {
		sprintf ( time, "%s:%s%s", stunde, minute, ampm );
	}
	else {
		sprintf ( time, "%i:%s", timeDispH, minute );
	}
}

void timeCorrectFormat () {
	tft.setTextSize ( MEDIUM );
	tft.setTextColor ( ILI9341_WHITE );
//	if (dispScreen != 14) {
	buildCorrectTime ();
//	}
	//if ( timeDispH ) 
	tft.setCursor ( xTimeH, yTime );
	tft.fillRoundRect ( xTimeH, yTime , (xTimeH + 40), 20, 20/8 , ILI9341_BLACK);
	tft.setTextColor ( ILI9341_WHITE );
	tft.print ( time );            //Display time

}
/**************************** END OF TIME and DATE SCREEN *****************************/

/*********** H2O TEMP CONTROL SCREEN ********** dispScreen = 3 ************************/
void tempScreen ( boolean refreshAll = false ) {		

	
		printHeader ( "H2O Temperature Control Settings" );

//		myGLCD.setColor ( 64, 64, 64 );                    //Draw Dividers in Grey
//		myGLCD.drawRect ( 0, 196, 319, 194 );              //Bottom Horizontal Divider
		printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );
		printButton ( "SAVE", prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3], SMALL );
		printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );
	

}
/************************** END of H20 TEMP CONTROL SCREEN ****************************/

/********** LIGHT CONTROL SCREEN ******** dispScreen = 4 ************************/
void LightControlScreen ( boolean refreshAll = false ) {
	printHeader ( "Light Control" );

	tft.drawRect ( 0, 196, 319, -2, ILI9341_GRAY );                 //Bottom Horizontal Divider
	printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );
	printButton ( "SAVE", prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3], SMALL );
	printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );

	tft.setTextSize( SMALL );
	tft.setCursor ( 25, 46 );
	tft.print ( "Lights On" );
	tft.setCursor ( 25, 136 );
	tft.print ( "Lights Off" );

	printButton ( "+", hou1U [0], hou1U [1], hou1U [2], hou1U [3], SMALL );     //hour up
	printButton ( "+", min1U [0], min1U [1], min1U [2], min1U [3], SMALL );     //min up
	printButton ( "-", hou1D [0], hou1D [1], hou1D [2], hou1D [3], SMALL );     //hour down
	printButton ( "-", min1D [0], min1D [1], min1D [2], min1D [3], SMALL );     //min down
	if ( setTimeFormat == 1 ) {
		printButton ( "+", ampm1U [0], ampm1U [1], ampm1U [2], ampm1U [3], SMALL );  //AM/PM up
		printButton ( "-", ampm1D [0], ampm1D [1], ampm1D [2], ampm1D [3], SMALL );
	}  //AM/PM down
	printButton ( "+", hou2U [0], hou2U [1], hou2U [2], hou2U [3], SMALL );     //hour up
	printButton ( "+", min2U [0], min2U [1], min2U [2], min2U [3], SMALL );     //min up
	printButton ( "-", hou2D [0], hou2D [1], hou2D [2], hou2D [3], SMALL );     //hour down
	printButton ( "-", min2D [0], min2D [1], min2D [2], min2D [3], SMALL );     //min down
	if ( setTimeFormat == 1 ) {
		printButton ( "+", ampm2U [0], ampm2U [1], ampm2U [2], ampm2U [3], SMALL );  //AM/PM up
		printButton ( "-", ampm2D [0], ampm2D [1], ampm2D [2], ampm2D [3], SMALL );
	}  //AM/PM down
}

/*************************** END OF LIGHT CONTROL SCREEN ***************************/

/******** GENERAL SETTINGS SCREEN ************* dispScreen = 12 ***********************/
void generalSettingsScreen () {
	printHeader ( "View/Change General Settings" );

	printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );
	printButton ( "SAVE", prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3], SMALL );
	printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );

	tft.setTextSize( SMALL );
	tft.setCursor ( 25, 36 );
	tft.print ( "Calendar Format" );
	tft.setCursor ( 25, 80 );
	tft.print ( "Time Format" );
	tft.setCursor ( 25, 111 );
	tft.print ( "Temperature Scale" );
	tft.setCursor ( 25, 142 );
	tft.print ( "Screensaver" );
	tft.setCursor ( 25, 173 );
	tft.print ( "Auto-Stop on Feed" );

	genSetSelect ();
}
/*********************** END OF GENERAL SETTINGS SETTINGS SCREEN **********************/

/******** AUTOMATIC FEEDER SCREEN ************* dispScreen = 13 ***********************/
void autoFeederScreen () {
	printHeader ( "Automatic Fish Feeder Page" );

	tft.drawRect ( 0, 196, 319, 2, ILI9341_GRAY  );
	printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );
	printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );
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
	tft.fillRoundRect ( 85, 94, 150, 20, 20/8, 0x980C );           //Feed Fish Now Button
	tft.setCursor ( 120, 100 );
	tft.print ( "Feed Fish Now!");

	if ( FEEDTime1 == 0 )                                 //Feeding Time 1 Button
			{
		tft.fillRoundRect ( 5, 20, 150, 20, 20/8, ILI9341_RED );
		tft.setCursor ( 34, 24);
		tft.setTextSize ( SMALL );
		tft.setTextColor ( ILI9341_WHITE );
		tft.print ( "Feeding Time 1" );
		tft.setTextColor (ILI9341_RED);
		tft.setCursor ( 22, 52 );
		tft.print ( "This time has not" );
		tft.setCursor ( 34, 65 );
		tft.print ( "been scheduled" );
	}
	else {	
		tft.fillRoundRect ( 5, 20, 150, 20, 20/8, ILI9341_GREEN );
		tft.setTextSize ( SMALL );
		tft.setTextColor ( ILI9341_BLACK );
		tft.setCursor ( 34, 24 );
		tft.print ( "Feeding Time 1" );

///////////////////////////////////		
//		timeDispH = feedFish1H;
//		timeDispM = feedFish1M;
//		timeDispH = 8;  // testing only
//		timeDispM = 30;
///////////////////////////////////

		if ( setTimeFormat == 0 ) {
			xTimeH = 45;
		}
		if ( setTimeFormat == 1 ) {
			xTimeH = 16;
		}
		ReadFromEEPROM ();
		if ( ( timeDispH >= 0 ) && ( timeDispH <= 11 ) ) {
			AM_PM = 1;
		}
		else {
			AM_PM = 2;
		}
		yTime = 58;
		xColon = xTimeH + 42;
		xTimeM10 = xTimeH + 48;
		xTimeM1 = xTimeH + 64;
		xTimeAMPM = xTimeH + 96;
		timeCorrectFormat ();
	}	
	if ( FEEDTime2 == 0 ) {                                //Feeding Time 2 Button

		tft.fillRoundRect ( 165, 20, 150, 20, 20/8, ILI9341_RED );
		tft.setCursor ( 194, 24);
		tft.setTextSize ( SMALL );
		tft.setTextColor ( ILI9341_WHITE );
		tft.print ( "Feeding Time 2" );
		tft.setTextColor (ILI9341_RED);
		tft.setCursor ( 182, 52 );
		tft.print ( "This time has not" );
		tft.setCursor ( 194, 65 );
		tft.print ( "been scheduled" );
	}
	else {	
		tft.fillRoundRect ( 165, 20, 150, 20, 20/8, ILI9341_GREEN );
		tft.setTextSize ( SMALL );
		tft.setTextColor ( ILI9341_BLACK );
		tft.setCursor ( 194, 24 );
		tft.print ( "Feeding Time 2" );
		if ( setTimeFormat == 0 ) {
			xTimeH = 200;
		}
		if ( setTimeFormat == 1 ) {
			xTimeH = 176;
		}
		ReadFromEEPROM ();
		if ( ( timeDispH >= 0 ) && ( timeDispH <= 11 ) ) {
			AM_PM = 1;
		}
		else {
			AM_PM = 2;
		}	
		yTime = 56;
		xColon = xTimeH + 32;
		xTimeM10 = xTimeH + 48;
		xTimeM1 = xTimeH + 64;
		xTimeAMPM = xTimeH + 96;
		timeCorrectFormat ();
	}	
	if ( FEEDTime3 == 0 )                                 //Feeding Time 3 Button
			{
		tft.fillRoundRect ( 5, 168, 150, 20, 20/8,ILI9341_RED );
		tft.setCursor ( 34, 172 );
		tft.setTextSize ( SMALL );
		tft.setTextColor ( ILI9341_WHITE );
		tft.print ( "Feeding Time 3" );
		tft.setTextColor ( ILI9341_RED );
		tft.setCursor ( 22, 133 );
		tft.print ( "This time has not" );
		tft.setCursor ( 34, 146 );
		tft.print ( "been scheduled" );
	}
	else {	
		tft.fillRoundRect ( 5, 168, 150, 20, 20/8, ILI9341_GREEN );
		tft.setTextSize ( SMALL );
		tft.setTextColor ( ILI9341_BLACK );
		tft.setCursor ( 24, 172 );
		tft.print ( "Feeding Time 3" );
		if ( setTimeFormat == 0 ) {
			xTimeH = 45;
		}
		if ( setTimeFormat == 1 ) {
			xTimeH = 16;
		}
		ReadFromEEPROM ();
		if ( ( timeDispH >= 0 ) && ( timeDispH <= 11 ) ) {
			AM_PM = 1;
		}
		else {
			AM_PM = 2;
		}	
		yTime = 136;
		xColon = xTimeH + 42;
		xTimeM10 = xTimeH + 63;
		xTimeM1 = xTimeH + 64;
		xTimeAMPM = xTimeH + 96;
		timeCorrectFormat ();
	}	
	if ( FEEDTime4 == 0 )                                 //Feeding Time 4 Button
			{
		tft.fillRoundRect ( 165, 168, 150, 20, 20/8, ILI9341_RED );
		tft.setCursor ( 194, 172 );
		tft.setTextSize ( SMALL );
		tft.setTextColor ( ILI9341_WHITE );
		tft.print ( "Feeding Time 4" );
		tft.setTextColor ( ILI9341_RED );
		tft.setCursor ( 182, 133 );
		tft.print ( "This time has not" );
		tft.setCursor ( 194, 146 );
		tft.print ( "been scheduled" );
	}
	else {	
		tft.fillRoundRect ( 165, 168, 150, 20, 20/8, ILI9341_GREEN );
		tft.setTextSize ( SMALL );
		tft.setTextColor ( ILI9341_BLACK );
		tft.setCursor ( 194, 172 );
		tft.print ( "Feeding Time 4" );
		if ( setTimeFormat == 0 ) {
			xTimeH = 200;
		}
		if ( setTimeFormat == 1 ) {
			xTimeH = 176;
		}
		ReadFromEEPROM ();
		if ( ( timeDispH >= 0 ) && ( timeDispH <= 11 ) ) {
			AM_PM = 1;
		}
		else {
			AM_PM = 2;
		}	
		yTime = 137;
		xColon = xTimeH + 32;
		xTimeM10 = xTimeH + 48;
		xTimeM1 = xTimeH + 64;
		xTimeAMPM = xTimeH + 96;
		timeCorrectFormat ();
	}	
	for ( int x = 0; x < 2; x++ ) {
		for ( int y = 0; y < 2; y++ ) {
			tft.drawRoundRect ( ( x * 160 ) + 5, ( y * 148 ) + 20, ( x * 1 ) + 150, ( y * 1 ) + 20, 20/8, ILI9341_WHITE );
		}
	}
}

/***** SET AUTOMATIC FEEDER TIMES SCREEN ********** dispScreen = 14 *******************/
void setFeederTimesScreen ( boolean refreshAll = true ) {
	
	if ( feedTime == 1 ) {
		printHeader ( "Set Feeding Time 1" );
	}
	if ( feedTime == 2 ) {
		printHeader ( "Set Feeding Time 2" );
	}
	if ( feedTime == 3 ) {
		printHeader ( "Set Feeding Time 3" );
	}
	if ( feedTime == 4 ) {
		printHeader ( "Set Feeding Time 4" );
	}

//	if ( refreshAll ) {
//		for ( int i = 0; i < 7; i++ ) {
//			rtcSet [i] = rtc [i];
//		}

		tft.drawRect ( 0, 196, 319, -2,  ILI9341_GRAY );
		printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );
		printButton ( "SAVE", prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3], SMALL );
		printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );

		feedingTimeOnOff ();

		printButton ( "+", houP [0], houP [1], houP [2], houP [3], SMALL );     //hour up
		printButton ( "+", minP [0], minP [1], minP [2], minP [3], SMALL );     //min up
		printButton ( "-", houM [0], houM [1], houM [2], houM [3], SMALL );     //hour down
		printButton ( "-", minM [0], minM [1], minM [2], minM [3], SMALL );     //min down
		if ( setTimeFormat == 1 ) {
			printButton ( "+", ampmP [0], ampmP [1], ampmP [2], ampmP [3], SMALL );  //AM/PM up
			printButton ( "-", ampmM [0], ampmM [1], ampmM [2], ampmM [3], SMALL );
		}  //AM/PM down
//	}

/////////////////////
//rtcSet [1] = 4;
//rtcSet [2] = 5;
/////////////////////

	timeDispH = rtcSet [2];
	timeDispM = rtcSet [1];
	xTimeH = 118;
	yTime = 68;
	xColon = xTimeH + 82;
	xTimeM10 = xTimeH + 80;
	xTimeM1 = xTimeH + 96;
	xTimeAMPM = xTimeH + 175;
	timeChange ();
}
/********************** END OF SET AUTOMATIC FEEDER TIMES SCREEN **********************/

/************** MAINTENANCE SCREEN ****************** dispScreen = 15 ***********************/
void AboutScreen () {
	printHeader ( "Maintenance Menu" );
	printButton ( "<< BACK >>", back [0], back [1], back [2], back [3], SMALL );
	printButton ( "CANCEL", canC [0], canC [1], canC [2], canC [3], SMALL );
}

/************************************ TOUCH SCREEN ************************************/
void processMyTouch () {
	TS_Point p = ctp.getPoint();

	x1 = p.x;
	y1 = p.y;
	// Print out raw data from screen touch controller
 // Serial.print("X = "); Serial.print(p.x);
 // Serial.print("\tY = "); Serial.print(p.y);
 // Serial.print(" -> ");
		// flip it around to match the screen.
	p.x = map(y1, 0, 320, 0, 320);
	p.y = map(x1, 0, 240, 240, 0);
	x = p.x;
	y = p.y;
	// Print out the remapped (rotated) coordinates
 // Serial.print("("); Serial.print(p.x);
 // Serial.print(", "); Serial.print(p.y);
 // Serial.println(")");


	returnTimer = 0;
	screenSaverTimer = 0;
	if ( ( x >= canC [0] ) && ( x <= ( canC [0] + canC [2] ) ) && ( y >= canC [1] ) && ( y <= (canC [1] + canC [3] ) )   //press cancel
			&& ( dispScreen != 0 ) && ( dispScreen != 5 ) && ( dispScreen != 6 ) && ( dispScreen != 8 ) && ( dispScreen != 11 ) ) {
		waitForIt ( canC [0], canC [1], canC [2], canC [3] );
		ReadFromEEPROM ();
		dispScreen = 0;
		clearScreen ();
		mainScreen ( true );
	}
	else if ( ( x >= back [0] ) && ( x <= ( back [0] + back [2] ) ) && ( y >= back [1] ) && ( y <= ( back [1] + back [3] ) )  //press back
			&& ( dispScreen != 0 ) && ( dispScreen != 1 ) && ( dispScreen != 5 ) && ( dispScreen != 6 ) && ( dispScreen != 8 ) && ( dispScreen != 11 )
			&& ( dispScreen != 14 ) && ( dispScreen != 18 ) && ( dispScreen != 19 ) ) {
		waitForIt ( back [0], back [1], back [2], back [3] );
		ReadFromEEPROM ();
		dispScreen = MENUSCREEN_ONE;
		clearScreen ();
		menuScreen ();
		
	}
	else 
	{
		switch ( dispScreen ) 
		{
			case 0 :		 //--------------- MAIN SCREEN (Press Any Key) ---------------
				dispScreen = MENUSCREEN_ONE;
				clearScreen ();
				menuScreen ();
				break;

			case 1 :     //--------------------- MENU SCREEN -------------------------
				if ( ( x >= tanD [0] ) && ( x <= ( tanD [0] + tanD [2] ) ) )                      //first column
				{
					if ( ( y >= tanD [1] ) && ( y <= ( tanD [1] + tanD [3] ) ) )                   //press Date & Clock Screen
					{
						waitForIt ( tanD [0], tanD [1], tanD [2], tanD [3] );
						if ( ( timeDispH >= 0 ) && ( timeDispH <= 11 ) ) 
						{
							AM_PM = 1;
						}
						else 
						{
							AM_PM = 2;
						}
						dispScreen = 2;
						clearScreen ();
						clockScreen ();
					}
					if ( ( y >= temC [1] ) && ( y <= ( temC [1] + temC [3] ) ) )                   //press H2O Temp Control
					{
						waitForIt ( temC [0], temC [1], temC [2], temC [3] );
						ReadFromEEPROM ();
						dispScreen = 3;
						clearScreen ();
						tempScreen ( true );
					}
					if ( ( y >= gSet [1] ) && ( y <= (gSet [1] + gSet [3] ) ) )                   //press General Settings
					{
						waitForIt ( gSet [0], gSet [1], gSet [2], gSet [3] );
						dispScreen = 12;
						clearScreen ();
						generalSettingsScreen ();
					}
				}

				if ( ( x >= aFeed [0] ) && ( x <= ( aFeed [0] + aFeed [2] ) ) )                      //first column
				{
					if ( ( y >= aFeed [1] ) && ( y <= ( aFeed [1] + aFeed [3] ) ) )              //press Automatic Feeder screen
					{
						waitForIt ( aFeed [0], aFeed [1], aFeed [2], aFeed [3] );
						dispScreen = 13;
						clearScreen ();
						autoFeederScreen ();
					}
					if ( ( y >= about [1] ) && ( y <= ( about [1] + about [3] ) ) )              //press About sketch
					{
						waitForIt ( about [0], about [1], about [2], about [3] );
						dispScreen = 15;
						clearScreen ();
						AboutScreen ();
					}
					if ( ( y >= liteS [1] ) && ( y <= ( liteS [1] + liteS [3] ) ) )              //press About sketch
					{
						waitForIt ( liteS [0], liteS [1], liteS [2], liteS [3] );
						dispScreen = 15;
						clearScreen ();
						LightControlScreen ();
					}
				}		
				break;
			case 2  :     //--------------- CLOCK & DATE SETUP SCREEN -----------------
/*				if ( ( x >= prSAVE [0] ) && ( x <= prSAVE [2] ) && ( y >= prSAVE [1] ) && ( y <= prSAVE [3] ) )  //press SAVE
						{
					waitForIt ( prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3] );
					if ( setTimeFormat == 1 ) {
						if ( ( rtcSet [2] == 0 ) && ( AM_PM == 2 ) ) {
							rtcSet [2] += 12;
						}
						if ( ( ( rtcSet [2] >= 1 ) && ( rtcSet [2] <= 11 ) ) && ( AM_PM == 2 ) ) {
							rtcSet [2] += 12;
						}
						if ( ( ( rtcSet [2] >= 12 ) && ( rtcSet [2] <= 23 ) ) && ( AM_PM == 1 ) ) {
							rtcSet [2] -= 12;
						}
					}
//					SaveRTC ();
					dispScreen = 0;
					clearScreen ();
					mainScreen ( true );
				}
				else {
					if ( ( y >= houU [1] ) && ( y <= houU [3] ) )                    //FIRST ROW
							{
						if ( ( x >= houU [0] ) && ( x <= houU [2] ) )                 //press hour up
								{
							waitForIt ( houU [0], houU [1], houU [2], houU [3] );
							rtcSet [2]++;
							if ( rtcSet [2] >= 24 ) {
								rtcSet [2] = 0;
							}
						}
						if ( ( x >= minU [0] ) && ( x <= minU [2] ) )                 //press min up
								{
							waitForIt ( minU [0], minU [1], minU [2], minU [3] );
							rtcSet [1]++;
							if ( rtcSet [1] >= 60 ) {
								rtcSet [1] = 0;
							}
						}
						if ( ( x >= ampmU [0] ) && ( x <= ampmU [2] ) )               //press AMPM up
								{
							waitForIt ( ampmU [0], ampmU [1], ampmU [2], ampmU [3] );
							if ( AM_PM == 1 ) {
								AM_PM = 2;
							}
							else {
								AM_PM = 1;
							}
						}
					}
					if ( ( y >= houD [1] ) && ( y <= houD [3] ) )                    //SECOND ROW
							{
						if ( ( x >= houD [0] ) && ( x <= houD [2] ) )                 //press hour down
								{
							waitForIt ( houD [0], houD [1], houD [2], houD [3] );
							rtcSet [2]--;
							if ( rtcSet [2] < 0 ) {
								rtcSet [2] = 23;
							}
						}
						if ( ( x >= minD [0] ) && ( x <= minD [2] ) )                 //press min down
								{
							waitForIt ( minD [0], minD [1], minD [2], minD [3] );
							rtcSet [1]--;
							if ( rtcSet [1] < 0 ) {
								rtcSet [1] = 59;
							}
						}
						if ( ( x >= ampmD [0] ) && ( x <= ampmD [2] ) )               //press AMPM down
								{
							waitForIt ( ampmD [0], ampmD [1], ampmD [2], ampmD [3] );
							if ( AM_PM == 1 ) {
								AM_PM = 2;
							}
							else {
								AM_PM = 1;
							}
						}
					}
					if ( ( y >= dayU [1] ) && ( y <= dayU [3] ) )                    //THIRD ROW
							{
						if ( setCalendarFormat == 0 )                         //DD/MM/YYYY Format
								{
							if ( ( x >= dayU [0] ) && ( x <= dayU [2] ) )              //press day up
									{
								waitForIt ( dayU [0], dayU [1], dayU [2], dayU [3] );
								rtcSet [4]++;
								if ( rtcSet [4] > 31 ) {
									rtcSet [4] = 1;
								}
							}
							if ( ( x >= monU [0] ) && ( x <= monU [2] ) )              //press month up
									{
								waitForIt ( monU [0], monU [1], monU [2], monU [3] );
								rtcSet [5]++;
								if ( rtcSet [5] > 12 ) {
									rtcSet [5] = 1;
								}
							}
						}
						else {
							if ( setCalendarFormat == 1 )                         //MM/DD/YYYY Format
									{
								if ( ( x >= dayU [0] ) && ( x <= dayU [2] ) )              //press month up
										{
									waitForIt ( dayU [0], dayU [1], dayU [2], dayU [3] );
									rtcSet [5]++;
									if ( rtcSet [5] > 12 ) {
										rtcSet [5] = 1;
									}
								}
								if ( ( x >= monU [0] ) && ( x <= monU [2] ) )              //press day up
										{
									waitForIt ( monU [0], monU [1], monU [2], monU [3] );
									rtcSet [4]++;
									if ( rtcSet [4] > 31 ) {
										rtcSet [4] = 1;
									}
								}
							}
						}
						if ( ( x >= yeaU [0] ) && ( x <= yeaU [2] ) )                 //press year up
								{
							waitForIt ( yeaU [0], yeaU [1], yeaU [2], yeaU [3] );
							rtcSet [6]++;
							if ( rtcSet [6] > 2100 ) {
								rtcSet [6] = 2000;
							}
						}
					}
					if ( ( y >= dayD [1] ) && ( y <= dayD [3] ) )                    //FOURTH ROW
							{
						if ( setCalendarFormat == 0 )                         //DD/MM/YYYY Format
								{
							if ( ( x >= dayD [0] ) && ( x <= dayD [2] ) )              //press day down
									{
								waitForIt ( dayD [0], dayD [1], dayD [2], dayD [3] );
								rtcSet [4]--;
								if ( rtcSet [4] < 1 ) {
									rtcSet [4] = 31;
								}
							}
							if ( ( x >= monD [0] ) && ( x <= monD [2] ) )              //press month down
									{
								waitForIt ( monD [0], monD [1], monD [2], monD [3] );
								rtcSet [5]--;
								if ( rtcSet [5] < 1 ) {
									rtcSet [5] = 12;
								}
							}
						}
						else {
							if ( setCalendarFormat == 1 )                         //MM/DD/YYYY Format
									{
								if ( ( x >= dayD [0] ) && ( x <= dayD [2] ) )              //press month down
										{
									waitForIt ( dayD [0], dayD [1], dayD [2], dayD [3] );
									rtcSet [5]--;
									if ( rtcSet [5] < 1 ) {
										rtcSet [5] = 12;
									}
								}
								if ( ( x >= monD [0] ) && ( x <= monD [2] ) )              //press day down
										{
									waitForIt ( monD [0], monD [1], monD [2], monD [3] );
									rtcSet [4]--;
									if ( rtcSet [4] < 1 ) {
										rtcSet [4] = 31;
									}
								}
							}
						}
						if ( ( x >= yeaD [0] ) && ( x <= yeaD [2] ) )                 //press year down
								{
							waitForIt ( yeaD [0], yeaD [1], yeaD [2], yeaD [3] );
							rtcSet [6]--;
							if ( rtcSet [6] < 2000 ) {
								rtcSet [6] = 2100;
							}
						}
					}
					clockScreen ( false );
				}*/
				break;
			case 3  :
				break;
			case 4  :    //-------------------- LIGHT CONTROL -------------------
				if ( ( x >= prSAVE [0] ) && ( x <= ( prSAVE [0] + prSAVE [2] ) ) && ( y >= prSAVE [1] ) && ( y <= ( prSAVE [1] + prSAVE [3] ) ) )  //press SAVE
				{
					waitForIt ( prSAVE [0], prSAVE [1], prSAVE [2], prSAVE [3] );
					SaveGenSetsToEEPROM ();
					dispScreen = MENUSCREEN_ONE;
					clearScreen ();
					menuScreen ();
				}
				break;
			case 5  : 
				break;
			case 6  :
				break;
			case 7  :
				break;
			case 8  :
				break;
			case 9  :
				break;
			case 10 :
				break;
			case 11 :
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

				if ( ( x >= 195 ) && ( x <= 235 ) )                         //first column
				{
					if ( ( y >= 76 ) && ( y <= 96 ) )                        //press 12HR Button
					{
						waitForIt ( 195, 76, 40, 20 );
						setTimeFormat = 1;
						genSetSelect ();
					}
					if ( ( y >= 107 ) && ( y <= 127 ) )                      //press deg C
					{
						waitForIt ( 195, 107, 40, 20 );
						setTempScale = 0;
						genSetSelect ();
					}
					if ( ( y >= 138 ) && ( y <= 158 ) )                      //press Screensaver ON
					{
						waitForIt ( 195, 138, 40, 20 );
						setScreensaver = 1;
						genSetSelect ();
					}
					if ( ( y >= 169 ) && ( y <= 189 ) )                      //press Auto-Stop on Feed ON
					{
						waitForIt ( 195, 169, 40, 20 );
						setAutoStop = 1;
						genSetSelect ();
					}
				}
				if ( ( x >= 255 ) && ( x <= 295 ) )                         //second column
				{
					if ( ( y >= 76 ) && ( y <= 96 ) )                        //press 24HR Button
					{
						waitForIt ( 255, 76, 40, 20 );
						setTimeFormat = 0;
						genSetSelect ();
					}
					if ( ( y >= 107 ) && ( y <= 127 ) )                      //press deg F
					{
						waitForIt ( 255, 107, 40, 20 );
						setTempScale = 1;
						genSetSelect ();
					}
					if ( ( y >= 138 ) && ( y <= 158 ) )                      //press Screensaver OFF
					{
						waitForIt ( 255, 138, 40, 20 );
						setScreensaver = 2;
						genSetSelect ();
					}
					if ( ( y >= 169 ) && ( y <= 189 ) )                      //press Auto-Stop on Feed OFF
					{
						waitForIt ( 255, 169, 40, 20 );
						setAutoStop = 2;
						genSetSelect ();
					}
				}

				break;

			case 13 :     //--------------- AUTOMATIC FISH FEEDER PAGE --------------
				if ( ( x >= 5 ) && ( x <= 150 ) && ( y >= 20 ) && ( y <= 40 ) )      //press Feeding Time 1
				{
					waitForIt ( 5, 20, 150, 20 );
					feedTime = 1;
					dispScreen = 14;
					clearScreen ();
					setFeederTimesScreen ();
				}
				else if ( ( x >= 165 ) && ( x <= 315 ) && ( y >= 20 ) && ( y <= 40 ) )    //press Feeding Time 2
				{
					waitForIt ( 165, 20, 150, 20 );
					feedTime = 2;
					dispScreen = 14;
					clearScreen ();
					setFeederTimesScreen ();
				}
				else if ( ( x >= 5 ) && ( x <= 150 ) && ( y >= 168 ) && ( y <= 188 ) )      //press Feeding Time 3
				{
//					waitForIt ( 5, 168, 150, 20 );
//					feedTime = 3;
//					dispScreen = 14;
//					clearScreen ();
//					setFeederTimesScreen ();
				}
				else if ( ( x >= 165 ) && ( x <= 315 ) && ( y >= 168 ) && ( y <= 188 ) )    //press Feeding Time 4
				{
//					waitForIt ( 165, 168, 315, 20 );
//					feedTime = 4;
//					dispScreen = 14;
//					clearScreen ();
//					setFeederTimesScreen ();
				}
				else if ( ( x >= 85 ) && ( x <= 230 ) && ( y >= 94 ) && ( y <= 114 ) )      //press Feeding Fish Now!
				{
					waitForIt ( 85, 94, 150, 20 );
					tft.fillRoundRect ( 85, 94, 150, 20, 20/8, ILI9341_GREEN );
					tft.drawRoundRect ( 85, 94, 150, 20, 20/8, ILI9341_WHITE );
					tft.setTextColor ( ILI9341_BLACK );
					tft.setCursor ( 120, 100 );
					tft.print ( "Now Feeding" );
//					digitalWrite ( autoFeeder, HIGH );
					delay ( 5000 );
					tft.fillRoundRect ( 85, 94, 150, 20, 20/8, 	0x980C );
					tft.drawRoundRect ( 85, 94, 150, 20, 20/8, 	0x980C );
					tft.setTextColor ( ILI9341_WHITE );
					tft.setCursor ( 120, 100 );
					tft.print ( "Feed Fish Now!" );
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
				else if ( ( x >= prSAVE [0] ) && ( x <= prSAVE [2] ) && ( y >= prSAVE [1] ) && ( y <= prSAVE [3] ) )  //press SAVE
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
				else if ( ( x >= 70 ) && ( x <= 250 ) && ( y >= 150 ) && ( y <= 170 ) )     //Feeding ON/OFF
						{
					waitForIt ( 70, 150, 180, 20 );
					if ( feedTime == 1 ) {
						if ( FEEDTime1 == 1 ) {
							FEEDTime1 = 0;
						}
						else {
							FEEDTime1 = 1;
						}
					}
					if ( feedTime == 2 ) {
						if ( FEEDTime2 == 1 ) {
							FEEDTime2 = 0;
						}
						else {
							FEEDTime2 = 1;
						}
					}
					if ( feedTime == 3 ) {
						if ( FEEDTime3 == 1 ) {
							FEEDTime3 = 0;
						}
						else {
							FEEDTime3 = 1;
						}
					}
					if ( feedTime == 4 ) {
						if ( FEEDTime4 == 1 ) {
							FEEDTime4 = 0;
						}
						else {
							FEEDTime4 = 1;
						}
					}
					feedingTimeOnOff ();
				}
				else {
					if ( ( y >= houP [1] ) && ( y <= (houP [1] + houP [3] ) ) )                 //FIRST ROW
							{
						if ( ( x >= houP [0] ) && ( x <= ( houP [0] + houP [2] ) ) )              //press hour up
								{
							waitForIt ( houP [0], houP [1], houP [2], houP [3] );
							rtcSet [2]++;
							if ( rtcSet [2] >= 24 ) {
								rtcSet [2] = 0;
							}
						}
						if ( ( x >= minP [0] ) && ( x <= ( minP [0] + minP [2] ) ) )             //press min up
								{
							waitForIt ( minP [0], minP [1], minP [2], minP [3] );
							rtcSet [1]++;
							if ( rtcSet [1] >= 60 ) {
								rtcSet [1] = 0;
							}
						}
						if ( ( x >= ampmP [0] ) && ( x <= ( ampmP [0] + ampmP [2] ) ) )           //press AMPM up
								{
							waitForIt ( ampmP [0], ampmP [1], ampmP [2], ampmP [3] );
							if ( AM_PM == 1 ) {
								AM_PM = 2;
							}
							else {
								AM_PM = 1;
							}
						}
					}
					if ( ( y >= houM [1] ) && ( y <= ( houM [1] + houM [3] ) ) )                //SECOND ROW
							{
						if ( ( x >= houM [0] ) && ( x <= ( houM [0] + houM [2] ) ) )              //press hour down
								{
							waitForIt ( houM [0], houM [1], houM [2], houM [3] );
							rtcSet [2]--;
							if ( rtcSet [2] < 0 ) {
								rtcSet [2] = 23;
							}
						}
						if ( ( x >= minM [0] ) && ( x <= ( minM [0] + minM [2] ) ) )             //press min down
								{
							waitForIt ( minM [0], minM [1], minM [2], minM [3] );
							rtcSet [1]--;
							if ( rtcSet [1] < 0 ) {
								rtcSet [1] = 59;
							}
						}
						if ( ( x >= ampmM [0] ) && ( x <= ( ampmM [0] + ampmM [2] ) ) )           //press AMPM down
								{
							waitForIt ( ampmM [0], ampmM [1], ampmM [2], ampmM [3] );
							if ( AM_PM == 1 ) {
								AM_PM = 2;
							}
							else {
								AM_PM = 1;
							}
						}
					}
					setFeederTimesScreen ( false );
				}
				break;


		}
	}	
}


///////////////////////////////////////////////////////////////////////////////////////
///////                         Setup                                     ////////////
///////////////////////////////////////////////////////////////////////////////////////
void setup() {
	Serial.begin(9600);
	tft.begin();
	tft.setRotation(rotation);
	if (! ctp.begin(40)) {  // pass in 'sensitivity' coefficient
    Serial.println("Couldn't start FT6206 touchscreen controller");
    while (1);
  }
  dispScreen = 1;
	mainScreen( true );
}


void loop(){
	if ( ( ctp.touched () ) && ( screenSaverTimer >= setScreenSaverTimer ) ) {
		screenSaverTimer = 0;
		processMyTouch();
		clearScreen ();
		mainScreen ();
		dispScreen = 0;
	}
	else {
		if ( ctp.touched () ) {
			processMyTouch ();
		}
	}
	unsigned long currentMillis = millis ();
	if ( currentMillis - previousMillisFive > 5000 )   //check time, temp and LED levels every 5s
			{
		previousMillisFive = currentMillis;
		if ( screenSaverTimer < setScreenSaverTimer ) {
			TimeDateBar ();
		}
//		screenReturn ();
		screenSaver ();

		if ( ( dispScreen == 0 ) && ( screenSaverTimer < setScreenSaverTimer ) ) {
			mainScreen ();
		}
	}		
}	


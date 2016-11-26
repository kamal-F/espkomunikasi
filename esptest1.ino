#include "Arduino.h"
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include "MyKey.h"

#define DEBUG false

const String HOST = "192.168.43.114"; //your localhost at same ssid, ifconfig

const int portFertigation = A4;   // A4=18 port output for fertigation LOW/HIGH

unsigned long previousMillisSD = 0; //SD, Send Data
const long intervalSendData = 30000;

unsigned long previousMillisGD = 0; //GD, Get Data
const long intervalGetData = 200;

//from hidrodono
int valFertInterval = 0;
int valFertLong = 1;

int valHumidity = 0;
int valTemperature = 0;

int trHumidity = 70; // treshold Humidity
int trTemperature = 20; // treshold Temperature

//pump
int pumpFertigation = 0; //LOW;

SoftwareSerial ser(A2, A3); // RX 2, TX 3

//The setup function is called once at startup of the sketch
void setup()
{
	delay(500);
	pinMode(portFertigation, OUTPUT);
	digitalWrite(portFertigation,LOW);
	Serial.begin(9600);
	ser.begin(9600);// 9600 stable baud rate for uno
	delay(100);
	connectWifi();
	//delay(500);
}

// The loop function is called in an endless loop
void loop()
{

	unsigned long currentMillisSD = millis();
	long randNumber;
	String randNumberString;

	if (currentMillisSD - previousMillisSD >= intervalSendData) {
		randNumber = random(20,40);
		valTemperature = int(randNumber);

		sendDataID(valFertInterval, valFertLong, valHumidity, valTemperature,
				trHumidity,trTemperature,pumpFertigation);

	    previousMillisSD = currentMillisSD;
	}

	unsigned long currentMillisGD = millis();

	if (currentMillisGD - previousMillisGD >= intervalGetData) {

		getDataID("0");

		previousMillisGD = currentMillisGD;
	}
}

void sendDataID(int svalFertInterval, int svalFertLong, int svalHumidity,
		int svalTemperature, int strHumidity, int strTemperature, int spumpFertigation)
{
	String cmd = "AT+CIPSTART=\"TCP\",\"";

	cmd += HOST;
	cmd += "\",80\r\n";
	sendCommand(cmd,500,DEBUG);
	delay(30);

	String cmd2 = "GET /atest/baca.php?valFertInterval=";
	cmd2 += String(svalFertInterval, DEC);
	cmd2 += "&valFertLong=";
	cmd2 += String(svalFertLong, DEC);

	cmd2 += "&valHumidity=";
	cmd2 += String(svalHumidity, DEC);

	cmd2 += "&valTemperature=";
	cmd2 += String(svalTemperature, DEC);

	cmd2 += "&trHumidity=";
	cmd2 += String(strHumidity, DEC);

	cmd2 += "&trTemperature=";
	cmd2 += String(strTemperature, DEC);

	cmd2 += "&pumpFertigation=";
	cmd2 += String(spumpFertigation, DEC);

	cmd2 += " HTTP/1.1\r\n";
	cmd2 += "Host: " +HOST+ "\r\n\r\n";



	String pjg="AT+CIPSEND=";
	pjg += cmd2.length();
	pjg += "\r\n";

	String closeCommand = "AT+CIPCLOSE";
	closeCommand+="\r\n";

	sendCommand(pjg,500,DEBUG);
	delay(10);
	sendCommand(cmd2,800,DEBUG);
	delay(20);
	sendCommand(closeCommand,500,DEBUG);
	delay(10);
}


void getDataID(String id) {
	String cmd = "AT+CIPSTART=\"TCP\",\"";

	cmd += HOST;
	cmd += "\",80\r\n";
	sendCommand(cmd,500,DEBUG);
	delay(30);

	String cmd2 = "GET /atest/pump.json";
	cmd2 += " HTTP/1.1\r\n";
	cmd2 += "Host: " +HOST+ "\r\n\r\n";

	String pjg="AT+CIPSEND=";
	pjg += cmd2.length();
	pjg += "\r\n";

	String closeCommand = "AT+CIPCLOSE";
	closeCommand+="\r\n";

	sendCommand(pjg,500,DEBUG);
	delay(10);

	String isi = sendCommand(cmd2,500,DEBUG);
	delay(20);
	//Serial.print(isi);
	//parsing isi

	ser.find("fpump");

	//Serial.print(isi);

	int ygke = 0;
	ygke = isi.indexOf("fpump")-3;

	String result = isi.substring(ygke,isi.length());

	// Parse JSON
	int size = result.length() + 1;
	char json[size];
	result.toCharArray(json, size);
	StaticJsonBuffer<200> jsonBuffer;
	JsonObject& json_parsed = jsonBuffer.parseObject(json);
	if (!json_parsed.success())
	{
		Serial.println("parseObject() failed");
		return;
	}

	// Make the decision to turn off or on the LED
	if (strcmp(json_parsed["fpump"], "on") == 0) {
		digitalWrite(portFertigation, HIGH);
		//Serial.println("LED ON");
	}
	else {
		digitalWrite(portFertigation, LOW);
		//Serial.println("led off");
	}


	sendCommand(closeCommand,500,DEBUG);
	delay(10);
}




String sendCommand(String command, const int timeout, boolean debug) {
	String response = "";

	ser.print(command); // send the read character to the esp8266

	long int time = millis();

	while( (time+timeout) > millis()) {
		while(ser.available()) {
			// The esp has data so display its output to the serial window
			char c = ser.read(); // read the next character.
			response+=c;
		}
	}

	if(debug) {
		Serial.print(response);
	}

	return response;
}

void connectWifi() {
	//Set-wifi
	Serial.print("Restart Module...\n");
	sendCommand("AT+RST\r\n",1000,DEBUG);
	delay(100);
	Serial.print("Set wifi mode...\n");
	sendCommand("AT+CWMODE=1\r\n",1000,DEBUG); //
	delay(100);
	Serial.print("Connect to access point...\n");
	sendCommand("AT+CWJAP=\"" +SSID+ "\",\""+PASS+"\"\r\n",1000,DEBUG);
	delay(2000);
	Serial.print("Check IP Address...\n");
	sendCommand("AT+CIFSR\r\n",1000,DEBUG); // get ip address

	delay(100);
}

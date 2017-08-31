#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>

#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

#include "StructDefs.h"

#define DEBUG 1

#ifdef DEBUG
#define DEBUG_PRINT(x)      Serial.print (x)
#define DEBUG_PRINTDEC(x,DEC) Serial.print (x, DEC)
#define DEBUG_PRINTLN(x)    Serial.println (x)
#define DEBUG_PRINTLNDEC(x,DEC) Serial.println (x, DEC)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTDEC(x,DEC)
#define DEBUG_PRINTLN(x) 
#define DEBUG_PRINTLNDEC(x,DEC)
#endif

char mqtt_clientname[60] = "esptransit";

WiFiClient espClient;
String _lastMQTTMessage = "";

ESP8266HTTPUpdateServer _httpUpdater;

const char* _updateWebPath = "/firmware";
const char* _updateWebUsername = "admin";
const char* _updateWebPassword = "admin";
const char* _wifiAP = "********";
const char* _wifiAPPwd = "********";

ESP8266WebServer _httpServer(80);
String __webClientReturnString = "";

const char* host = "maps.googleapis.com";
const int httpsPort = 443;

float _normalDuration = 0;
float _transitDuration = 0;
float _tempVal = 0;

bool _displayTemp = false;

const long timebetweenruns = 5 * 60 * 1000;  //300 000 = 5 min
const long timebetweenanimationruns = 60 * 1000;  //every 60 seconds
const long timebetweentransitanimationruns = 15 * 1000;  //every 15 seconds
unsigned long currentrun; //set variable to time store
unsigned long animationrun; //set variable to time store
unsigned long transitrun; //set variable to time store
unsigned long animationtransitrun; //set variable to time store

void SetIsDisplayOn(bool __isOn) {	
	if (__isOn==false)
		clearcolors(true);	
	_isDisplayOn = __isOn;
}

String IpAddress2String(const IPAddress& ipAddress) {
	return String(ipAddress[0]) + String(".") + \
		String(ipAddress[1]) + String(".") + \
		String(ipAddress[2]) + String(".") + \
		String(ipAddress[3]);
}

void fetchTransit() {
	DEBUG_PRINTLN("Fetching Transit Info");
	// Use WiFiClientSecure class to create TLS connection
	WiFiClientSecure client;
	DEBUG_PRINT("connecting to ");
	DEBUG_PRINTLN(host);
	if (!client.connect(host, httpsPort)) {
		DEBUG_PRINTLN("connection failed");
		return;
	}

	String url = "/maps/api/distancematrix/json?units=metric&origins=1+Sandton+Drive&destinations=3+Melrose+Boulevard&Transit_model=best_guess&key=xxxxxxxxxxxxxxxxxxxx&departure_time=now";
	DEBUG_PRINT("requesting URL: ");
	DEBUG_PRINTLN(url);

	client.print(String("GET ") + url + " HTTP/1.1\r\n" +
		"Host: " + host + "\r\n" +
		"User-Agent: BuildFailureDetectorESP8266\r\n" +
		"Connection: close\r\n\r\n");

	DEBUG_PRINTLN("request sent");
	while (client.connected()) {
		String line = client.readStringUntil('\n');
		if (line == "\r") {
			DEBUG_PRINTLN("headers received");
			break;
		}
	}
	String line = "";
	String json = "";

	boolean httpBody = false;
	while (client.available()) {
		line = client.readStringUntil('\r');
		json += line;
	}

	StaticJsonBuffer<1000> jsonBuffer;
	DEBUG_PRINTLN("Got data:");
	DEBUG_PRINTLN(json);
	JsonObject& _jsonTransitObjRoot = jsonBuffer.parseObject(json);

	_normalDuration = _jsonTransitObjRoot["rows"][0]["elements"][0]["duration"]["value"];
	DEBUG_PRINT("Normal duration: ");
	DEBUG_PRINTLN(_normalDuration);

	_transitDuration = _jsonTransitObjRoot["rows"][0]["elements"][0]["duration_in_traffic"]["value"];
	DEBUG_PRINT("Transit duration: ");
	DEBUG_PRINTLN(_transitDuration);
	DEBUG_PRINTLN("==========");
	DEBUG_PRINTLN("closing connection");

	// close any connection before send a new request.
	// This will free the socket on the WiFi shield
	client.stop();
}

///////////////////// Transit Mode //////////////////////////////////
void updateTransitColours() {

	fetchTransit();

	float __percentageDiff = 0;

	//// set RGB colors for the quickest commute and before
	if (_transitDuration < _normalDuration) {
		_currentColour = {255, 255, 255};
	}
	else
	{
		__percentageDiff = (_transitDuration / _normalDuration) * 100;
    _currentColour = findColourInRange(__percentageDiff);  
    
    fade(_oldColour, _currentColour);	
	  _oldColour = _currentColour;
    
    OutputColour(_currentColour);
	}
}

void setupHTTPUpdateServer() {
	_httpUpdater.setup(&_httpServer, _updateWebPath, _updateWebUsername, _updateWebPassword);
	MDNS.addService("http", "tcp", 80);
	DEBUG_PRINTLN("HTTPUpdateServer ready! Open http://" + String(mqtt_clientname) + String(_updateWebPath) + " in your browser and login with username " + String(_updateWebUsername) + " and password " + String(_updateWebPassword) + "\n");
}

void setupWebServer() {
	DEBUG_PRINTLN("Handling Web Request...");

	_httpServer.on("/", []() {
		__webClientReturnString = "<HTML>";
		__webClientReturnString += "<HEAD>";
		__webClientReturnString += "<TITLE>MCMD Arduino</TITLE>";
		__webClientReturnString += "</HEAD>";
		__webClientReturnString += "<style>body {font: normal 12px Calibri, Arial;}</style>";
		__webClientReturnString += "<BODY>";

		__webClientReturnString += "<H1>" + String(mqtt_clientname) + " Control</H1>";
		__webClientReturnString += "<hr><a href=\"/sendstat\">MQTT, re-read& send status</a><br>";
		__webClientReturnString += "<hr><a href=\"/firmware\">Upgrade Firmware</a><br>";
		__webClientReturnString += "<hr><a href=\"/displayon\">Display On</a><br>";
		__webClientReturnString += "<a href=\"/displayoff\">Display Off</a><br>";


		__webClientReturnString += "<hr><br>Transit without Traffic: " + String(_normalDuration / 60,2) + " minutes";
		__webClientReturnString += "<br>Transit <b>with</b> Traffic: " + String(_transitDuration / 60,2) + " minutes";
		float __percTransit = (_transitDuration / _normalDuration) * 100;
		__webClientReturnString += "<br>Traffic / Transit: " + String(__percTransit,2) + " \%";
		
		__webClientReturnString += "<br><a href=\"/UpdateTransit\">Update Transit</a><br>";

		__webClientReturnString += "<hr><br>IP Address: " + IpAddress2String(WiFi.localIP());

		__webClientReturnString += "<br>MAC Address: " + WiFi.macAddress();
		__webClientReturnString += "<br>Last MQTT message recieved: " + _lastMQTTMessage;

		__webClientReturnString += "</BODY>";
		__webClientReturnString += "</HTML>";

		_httpServer.send(200, "text/html", __webClientReturnString);

	});

	DEBUG_PRINTLN("Arg(0): " + _httpServer.argName(0));

	_httpServer.on("/UpdateTransit", []() {
		fetchTransit();
		_httpServer.send(200, "text/plain", String("{\"state\": \"true\"}"));
	});

	_httpServer.on("/displayon", []() {
		SetIsDisplayOn(true);
		_httpServer.send(200, "text/plain", String("{\"state\": \"true\"}"));
	});

	_httpServer.on("/displayoff", []() {
		SetIsDisplayOn(false);
		_httpServer.send(200, "text/plain", String("{\"state\": \"false\"}"));
	});

	_httpServer.on("/reset", []() {
		_httpServer.send(200, "text/plain", "Resetting");
		ESP.reset();
	});

	DEBUG_PRINTLN("Web Request Completed...");
}

void setup() {
#ifdef DEBUG
	Serial.begin(115200);
#endif // DEBUG
	
  WiFi.begin(_wifiAP,_wifiAPPwd);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DEBUG_PRINTLN(".");
  }
  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("WiFi connected");

  // Print the IP address
  DEBUG_PRINTLN(WiFi.localIP());

	// Start the server
	DEBUG_PRINTLN("Web Server config");
	setupWebServer();
	DEBUG_PRINTLN("Update Server config");
	setupHTTPUpdateServer();

	DEBUG_PRINTLN("Server starting");
	_httpServer.begin();
	MDNS.begin(mqtt_clientname);

	pixels.begin();

	currentrun = millis();
  loading_colors();  // loading light sequence

	updateTransitColours();
}

void loop() {
	// Check if a client has connected
	_httpServer.handleClient();

  if (_isDisplayOn)  {
		currentrun = millis(); //sets the counter
	
			//transit update
			if (currentrun - transitrun >= timebetweenruns) {
				DEBUG_PRINTLN("loading transit"); //for reference
				transitrun = currentrun;
        loading_colors();  // loading light sequence
				updateTransitColours(); //update traffic info
			}

			//transit animation
			if (currentrun - animationtransitrun >= timebetweentransitanimationruns) {
				DEBUG_PRINTLN("transit animation"); //for reference
				animationtransitrun = currentrun;
	      breathe2(50, 250, _currentColour.r, _currentColour.g, _currentColour.b);
			}
  }
}

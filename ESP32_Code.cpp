/***************************************************
 *              SMART SOCKET ESP32
 *
 *        BLYNK + WEB API + SOFT AP
 *
 *  socket01 -> Relay 1 -> GPIO 2  -> Blynk V0
 *  socket02 -> Relay 2 -> GPIO 18 -> Blynk V1
 ***************************************************/


/***************************************************
 * BLYNK TEMPLATE INFO
 ***************************************************/

#define BLYNK_TEMPLATE_ID "TMPL396ujBJnH"
#define BLYNK_TEMPLATE_NAME "Smart Socket 2"
#define BLYNK_AUTH_TOKEN "Vi0sJxv9D5QOeXdTQY4KraatyMXjbtah"

#define BLYNK_PRINT Serial


/***************************************************
 * LIBRARIES
 ***************************************************/

#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <BlynkSimpleEsp32.h>


/***************************************************
 * WIFI CREDENTIALS
 ***************************************************/

char ssid[] = "PICACHU";
char pass[] = "8g06J73$";


/***************************************************
 * WEB API CONFIGURATION
 ***************************************************
 *
 * Windows PC IP:
 *
 * 192.168.1.100
 *
 * Node / Express:
 *
 * Port 3000
 ***************************************************/

const char* API_BASE_URL =
  "http://192.168.137.1:3000";


/***************************************************
 * SOCKET SLUGS
 ***************************************************/

const char* SOCKET1_SLUG = "socket01";
const char* SOCKET2_SLUG = "socket02";


/***************************************************
 * API SETTINGS
 ***************************************************/

/*
 * ESP32 checks backend every 1 second.
 *
 * Web -> Relay response will normally be
 * around 0-1 second depending on network.
 */

const unsigned long API_INTERVAL = 1000;


/*
 * HTTP timeout.
 *
 * Prevents ESP32 from getting stuck for
 * several seconds when server is unavailable.
 */

const int HTTP_CONNECT_TIMEOUT = 500;
const int HTTP_TIMEOUT = 700;


/***************************************************
 * RELAY PINS
 ***************************************************/

#define RELAY1 2
#define RELAY2 18


/***************************************************
 * GLOBAL OBJECTS
 ***************************************************/

WebServer server(80);


/***************************************************
 * GLOBAL VARIABLES
 *************************************************/

bool apMode = false;

int relay1State = 0;
int relay2State = 0;

unsigned long lastApiCheck = 0;


/*
 * Prevents API synchronization from creating
 * unnecessary PATCH requests.
 */

bool updatingFromAPI = false;


/***************************************************
 * FUNCTION DECLARATIONS
 *************************************************/

void setRelay1(int state);
void setRelay2(int state);

bool updateSocketAPI(
  const char* slug,
  int state);

void checkSocketAPI();

void startAP();

void handleRoot();

void r1on();
void r1off();

void r2on();
void r2off();


/***************************************************
 * RELAY 1 CONTROL
 *
 * Relay 1 hardware is inverted:
 *
 * ON  = HIGH
 * OFF = LOW
 ***************************************************/

void setRelay1(int state) {
  relay1State = state ? 1 : 0;

  digitalWrite(
    RELAY1,
    relay1State ? HIGH : LOW);

  Serial.print("Relay 1 -> ");
  Serial.println(
    relay1State ? "ON" : "OFF");
}


/***************************************************
 * RELAY 2 CONTROL
 *
 * Relay 2 hardware is normal:
 *
 * ON  = LOW
 * OFF = HIGH
 ***************************************************/

void setRelay2(int state) {
  relay2State = state ? 1 : 0;

  digitalWrite(
    RELAY2,
    relay2State ? LOW : HIGH);

  Serial.print("Relay 2 -> ");
  Serial.println(
    relay2State ? "ON" : "OFF");
}


/***************************************************
 * BLYNK V0
 *
 * socket01 -> Relay 1
 ***************************************************/

BLYNK_WRITE(V0) {
  int state = param.asInt();

  Serial.println();
  Serial.print("Blynk V0 -> ");
  Serial.println(
    state ? "ON" : "OFF");


  /*
     * Physical relay
     */

  setRelay1(state);


  /*
     * Update backend.
     *
     * If state came from Web API synchronization,
     * do not send PATCH again.
     */

  if (!updatingFromAPI) {
    updateSocketAPI(
      SOCKET1_SLUG,
      state);
  }
}


/***************************************************
 * BLYNK V1
 *
 * socket02 -> Relay 2
 ***************************************************/

BLYNK_WRITE(V1) {
  int state = param.asInt();

  Serial.println();
  Serial.print("Blynk V1 -> ");
  Serial.println(
    state ? "ON" : "OFF");


  /*
     * Physical relay
     */

  setRelay2(state);


  /*
     * Update backend
     */

  if (!updatingFromAPI) {
    updateSocketAPI(
      SOCKET2_SLUG,
      state);
  }
}


/***************************************************
 * UPDATE SOCKET STATUS
 *
 * PATCH:
 *
 * /sockets/socket01/status
 *
 * OR
 *
 * /sockets/socket02/status
 *
 *
 * JSON:
 *
 * {
 *   "isActive": true
 * }
 ***************************************************/

bool updateSocketAPI(
  const char* slug,
  int state) {
  if (
    WiFi.status() != WL_CONNECTED) {
    Serial.println(
      "PATCH skipped - WiFi disconnected");

    return false;
  }


  HTTPClient http;


  /*
     * Build URL using SLUG.
     *
     * No MongoDB _id is used.
     */

  String url =
    String(API_BASE_URL) + "/sockets/" + String(slug) + "/status";


  Serial.println();
  Serial.print("API PATCH -> ");
  Serial.println(url);


  /*
     * Start HTTP connection
     */

  http.begin(url);


  /*
     * Short timeout
     */

  http.setConnectTimeout(
    HTTP_CONNECT_TIMEOUT);

  http.setTimeout(
    HTTP_TIMEOUT);


  /*
     * JSON header
     */

  http.addHeader(
    "Content-Type",
    "application/json");


  /*
     * Create JSON:
     *
     * state = 1
     * {"isActive":true}
     *
     * state = 0
     * {"isActive":false}
     */

  String json =
    String("{\"isActive\":") + (state ? "true" : "false") + "}";


  Serial.print(
    "PATCH JSON -> ");

  Serial.println(json);


  /*
     * Send PATCH
     */

  int httpCode =
    http.PATCH(json);


  Serial.print(
    "PATCH Response -> ");

  Serial.println(httpCode);


  /*
     * Read response
     */

  if (httpCode > 0) {
    String response =
      http.getString();

    Serial.print(
      "PATCH Body -> ");

    Serial.println(response);
  } else {
    Serial.print(
      "PATCH Error -> ");

    Serial.println(
      http.errorToString(httpCode));
  }


  http.end();


  /*
     * 2xx = success
     */

  return (
    httpCode >= 200 && httpCode < 300);
}


/***************************************************
 * CHECK WEB API
 *
 * GET:
 *
 * /sockets
 *
 *
 * Expected response:
 *
 * [
 *   {
 *     "isActive": false,
 *     "slug": "socket01"
 *   },
 *
 *   {
 *     "isActive": false,
 *     "slug": "socket02"
 *   }
 * ]
 ***************************************************/

void checkSocketAPI() {
  if (
    WiFi.status() != WL_CONNECTED) {
    return;
  }


  HTTPClient http;


  String url =
    String(API_BASE_URL) + "/sockets";


  Serial.print(
    "API GET -> ");

  Serial.println(url);


  /*
     * Start connection
     */

  http.begin(url);


  http.setConnectTimeout(
    HTTP_CONNECT_TIMEOUT);

  http.setTimeout(
    HTTP_TIMEOUT);


  http.addHeader(
    "Accept",
    "application/json");


  /*
     * GET request
     */

  int httpCode =
    http.GET();


  /*
     * Connection error
     */

  if (httpCode <= 0) {
    Serial.print(
      "GET API Error -> ");
    Serial.print(
      httpCode);

    Serial.println(
      http.errorToString(httpCode));

    http.end();

    return;
  }


  Serial.print(
    "GET Response -> ");

  Serial.println(httpCode);


  /*
     * Server error
     */

  if (httpCode != 200) {
    Serial.println(
      "GET API returned non-200 response");

    http.end();

    return;
  }


  /*
     * Get JSON response
     */

  String response =
    http.getString();


  Serial.print(
    "API Body -> ");

  Serial.println(response);


  /************************************************
     * SOCKET 01
     ************************************************/

  /*
     * Find socket01
     */

  int socket1Pos =
    response.indexOf(
      "\"slug\":\"socket01\"");


  /*
     * Handle JSON with spaces:
     *
     * "slug": "socket01"
     */

  if (socket1Pos < 0) {
    socket1Pos =
      response.indexOf(
        "\"slug\": \"socket01\"");
  }


  if (socket1Pos >= 0) {
    /*
         * Find isActive AFTER socket01.
         */

    int activePos =
      response.indexOf(
        "\"isActive\":",
        socket1Pos);


    if (activePos < 0) {
      activePos =
        response.indexOf(
          "\"isActive\": ",
          socket1Pos);
    }


    if (activePos >= 0) {
      /*
             * Find value after colon.
             */

      int valueStart =
        response.indexOf(
          ":",
          activePos);


      valueStart++;


      /*
             * Skip spaces.
             */

      while (
        valueStart < response.length() && response[valueStart] == ' ') {
        valueStart++;
      }


      /*
             * Check true/false.
             */

      bool webState =
        response.startsWith(
          "true",
          valueStart);


      /*
             * WEB -> Relay 1 ON
             */

      if (
        webState && relay1State == 0) {
        Serial.println(
          "WEB API -> Relay 1 ON");


        updatingFromAPI = true;


        setRelay1(1);


        /*
                 * Synchronize Blynk switch.
                 */

        if (
          Blynk.connected()) {
          Blynk.virtualWrite(
            V0,
            1);
        }


        updatingFromAPI = false;
      }


      /*
             * WEB -> Relay 1 OFF
             */

      else if (
        !webState && relay1State == 1) {
        Serial.println(
          "WEB API -> Relay 1 OFF");


        updatingFromAPI = true;


        setRelay1(0);


        /*
                 * Synchronize Blynk switch.
                 */

        if (
          Blynk.connected()) {
          Blynk.virtualWrite(
            V0,
            0);
        }


        updatingFromAPI = false;
      }
    }
  }


  /************************************************
     * SOCKET 02
     ************************************************/

  /*
     * Find socket02
     */

  int socket2Pos =
    response.indexOf(
      "\"slug\":\"socket02\"");


  /*
     * Handle spaces
     */

  if (socket2Pos < 0) {
    socket2Pos =
      response.indexOf(
        "\"slug\": \"socket02\"");
  }


  if (socket2Pos >= 0) {
    /*
         * Find isActive AFTER socket02.
         */

    int activePos =
      response.indexOf(
        "\"isActive\":",
        socket2Pos);


    if (activePos < 0) {
      activePos =
        response.indexOf(
          "\"isActive\": ",
          socket2Pos);
    }


    if (activePos >= 0) {
      int valueStart =
        response.indexOf(
          ":",
          activePos);


      valueStart++;


      /*
             * Skip spaces
             */

      while (
        valueStart < response.length() && response[valueStart] == ' ') {
        valueStart++;
      }


      /*
             * Check true/false
             */

      bool webState =
        response.startsWith(
          "true",
          valueStart);


      /*
             * WEB -> Relay 2 ON
             */

      if (
        webState && relay2State == 0) {
        Serial.println(
          "WEB API -> Relay 2 ON");


        updatingFromAPI = true;


        setRelay2(1);


        /*
                 * Synchronize Blynk.
                 */

        if (
          Blynk.connected()) {
          Blynk.virtualWrite(
            V1,
            1);
        }


        updatingFromAPI = false;
      }


      /*
             * WEB -> Relay 2 OFF
             */

      else if (
        !webState && relay2State == 1) {
        Serial.println(
          "WEB API -> Relay 2 OFF");


        updatingFromAPI = true;


        setRelay2(0);


        /*
                 * Synchronize Blynk.
                 */

        if (
          Blynk.connected()) {
          Blynk.virtualWrite(
            V1,
            0);
        }


        updatingFromAPI = false;
      }
    }
  }


  /*
     * Close HTTP connection
     */

  http.end();
}


/***************************************************
 * SOFT AP WEB PAGE
 ***************************************************/

void handleRoot() {
  String html =
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<meta name='viewport' "
    "content='width=device-width,initial-scale=1'>"
    "</head>"

    "<body "
    "style='text-align:center;"
    "font-family:Arial;'>"

    "<h2>Smart Socket</h2>"


    "<h3>Socket Div01</h3>"

    "<a href='/r1on'>"
    "<button "
    "style='padding:15px;margin:5px;'>"
    "ON"
    "</button>"
    "</a>"

    "<a href='/r1off'>"
    "<button "
    "style='padding:15px;margin:5px;'>"
    "OFF"
    "</button>"
    "</a>"


    "<h3>Socket Div02</h3>"

    "<a href='/r2on'>"
    "<button "
    "style='padding:15px;margin:5px;'>"
    "ON"
    "</button>"
    "</a>"

    "<a href='/r2off'>"
    "<button "
    "style='padding:15px;margin:5px;'>"
    "OFF"
    "</button>"
    "</a>"


    "<h3>Status</h3>"

    "<p>Socket Div01: "
    + String(
      relay1State ? "ON" : "OFF")
    + "</p>"

      "<p>Socket Div02: "
    + String(
      relay2State ? "ON" : "OFF")
    + "</p>"

      "</body>"
      "</html>";


  server.send(
    200,
    "text/html",
    html);
}


/***************************************************
 * AP -> RELAY 1 ON
 ***************************************************/

void r1on() {
  setRelay1(1);


  /*
     * Update Blynk
     */

  if (
    Blynk.connected()) {
    Blynk.virtualWrite(
      V0,
      1);
  }


  /*
     * Update backend using slug
     */

  updateSocketAPI(
    SOCKET1_SLUG,
    1);


  server.sendHeader(
    "Location",
    "/");


  server.send(
    303);
}


/***************************************************
 * AP -> RELAY 1 OFF
 ***************************************************/

void r1off() {
  setRelay1(0);


  if (
    Blynk.connected()) {
    Blynk.virtualWrite(
      V0,
      0);
  }


  updateSocketAPI(
    SOCKET1_SLUG,
    0);


  server.sendHeader(
    "Location",
    "/");


  server.send(
    303);
}


/***************************************************
 * AP -> RELAY 2 ON
 ***************************************************/

void r2on() {
  setRelay2(1);


  if (
    Blynk.connected()) {
    Blynk.virtualWrite(
      V1,
      1);
  }


  updateSocketAPI(
    SOCKET2_SLUG,
    1);


  server.sendHeader(
    "Location",
    "/");


  server.send(
    303);
}


/***************************************************
 * AP -> RELAY 2 OFF
 ***************************************************/

void r2off() {
  setRelay2(0);


  if (
    Blynk.connected()) {
    Blynk.virtualWrite(
      V1,
      0);
  }


  updateSocketAPI(
    SOCKET2_SLUG,
    0);


  server.sendHeader(
    "Location",
    "/");


  server.send(
    303);
}


/***************************************************
 * START SOFT AP
 ***************************************************/

void startAP() {
  /*
     * Avoid starting AP multiple times.
     */

  if (apMode) {
    return;
  }


  apMode = true;


  /*
     * AP mode
     */

  WiFi.mode(
    WIFI_AP);


  WiFi.softAP(
    "SmartPlug_AP",
    "12345678");


  Serial.println();
  Serial.println(
    "================================");

  Serial.println(
    "WiFi Failed -> AP MODE");


  Serial.print(
    "AP IP -> ");

  Serial.println(
    WiFi.softAPIP());


  /*
     * AP routes
     */

  server.on(
    "/",
    handleRoot);

  server.on(
    "/r1on",
    r1on);

  server.on(
    "/r1off",
    r1off);

  server.on(
    "/r2on",
    r2on);

  server.on(
    "/r2off",
    r2off);


  server.begin();


  Serial.println(
    "AP Web Server Started");

  Serial.println(
    "================================");
}


/***************************************************
 * SETUP
 ***************************************************/

void setup() {
  Serial.begin(
    115200);


  delay(500);


  Serial.println();
  Serial.println();

  Serial.println(
    "================================");

  Serial.println(
    "SMART SOCKET STARTING");

  Serial.println(
    "================================");


  /************************************************
     * RELAY INITIALIZATION
     ************************************************/

  pinMode(
    RELAY1,
    OUTPUT);

  pinMode(
    RELAY2,
    OUTPUT);


  /*
     * Initial state:
     *
     * Relay 1 OFF -> LOW
     * Relay 2 OFF -> HIGH
     */

  digitalWrite(
    RELAY1,
    LOW);

  digitalWrite(
    RELAY2,
    HIGH);


  relay1State = 0;
  relay2State = 0;


  /************************************************
     * WIFI
     ************************************************/

  WiFi.mode(
    WIFI_STA);


  WiFi.begin(
    ssid,
    pass);


  Serial.print(
    "Connecting to WiFi");


  unsigned long start =
    millis();


  while (
    WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    Serial.print(
      ".");

    delay(300);
  }


  /************************************************
     * WIFI SUCCESS
     ************************************************/

  if (
    WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println(
      "WiFi Connected");


    Serial.print(
      "ESP32 IP -> ");

    Serial.println(
      WiFi.localIP());


    /********************************************
         * BLYNK
         ********************************************/

    Serial.println(
      "Connecting to Blynk...");


    Blynk.config(
      BLYNK_AUTH_TOKEN);


    Blynk.connect(
      3000);


    if (
      Blynk.connected()) {
      Serial.println(
        "Blynk Connected");
    } else {
      Serial.println(
        "Blynk connection failed");
    }


    /********************************************
         * INITIAL WEB API SYNC
         ********************************************/

    Serial.println();
    Serial.println(
      "Checking Web API...");


    checkSocketAPI();


    /********************************************
         * BLYNK INITIAL STATE
         ********************************************/

    if (
      Blynk.connected()) {
      Blynk.virtualWrite(
        V0,
        relay1State);

      Blynk.virtualWrite(
        V1,
        relay2State);
    }


    Serial.println();
    Serial.println(
      "================================");

    Serial.println(
      "SMART SOCKET READY");

    Serial.print(
      "API -> ");

    Serial.println(
      API_BASE_URL);

    Serial.println(
      "================================");
  }


  /************************************************
     * WIFI FAILURE
     ************************************************/

  else {
    startAP();
  }
}


/***************************************************
 * LOOP
 ***************************************************/

void loop() {
  /************************************************
     * AP MODE
     ************************************************/

  if (apMode) {
    server.handleClient();

    return;
  }


  /************************************************
     * BLYNK
     ************************************************/

  Blynk.run();


  /************************************************
     * WIFI STATUS
     ************************************************/

  if (
    WiFi.status() != WL_CONNECTED) {
    Serial.println(
      "WiFi disconnected");


    startAP();

    return;
  }


  /************************************************
     * WEB API CHECK
     *************************************************/

  if (
    millis() - lastApiCheck >= API_INTERVAL) {
    lastApiCheck =
      millis();


    checkSocketAPI();
  }
}
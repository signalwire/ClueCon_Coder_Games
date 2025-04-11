// Coder Games 2025
// Dev: Len Graham
// led_receive_sms.ino
// Send an sms message to turn on an LED. Send an sms to any of your SignalWire phone numbers that has green, red or yellow. This will toggle that color LED on or off.
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "credentials.h"

WebServer server(80);

String filterPhoneNumber = "";
String receivedMessages = "";
String lastMessage = "";
int pageSize = 3; // Default to 3 messages

const int GREEN_LED_PIN = 13;
const int YELLOW_LED_PIN = 14;
const int RED_LED_PIN = 15;
bool greenLedState = false;
bool yellowLedState = false;
bool redLedState = false;

bool filterReceived = true;
bool filterUndelivered = true;

String getHTML() {
  String html = "";
  html.reserve(4000); // Increased size to account for the new table

  html += R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <title>Coder Games - SignalWire Challenge - 2025 ESP32 SMS LED Toggle Monitor</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
    <style>
      body {
        background: radial-gradient(circle at center, #4B0082 0%, #7B68EE 30%, #ADD8E6 70%, #FFFFFF 100%);
        font-family: Arial, sans-serif;
        margin: 0;
        padding: 0;
      }
      .container-fluid { padding: 20px; }
      .row { min-height: calc(100vh - 80px); display: flex; flex-wrap: wrap; }
      .filter-section { min-height: 100%; overflow-y: auto; }
      .messages-section { min-height: 100%; overflow-y: auto; }
      .references-section { margin-top: 20px; }
      .card { background: rgba(255, 255, 255, 0.9); min-height: 100%; }
      .table { margin-bottom: 0; width: 100%; }
      .table th, .table td { overflow: hidden; text-overflow: ellipsis; }
      .table-responsive { overflow-x: auto; max-height: 70vh; }
      .input-group { max-width: 300px; }
      .led-status { display: inline-block; width: 20px; height: 20px; border-radius: 50%; margin-right: 5px; }
      .led-on { background-color: #00ff00; }
      .led-off { background-color: #cccccc; }
      .led-yellow-on { background-color: #ffff00; }
      .led-red-on { background-color: #ff0000; }
      .refresh-btn { margin-top: 10px; display: block; width: 100%; text-align: center; }
      .led-table { margin-top: 10px; }
      .led-table th, .led-table td { padding: 5px; text-align: left; }
      .references-table th, .references-table td { padding: 5px; text-align: left; }
      .text-white { text-shadow: 1px 1px 2px rgba(0, 0, 0, 0.5); }
      @media (min-width: 992px) {
        .row { flex-direction: row !important; }
        .table th, .table td { white-space: nowrap; max-width: 200px; font-size: 1rem; }
        .card-body { padding: 30px; }
        .filter-section { max-width: 350px; }
      }
      @media (min-width: 768px) and (max-width: 991px) {
        .row { flex-direction: row !important; }
        .table th, .table td { white-space: nowrap; max-width: 150px; font-size: 0.9rem; }
        .card-body { padding: 20px; }
      }
      @media (max-width: 767px) {
        .row { flex-direction: column !important; }
        .filter-section, .messages-section { min-height: auto; }
        .table th, .table td { max-width: 100px; font-size: 0.8rem; }
        .card-body { padding: 15px; }
        .table-responsive { max-height: 50vh; }
      }
    </style>
  </head>
  <body>
    <div class="container-fluid">
      <h1 class="text-center text-white mb-4">Coder Games - SignalWire Challenge 2025 - ESP32 SMS LED Toggle Monitor</h1>
      <div class="row">
        <div class="col-lg-3 col-md-4 col-sm-12 filter-section">
          <div class="card">
            <div class="card-body">
              <h3 class="card-title">Filter Messages</h3>
              <form action="/setfilters" method="POST">
                <div class="input-group mb-3">
                  <input type="text" name="phone" class="form-control" placeholder="Filter by recipient (+1234567890)" value=")rawliteral";
  html += filterPhoneNumber;
  html += R"rawliteral(">
                  <button type="submit" class="btn btn-primary">Set Filter</button>
                </div>
                <div class="mb-3">
                  <label>Number of Messages:</label>
                  <input type="number" name="pagesize" class="form-control" value=")rawliteral";
  html += String(pageSize);
  html += R"rawliteral(" min="1" max="50">
                </div>
                <div class="form-check">
                  <input type="checkbox" name="received" class="form-check-input" )rawliteral";
  html += filterReceived ? "checked" : "";
  html += R"rawliteral(>
                  <label class="form-check-label">Received</label>
                </div>
                <div class="form-check">
                  <input type="checkbox" name="undelivered" class="form-check-input" )rawliteral";
  html += filterUndelivered ? "checked" : "";
  html += R"rawliteral(>
                  <label class="form-check-label">Undelivered</label>
                </div>
                <button type="submit" class="btn btn-primary mt-2">Apply Filters</button>
              </form>
              <h3 class="mt-3">LED Status</h3>
              <table class="led-table">
                <thead>
                  <tr>
                    <th>LED</th>
                    <th>Pin</th>
                    <th>Status</th>
                  </tr>
                </thead>
                <tbody>
                  <tr>
                    <td>Green</td>
                    <td>)rawliteral";
  html += String(GREEN_LED_PIN);
  html += R"rawliteral(</td>
                    <td><span class="led-status )rawliteral";
  html += greenLedState ? "led-on" : "led-off";
  html += R"rawliteral("></span>)rawliteral";
  html += greenLedState ? "On" : "Off";
  html += R"rawliteral(</td>
                  </tr>
                  <tr>
                    <td>Yellow</td>
                    <td>)rawliteral";
  html += String(YELLOW_LED_PIN);
  html += R"rawliteral(</td>
                    <td><span class="led-status )rawliteral";
  html += yellowLedState ? "led-yellow-on" : "led-off";
  html += R"rawliteral("></span>)rawliteral";
  html += yellowLedState ? "On" : "Off";
  html += R"rawliteral(</td>
                  </tr>
                  <tr>
                    <td>Red</td>
                    <td>)rawliteral";
  html += String(RED_LED_PIN);
  html += R"rawliteral(</td>
                    <td><span class="led-status )rawliteral";
  html += redLedState ? "led-red-on" : "led-off";
  html += R"rawliteral("></span>)rawliteral";
  html += redLedState ? "On" : "Off";
  html += R"rawliteral(</td>
                  </tr>
                </tbody>
              </table>
              <button onclick="location.reload()" class="btn btn-secondary refresh-btn">Refresh Now</button>
            </div>
          </div>
        </div>
        <div class="col-lg-9 col-md-8 col-sm-12 messages-section">
          <div class="card">
            <div class="card-body">
              <h3 class="card-title">Last )rawliteral";
  html += String(pageSize);
  html += R"rawliteral( Messages</h3>
              <div class="table-responsive">
                <table class="table table-striped">
                  <thead>
                    <tr>
                      <th>SID</th>
                      <th>Direction</th>
                      <th>From</th>
                      <th>To</th>
                      <th>Message</th>
                      <th>Status</th>
                      <th>Date</th>
                    </tr>
                  </thead>
                  <tbody>
                    )rawliteral";
  html += receivedMessages;
  html += R"rawliteral(
                  </tbody>
                </table>
              </div>
            </div>
          </div>
        </div>
      </div>
      <div class="row references-section">
        <div class="col-12">
          <div class="card">
            <div class="card-body">
              <h3 class="card-title">Reference Links</h3>
              <table class="references-table table table-striped">
                <thead>
                  <tr>
                    <th>Resource</th>
                    <th>Link</th>
                  </tr>
                </thead>
                <tbody>
                  <tr>
                    <td>Coder Games</td>
                    <td><a href="https://www.cluecon.com/coder-games" target="_blank">https://www.cluecon.com/coder-games</a></td>
                  </tr>
                  <tr>
                    <td>Message API Info</td>
                    <td><a href="https://developer.signalwire.com/rest/compatibility-api/endpoints/retrieve-message" target="_blank">https://developer.signalwire.com/rest/compatibility-api/endpoints/retrieve-message</a></td>
                  </tr>
                  <tr>
                    <td>Create a SignalWire Account</td>
                    <td><a href="https://developer.signalwire.com/platform/dashboard#get-started" target="_blank">https://developer.signalwire.com/platform/dashboard#get-started</a></td>
                  </tr>
                </tbody>
              </table>
            </div>
          </div>
        </div>
      </div>
    </div>
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js"></script>
    <script>
      document.addEventListener('DOMContentLoaded', function() {
        const dateCells = document.querySelectorAll('td:nth-child(7)');
        dateCells.forEach(cell => {
          const utcDateStr = cell.getAttribute('data-utc');
          if (utcDateStr) {
            const date = new Date(utcDateStr);
            if (!isNaN(date)) {
              const options = {
                weekday: 'short',
                day: '2-digit',
                month: 'short',
                year: 'numeric',
                hour: '2-digit',
                minute: '2-digit',
                hour12: true
              };
              cell.textContent = date.toLocaleString('en-US', options);
            }
          }
        });
      });
    </script>
  </body>
  </html>
  )rawliteral";
  return html;
}

void updateLeds(String message) {
  String msgLower = message;
  msgLower.toLowerCase();
  Serial.println("Processing message: " + msgLower);

  Serial.println("Initial LED States - Green: " + String(greenLedState) + ", Yellow: " + String(yellowLedState) + ", Red: " + String(redLedState));

  if (msgLower.indexOf("green") != -1) {
    greenLedState = !greenLedState;
    Serial.println("Toggling Green LED to: " + String(greenLedState));
  }
  if (msgLower.indexOf("yellow") != -1) {
    yellowLedState = !yellowLedState;
    Serial.println("Toggling Yellow LED to: " + String(yellowLedState));
  }
  if (msgLower.indexOf("red") != -1) {
    redLedState = !redLedState;
    Serial.println("Toggling Red LED to: " + String(redLedState));
  }

  digitalWrite(GREEN_LED_PIN, greenLedState ? HIGH : LOW);
  digitalWrite(YELLOW_LED_PIN, yellowLedState ? HIGH : LOW);
  digitalWrite(RED_LED_PIN, redLedState ? HIGH : LOW);

  Serial.println("Final LED States - Green: " + String(greenLedState) + ", Yellow: " + String(yellowLedState) + ", Red: " + String(redLedState));
}

bool testConnectivity() {
  HTTPClient http;
  String testUrl = "http://www.google.com";
  Serial.println("Testing connectivity with URL: " + testUrl);
  
  http.begin(testUrl);
  int httpCode = http.GET();
  if (httpCode > 0) {
    Serial.println("Connectivity test successful, HTTP Code: " + String(httpCode));
    http.end();
    return true;
  } else {
    Serial.println("Connectivity test failed, HTTP Code: " + String(httpCode));
    http.end();
    return false;
  }
}

void fetchSMSMessages() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, cannot fetch messages");
    receivedMessages = "<tr><td colspan='7'>Error: WiFi not connected</td></tr>";
    return;
  }

  Serial.println("WiFi RSSI before request: " + String(WiFi.RSSI()) + " dBm");

  if (!testConnectivity()) {
    receivedMessages = "<tr><td colspan='7'>Error: Cannot reach external servers (connectivity test failed)</td></tr>";
    return;
  }

  HTTPClient http;
  String url;
  url.reserve(200);
  url = "https://" + String(SIGNALWIRE_SPACE) + ".signalwire.com/api/laml/2010-04-01/Accounts/" + String(SIGNALWIRE_ACCOUNT_SID) + "/Messages.json?PageSize=" + String(pageSize);

  if (filterPhoneNumber != "") {
    String formattedNumber = filterPhoneNumber;
    if (!formattedNumber.startsWith("+")) formattedNumber = "+" + formattedNumber;
    url += "&To=" + formattedNumber;
    Serial.println("Filtering by recipient: " + formattedNumber);
  }

  Serial.println("Requesting URL: " + url);
  
  const int maxRetries = 3;
  int retryCount = 0;
  int httpCode = 0;

  while (retryCount < maxRetries) {
    http.setTimeout(30000);
    http.begin(url);
    http.setAuthorization(SIGNALWIRE_ACCOUNT_SID, SIGNALWIRE_AUTH_TOKEN);
    http.addHeader("Accept", "application/json");
    http.addHeader("Connection", "keep-alive");

    unsigned long startTime = millis();
    httpCode = http.GET();
    unsigned long duration = millis() - startTime;
    Serial.println("HTTP Request Duration: " + String(duration) + " ms");
    Serial.println("HTTP Response Code: " + String(httpCode));

    if (httpCode == HTTP_CODE_OK) {
      break;
    } else {
      Serial.println("Request failed, retrying (" + String(retryCount + 1) + "/" + String(maxRetries) + ")");
      http.end();
      retryCount++;
      delay(1000);
      continue;
    }
  }

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println("Response size: " + String(payload.length()) + " bytes");
    Serial.println("Response: " + payload);
    
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.println("JSON parsing failed: " + String(error.c_str()));
      receivedMessages = "<tr><td colspan='7'>Error parsing JSON response</td></tr>";
      http.end();
      return;
    }
    
    JsonArray messages = doc["messages"];
    
    size_t estimatedSize = messages.size() * 200;
    receivedMessages = "";
    receivedMessages.reserve(estimatedSize);
    
    size_t messageCount = messages.size();
    if (messageCount > pageSize) messageCount = pageSize;
    
    if (messageCount == 0) {
      receivedMessages = "<tr><td colspan='7'>No messages found";
      if (filterPhoneNumber != "") receivedMessages += " sent to " + filterPhoneNumber;
      receivedMessages += "</td></tr>";
    } else {
      JsonObject lastMsg = messages[0];
      lastMessage = lastMsg["body"].as<String>();
      updateLeds(lastMessage);

      for (size_t i = 0; i < messageCount; i++) {
        JsonObject msg = messages[i];
        String status = msg["status"].as<String>();
        
        if ((status == "received" && filterReceived) || (status == "undelivered" && filterUndelivered)) {
          String sid = msg["sid"].as<String>();
          String direction = msg["direction"].as<String>();
          String body = msg["body"].as<String>();
          String from = msg["from"].as<String>();
          String to = msg["to"].as<String>();
          String date = msg["date_created"].as<String>();

          receivedMessages += "<tr>";
          receivedMessages += "<td>" + sid + "</td>";
          receivedMessages += "<td>" + direction + "</td>";
          receivedMessages += "<td>" + from + "</td>";
          receivedMessages += "<td>" + to + "</td>";
          receivedMessages += "<td>" + body + "</td>";
          receivedMessages += "<td>" + status + "</td>";
          receivedMessages += "<td data-utc='" + date + "'>" + date + "</td>";
          receivedMessages += "</tr>";
        }
      }
    }
  } else {
    receivedMessages = "<tr><td colspan='7'>Error fetching messages: HTTP " + String(httpCode) + " (connection refused, check SignalWire credentials or network)</td></tr>";
  }
  http.end();

  Serial.println("Rendering HTML with LED States - Green: " + String(greenLedState) + ", Yellow: " + String(yellowLedState) + ", Red: " + String(redLedState));
}

void handleRoot() {
  fetchSMSMessages();
  server.send(200, "text/html", getHTML());
}

void handleSetFilters() {
  if (server.method() == HTTP_POST) {
    filterPhoneNumber = server.arg("phone");
    pageSize = server.arg("pagesize").toInt();
    if (pageSize < 1) pageSize = 1;
    if (pageSize > 50) pageSize = 50;
    
    filterReceived = server.arg("received") == "on";
    filterUndelivered = server.arg("undelivered") == "on";
    
    Serial.println("Filters set - Phone: " + filterPhoneNumber + ", PageSize: " + String(pageSize) + 
                  ", Received: " + String(filterReceived) + ", Undelivered: " + String(filterUndelivered));
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);

  greenLedState = false;
  yellowLedState = false;
  redLedState = false;
  Serial.println("Initial LED States Reset - Green: " + String(greenLedState) + ", Yellow: " + String(yellowLedState) + ", Red: " + String(redLedState));

  Serial.println("SignalWire Space: " + String(SIGNALWIRE_SPACE));
  Serial.println("SignalWire Account SID: " + String(SIGNALWIRE_ACCOUNT_SID));
  Serial.println("SignalWire Auth Token: " + String(SIGNALWIRE_AUTH_TOKEN));

  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_19_5dBm); // Set to maximum power
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.println("WiFi SSID: " + String(WIFI_SSID));
  Serial.println("Local IP: " + WiFi.localIP().toString());
  Serial.println("WiFi RSSI: " + String(WiFi.RSSI()) + " dBm");

  server.on("/", handleRoot);
  server.on("/setfilters", handleSetFilters);
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, attempting to reconnect...");
    WiFi.reconnect();
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
      delay(500);
      Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nReconnected to WiFi");
      Serial.println("WiFi RSSI: " + String(WiFi.RSSI()) + " dBm");
    } else {
      Serial.println("\nFailed to reconnect to WiFi");
    }
  }
  server.handleClient();
  static unsigned long lastHeapCheck = 0;
  if (millis() - lastHeapCheck > 60000) {
    Serial.println("Free heap: " + String(ESP.getFreeHeap()) + " bytes");
    Serial.println("WiFi RSSI: " + String(WiFi.RSSI()) + " dBm");
    lastHeapCheck = millis();
  }
  delay(100);
}

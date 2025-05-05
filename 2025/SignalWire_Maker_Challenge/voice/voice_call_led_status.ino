// Coder Games 2025
// Dev: Len Graham
// voice_call_led_status.ino
// Call out to a defined e.164 phone number. The led status will change on the call state.
// Call initiated - Yellow LED on.
// Call ringing - Yellow LED blinking.
// Call answered - Green LED remains on (if answered, while on an active call)
// Call completed - Red LED blinks 6 times when call ends.
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "credentials.h"

WebServer server(80);

String targetPhoneNumber = "";
String callStatusMessage = "";
String currentCallSid = "";
bool callInProgress = false;

const int GREEN_LED_PIN = 13;
const int YELLOW_LED_PIN = 14;
const int RED_LED_PIN = 15;
bool greenLedState = false;
bool yellowLedState = false;
bool redLedState = false;
bool yellowBlinking = false;

unsigned long lastBlinkTime = 0;
const unsigned long blinkInterval = 250; // Blink interval in milliseconds
unsigned long callEndTime = 0;
const unsigned long callEndResetDelay = 5000; // 5 seconds delay before resetting LEDs after call ends

// Helper function to URL-encode a string (specifically for phone numbers)
String urlEncodePhoneNumber(String number) {
  String encoded = "";
  encoded.reserve(number.length() + 10);
  for (size_t i = 0; i < number.length(); i++) {
    char c = number[i];
    if (c == '+') {
      encoded += "%2B";
    } else {
      encoded += c;
    }
  }
  return encoded;
}

// Function to blink the Red LED 3 times
void blinkRedLed() {
  const int blinkCount = 3;
  const unsigned long blinkDuration = 250; // 500 ms on, 500 ms off

  for (int i = 0; i < blinkCount; i++) {
    digitalWrite(RED_LED_PIN, HIGH);
    Serial.println("Red LED blink " + String(i + 1) + " - ON");
    delay(blinkDuration);
    digitalWrite(RED_LED_PIN, LOW);
    Serial.println("Red LED blink " + String(i + 1) + " - OFF");
    delay(blinkDuration);
  }
  redLedState = false; // Ensure Red LED is off after blinking
}

String getHTML() {
  String html = "";
  html.reserve(4000);

  html += R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <title>Coder Games - SignalWire Maker Challenge 2025 - ESP32 Voice Call LED Status Monitor</title>
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
      .row { display: flex; flex-wrap: wrap; }
      .call-section { min-height: 100%; overflow-y: auto; }
      .status-section { min-height: 100%; overflow-y: auto; }
      .references-section { margin-top: 20px; }
      .card { background: rgba(255, 255, 255, 0.9); min-height: 100%; }
      .input-group { max-width: 300px; }
      .led-status { display: inline-block; width: 20px; height: 20px; border-radius: 50%; margin-right: 5px; }
      .led-on { background-color: #00ff00; }
      .led-off { background-color: #cccccc; }
      .led-yellow-on { background-color: #ffff00; }
      .led-red-on { background-color: #ff0000; }
      .call-btn { margin-top: 10px; display: block; width: 100%; text-align: center; }
      .led-table { margin-top: 10px; }
      .led-table th, .led-table td { padding: 5px; text-align: left; }
      .references-table th, .references-table td { padding: 5px; text-align: left; }
      .text-white { text-shadow: 1px 1px 2px rgba(0, 0, 0, 0.5); }
      @media (min-width: 992px) {
        .row { flex-direction: row !important; }
        .card-body { padding: 30px; }
        .call-section { max-width: 350px; }
      }
      @media (min-width: 768px) and (max-width: 991px) {
        .row { flex-direction: row !important; }
        .card-body { padding: 20px; }
      }
      @media (max-width: 767px) {
        .row { flex-direction: column !important; }
        .call-section, .status-section { min-height: auto; }
        .card-body { padding: 15px; }
      }
    </style>
  </head>
  <body>
    <div class="container-fluid">
      <h1 class="text-center text-white mb-4">Coder Games - SignalWire Maker Challenge 2025 - ESP32 Voice Call LED Status Monitor</h1>
      <div class="row">
        <div class="col-lg-3 col-md-4 col-sm-12 call-section">
          <div class="card">
            <div class="card-body">
              <h3 class="card-title">Make a Call</h3>
              <form action="/makecall" method="POST">
                <div class="input-group mb-3">
                  <input type="text" name="phone" class="form-control" placeholder="Target Phone Number (+1234567890)" value=")rawliteral";
  html += targetPhoneNumber;
  html += R"rawliteral(">
                  <button type="submit" class="btn btn-primary">Call Now</button>
                </div>
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
                    <td><span id="green-led" class="led-status )rawliteral";
  html += greenLedState ? "led-on" : "led-off";
  html += R"rawliteral("></span><span id="green-led-text">)rawliteral";
  html += greenLedState ? "On" : "Off";
  html += R"rawliteral(</span></td>
                  </tr>
                  <tr>
                    <td>Yellow</td>
                    <td>)rawliteral";
  html += String(YELLOW_LED_PIN);
  html += R"rawliteral(</td>
                    <td><span id="yellow-led" class="led-status )rawliteral";
  html += yellowLedState ? "led-yellow-on" : "led-off";
  html += R"rawliteral("></span><span id="yellow-led-text">)rawliteral";
  html += yellowLedState ? "On" : "Off";
  html += R"rawliteral(</span></td>
                  </tr>
                  <tr>
                    <td>Red</td>
                    <td>)rawliteral";
  html += String(RED_LED_PIN);
  html += R"rawliteral(</td>
                    <td><span id="red-led" class="led-status )rawliteral";
  html += redLedState ? "led-red-on" : "led-off";
  html += R"rawliteral("></span><span id="red-led-text">)rawliteral";
  html += redLedState ? "On" : "Off";
  html += R"rawliteral(</span></td>
                  </tr>
                </tbody>
              </table>
              <button onclick="location.reload()" class="btn btn-secondary call-btn">Refresh</button>
            </div>
          </div>
        </div>
        <div class="col-lg-9 col-md-8 col-sm-12 status-section">
          <div class="card">
            <div class="card-body">
              <h3 class="card-title">Call Status</h3>
              <p id="call-status">)rawliteral";
  html += callStatusMessage;
  html += R"rawliteral(</p>
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
                    <td>Voice API Info</td>
                    <td><a href="https://developer.signalwire.com/rest/compatibility-api/endpoints/create-a-call" target="_blank">https://developer.signalwire.com/rest/compatibility-api/endpoints/create-a-call</a></td>
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
      // Function to fetch and update status
      function updateStatus() {
        fetch('/status')
          .then(response => response.json())
          .then(data => {
            // Update Call Status
            document.getElementById('call-status').innerText = data.callStatus;

            // Update Green LED
            const greenLed = document.getElementById('green-led');
            const greenLedText = document.getElementById('green-led-text');
            if (data.greenLed) {
              greenLed.classList.remove('led-off');
              greenLed.classList.add('led-on');
              greenLedText.innerText = 'On';
            } else {
              greenLed.classList.remove('led-on');
              greenLed.classList.add('led-off');
              greenLedText.innerText = 'Off';
            }

            // Update Yellow LED
            const yellowLed = document.getElementById('yellow-led');
            const yellowLedText = document.getElementById('yellow-led-text');
            if (data.yellowLed) {
              yellowLed.classList.remove('led-off');
              yellowLed.classList.add('led-yellow-on');
              yellowLedText.innerText = 'On';
            } else {
              yellowLed.classList.remove('led-yellow-on');
              yellowLed.classList.add('led-off');
              yellowLedText.innerText = 'Off';
            }

            // Update Red LED
            const redLed = document.getElementById('red-led');
            const redLedText = document.getElementById('red-led-text');
            if (data.redLed) {
              redLed.classList.remove('led-off');
              redLed.classList.add('led-red-on');
              redLedText.innerText = 'On';
            } else {
              redLed.classList.remove('led-red-on');
              redLed.classList.add('led-off');
              redLedText.innerText = 'Off';
            }
          })
          .catch(error => console.error('Error fetching status:', error));
      }

      // Update status every 2 seconds
      setInterval(updateStatus, 500);

      // Initial update on page load
      updateStatus();
    </script>
  </body>
  </html>
  )rawliteral";
  return html;
}

void updateLeds(String callState, bool callFailed = false) {
  Serial.println("Updating LEDs for call state: " + callState);
  Serial.println("Initial LED States - Green: " + String(greenLedState) + ", Yellow: " + String(yellowLedState) + ", Red: " + String(redLedState));

  if (callState == "initiated") {
    greenLedState = false;
    yellowLedState = true;
    yellowBlinking = false;
    redLedState = false;
    Serial.println("Call initiated - Yellow LED on");
  } else if (callState == "ringing") {
    greenLedState = false;
    yellowLedState = true;
    yellowBlinking = true;
    redLedState = false;
    Serial.println("Call ringing - Yellow LED blinking");
  } else if (callState == "answered" || callState == "in-progress") {
    greenLedState = true;
    yellowLedState = false;
    yellowBlinking = false;
    redLedState = false;
    Serial.println("Call " + callState + " - Green LED on");
  } else if (callState == "completed") {
    yellowBlinking = false;
    if (callFailed) {
      greenLedState = false;
      yellowLedState = false;
      redLedState = true;
      Serial.println("Call failed - Red LED on");
      // Blink Red LED 3 times to indicate call failure
      blinkRedLed();
    } else {
      // Call completed successfully (answered)
      yellowLedState = false;
      redLedState = false;
      Serial.println("Call completed - Green LED remains on (if answered)");
      // Blink Red LED 3 times to indicate call end
      blinkRedLed();
    }
    callEndTime = millis(); // Start the reset timer
  }

  // Update LED states immediately (except for blinking, which is handled in loop())
  digitalWrite(GREEN_LED_PIN, greenLedState ? HIGH : LOW);
  digitalWrite(YELLOW_LED_PIN, yellowLedState && !yellowBlinking ? HIGH : LOW);
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

void makeVoiceCall() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, cannot make call");
    callStatusMessage = "Error: WiFi not connected";
    updateLeds("completed", true);
    return;
  }

  Serial.println("WiFi RSSI before request: " + String(WiFi.RSSI()) + " dBm");

  if (!testConnectivity()) {
    callStatusMessage = "Error: Cannot reach external servers (connectivity test failed)";
    updateLeds("completed", true);
    return;
  }

  if (targetPhoneNumber == "") {
    callStatusMessage = "Error: Please enter a target phone number";
    updateLeds("completed", true);
    return;
  }

  // Format the phone number if needed
  String formattedNumber = targetPhoneNumber;
  if (!formattedNumber.startsWith("+")) {
    formattedNumber = "+" + formattedNumber;
  }

  HTTPClient http;
  String url = "https://" + String(SIGNALWIRE_SPACE) + ".signalwire.com/api/laml/2010-04-01/Accounts/" + String(SIGNALWIRE_ACCOUNT_SID) + "/Calls";
  Serial.println("Requesting URL: " + url);

  http.begin(url);
  http.setAuthorization(SIGNALWIRE_ACCOUNT_SID, SIGNALWIRE_AUTH_TOKEN);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Define the From number (you need to set this in credentials.h)
  String fromNumber = String(SIGNALWIRE_FROM_NUMBER);
  if (fromNumber == "") {
    callStatusMessage = "Error: From number not configured in credentials";
    updateLeds("completed", true);
    http.end();
    return;
  }

  // URL-encode the To and From numbers
  String encodedToNumber = urlEncodePhoneNumber(formattedNumber);
  String encodedFromNumber = urlEncodePhoneNumber(fromNumber);

  // Define the URL for call handling
  String callUrl = String(CALL_URL);

  // Use the CALLBACK_URL from credentials.h
  String callbackUrl = String(CALLBACK_URL);
  String postData = "From=" + encodedFromNumber + "&To=" + encodedToNumber + "&Url=" + callUrl +
                    "&StatusCallback=" + callbackUrl +
                    "&StatusCallbackEvent=initiated&StatusCallbackEvent=ringing&StatusCallbackEvent=answered&StatusCallbackEvent=completed" +
                    "&StatusCallbackMethod=POST";
  Serial.println("POST Data: " + postData);

  unsigned long startTime = millis();
  int httpCode = http.POST(postData);
  unsigned long duration = millis() - startTime;
  Serial.println("HTTP Request Duration: " + String(duration) + " ms");
  Serial.println("HTTP Response Code: " + String(httpCode));

  if (httpCode == 201) {
    String payload = http.getString();
    Serial.println("Response: " + payload);

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.println("JSON parsing failed: " + String(error.c_str()));
      callStatusMessage = "Call initiated, but failed to parse response";
      updateLeds("completed", true);
    } else {
      currentCallSid = doc["sid"].as<String>();
      callStatusMessage = "Call initiated. Call SID: " + currentCallSid;
      updateLeds("initiated");
    }
  } else {
    String payload = http.getString();
    Serial.println("Error Response: " + payload);
    callStatusMessage = "Failed to initiate call: HTTP " + String(httpCode);
    updateLeds("completed", true);
  }
  http.end();

  callInProgress = false;
}

void handleRoot() {
  server.send(200, "text/html", getHTML());
}

void handleMakeCall() {
  if (server.method() == HTTP_POST) {
    targetPhoneNumber = server.arg("phone");
    Serial.println("Target phone number set: " + targetPhoneNumber);

    if (callInProgress) {
      callStatusMessage = "Error: A call is already in progress. Please wait.";
    } else {
      callInProgress = true;
      currentCallSid = "";
      makeVoiceCall();
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleCallback() {
  if (server.method() == HTTP_POST) {
    // Log all arguments for debugging
    Serial.println("Received callback with arguments:");
    for (int i = 0; i < server.args(); i++) {
      Serial.println("Arg " + String(i) + ": " + server.argName(i) + " = " + server.arg(i));
    }

    // Extract the required fields directly from the form data
    String callSid = server.arg("CallSid");
    String callStatus = server.arg("CallStatus");
    String errorCode = server.arg("ErrorCode");

    if (callSid == "") {
      Serial.println("No CallSid received in callback");
      server.send(200, "text/plain", "OK");
      return;
    }

    if (callSid != currentCallSid) {
      Serial.println("Callback for unknown Call SID: " + callSid);
      server.send(200, "text/plain", "OK");
      return;
    }

    callStatusMessage = "Call SID: " + callSid + ", Status: " + callStatus;
    if (errorCode != "") {
      callStatusMessage += ", Error: " + errorCode;
    }

    bool callFailed = (errorCode != "" || callStatus == "failed" || callStatus == "busy" || callStatus == "no-answer");
    updateLeds(callStatus, callFailed);
  }
  server.send(200, "text/plain", "OK");
}

void handleStatus() {
  // Create a JSON object with the current status
  DynamicJsonDocument doc(512);
  doc["callStatus"] = callStatusMessage;
  doc["greenLed"] = greenLedState;
  doc["yellowLed"] = yellowLedState;
  doc["redLed"] = redLedState;

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
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
  yellowBlinking = false;
  Serial.println("Initial LED States Reset - Green: " + String(greenLedState) + ", Yellow: " + String(yellowLedState) + ", Red: " + String(redLedState));

  Serial.println("SignalWire Space: " + String(SIGNALWIRE_SPACE));
  Serial.println("SignalWire Account SID: " + String(SIGNALWIRE_ACCOUNT_SID));
  Serial.println("SignalWire Auth Token: " + String(SIGNALWIRE_AUTH_TOKEN));
  Serial.println("Callback URL: " + String(CALLBACK_URL));

  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
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
  server.on("/makecall", handleMakeCall);
  server.on("/callback", handleCallback);
  server.on("/status", handleStatus); // New endpoint for status updates
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

  // Handle Yellow LED blinking for "ringing" state
  if (yellowBlinking) {
    unsigned long currentTime = millis();
    if (currentTime - lastBlinkTime >= blinkInterval) {
      yellowLedState = !yellowLedState;
      digitalWrite(YELLOW_LED_PIN, yellowLedState ? HIGH : LOW);
      lastBlinkTime = currentTime;
    }
  }

  // Reset LEDs after call ends (after a delay)
  if (callEndTime > 0 && millis() - callEndTime >= callEndResetDelay) {
    greenLedState = false;
    yellowLedState = false;
    yellowBlinking = false;
    redLedState = false;
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    callEndTime = 0;
    Serial.println("Resetting LEDs after call end delay");
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

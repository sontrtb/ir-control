#include <EEPROM.h> //Tên wifi và mật khẩu lưu vào ô nhớ 0->96
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WebServer.h>   //Thêm thư viện web server
WebServer webServer(80); // Khởi tạo đối tượng webServer port 80
#include <Ticker.h>
Ticker blinker;
#include "mqtt_handler.h"
#include "led_control.h"
String ssid;
String password;
#define ledPin 2
#define btnPin 0

#define PUSHTIME 5000
unsigned long lastTimePress = millis();

// Tạo biến chứa mã nguồn trang web HTML để hiển thị trên trình duyệt
const char html[] PROGMEM = R"html( 
  <!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>WiFi Configuration</title>
    <style>
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        }
        
        body {
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            background-color: #f5f5f5;
            padding: 20px;
        }
        
        .container {
            background-color: white;
            border-radius: 10px;
            box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
            padding: 30px;
            width: 100%;
            max-width: 400px;
        }
        
        h1 {
            text-align: center;
            color: #333;
            font-size: 24px;
            margin-bottom: 20px;
        }
        
        .status {
            text-align: center;
            padding: 12px;
            margin-bottom: 20px;
            background-color: #e8f4fd;
            border-radius: 6px;
            color: #0066cc;
            font-weight: 500;
        }
        
        .form-group {
            margin-bottom: 20px;
        }
        
        label {
            display: block;
            margin-bottom: 8px;
            font-weight: 600;
            color: #444;
        }
        
        select, input {
            width: 100%;
            padding: 12px;
            border: 1px solid #ddd;
            border-radius: 6px;
            font-size: 16px;
            transition: border-color 0.3s;
        }
        
        select:focus, input:focus {
            outline: none;
            border-color: #0066cc;
            box-shadow: 0 0 0 2px rgba(0, 102, 204, 0.2);
        }
        
        .button-group {
            display: flex;
            gap: 10px;
            margin-top: 10px;
        }
        
        button {
            flex: 1;
            padding: 12px;
            border: none;
            border-radius: 6px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.2s;
        }
        
        button:hover {
            opacity: 0.9;
        }
        
        button:active {
            transform: scale(0.98);
        }
        
        .save-btn {
            background-color: #0066cc;
            color: white;
        }
        
        .restart-btn {
            background-color: #ff6b6b;
            color: white;
        }
        
        @media (max-width: 480px) {
            .container {
                padding: 20px;
            }
            
            h1 {
                font-size: 20px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>WiFi Configuration</h1>
        <div id="info" class="status">Scanning WiFi networks...</div>
        
        <div class="form-group">
            <label for="ssid">WiFi Network:</label>
            <select id="ssid">
                <option value="">Select a WiFi network</option>
            </select>
        </div>
        
        <div class="form-group">
            <label for="password">Password:</label>
            <input id="password" type="text" placeholder="Enter WiFi password">
        </div>
        
        <div class="button-group">
            <button onclick="saveWifi()" class="save-btn">Save</button>
            <button onclick="reStart()" class="restart-btn">Restart</button>
        </div>
    </div>

    <script>
        window.onload = function() {
            scanWifi();
        }
        
        var xhttp = new XMLHttpRequest();
        
        function scanWifi() {
            xhttp.onreadystatechange = function() {
                if (xhttp.readyState == 4 && xhttp.status == 200) {
                    data = xhttp.responseText;
                    document.getElementById("info").innerHTML = "WiFi scan completed!";
                    var obj = JSON.parse(data);
                    var select = document.getElementById("ssid");
                    
                    // Clear existing options except the first one
                    while (select.options.length > 1) {
                        select.remove(1);
                    }
                    
                    // Add new options
                    for (var i = 0; i < obj.length; ++i) {
                        select[select.length] = new Option(obj[i], obj[i]);
                    }
                }
            }
            xhttp.open("GET", "/scanWifi", true);
            xhttp.send();
        }
        
        function saveWifi() {
            var ssid = document.getElementById("ssid").value;
            var pass = document.getElementById("password").value;
            
            if (!ssid || ssid === "Select a WiFi network") {
                alert("Please select a WiFi network");
                return;
            }
            
            document.getElementById("info").innerHTML = "Saving WiFi configuration...";
            
            xhttp.onreadystatechange = function() {
                if (xhttp.readyState == 4 && xhttp.status == 200) {
                    data = xhttp.responseText;
                    alert(data);
                    document.getElementById("info").innerHTML = "Configuration saved!";
                }
            }
            xhttp.open("GET", "/saveWifi?ssid=" + encodeURIComponent(ssid) + "&pass=" + encodeURIComponent(pass), true);
            xhttp.send();
        }
        
        function reStart() {
            document.getElementById("info").innerHTML = "Restarting device...";
            
            xhttp.onreadystatechange = function() {
                if (xhttp.readyState == 4 && xhttp.status == 200) {
                    data = xhttp.responseText;
                    alert(data);
                }
            }
            xhttp.open("GET", "/reStart", true);
            xhttp.send();
        }
    </script>
</body>
</html>
)html";

// Chương trình xử lý sự kiện wifi
void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case SYSTEM_EVENT_STA_GOT_IP: // Gửi thông tin về PC khi kết nối wifi
    Serial.println("Connected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    wifiMode = 1;
    setupMQTT();
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED: // Tự kết nối lại khi mất wifi
    Serial.println("Disconnected from WiFi");
    wifiMode = 2;
    WiFi.begin(ssid, password);
    break;
  default:
    break;
  }
}

void setupWifi()
{
  if (ssid != "")
  {
    Serial.println("Connecting to wifi...!");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    WiFi.onEvent(WiFiEvent); // Đăng ký chương trình bắt sự kiện wifi
  }
  else
  {
    Serial.println("ESP32 wifi network created!");
    WiFi.mode(WIFI_AP);
    uint8_t macAddr[6];
    WiFi.softAPmacAddress(macAddr);
    String ssid_ap = "IR-Controler";
    ssid_ap.toUpperCase();
    WiFi.softAP(ssid_ap.c_str());
    Serial.println("Access point name:" + ssid_ap);
    Serial.println("Web server access address:" + WiFi.softAPIP().toString());
    wifiMode = 0;
  }
}

void setupWebServer()
{
  // Thiết lập xử lý các yêu cầu từ client(trình duyệt web)
  webServer.on("/", []
               {
                 webServer.send(200, "text/html", html); // Gửi nội dung HTML
               });
  webServer.on("/scanWifi", []
               {
    Serial.println("Scanning wifi network...!");
    int wifi_nets = WiFi.scanNetworks(true, true);
    const unsigned long t = millis();
    while(wifi_nets<0 && millis()-t<10000){
      delay(20);
      wifi_nets = WiFi.scanComplete();
    }
    DynamicJsonDocument doc(200);
    for(int i=0; i<wifi_nets; ++i){
      Serial.println(WiFi.SSID(i));
      doc.add(WiFi.SSID(i));
    }
    //["tên wifi1","tên wifi2","tên wifi3",.....]
    String wifiList = "";
    serializeJson(doc, wifiList);
    Serial.println("Wifi list: "+wifiList);
    webServer.send(200,"application/json",wifiList); });
  webServer.on("/saveWifi", []
               {
    String ssid_temp = webServer.arg("ssid");
    String password_temp = webServer.arg("pass");
    Serial.println("SSID:"+ssid_temp);
    Serial.println("PASS:"+password_temp);
    EEPROM.writeString(0,ssid_temp);
    EEPROM.writeString(32,password_temp);
    EEPROM.commit();
    webServer.send(200,"text/plain","Wifi has been saved!"); });
  webServer.on("/reStart", []
               {
    webServer.send(200,"text/plain","Esp32 is restarting!");
    delay(3000);
    ESP.restart(); });
  webServer.begin(); // Khởi chạy dịch vụ web server trên ESP32
}

void checkButton()
{
  if (digitalRead(btnPin) == LOW)
  {
    Serial.println("Press and hold for 5 seconds to reset to default!");
    if (millis() - lastTimePress > PUSHTIME)
    {
      // Chỉ xóa phần WiFi, không xóa toàn bộ EEPROM
      for (int i = 0; i < 64; i++)
      {
        EEPROM.write(i, 0);
      }
      EEPROM.commit();
      Serial.println("WiFi configuration memory erased!");
      delay(2000);
      ESP.restart();
    }
    delay(1000);
  }
  else
  {
    lastTimePress = millis();
  }
}

class Config
{
public:
  void begin()
  {

    pinMode(ledPin, OUTPUT);
    pinMode(btnPin, INPUT_PULLUP);

    // Khởi tạo với kích thước 512
    EEPROM.begin(512);

    char ssid_temp[32], password_temp[64];
    // Đọc từ địa chỉ 0 và 32 như trước
    EEPROM.readString(0, ssid_temp, sizeof(ssid_temp));
    EEPROM.readString(32, password_temp, sizeof(password_temp));

    ssid = String(ssid_temp);
    password = String(password_temp);

    if (ssid != "")
    {
      Serial.println("Wifi name:" + ssid);
      Serial.println("Password:" + password);
    }

    setupWifi(); // Thiết lập wifi

    if (wifiMode == 0)
      setupWebServer(); // Thiết lập webserver
  }
  void run()
  {
    checkButton();
    if (wifiMode == 0)
      webServer.handleClient(); // Lắng nghe yêu cầu từ trình client
  }
} wifiConfig;
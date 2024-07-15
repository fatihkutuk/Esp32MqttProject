#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <PubSubClient.h>
#include <SPIFFS.h>
#include <WiFiClientSecure.h>

// Pin tanımlamaları
const int digitalInput1 = 2;
const int digitalInput2 = 4;
const int digitalOutput = 5; // Dahili LED

WiFiClientSecure espClient;
PubSubClient client(espClient);
AsyncWebServer server(80);
DNSServer dnsServer;

// Ayar değişkenleri
char ssid[32] = "";
char password[64] = "";
char mqttServer[64] = "";
int mqttPort;
char mqttUser[32] = "";
char mqttPass[32] = "";
char controlTopic[64] = "";
char statusTopic[64] = "";
bool publishOnChange;
int publishInterval;
unsigned long lastPublishTime = 0;

// Zamanlama değişkenleri
unsigned long previousMillis = 0;
const long interval = 10000; // 10 saniye

// Fonksiyon prototipleri
void loadSettings();
void saveSettings();
void reconnectMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void handlePublish();
void publishStatus(int digitalInput1State, int digitalInput2State);

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

void setup() {
    Serial.begin(115200);

    if (!SPIFFS.begin(true)) {
        Serial.println("An error has occurred while mounting SPIFFS");
        return;
    }

    loadSettings();

    pinMode(digitalInput1, INPUT);
    pinMode(digitalInput2, INPUT);
    pinMode(digitalOutput, OUTPUT);

    // AP modunda başlat
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    bool apStarted = WiFi.softAP("ESP32-Config", "password123");
    if (apStarted) {
        Serial.println("AP Mode started");
        Serial.println("AP IP address: " + WiFi.softAPIP().toString());
    } else {
        Serial.println("AP Mode start failed!");
    }

    dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
    Serial.println("DNS server started");

    connectToWiFi();

    espClient.setCACert(root_ca);
    client.setServer(mqttServer, mqttPort);
    client.setCallback(mqttCallback);

    // Ana sayfa ve ayar sayfaları için sunucu yolları
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", String(), false);
    });

    server.on("/wifi_settings", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/wifi_settings.html", String(), false);
    });

    server.on("/mqtt_settings", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/mqtt_settings.html", String(), false);
    });


    // Bootstrap ve jQuery dosyaları için sunucu yolları
    server.on("/css/sb-admin-2.min.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/css/sb-admin-2.min.css", "text/css");
    });
    server.on("/css/all.min.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/css/all.min.css", "text/css");
    });
    server.on("/js/sb-admin-2.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/js/sb-admin-2.min.js", "application/javascript");
    });

    server.on("/js/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/js/jquery.min.js", "application/javascript");
    });

    server.on("/js/bootstrap.bundle.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/js/bootstrap.bundle.min.js", "application/javascript");
    });

    server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = scanNetworks();
        request->send(200, "application/json", json);
    });

    server.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request) {
        String s = request->arg("ssid");
        String p = request->arg("password");
        s.toCharArray(ssid, sizeof(ssid));
        p.toCharArray(password, sizeof(password));
        if (strlen(ssid) > 0) {
            saveSettings();
            WiFi.begin(ssid, password);
            int attempts = 0;
            while (WiFi.status() != WL_CONNECTED && attempts < 20) {
                delay(500);
                Serial.print(".");
                attempts++;
            }
            if (WiFi.status() == WL_CONNECTED) {
                request->send(200, "text/plain", "Connected to " + String(ssid));
            } else {
                request->send(200, "text/plain", "Failed to connect to " + String(ssid));
            }
        } else {
            request->send(400, "text/plain", "SSID cannot be empty.");
        }
    });

    server.on("/save_wifi", HTTP_POST, [](AsyncWebServerRequest *request) {
        String s = request->arg("ssid");
        String p = request->arg("password");
        s.toCharArray(ssid, sizeof(ssid));
        p.toCharArray(password, sizeof(password));
        if (strlen(ssid) > 0) {
            saveSettings();
            request->send(200, "text/plain", "WiFi settings saved.");
            request->send(200, "text/html", "<script>alert('Settings saved. Please restart the device.'); window.location.href = '/';</script>");
        } else {
            request->send(400, "text/plain", "SSID cannot be empty.");
        }
    });

    server.on("/save_mqtt", HTTP_POST, [](AsyncWebServerRequest *request) {
        String server = request->arg("mqtt_server");
        String port = request->arg("mqtt_port");
        String user = request->arg("mqtt_user");
        String pass = request->arg("mqtt_pass");
        String control = request->arg("control_topic");
        String status = request->arg("status_topic");

        server.toCharArray(mqttServer, sizeof(mqttServer));
        mqttPort = port.toInt();
        user.toCharArray(mqttUser, sizeof(mqttUser));
        pass.toCharArray(mqttPass, sizeof(mqttPass));
        control.toCharArray(controlTopic, sizeof(controlTopic));
        status.toCharArray(statusTopic, sizeof(statusTopic));

        publishOnChange = request->arg("publish_on_change") == "on";
        publishInterval = request->arg("publish_interval").toInt();

        saveSettings();
        client.setServer(mqttServer, mqttPort);

        request->send(200, "text/plain", "MQTT settings saved.");
        request->send(200, "text/html", "<script>alert('Settings saved. Please restart the device.'); window.location.href = '/';</script>");
    });

    server.on("/get_wifi_settings", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{\"ssid\":\"" + String(ssid) + "\",\"password\":\"" + String(password) + "\"}";
        request->send(200, "application/json", json);
    });

    server.on("/get_mqtt_settings", HTTP_GET, [](AsyncWebServerRequest *request) {
        loadSettings(); // En son ayarları yükle
        String json = "{\"mqtt_server\":\"" + String(mqttServer) + "\",\"mqtt_port\":" + String(mqttPort) + 
                      ",\"mqtt_user\":\"" + String(mqttUser) + "\",\"mqtt_pass\":\"" + String(mqttPass) + 
                      "\",\"control_topic\":\"" + String(controlTopic) + "\",\"status_topic\":\"" + String(statusTopic) + 
                      "\",\"publish_on_change\":" + String(publishOnChange) + 
                      ",\"publish_interval\":" + String(publishInterval) + "}";
        request->send(200, "application/json", json);
    });

    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        String wifiStatus = (WiFi.status() == WL_CONNECTED) ? "Connected to " + String(ssid) : "Not Connected";
        String mqttStatus = client.connected() ? "Connected" : "Not Connected";

        String json = "{\"wifi_status\":\"" + wifiStatus + "\",\"mqtt_status\":\"" + mqttStatus + "\"}";
        request->send(200, "application/json", json);
    });

    server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", "<script>if(confirm('Are you sure you want to restart the device?')){ fetch('/confirm_restart'); }</script>");
    });

    server.on("/confirm_restart", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Restarting device...");
        delay(1000);
        ESP.restart();
    });

    server.begin();
    Serial.println("Web server started.");
}

void loop() {
    dnsServer.processNextRequest();

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        checkWiFiAndReconnect();
    }

    if (WiFi.status() == WL_CONNECTED) {
        if (!client.connected()) {
            reconnectMQTT();
        }
        client.loop();
    }

    handlePublish();
}

void saveSettings() {
    File file = SPIFFS.open("/config.ini", FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open config file for writing");
        return;
    }
    file.printf("ssid=%s\n", ssid);
    file.printf("password=%s\n", password);
    file.printf("mqtt_server=%s\n", mqttServer);
    file.printf("mqtt_port=%d\n", mqttPort);
    file.printf("mqtt_user=%s\n", mqttUser);
    file.printf("mqtt_pass=%s\n", mqttPass);
    file.printf("control_topic=%s\n", controlTopic);
    file.printf("status_topic=%s\n", statusTopic);
    file.printf("publish_on_change=%d\n", publishOnChange);
    file.printf("publish_interval=%d\n", publishInterval);
    file.close();
    Serial.println("Settings saved.");
}

void loadSettings() {
    File file = SPIFFS.open("/config.ini", FILE_READ);
    if (!file) {
        Serial.println("Failed to open config file for reading");
        return;
    }

    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        int separatorIndex = line.indexOf('=');
        if (separatorIndex == -1) continue;

        String key = line.substring(0, separatorIndex);
        String value = line.substring(separatorIndex + 1);

        if (key == "ssid") value.toCharArray(ssid, sizeof(ssid));
        else if (key == "password") value.toCharArray(password, sizeof(password));
        else if (key == "mqtt_server") value.toCharArray(mqttServer, sizeof(mqttServer));
        else if (key == "mqtt_port") mqttPort = value.toInt();
        else if (key == "mqtt_user") value.toCharArray(mqttUser, sizeof(mqttUser));
        else if (key == "mqtt_pass") value.toCharArray(mqttPass, sizeof(mqttPass));
        else if (key == "control_topic") value.toCharArray(controlTopic, sizeof(controlTopic));
        else if (key == "status_topic") value.toCharArray(statusTopic, sizeof(statusTopic));
        else if (key == "publish_on_change") publishOnChange = value.toInt();
        else if (key == "publish_interval") publishInterval = value.toInt();
    }
    file.close();
    Serial.println("Settings loaded:");
    Serial.println("SSID: " + String(ssid));
    Serial.println("Password: " + String(password));
    Serial.println("MQTT Server: " + String(mqttServer));
    Serial.println("MQTT Port: " + String(mqttPort));
    Serial.println("MQTT User: " + String(mqttUser));
    Serial.println("Control Topic: " + String(controlTopic));
    Serial.println("Status Topic: " + String(statusTopic));
    Serial.println("Publish on Change: " + String(publishOnChange));
    Serial.println("Publish Interval: " + String(publishInterval));
}

void reconnectMQTT() {
    while (!client.connected()) {
        Serial.print("Connecting to MQTT server ");
        Serial.print(mqttServer);
        Serial.print("...");
        String clientId = "ESP32Client-";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str(), mqttUser, mqttPass)) {
            Serial.println("connected");
            client.subscribe(controlTopic);
            Serial.print("Subscribed to topic: ");
            Serial.println(controlTopic);
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.print(": ");
            printState(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(message);

    if (String(topic) == controlTopic) {
        if (message == "ON") {
            digitalWrite(digitalOutput, HIGH);
            Serial.println("Output turned ON");
        } else if (message == "OFF") {
            digitalWrite(digitalOutput, LOW);
            Serial.println("Output turned OFF");
        }
    }
}

void handlePublish() {
    static int lastDigitalInput1State = LOW;
    static int lastDigitalInput2State = LOW;
    int currentDigitalInput1State = digitalRead(digitalInput1);
    int currentDigitalInput2State = digitalRead(digitalInput2);
    bool dataChanged = false;

    if (currentDigitalInput1State != lastDigitalInput1State) {
        lastDigitalInput1State = currentDigitalInput1State;
        dataChanged = true;
    }

    if (currentDigitalInput2State != lastDigitalInput2State) {
        lastDigitalInput2State = currentDigitalInput2State;
        dataChanged = true;
    }

    if (publishOnChange && dataChanged) {
        publishStatus(currentDigitalInput1State, currentDigitalInput2State);
    } else if (!publishOnChange && (millis() - lastPublishTime >= publishInterval)) {
        publishStatus(currentDigitalInput1State, currentDigitalInput2State);
        lastPublishTime = millis();
    }
}

void publishStatus(int digitalInput1State, int digitalInput2State) {
    String payload = "{\"digital_input_1\":" + String(digitalInput1State) + ",\"digital_input_2\":" + String(digitalInput2State) + "}";
    client.publish(statusTopic, payload.c_str());
    Serial.println("Status published: " + payload);
}

void printState(int state) {
    switch (state) {
        case -4: Serial.println("MQTT_CONNECTION_TIMEOUT"); break;
        case -3: Serial.println("MQTT_CONNECTION_LOST"); break;
        case -2: Serial.println("MQTT_CONNECT_FAILED"); break;
        case -1: Serial.println("MQTT_DISCONNECTED"); break;
        case 0: Serial.println("MQTT_CONNECTED"); break;
        case 1: Serial.println("MQTT_CONNECT_BAD_PROTOCOL"); break;
        case 2: Serial.println("MQTT_CONNECT_BAD_CLIENT_ID"); break;
        case 3: Serial.println("MQTT_CONNECT_UNAVAILABLE"); break;
        case 4: Serial.println("MQTT_CONNECT_BAD_CREDENTIALS"); break;
        case 5: Serial.println("MQTT_CONNECT_UNAUTHORIZED"); break;
        default: Serial.println("Unknown error"); break;
    }
}

void connectToWiFi() {
    if (strlen(ssid) > 0) {
        WiFi.begin(ssid, password);
        Serial.println("Connecting to WiFi...");
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("Connected to WiFi");
        } else {
            Serial.println("Failed to connect to WiFi");
        }
    }
}

void checkWiFiAndReconnect() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected, attempting to reconnect...");
        connectToWiFi();
    }
}

String scanNetworks() {
    int n = WiFi.scanNetworks();
    String json = "[";
    for (int i = 0; i < n; ++i) {
        if (i) json += ",";
        json += "{";
        json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
        json += "\"rssi\":" + String(WiFi.RSSI(i));
        json += "}";
    }
    json += "]";
    return json;
}

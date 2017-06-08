#include <string>
#include <map>
#include <cstdint>

#include <ArduinoOTA.h>
#include <DHT.h>

#include "../lib/Relay/Relay.h"
#include "../lib/LED/LED.h"
#include "../lib/Button/Button.h"
#include "../lib/PIR/PIR.h"
#include "../lib/DataManager/DataManager.h"
#include "../lib/MqttManager/MqttManager.h"
#include "../lib/WifiManager/WifiManager.h"
#include "../lib/WebServer/WebServer.h"
#include "../lib/UpdateManager/UpdateManager.h"


//#################### FIRMWARE ####################

#define FIRMWARE "pipersense"
#define FIRMWARE_VERSION "0.0.1"

//#################### ======= ####################


//################## HARDWARE ##################

#ifdef ENABLE_NODEMCU
#define HARDWARE "nodemcu"
#define BUTTON_PIN 4
//#define LED_PIN D0
//#define LED_MODE LED_HIGH_LVL
#define RGBLED_RED_PIN D1
#define RGBLED_GREEN_PIN D2
#define RGBLED_BLUE_PIN D3
#define PIR_PIN 5
#define LDR_PIN A0
#define DHT_PIN D7
#endif

//################## ============ ##################


UpdateManager updateManager;
DataManager dataManager;
WifiManager wifiManager;
MqttManager mqttManager;
Relay relay1;
Relay relay2;
Button button;
LED led;
PIR pir(PIR_PIN, 0);
DHT dht(DHT_PIN, DHT22);
SimpleTimer dhtTimer;

std::string wifi_ssid = dataManager.get("wifi_ssid");
std::string wifi_password = dataManager.get("wifi_password");
std::string ip = dataManager.get("ip");
std::string mask = dataManager.get("mask");
std::string gateway = dataManager.get("gateway");
std::string ota_server = dataManager.get("ota_server");
std::string mqtt_server = dataManager.get("mqtt_server");
std::string mqtt_port = dataManager.get("mqtt_port");
std::string mqtt_username = dataManager.get("mqtt_username");
std::string mqtt_password = dataManager.get("mqtt_password");
std::string device_name = dataManager.get("device_name");
std::string mqtt_status_sensors = dataManager.get("mqtt_status_sensors");
std::string mqtt_status_motion = mqtt_status_sensors + "/motion";
std::string mqtt_status_temperature = mqtt_status_sensors + "/temperature";
std::string mqtt_status_humidity = mqtt_status_sensors + "/humidity";


std::vector<std::pair<std::string, std::string>> getWebServerData()
{
    std::vector<std::pair<std::string, std::string>> webServerData;

    std::pair<std::string, std::string> generic_pair;

    generic_pair.first = "wifi_ssid";
    generic_pair.second = wifi_ssid;
    webServerData.push_back(generic_pair);

    generic_pair.first = "wifi_password";
    generic_pair.second = wifi_password;
    webServerData.push_back(generic_pair);

    generic_pair.first = "ip";
    generic_pair.second = ip;
    webServerData.push_back(generic_pair);

    generic_pair.first = "mask";
    generic_pair.second = mask;
    webServerData.push_back(generic_pair);

    generic_pair.first = "gateway";
    generic_pair.second = gateway;
    webServerData.push_back(generic_pair);

    generic_pair.first = "ota_server";
    generic_pair.second = ota_server;
    webServerData.push_back(generic_pair);

    generic_pair.first = "mqtt_server";
    generic_pair.second = mqtt_server;
    webServerData.push_back(generic_pair);

    generic_pair.first = "mqtt_port";
    generic_pair.second = mqtt_port;
    webServerData.push_back(generic_pair);

    generic_pair.first = "mqtt_username";
    generic_pair.second = mqtt_username;
    webServerData.push_back(generic_pair);

    generic_pair.first = "mqtt_password";
    generic_pair.second = mqtt_password;
    webServerData.push_back(generic_pair);

    generic_pair.first = "device_name";
    generic_pair.second = device_name;
    webServerData.push_back(generic_pair);

    generic_pair.first = "mqtt_status_sensors";
    generic_pair.second = mqtt_status_sensors;
    webServerData.push_back(generic_pair);

    generic_pair.first = "firmware_version";
    generic_pair.second = FIRMWARE_VERSION;
    webServerData.push_back(generic_pair);

    generic_pair.first = "hardware";
    generic_pair.second = HARDWARE;
    webServerData.push_back(generic_pair);

    return webServerData;
}

void webServerSubmitCallback(std::map<std::string, std::string> inputFieldsContent)
{
    //Save config to dataManager
    Serial.println("webServerSubmitCallback()");

    dataManager.set("wifi_ssid", inputFieldsContent["wifi_ssid"]);
    dataManager.set("wifi_password", inputFieldsContent["wifi_password"]);
    dataManager.set("ip", inputFieldsContent["ip"]);
    dataManager.set("mask", inputFieldsContent["mask"]);
    dataManager.set("gateway", inputFieldsContent["gateway"]);
    dataManager.set("ota_server", inputFieldsContent["ota_server"]);
    dataManager.set("mqtt_server", inputFieldsContent["mqtt_server"]);
    dataManager.set("mqtt_port", inputFieldsContent["mqtt_port"]);
    dataManager.set("mqtt_username", inputFieldsContent["mqtt_username"]);
    dataManager.set("mqtt_password", inputFieldsContent["mqtt_password"]);
    dataManager.set("device_name", inputFieldsContent["device_name"]);
    dataManager.set("mqtt_port", inputFieldsContent["mqtt_port"]);
    dataManager.set("mqtt_status_sensors", inputFieldsContent["mqtt_status_sensors"]);

    ESP.restart(); // Restart device with new config
}

void MQTTcallback(std::string topicString, std::string payloadString)
{
    Serial.println(topicString.c_str());

//        else
//        {
//            Serial.print("MQTT payload unknown: ");
//            Serial.println(payloadString.c_str());
//        }
//    }
//    else
//    {
//        Serial.print("MQTT topic unknown:");
//    }
}

void shortPress()
{
    Serial.println("button.shortPress()");
}

void longPress()
{
    Serial.println("button.longPress()");
}

void longlongPress()
{
    Serial.println("button.longlongPress()");

    if(wifiManager.apModeEnabled())
    {
        WebServer::getInstance().stop();
        wifiManager.destroyApWifi();

        ESP.restart();
    }
    else
    {
        mqttManager.stopConnection();
        wifiManager.createApWifi();
        WebServer::getInstance().start();
    }
}

void motionDetected()
{
    Serial.println("motionDetected()");
    mqttManager.publishMQTT(mqtt_status_motion, "ON");
}

void motionNotDetected()
{
    Serial.println("motionNotDetected()");
    mqttManager.publishMQTT(mqtt_status_motion, "OFF");
}

void setup()
{
    // Init serial comm
    Serial.begin(115200);

    // Configure Button
    button.setup(BUTTON_PIN, ButtonType::PULLUP_INTERNAL);
    button.setShortPressCallback(shortPress);
    button.setLongPressCallback(longPress);
    button.setLongLongPressCallback(longlongPress);

    #ifdef LED_PIN
        led.setup(LED_PIN, LED_MODE);
        led.on();
        delay(300);
        led.off();
    #endif

    // Configure Wifi
    wifiManager.setup(wifi_ssid, wifi_password, ip, mask, gateway, HARDWARE);
    wifiManager.connectStaWifi();

    // Configure MQTT
    mqttManager.setCallback(MQTTcallback);
    mqttManager.setup(mqtt_server, mqtt_port.c_str(), mqtt_username, mqtt_password);
    mqttManager.setDeviceData(device_name, HARDWARE, ip, FIRMWARE, FIRMWARE_VERSION);
    mqttManager.addStatusTopic(mqtt_status_motion);
    mqttManager.addStatusTopic(mqtt_status_humidity);
    mqttManager.startConnection();

    //Configure WebServer
    WebServer::getInstance().setup("/index.html.gz", webServerSubmitCallback);
    WebServer::getInstance().setData(getWebServerData());

    // OTA setup
    ArduinoOTA.setHostname(device_name.c_str());
    ArduinoOTA.begin();

    // UpdateManager setup
    updateManager.setup(ota_server, FIRMWARE, FIRMWARE_VERSION, HARDWARE);

    //Configure PIR
    pir.setRisingEdgeCallback(motionDetected);
    pir.setFallingEdgeCallback(motionNotDetected);

    //Configure DHT
    dht.begin();
    dhtTimer.setup(RT_ON, 30000);
}

void loop()
{
    // Process Button events
    button.loop();

    // Check Wifi status
    wifiManager.loop();

    // Check PIR sensor
    pir.loop();

    // Check DHT sensor
    if (dhtTimer.check())
    {
        float humidity = dht.readHumidity();
        float temperature = dht.readTemperature();

        mqttManager.publishMQTT(mqtt_status_humidity, humidity);
        mqttManager.publishMQTT(mqtt_status_temperature, temperature);

        dhtTimer.start();
    }

    // Check MQTT status and OTA Updates
    if (wifiManager.connected())
    {
        mqttManager.loop();
        updateManager.loop();
        ArduinoOTA.handle();
    }

    // Handle WebServer connections
    if(wifiManager.apModeEnabled())
    {
        WebServer::getInstance().loop();
    }

    // LED Status
    #ifdef LED_PIN
        if (mqttManager.connected())
        {
            led.on();
        }
        else if(wifiManager.apModeEnabled())
        {
            led.blink(1000);
        }
        else
        {
            led.off();
        }
    #endif
}

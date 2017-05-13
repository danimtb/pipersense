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

#ifdef ENABLE_ELECTRODRAGON
#define HARDWARE "electrodragon"
#define BUTTON_PIN 0
#define RELAY1_PIN 12
#define RELAY2_PIN 13
#define LED_PIN 16
#define LED_MODE LED_HIGH_LVL
#define PIR_PIN 4
#define LDR_PIN adcpin
#define DHT_PIN 14
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
PIR pir(PIR_PIN, 300000);
DHT dht(DHT_PIN, DHT22);


std::string wifi_ssid = dataManager.getWifiSSID();
std::string wifi_password = dataManager.getWifiPass();
std::string ip = dataManager.getIP();
std::string mask = dataManager.getMask();
std::string gateway = dataManager.getGateway();
std::string ota = dataManager.getOta();
std::string mqtt_server = dataManager.getMqttServer();
std::string mqtt_port = dataManager.getMqttPort();
std::string mqtt_username = dataManager.getMqttUser();
std::string mqtt_password = dataManager.getMqttPass();
std::string device_name = dataManager.getDeviceName();
std::string mqtt_status = dataManager.getMqttTopic(0);
std::string mqtt_command = dataManager.getMqttTopic(1);
std::string mqtt_secondary = dataManager.getMqttTopic(2);


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
    generic_pair.second = ota;
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

    generic_pair.first = "mqtt_status";
    generic_pair.second = mqtt_status;
    webServerData.push_back(generic_pair);

    generic_pair.first = "mqtt_command";
    generic_pair.second = mqtt_command;
    webServerData.push_back(generic_pair);

    generic_pair.first = "mqtt_secondary";
    generic_pair.second = mqtt_secondary;
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
    Serial.println("webServerSubmitCallback");

    dataManager.setWifiSSID(inputFieldsContent["wifi_ssid"]);
    dataManager.setWifiPass(inputFieldsContent["wifi_password"]);
    dataManager.setIP(inputFieldsContent["ip"]);
    dataManager.setMask(inputFieldsContent["mask"]);
    dataManager.setGateway(inputFieldsContent["gateway"]);
    dataManager.setOta(inputFieldsContent["ota_server"]);
    dataManager.setMqttServer(inputFieldsContent["mqtt_server"]);
    dataManager.setMqttPort(inputFieldsContent["mqtt_port"]);
    dataManager.setMqttUser(inputFieldsContent["mqtt_username"]);
    dataManager.setMqttPass(inputFieldsContent["mqtt_password"]);
    dataManager.setDeviceName(inputFieldsContent["device_name"]);
    dataManager.setMqttTopic(0, inputFieldsContent["mqtt_status"]);
    dataManager.setMqttTopic(1, inputFieldsContent["mqtt_command"]);
    dataManager.setMqttTopic(2, inputFieldsContent["mqtt_secondary"]);

    ESP.restart(); // Restart device with new config
}

void MQTTcallback(std::string topicString, std::string payloadString)
{
    if (topicString == mqtt_command)
    {
        if (payloadString == "ON")
        {
            Serial.println("ON");
            relay1.on();
            mqttManager.publishMQTT(mqtt_status, "ON");
        }
        else if (payloadString == "OFF")
        {
            Serial.println("OFF");
            relay1.off();
            mqttManager.publishMQTT(mqtt_status, "OFF");
        }
        else if (payloadString == "TOGGLE")
        {
            Serial.println("TOGGLE");
            relay1.commute();
            mqttManager.publishMQTT(mqtt_status, relay1.getState() ? "ON" : "OFF");
        }
        else
        {
            Serial.print("MQTT payload unknown: ");
            Serial.println(payloadString.c_str());
        }
    }
    else
    {
        Serial.print("MQTT topic unknown:");
        Serial.println(topicString.c_str());
    }
}

void shortPress()
{
    Serial.println("button.shortPress()");
    relay1.commute();

    mqttManager.publishMQTT(mqtt_status, relay1.getState() ? "ON" : "OFF");
}

void longPress()
{
    Serial.println("button.longPress()");

    relay2.commute();

    mqttManager.publishMQTT(mqtt_status, relay2.getState() ? "ON" : "OFF");
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



void setup()
{
    // Init serial comm
    Serial.begin(115200);

    // Configure Relays
    relay1.setup(RELAY1_PIN, RELAY_HIGH_LVL);
    relay1.off();
    relay2.setup(RELAY2_PIN, RELAY_HIGH_LVL);
    relay2.off();

    // Configure Button
    button.setup(BUTTON_PIN, PULLDOWN);
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
    mqttManager.setLastWillMQTT(mqtt_status, "OFF");
    mqttManager.setDeviceData(device_name, HARDWARE, ip, FIRMWARE, FIRMWARE_VERSION);
    mqttManager.addStatusTopic(mqtt_status);
    mqttManager.addSubscribeTopic(mqtt_command);
    mqttManager.startConnection();

    //Configure WebServer
    WebServer::getInstance().setup("/index.html.gz", webServerSubmitCallback);
    WebServer::getInstance().setData(getWebServerData());

    // OTA setup
    ArduinoOTA.setHostname(device_name.c_str());
    ArduinoOTA.begin();

    // UpdateManager setup
    updateManager.setup(ota, FIRMWARE, FIRMWARE_VERSION, HARDWARE);
}

void loop()
{
    // Process Button events
    button.loop();

    // Check Wifi status
    wifiManager.loop();

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

#include <map>
#include <cstdint>

#include <ArduinoOTA.h>
#include <DHT.h>
#include <ArduinoJson.h>

#include "../lib/Button/Button.h"
#include "../lib/DataManager/DataManager.h"
#include "../lib/MqttManager/MqttManager.h"
#include "../lib/MqttManager/MqttDiscoveryComponent.h"
#include "../lib/PIR/PIR.h"
#include "../lib/RgbLED/RgbLED.h"
#include "../lib/TEMT6000/TEMT6000.h"
#include "../lib/TimeWatchdog/TimeWatchdog.h"
#include "../lib/UpdateManager/UpdateManager.h"
#include "../lib/WifiManager/WifiManager.h"
#include "../lib/WebServer/WebServer.h"


//#################### FIRMWARE ####################

#define FIRMWARE "pipersense"
#define FIRMWARE_VERSION "0.0.2"

//#################### ======= ####################


//################## HARDWARE ##################

#ifdef ENABLE_NODEMCU
#define HARDWARE "nodemcuv2"
#define BUTTON_PIN 4
#define RGBLED_RED_PIN 14
#define RGBLED_GREEN_PIN 12
#define RGBLED_BLUE_PIN 13
#define PIR_PIN 5
#define LDR_PIN A0
#define DHT_PIN 0
#endif

//################## ============ ##################

MqttDiscoveryComponent discoveryMotion;
MqttDiscoveryComponent discoveryHumidity;
MqttDiscoveryComponent discoveryTemperature;
MqttDiscoveryComponent discoveryIlluminance;
MqttDiscoveryComponent discoveryLed;
Button button;
DataManager dataManager;
DHT dht(DHT_PIN, DHT22);
MqttManager mqttManager;
PIR pir(PIR_PIN, 0);
RgbLED rgbLED;
SimpleTimer dhtTimer;
TEMT6000 temt6000;
TimeWatchdog connectionWatchdog;
UpdateManager updateManager;
WifiManager wifiManager;


String wifi_ssid = dataManager.get("wifi_ssid");
String wifi_password = dataManager.get("wifi_password");
String ip = dataManager.get("ip");
String mask = dataManager.get("mask");
String gateway = dataManager.get("gateway");
String ota_server = dataManager.get("ota_server");

String mqtt_server = dataManager.get("mqtt_server");
String mqtt_port = dataManager.get("mqtt_port");
String mqtt_username = dataManager.get("mqtt_username");
String mqtt_password = dataManager.get("mqtt_password");

String device_name = dataManager.get("device_name");
String discovery_prefix = dataManager.get("discovery_prefix");
String motion_name = dataManager.get("motion_name");
String humidity_name = dataManager.get("humidity_name");
String temperature_name = dataManager.get("temperature_name");
String illuminance_name = dataManager.get("illuminance_name");

String mqtt_status_sensors = dataManager.get("mqtt_status_sensors");
String mqtt_status_motion = mqtt_status_sensors + "/motion";
String mqtt_status_humidity = mqtt_status_sensors + "/humidity";
String mqtt_status_temperature = mqtt_status_sensors + "/temperature";
String mqtt_status_illuminance = mqtt_status_sensors + "/illuminance";

String led_name = dataManager.get("led_name");
String mqtt_status_led = dataManager.get("mqtt_status_led");
String mqtt_command_led = mqtt_status_led + "/set";
String mqtt_button_toggle = dataManager.get("mqtt_button_toggle");


std::vector<std::pair<String, String>> getWebServerData()
{
    std::vector<std::pair<String, String>> webServerData;

    std::pair<String, String> generic_pair;

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

    generic_pair.first = "discovery_prefix";
    generic_pair.second = discovery_prefix;
    webServerData.push_back(generic_pair);

    generic_pair.first = "motion_name";
    generic_pair.second = motion_name;
    webServerData.push_back(generic_pair);

    generic_pair.first = "humidity_name";
    generic_pair.second = humidity_name;
    webServerData.push_back(generic_pair);

    generic_pair.first = "temperature_name";
    generic_pair.second = temperature_name;
    webServerData.push_back(generic_pair);

    generic_pair.first = "illuminance_name";
    generic_pair.second = illuminance_name;
    webServerData.push_back(generic_pair);

    generic_pair.first = "mqtt_status_sensors";
    generic_pair.second = mqtt_status_sensors;
    webServerData.push_back(generic_pair);

    generic_pair.first = "led_name";
    generic_pair.second = led_name;
    webServerData.push_back(generic_pair);

    generic_pair.first = "mqtt_status_led";
    generic_pair.second = mqtt_status_led;
    webServerData.push_back(generic_pair);

    generic_pair.first = "mqtt_button_toggle";
    generic_pair.second = mqtt_button_toggle;
    webServerData.push_back(generic_pair);

    generic_pair.first = "firmware_version";
    generic_pair.second = FIRMWARE_VERSION;
    webServerData.push_back(generic_pair);

    generic_pair.first = "hardware";
    generic_pair.second = HARDWARE;
    webServerData.push_back(generic_pair);

    return webServerData;
}

void webServerSubmitCallback(std::map<String, String> inputFieldsContent)
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
    dataManager.set("discovery_prefix", inputFieldsContent["discovery_prefix"]);
    dataManager.set("motion_name", inputFieldsContent["motion_name"]);
    dataManager.set("humidity_name", inputFieldsContent["humidity_name"]);
    dataManager.set("temperature_name", inputFieldsContent["temperature_name"]);
    dataManager.set("illuminance_name", inputFieldsContent["illuminance_name"]);
    dataManager.set("mqtt_status_sensors", inputFieldsContent["mqtt_status_sensors"]);
    dataManager.set("led_name", inputFieldsContent["led_name"]);
    dataManager.set("mqtt_status_led", inputFieldsContent["mqtt_status_led"]);
    dataManager.set("mqtt_button_toggle", inputFieldsContent["mqtt_button_toggle"]);

    ESP.restart(); // Restart device with new config
}

void publishStateRgbLED(String mqtt_topic)
{
    Serial.println("publishStateRgbLED()");

    StaticJsonBuffer<200> dynamicJsonBuffer;
    JsonObject& root = dynamicJsonBuffer.createObject();
    String jsonString;

    if (rgbLED.getState())
    {
        root["state"] = "ON";

        JsonObject& color = root.createNestedObject("color");
        color["r"] = rgbLED.getColor().red;
        color["g"] = rgbLED.getColor().green;
        color["b"] = rgbLED.getColor().blue;
    }
    else
    {
        root["state"] = "OFF";
    }

    root.printTo(jsonString);
    mqttManager.publishMQTT(mqtt_topic, jsonString);
}

void onLuxChangeCallback(float lux)
{
    Serial.println("onLuxChangeCallback()");
    mqttManager.publishMQTT(mqtt_status_illuminance, lux);
}

void MQTTcallback(String topicString, String payloadString)
{
    Serial.print("MQTTcallback(): ");
    Serial.println(topicString);

    if (topicString == mqtt_command_led)
    {
        if (payloadString == "TOGGLE")
        {
            rgbLED.commute();
        }
        else
        {
            StaticJsonBuffer<200> jsonBuffer;
            JsonObject& root = jsonBuffer.parseObject(payloadString);

            if (root.containsKey("state"))
            {
                String state = root["state"];
                String stateString(state);

                if (stateString == "ON")
                {
                    rgbLED.on();
                }
                else
                {
                    rgbLED.off();
                }
            }

            if (root.containsKey("color"))
            {
                uint8_t red = root["color"]["r"];
                uint8_t green = root["color"]["g"];
                uint8_t blue = root["color"]["b"];

                if (root.containsKey("transition"))
                {
                    uint16_t seconds = root["transition"];
                    rgbLED.setColor(red, green, blue, seconds);
                }
                else
                {
                    rgbLED.setColor(red, green, blue);
                }
            }
        }

        publishStateRgbLED(mqtt_status_led);
    }
    else
    {
        Serial.print("MQTT topic unknown");
    }
}

void shortPress()
{
    Serial.println("button.shortPress()");
    rgbLED.commute();
    publishStateRgbLED(mqtt_status_led);
    publishStateRgbLED(mqtt_command_led);
}

void longPress()
{
    Serial.println("button.longPress()");
    mqttManager.publishMQTT(mqtt_button_toggle, "TOGGLE");
}

void veryLongPress()
{
    //Disconnect and Restart device
    mqttManager.stopConnection();
    wifiManager.disconnectStaWifi();
    ESP.restart();
}

void ultraLongPress()
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

void connectionWatchdogCallback()
{
    ESP.restart();
}

void setup()
{
    // Init serial comm
    Serial.begin(115200);

    // Configure Button
    button.setup(BUTTON_PIN, ButtonType::PULLUP_INTERNAL);
    button.setShortPressCallback(shortPress);
    button.setLongPressCallback(longPress);
    button.setVeryLongPressCallback(veryLongPress);
    button.setUltraLongPressCallback(ultraLongPress);

    // Configure LED
    rgbLED.setup(RGBLED_RED_PIN, RGBLED_GREEN_PIN, RGBLED_BLUE_PIN);

    // Configure TEMT6000
    temt6000.setup(LDR_PIN, 3.3);
    temt6000.setOnChangeCallback(onLuxChangeCallback, 10000);

    // Configure Wifi
    wifiManager.setup(wifi_ssid, wifi_password, ip, mask, gateway, HARDWARE);
    wifiManager.connectStaWifi();

    // Configure MQTT
    mqttManager.setCallback(MQTTcallback);
    mqttManager.setup(mqtt_server, mqtt_port, mqtt_username, mqtt_password);
    mqttManager.setDeviceData(device_name, HARDWARE, ip, FIRMWARE, FIRMWARE_VERSION);

    //Configure WebServer
    WebServer::getInstance().setup("/index.html.gz", webServerSubmitCallback);
    WebServer::getInstance().setData(getWebServerData());

    // OTA setup
    ArduinoOTA.setHostname(device_name.c_str());
    ArduinoOTA.begin();

    // UpdateManager setup
    updateManager.setup(ota_server, FIRMWARE, FIRMWARE_VERSION, HARDWARE);

    // ConnectionWatchdog setup
    connectionWatchdog.setup(1200000, connectionWatchdogCallback); //20 min

    //Configure PIR
    pir.setRisingEdgeCallback(motionDetected);
    pir.setFallingEdgeCallback(motionNotDetected);

    //Configure DHT
    dht.begin();
    dhtTimer.setup(RT_ON, 30000);

    // Configure MQTT Discovery
    discoveryMotion = MqttDiscoveryComponent("binary_sensor", motion_name);
    discoveryMotion.discovery_prefix = discovery_prefix;
    discoveryMotion.setConfigurtionVariable("device_class", "motion");
    discoveryMotion.setConfigurtionVariable("state_topic", mqtt_status_motion);
    discoveryMotion.setConfigurtionVariable("qos", "1");
    mqttManager.addDiscoveryComponent(discoveryMotion);

    discoveryHumidity = MqttDiscoveryComponent("sensor", humidity_name);
    discoveryHumidity.discovery_prefix = discovery_prefix;
    discoveryHumidity.setConfigurtionVariable("state_topic", mqtt_status_humidity);
    discoveryHumidity.setConfigurtionVariable("qos", "1");
    discoveryHumidity.setConfigurtionVariable("unit_of_measurement", "%");
    mqttManager.addDiscoveryComponent(discoveryHumidity);

    discoveryTemperature = MqttDiscoveryComponent("sensor", temperature_name);
    discoveryTemperature.discovery_prefix = discovery_prefix;
    discoveryTemperature.setConfigurtionVariable("state_topic", mqtt_status_temperature);
    discoveryTemperature.setConfigurtionVariable("qos", "1");
    discoveryTemperature.setConfigurtionVariable("unit_of_measurement", "ºC");
    mqttManager.addDiscoveryComponent(discoveryTemperature);

    discoveryIlluminance = MqttDiscoveryComponent("sensor", illuminance_name);
    discoveryIlluminance.discovery_prefix = discovery_prefix;
    discoveryIlluminance.setConfigurtionVariable("state_topic", mqtt_status_illuminance);
    discoveryIlluminance.setConfigurtionVariable("qos", "1");
    discoveryIlluminance.setConfigurtionVariable("unit_of_measurement", "lx");
    mqttManager.addDiscoveryComponent(discoveryIlluminance);

    discoveryLed = MqttDiscoveryComponent("light", led_name);
    discoveryLed.discovery_prefix = discovery_prefix;
    discoveryLed.setConfigurtionVariable("platform", "mqtt_json");
    discoveryLed.setConfigurtionVariable("command_topic", mqtt_command_led);
    discoveryLed.setConfigurtionVariable("state_topic", mqtt_status_led);
    discoveryLed.setConfigurtionVariable("rgb", "true");
    discoveryLed.setConfigurtionVariable("qos", "1");
    discoveryLed.setConfigurtionVariable("retain", "true");
    mqttManager.addDiscoveryComponent(discoveryLed);

    mqttManager.startConnection();
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

    // ConnectionWatchdog
    connectionWatchdog.loop();

    if (mqttManager.connected())
    {
        connectionWatchdog.init();
        connectionWatchdog.feed();
    }
    else if(wifiManager.apModeEnabled())
    {
        connectionWatchdog.deinit();

        rgbLED.setColor(255, 255, 255);
    }

    // LED Status
    rgbLED.loop();

    // TEMT6000 loop
    temt6000.loop();
}

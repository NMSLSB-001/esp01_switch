#include "WiFiManagerFunctions.h"
#include <PubSubClient.h>
#include <Ticker.h>
#include <ArduinoJson.h>

// MQTT服务器地址和端口
const char* mqtt_server = "192.168.11.150";
const int mqtt_port = 1883;

const char* mqtt_username = "vtblive_IoT";
const char* mqtt_password = "vtblive@123";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

const char* initialSSID = "VTBLIVE-ConfigAP";  // 初始WiFi名称
const char* initialPassword = "123456Aa";      // 初始WiFi密码

Ticker ticker;

int count;    // Ticker计数用变量
int pin = 0;  // 设置一个变量代表控制继电器开关的GPIO

void setup() {


  Serial.begin(9600);  // 波特率

  //pinMode(LED_BUILTIN, OUTPUT);  // 这里换成变量
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);  // GPIO0 高电平，断开继电器，作为初始状态

  // Ticker定时对象
  ticker.attach(1, tickerCount);

  // resetWiFiSettings();
  setupWiFi(initialSSID, initialPassword);

  // 等待WiFi连接成功
  while (!isWiFiConnected()) {
    delay(100);
    Serial.println("Waiting for WiFi connection...");
  }

  // 测试
  httpClientRequest();

  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(receiveCallback);

  MQTTConnection();
}

void loop() {
  MQTTKeepConnection();
}

void tickerCount() {
  count++;
}

void MQTTConnection() {

  String clientId = "esp8266-" + WiFi.macAddress();
  // 尝试连接MQTT服务器
  if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
    Serial.println("Connected to MQTT server");
    subscribeTopic();
  } else {
    Serial.println("Failed to connect to MQTT server");
    Serial.println(mqttClient.state());
  }
}

void MQTTKeepConnection() {

  // 保持MQTT连接
  if (mqttClient.connected()) {
    if (count >= 10) {
      publishTopic();
      count = 0;
    }
    mqttClient.loop();
  } else {
    MQTTConnection();
    delay(1000);
  }
}

// 订阅指定主题
void subscribeTopic() {
  // 建立订阅主题
  String topicString = "wireless/control/" + WiFi.macAddress();  // 下行主题，控制端到设备
  char subTopic[topicString.length() + 1];
  strcpy(subTopic, topicString.c_str());

  // 重试订阅直到成功
  while (!mqttClient.subscribe(subTopic)) {
    Serial.println("Subscribe Fail... Retrying");
    delay(1000);  // 等待1秒后重试
  }

  Serial.print("Subscribe Topic: ");
  Serial.println(subTopic);
}

// 发布信息
void publishTopic() {
  // 建立发布主题
  String topicString = "wireless/status/" + WiFi.macAddress();  // 上行主题,设备到控制端

  char publishTopic[topicString.length() + 1];
  strcpy(publishTopic, topicString.c_str());

  // 定时向服务器主题发布当前GPIO0引脚状态
  String state;
  if (digitalRead(0)) {
    state = "{\"state\": false}";  // 高电平断开继电器
  } else {
    state = "{\"state\": true}";  // 低电平吸合继电器
  }

  char publishMsg[state.length() + 1];
  strcpy(publishMsg, state.c_str());

  // 重试发布直到成功
  while (!mqttClient.publish(publishTopic, publishMsg)) {
    Serial.println("Message Publish Failed... Retrying");
    delay(1000);  // 等待1秒后重试
  }

  Serial.print("Publish Topic: ");
  Serial.println(publishTopic);
  Serial.print("Publish message: ");
  Serial.println(publishMsg);
}

// 收到信息后的回调函数
void receiveCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message Received [");
  Serial.print(topic);
  Serial.println("] ");
  // 将获取到的pyte类payload消息转换为String类型的data
  String data = "";
  for (int i = 0; i < length; i++) {
    data = data + (char)payload[i];
  }
  Serial.println(data);

  StaticJsonDocument<200> jsonBuffer;  // 不需要计算大小
  deserializeJson(jsonBuffer, data);
  JsonObject object = jsonBuffer.as<JsonObject>();
  Serial.println(object["switch"].as<String>());

  if (object["switch"].as<String>() == "0") {
    //digitalWrite(LED_BUILTIN, HIGH); // 否则熄灭LED。
    digitalWrite(pin, HIGH);                          // GPIO0 低电平，断开继电器
  } else if (object["switch"].as<String>() == "1") {  // 如果收到的信息为"1"，注意双引号为String，单引号为char
    //digitalWrite(LED_BUILTIN, LOW);  // 则点亮LED。
    digitalWrite(pin, LOW);  // GPIO0 高电平，吸合继电器
  }
}

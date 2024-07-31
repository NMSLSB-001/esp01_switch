#include "WiFiManagerFunctions.h"

WiFiClient wifiClient;
HttpClient httpClient = HttpClient(wifiClient, SERVER, PORT);
WiFiManager wifiManager;

void setupWiFi(const char* initialSSID, const char* initialPassword) {
  // 初始化串口设置
  Serial.begin(9600);

  // 配置WiFiManager的AP IP
  IPAddress apIP(192, 168, 1, 1);      // AP模式下的IP地址
  IPAddress netMsk(255, 255, 255, 0);  // 子网掩码

  // 设置AP模式下的IP地址和子网掩码
  // WiFi.softAPConfig(apIP, apIP, netMsk);
  wifiManager.setAPStaticIPConfig(apIP, apIP, netMsk);

  // 自动连接WiFi，如果失败，启动配置portal
  if (!wifiManager.autoConnect(initialSSID, initialPassword)) {
    Serial.println("Failed to connect to WiFi. Starting AP mode for configuration.");

    // 显示WiFi配置页面
    wifiManager.startConfigPortal(initialSSID, initialPassword);
  }

  Serial.println("Connected to WiFi!");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP()); // 获取AP模式下的IP地址
}

void resetWiFiSettings() {
  wifiManager.resetSettings();
  Serial.println("WiFi settings have been reset.");
}

void httpClientRequest() {
  // 配置请求地址
  Serial.print("URL: ");
  Serial.println(URL);

  // 启动连接并发送HTTP GET请求
  httpClient.get("/");

  // 获取HTTP响应状态码
  int httpCode = httpClient.responseStatusCode();
  Serial.print("Send GET request to URL: ");
  Serial.println(URL);

  // 如果服务器响应HTTP_CODE_OK(200)则从服务器获取响应体信息并通过串口输出
  // 如果服务器不响应HTTP_CODE_OK(200)则将服务器响应状态码通过串口输出
  if (httpCode == 200) {
    // 使用responseBody函数获取服务器响应体内容
    String responsePayload = httpClient.responseBody();
    Serial.println("Server Response Payload: ");
    Serial.println(responsePayload);
  } else {
    Serial.println("Server Response Code: ");
    Serial.println(httpCode);
  }

  // 关闭ESP8266与服务器连接
  httpClient.stop();
}

bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

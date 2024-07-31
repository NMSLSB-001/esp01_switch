#ifndef WIFIMANAGERFUNCTIONS_H
#define WIFIMANAGERFUNCTIONS_H

#include <ESP8266WiFi.h>
#include <ArduinoHttpClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

// 测试HTTP请求用的URL。注意网址前面必须添加"http://"
#define URL "http://www.example.com"
#define SERVER "www.example.com"
#define PORT 80

extern WiFiClient wifiClient;
extern HttpClient httpClient;
extern WiFiManager wifiManager;

void setupWiFi(const char* initialSSID, const char* initialPassword);
void resetWiFiSettings();
void httpClientRequest();
bool isWiFiConnected();

#endif // WIFIMANAGERFUNCTIONS_H

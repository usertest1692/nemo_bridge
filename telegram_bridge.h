#ifndef TELEGRAM_BRIDGE_H
#define TELEGRAM_BRIDGE_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// ==========================================
// CONFIGURATION (Injected via GitHub Secrets)
// ==========================================
#ifndef TG_SSID
  #define TG_SSID "YourWiFi"
#endif

#ifndef TG_PASS
  #define TG_PASS "YourPassword"
#endif

#ifndef TG_TOKEN
  #define TG_TOKEN "YourBotToken"
#endif

#ifndef TG_CHAT_ID
  #define TG_CHAT_ID "YourChatID"
#endif

// ==========================================
// STATE
// ==========================================
WiFiClientSecure tg_client;
unsigned long tg_last_poll = 0;
const int tg_poll_interval = 3000; 
long tg_last_update_id = 0;

// ==========================================
// UTILS
// ==========================================

void tg_send_message(String text) {
  if (WiFi.status() != WL_CONNECTED) return;
  
  HTTPClient http;
  String url = "https://api.telegram.org/bot" + String(TG_TOKEN) + "/sendMessage?chat_id=" + String(TG_CHAT_ID) + "&text=" + text;
  
  http.begin(tg_client, url);
  int httpCode = http.GET();
  http.end();
}

void tg_process_command(String cmd) {
  cmd.trim();
  if (cmd == "/test") {
    tg_send_message("M5Stick Nemo is ONLINE! 🚀");
  } else if (cmd == "/status") {
    String status = "System Status:\n";
    status += "WiFi: Connected\n";
    status += "Uptime: " + String(millis() / 60000) + " min";
    tg_send_message(status);
  } else if (cmd == "/reboot") {
    tg_send_message("Rebooting device...");
    delay(1000);
    ESP.restart();
  }
}

void tg_poll() {
  if (WiFi.status() != WL_CONNECTED) return;
  
  HTTPClient http;
  String url = "https://api.telegram.org/bot" + String(TG_TOKEN) + "/getUpdates?offset=" + String(tg_last_update_id + 1) + "&limit=1";
  
  http.begin(tg_client, url);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    // Minimal parser to find "text":"..." and "update_id":...
    if (payload.indexOf("\"update_id\":") != -1) {
      int idPos = payload.indexOf("\"update_id\":") + 12;
      tg_last_update_id = payload.substring(idPos, payload.indexOf(",", idPos)).toInt();
      
      int textPos = payload.indexOf("\"text\":\"") + 8;
      if (textPos > 8) {
        String msgText = payload.substring(textPos, payload.indexOf("\"", textPos));
        tg_process_command(msgText);
      }
    }
  }
  http.end();
}

// ==========================================
// PUBLIC API
// ==========================================

void telegram_bridge_setup() {
  tg_client.setInsecure(); // Simple SSL
  WiFi.begin(TG_SSID, TG_PASS);
  Serial.println("[TG] Connecting to WiFi...");
}

void telegram_bridge_loop() {
  static bool wasConnected = false;
  
  if (WiFi.status() == WL_CONNECTED) {
    if (!wasConnected) {
      wasConnected = true;
      Serial.println("[TG] WiFi Connected!");
      tg_send_message("Device started and connected to WiFi. Send /test to verify.");
    }
    
    if (millis() - tg_last_poll > tg_poll_interval) {
      tg_last_poll = millis();
      tg_poll();
    }
  } else {
    wasConnected = false;
  }
}

#endif

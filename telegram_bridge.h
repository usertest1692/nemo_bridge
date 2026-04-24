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
  Serial.println("\n--- TELEGRAM BRIDGE STARTUP ---");
  Serial.print("[TG] Target SSID: "); Serial.println(TG_SSID);
  Serial.print("[TG] Bot Token: "); 
  String masked = String(TG_TOKEN).substring(0, 5) + "..." + String(TG_TOKEN).substring(String(TG_TOKEN).length()-5);
  Serial.println(masked);
  
  WiFi.begin(TG_SSID, TG_PASS);
}

void telegram_bridge_loop() {
  static bool wasConnected = false;
  static unsigned long lastDebug = 0;
  
  if (millis() - lastDebug > 5000) {
    lastDebug = millis();
    int status = WiFi.status();
    Serial.print("[TG] WiFi Status: ");
    Serial.print(status);
    switch(status) {
      case WL_CONNECTED: 
        Serial.print(" (CONNECTED) RSSI: "); 
        Serial.println(WiFi.RSSI());
        break;
      case WL_NO_SSID_AVAIL: Serial.println(" (SSID NOT FOUND)"); break;
      case WL_CONNECT_FAILED: Serial.println(" (AUTH FAILED)"); break;
      default: Serial.println(" (Connecting...)"); break;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (!wasConnected) {
      wasConnected = true;
      Serial.print("[TG] SUCCESS! IP: ");
      Serial.println(WiFi.localIP());
      tg_send_message("M5Stick Hybrid is ONLINE! 🚀\nIP: " + WiFi.localIP().toString());
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

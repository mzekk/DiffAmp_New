#pragma once
#include <esp_now.h>
#include <WiFi.h>

class EspNowManager {
public:
  static void begin(const uint8_t* peerMac = nullptr, esp_now_recv_cb_t recvCb = nullptr, esp_now_send_cb_t sendCb = nullptr) {
    if (_active) return;
    if (esp_now_init() == ESP_OK) {
      Serial.println("[ESP-NOW] Initialized");
      if (recvCb) esp_now_register_recv_cb(recvCb);
      if (sendCb) esp_now_register_send_cb(sendCb);
      if (peerMac) {
        esp_now_peer_info_t peerInfo = {};
        memcpy(peerInfo.peer_addr, peerMac, 6);
        peerInfo.channel = 0;  // same channel
        peerInfo.encrypt = false;
        if (!esp_now_is_peer_exist(peerMac)) {
          esp_now_add_peer(&peerInfo);
          Serial.println("[ESP-NOW] Peer added");
        }
      }
      _active = true;
    } else {
      Serial.println("[ESP-NOW] Failed to initialize");
    }
  }

  static void pause() {
    if (!_active) return;
    esp_now_deinit();
    _active = false;
    Serial.println("[ESP-NOW] Paused");
  }

  static void resume() {
    if (_active) return;
    // Resume after short delay (optional for safety)
    begin(_lastPeer, _recvCb, _sendCb);
    Serial.println("[ESP-NOW] Resumed");
  }

  static void configureCallbacks(const uint8_t* peerMac, esp_now_recv_cb_t recvCb, esp_now_send_cb_t sendCb) {
    _lastPeer = peerMac;
    _recvCb = recvCb;
    _sendCb = sendCb;
  }

  static bool isActive() { return _active; }

private:
  static inline bool _active = false;
  static inline const uint8_t* _lastPeer = nullptr;
  static inline esp_now_recv_cb_t _recvCb = nullptr;
  static inline esp_now_send_cb_t _sendCb = nullptr;
};
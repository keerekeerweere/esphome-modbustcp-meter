#pragma once
#include "esphome.h"

#include <map>
#include <vector>
#include <cmath>
#include <cstring>

#include "lwip/sockets.h"
#include "lwip/inet.h"

using esphome::sensor::Sensor;

class Em24ModbusTcpServer : public esphome::Component {
 public:
  Em24ModbusTcpServer(
      Sensor *u1_v,
      Sensor *i1_a,
      Sensor *p_delivered_kw,
      Sensor *p_returned_kw,
      Sensor *e_del_t1_kwh,
      Sensor *e_del_t2_kwh,
      Sensor *e_ret_t1_kwh,
      Sensor *e_ret_t2_kwh)
      : u1_v_(u1_v),
        i1_a_(i1_a),
        p_del_kw_(p_delivered_kw),
        p_ret_kw_(p_returned_kw),
        e_del_t1_kwh_(e_del_t1_kwh),
        e_del_t2_kwh_(e_del_t2_kwh),
        e_ret_t1_kwh_(e_ret_t1_kwh),
        e_ret_t2_kwh_(e_ret_t2_kwh) {}

  void setup() override {
    // Static EM24 identity/config registers (same as your C)
    set_u16_(0x000b, 1650);  // model: EM24DINAV23XE1PFB (your value)
    set_u16_(0xa000, 7);     // application set to H
    set_u16_(0x0302, 0);     // hardware version
    set_u16_(0x0304, 0);     // firmware version
    set_u16_(0x1002, 3);     // phase config: 1P (your simulator uses 3 for 1P)
    set_u16_(0x0032, 0);     // phase sequence L1-L2-L3
    set_u16_(0xa100, 0);     // switch position: unlocked kVARh
    set_u16_(0x0033, 50 * 10); // frequency scaled *10

    // Serial number regs (same as your C: "00..."):
    set_u16_(0x5000, ('0' << 8) | '0');
    set_u16_(0x5001, ('0' << 8) | '0');
    set_u16_(0x5002, ('0' << 8) | '0');
    set_u16_(0x5003, ('0' << 8) | '0');
    set_u16_(0x5004, ('0' << 8) | '0');
    set_u16_(0x5005, ('0' << 8) | '0');
    set_u16_(0x5006, ('0' << 8));  // trailing

    // Initialize dynamic registers to 0 (same addresses as your C)
    set_i32_rev_(0x0000, 0); // u1
    set_i32_rev_(0x0002, 0); // u2
    set_i32_rev_(0x0004, 0); // u3

    set_i32_rev_(0x000c, 0); // i1
    set_i32_rev_(0x000e, 0); // i2
    set_i32_rev_(0x0010, 0); // i3

    set_i32_rev_(0x0012, 0); // p1
    set_i32_rev_(0x0014, 0); // p2
    set_i32_rev_(0x0016, 0); // p3

    set_i32_rev_(0x0040, 0); // e1
    set_i32_rev_(0x0042, 0); // e2
    set_i32_rev_(0x0044, 0); // e3

    set_i32_rev_(0x0028, 0); // p total
    set_i32_rev_(0x0034, 0); // e total
    set_i32_rev_(0x004e, 0); // e total negative

    // Start Modbus TCP server task
    xTaskCreatePinnedToCore(&Em24ModbusTcpServer::task_trampoline_, "em24_mb_tcp",
                            8192, this, 1, nullptr, 0);
  }

  void loop() override {
    // Update dynamic registers periodically from DSMR
    const uint32_t now = millis();
    if (now - last_update_ms_ < 1000)
      return;
    last_update_ms_ = now;

    // Voltage
    float u1 = read_sensor_or_(u1_v_, 230.0f); // fallback 230V if DSMR voltage not available

    // Power: DSMR sensors are typically kW; emulate 1-0:16.7.0 style (net power)
    // Your C used p1 and p_total = p1 (single phase).
    float p_del_kw = read_sensor_or_(p_del_kw_, 0.0f);
    float p_ret_kw = read_sensor_or_(p_ret_kw_, 0.0f);
    float p_net_kw = p_del_kw - p_ret_kw;     // positive: consumption, negative: injection
    float p_net_w = p_net_kw * 1000.0f;

    // Current
    float i1 = read_sensor_or_(i1_a_, NAN);
    if (!std::isfinite(i1) || i1 < 0.0f) {
      // Derive from net power if current not available
      i1 = (u1 > 1.0f) ? (p_net_w / u1) : 0.0f;
    }

    // Energy totals (kWh) -> EM24-like scaling
    // EM24 commonly represents energy in 0.01 kWh units (10 Wh). Your simulator wrote (int32)round(e*0.01),
    // but that depended on the source unit. With DSMR energy in kWh, use *100 to get 0.01 kWh steps.
    float e_del_kwh = read_sensor_or_(e_del_t1_kwh_, 0.0f) + read_sensor_or_(e_del_t2_kwh_, 0.0f);
    float e_ret_kwh = read_sensor_or_(e_ret_t1_kwh_, 0.0f) + read_sensor_or_(e_ret_t2_kwh_, 0.0f);

    // Write registers exactly like your C’s addresses/scaling conventions for voltage/current/power
    set_i32_rev_(0x0000, (int32_t) lroundf(u1 * 10.0f));        // u1 *10
    set_i32_rev_(0x000c, (int32_t) lroundf(i1 * 1000.0f));      // i1 *1000
    set_i32_rev_(0x0012, (int32_t) lroundf(p_net_w * 10.0f));   // p1 *10
    set_i32_rev_(0x0028, (int32_t) lroundf(p_net_w * 10.0f));   // p total *10

    // Energy
    set_i32_rev_(0x0040, (int32_t) lroundf(e_del_kwh * 100.0f)); // e1 in 0.01 kWh
    set_i32_rev_(0x0034, (int32_t) lroundf(e_del_kwh * 100.0f)); // e total
    set_i32_rev_(0x004e, (int32_t) lroundf(e_ret_kwh * 100.0f)); // e total negative

    // Optional: you could also map phase 2/3 as 0 (already initialized)
  }

 private:
  // --- Register storage (sparse) ---
  std::map<uint16_t, uint16_t> holding_;

  void set_u16_(uint16_t reg, uint16_t value) { holding_[reg] = value; }

  // Same as your MODBUS_SET_INT32_TO_INT16_REV:
  // tab[index] = value (low 16), tab[index+1] = value >> 16 (high 16)
  void set_i32_rev_(uint16_t reg, int32_t v) {
    uint32_t uv = (uint32_t) v;
    holding_[reg]     = (uint16_t)(uv & 0xFFFF);
    holding_[reg + 1] = (uint16_t)((uv >> 16) & 0xFFFF);
  }

  uint16_t get_u16_(uint16_t reg) const {
    auto it = holding_.find(reg);
    if (it == holding_.end())
      return 0;
    return it->second;
  }

  static float read_sensor_or_(Sensor *s, float fallback) {
    if (s == nullptr)
      return fallback;
    float v = s->state;
    if (!std::isfinite(v))
      return fallback;
    return v;
  }

  // --- Modbus TCP server (single task) ---
  static void task_trampoline_(void *arg) {
    reinterpret_cast<Em24ModbusTcpServer *>(arg)->server_task_();
  }

  void server_task_() {
    int listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
      ESP_LOGE("em24_mb", "socket() failed");
      vTaskDelete(nullptr);
      return;
    }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(502);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd, (sockaddr *) &addr, sizeof(addr)) < 0) {
      ESP_LOGE("em24_mb", "bind() failed (port 502). Try running as root equivalent is not applicable; ensure no other service uses 502.");
      ::close(listen_fd);
      vTaskDelete(nullptr);
      return;
    }

    if (listen(listen_fd, 4) < 0) {
      ESP_LOGE("em24_mb", "listen() failed");
      ::close(listen_fd);
      vTaskDelete(nullptr);
      return;
    }

    ESP_LOGI("em24_mb", "Modbus-TCP slave listening on port 502");

    while (true) {
      sockaddr_in client{};
      socklen_t clen = sizeof(client);
      int cfd = accept(listen_fd, (sockaddr *) &client, &clen);
      if (cfd < 0) {
        vTaskDelay(pdMS_TO_TICKS(50));
        continue;
      }
      handle_client_(cfd);
      ::close(cfd);
    }
  }

  // Minimal Modbus TCP framing:
  // MBAP: TID(2) PID(2) LEN(2) UID(1) then PDU
  void handle_client_(int fd) {
    uint8_t buf[260];

    while (true) {
      int r = recv(fd, buf, 7, MSG_WAITALL);
      if (r != 7)
        return;

      uint16_t tid = (buf[0] << 8) | buf[1];
      uint16_t pid = (buf[2] << 8) | buf[3];
      uint16_t len = (buf[4] << 8) | buf[5];
      uint8_t uid = buf[6];

      if (pid != 0 || len < 2 || len > 253) {
        return;
      }

      int pdu_len = (int)len - 1; // minus UID
      r = recv(fd, buf + 7, pdu_len, MSG_WAITALL);
      if (r != pdu_len)
        return;

      const uint8_t fc = buf[7];

      if (fc == 0x03 || fc == 0x04) {
        if (pdu_len < 5) {
          send_exception_(fd, tid, uid, fc, 0x03); // illegal data value
          continue;
        }

        uint16_t start = (buf[8] << 8) | buf[9];
        uint16_t count = (buf[10] << 8) | buf[11];
        if (count < 1 || count > 125) {
          send_exception_(fd, tid, uid, fc, 0x03);
          continue;
        }

        // Build response: MBAP + FC + bytecount + data
        std::vector<uint8_t> out;
        out.resize(7 + 2 + (count * 2));

        // MBAP
        out[0] = (tid >> 8) & 0xFF; out[1] = tid & 0xFF;
        out[2] = 0; out[3] = 0; // pid
        uint16_t out_len = 1 /*uid*/ + 1 /*fc*/ + 1 /*bc*/ + (count * 2);
        out[4] = (out_len >> 8) & 0xFF; out[5] = out_len & 0xFF;
        out[6] = uid;

        // PDU
        out[7] = fc;
        out[8] = (uint8_t)(count * 2);

        // Data: Modbus uses big-endian per register
        for (uint16_t i = 0; i < count; i++) {
          uint16_t v = get_u16_(start + i);
          out[9 + i * 2] = (v >> 8) & 0xFF;
          out[10 + i * 2] = v & 0xFF;
        }

        send_all_(fd, out.data(), out.size());
      } else {
        // Unsupported function
        send_exception_(fd, tid, uid, fc, 0x01); // illegal function
      }
    }
  }

  void send_exception_(int fd, uint16_t tid, uint8_t uid, uint8_t fc, uint8_t ex) {
    uint8_t out[9];
    out[0] = (tid >> 8) & 0xFF; out[1] = tid & 0xFF;
    out[2] = 0; out[3] = 0;
    out[4] = 0; out[5] = 3; // len = uid + fc + ex
    out[6] = uid;
    out[7] = fc | 0x80;
    out[8] = ex;
    send_all_(fd, out, sizeof(out));
  }

  static bool send_all_(int fd, const uint8_t *data, size_t len) {
    size_t off = 0;
    while (off < len) {
      int w = send(fd, data + off, len - off, 0);
      if (w <= 0)
        return false;
      off += (size_t)w;
    }
    return true;
  }

  uint32_t last_update_ms_{0};

  Sensor *u1_v_{nullptr};
  Sensor *i1_a_{nullptr};
  Sensor *p_del_kw_{nullptr};
  Sensor *p_ret_kw_{nullptr};
  Sensor *e_del_t1_kwh_{nullptr};
  Sensor *e_del_t2_kwh_{nullptr};
  Sensor *e_ret_t1_kwh_{nullptr};
  Sensor *e_ret_t2_kwh_{nullptr};
};

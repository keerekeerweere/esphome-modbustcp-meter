#pragma once

#include "esphome.h"

#include <map>
#include <string>
#include <vector>

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
      Sensor *e_ret_t2_kwh);

  void setup() override;
  void loop() override;

  void set_port(uint16_t port) { port_ = port; }
  void set_unit_id(uint8_t unit_id) { unit_id_ = unit_id; }
  void set_phase_config(uint8_t phase_config) { phase_config_ = phase_config; }
  void set_serial(const std::string &serial) { serial_ = serial; }

 private:
  // --- Register storage (sparse) ---
  std::map<uint16_t, uint16_t> holding_;

  void set_u16_(uint16_t reg, uint16_t value);
  void set_i32_rev_(uint16_t reg, int32_t v);
  uint16_t get_u16_(uint16_t reg) const;
  static float read_sensor_or_(Sensor *s, float fallback);

  // --- Modbus TCP server (single task) ---
  static void task_trampoline_(void *arg);
  void server_task_();
  void handle_client_(int fd);
  void send_exception_(int fd, uint16_t tid, uint8_t uid, uint8_t fc, uint8_t ex);
  static bool send_all_(int fd, const uint8_t *data, size_t len);

  void set_serial_regs_();

  uint32_t last_update_ms_{0};
  uint16_t port_{502};
  uint8_t unit_id_{1};
  uint8_t phase_config_{1};
  std::string serial_{"00000000000000"};

  Sensor *u1_v_{nullptr};
  Sensor *i1_a_{nullptr};
  Sensor *p_del_kw_{nullptr};
  Sensor *p_ret_kw_{nullptr};
  Sensor *e_del_t1_kwh_{nullptr};
  Sensor *e_del_t2_kwh_{nullptr};
  Sensor *e_ret_t1_kwh_{nullptr};
  Sensor *e_ret_t2_kwh_{nullptr};
};

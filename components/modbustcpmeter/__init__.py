import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PORT
from esphome.components import sensor

CODEOWNERS = ["@keerekeerweere"]
DEPENDENCIES = ["sensor"]

Em24ModbusTcpServer = cg.global_ns.class_("Em24ModbusTcpServer", cg.Component)

CONF_UNIT_ID = "unit_id"
CONF_PHASE_CONFIG = "phase_config"
CONF_SERIAL = "serial"
CONF_VOLTAGE_SENSOR = "voltage_sensor"
CONF_CURRENT_SENSOR = "current_sensor"
CONF_POWER_DELIVERED_SENSOR = "power_delivered_sensor"
CONF_POWER_RETURNED_SENSOR = "power_returned_sensor"
CONF_POWER_DELIVERED_L1_SENSOR = "power_delivered_l1_sensor"
CONF_POWER_RETURNED_L1_SENSOR = "power_returned_l1_sensor"
CONF_POWER_DELIVERED_L2_SENSOR = "power_delivered_l2_sensor"
CONF_POWER_RETURNED_L2_SENSOR = "power_returned_l2_sensor"
CONF_POWER_DELIVERED_L3_SENSOR = "power_delivered_l3_sensor"
CONF_POWER_RETURNED_L3_SENSOR = "power_returned_l3_sensor"
CONF_ENERGY_DELIVERED_T1_SENSOR = "energy_delivered_t1_sensor"
CONF_ENERGY_DELIVERED_T2_SENSOR = "energy_delivered_t2_sensor"
CONF_ENERGY_RETURNED_T1_SENSOR = "energy_returned_t1_sensor"
CONF_ENERGY_RETURNED_T2_SENSOR = "energy_returned_t2_sensor"
CONF_VOLTAGE_L2_SENSOR = "voltage_l2_sensor"
CONF_VOLTAGE_L3_SENSOR = "voltage_l3_sensor"
CONF_CURRENT_L2_SENSOR = "current_l2_sensor"
CONF_CURRENT_L3_SENSOR = "current_l3_sensor"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(Em24ModbusTcpServer),
        cv.Optional(CONF_PORT, default=502): cv.port,
        cv.Optional(CONF_UNIT_ID, default=1): cv.int_range(min=0, max=255),
        cv.Optional(CONF_PHASE_CONFIG, default=1): cv.one_of(1, 3, int=True),
        cv.Optional(CONF_SERIAL, default="00000000000000"): cv.string_strict,
        cv.Optional(CONF_VOLTAGE_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_VOLTAGE_L2_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_VOLTAGE_L3_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_CURRENT_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_CURRENT_L2_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_CURRENT_L3_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_POWER_DELIVERED_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_POWER_RETURNED_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_POWER_DELIVERED_L1_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_POWER_RETURNED_L1_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_POWER_DELIVERED_L2_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_POWER_RETURNED_L2_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_POWER_DELIVERED_L3_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_POWER_RETURNED_L3_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_ENERGY_DELIVERED_T1_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_ENERGY_DELIVERED_T2_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_ENERGY_RETURNED_T1_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_ENERGY_RETURNED_T2_SENSOR): cv.use_id(sensor.Sensor),
    }
).extend(cv.COMPONENT_SCHEMA)


def to_code(config):
    cg.add_global(cg.RawExpression('#include "modbustcpmeter.h"'))

    def _sensor_or_null(key):
        if key in config:
            return (yield cg.get_variable(config[key]))
        return cg.nullptr

    u_l1_v = yield from _sensor_or_null(CONF_VOLTAGE_SENSOR)
    u_l2_v = yield from _sensor_or_null(CONF_VOLTAGE_L2_SENSOR)
    u_l3_v = yield from _sensor_or_null(CONF_VOLTAGE_L3_SENSOR)
    i_l1_a = yield from _sensor_or_null(CONF_CURRENT_SENSOR)
    i_l2_a = yield from _sensor_or_null(CONF_CURRENT_L2_SENSOR)
    i_l3_a = yield from _sensor_or_null(CONF_CURRENT_L3_SENSOR)
    p_total_del = yield from _sensor_or_null(CONF_POWER_DELIVERED_SENSOR)
    p_total_ret = yield from _sensor_or_null(CONF_POWER_RETURNED_SENSOR)
    p_del_l1 = yield from _sensor_or_null(CONF_POWER_DELIVERED_L1_SENSOR)
    p_ret_l1 = yield from _sensor_or_null(CONF_POWER_RETURNED_L1_SENSOR)
    p_del_l2 = yield from _sensor_or_null(CONF_POWER_DELIVERED_L2_SENSOR)
    p_ret_l2 = yield from _sensor_or_null(CONF_POWER_RETURNED_L2_SENSOR)
    p_del_l3 = yield from _sensor_or_null(CONF_POWER_DELIVERED_L3_SENSOR)
    p_ret_l3 = yield from _sensor_or_null(CONF_POWER_RETURNED_L3_SENSOR)
    e_del_t1 = yield from _sensor_or_null(CONF_ENERGY_DELIVERED_T1_SENSOR)
    e_del_t2 = yield from _sensor_or_null(CONF_ENERGY_DELIVERED_T2_SENSOR)
    e_ret_t1 = yield from _sensor_or_null(CONF_ENERGY_RETURNED_T1_SENSOR)
    e_ret_t2 = yield from _sensor_or_null(CONF_ENERGY_RETURNED_T2_SENSOR)

    var = cg.new_Pvariable(
        config[CONF_ID],
        u_l1_v,
        i_l1_a,
        u_l2_v,
        i_l2_a,
        u_l3_v,
        i_l3_a,
        p_total_del,
        p_total_ret,
        p_del_l1,
        p_ret_l1,
        p_del_l2,
        p_ret_l2,
        p_del_l3,
        p_ret_l3,
        e_del_t1,
        e_del_t2,
        e_ret_t1,
        e_ret_t2,
    )
    yield cg.register_component(var, config)

    cg.add(var.set_port(config[CONF_PORT]))
    cg.add(var.set_unit_id(config[CONF_UNIT_ID]))
    cg.add(var.set_phase_config(config[CONF_PHASE_CONFIG]))
    cg.add(var.set_serial(config[CONF_SERIAL]))

// deviceToCloud.cc ---
//
// Author: Majdi Toumi
// Created: Mon Jun 26 10:58:58 2017 (+0100)
// Last-Updated: Wed Jun 28 13:37:00 2017 (+0100)
//           By: Majdi Toumi
// Version: 1.0.0
//
// THE AIEOWTEA-WARE LICENSE
// Majdi Toumi wrote this file
// As long you retain this notice, you can do whatever
// you want with this stuff. If we meet some day, and you think
// this stuff is worth it, you can buy me a cup of tea in return.
//
// Let's Rock!
//

#include <functional>

#include "legato.h"
#include "interfaces.h"
#include "deviceToCloud.hh"

// Null, because instance will be initialized on demand.
DeviceToCloud* DeviceToCloud::_instance = NULL;

DeviceToCloud* DeviceToCloud::getInstance()
{
  if (DeviceToCloud::_instance == NULL) {
    DeviceToCloud::_instance = new DeviceToCloud();
  }
  // just return the single class instance
  return DeviceToCloud::_instance;
}

DeviceToCloud::DeviceToCloud() : _is_mqtt_connected(false)
{
  // blocks SIGTERM in the calling thread
  le_sig_Block(SIGTERM);

  // sets SIGTERM event handler for the calling thread.
  le_sig_SetEventHandler(SIGTERM, DeviceToCloud::appTerminationCallback);

  // register appCleanup as the method to be called on exit
  ::atexit(DeviceToCloud::appCleanUp);
}

void DeviceToCloud::configureMQTT()
{
  // connect to external services
  le_data_ConnectService();
  LE_INFO("AV CONNECTED");

  // connect to mqtt service
  mqtt_ConnectService();
  LE_INFO("MQTT CONNECTED!");

  // an experiment to see if we can close mqtt if has been left connected
  if(mqtt_GetConnectionState()) {
    DeviceToCloud::appCleanUp();
  }

  // lambda for mqtt session
  auto mqttSessionCallback = [](bool is_connected, int32_t connect_err_code, int32_t sub_err_code, void* ctxt_ptr) {
    if(is_connected) {
      DeviceToCloud::getInstance()->setMqttSession(true);
    }
    else {
      LE_INFO("MQTT disconnected - error:%d and sub erorr: %d", connect_err_code, sub_err_code);
    }
  };

  // setting up the mqtt service
  mqtt_AddSessionStateHandler(mqttSessionCallback, NULL);

  // configure mqtt session with AirVantage informations
  mqtt_Config(MQTT_BROKER_URL, MQTT_PORT_NUMBER, MQTT_KEEP_ALIVE, MQTT_QOS);
}

void DeviceToCloud::start()
{
  this->configureMQTT();

  // calling lsm6ds3 configuration section
  _lsm6ds3.begin();

  // create timer with an interval of every 10 seconds, repeating forever (0)
  le_clk_Time_t repeat_interval;

  repeat_interval.sec  = 10;
  repeat_interval.usec = 0;

  // create our timer object
  _timer_ref = le_timer_Create("mainLoopTimer");

  // lambda for our loop acquisition
  auto callback = [](le_timer* timer) {
    DeviceToCloud* s = DeviceToCloud::getInstance();

    // data acquisition
    s->fetch();
    s->dump();

    // -->> ☁
    s->send();
  };

  // about timer configuration
  le_timer_SetHandler(_timer_ref, callback);
  le_timer_SetInterval(_timer_ref, repeat_interval);
  le_timer_SetRepeat(_timer_ref, 0);
  le_timer_Start(_timer_ref);

  return;
}

void DeviceToCloud::setMqttSession(bool v)
{
  _is_mqtt_connected = v;
}

void DeviceToCloud::fetch()
{
  // get last temperature from LSM6DS3
  _last_temperature = _lsm6ds3.readTemperatureByUnity('C');

  // same for X->Z gyroscope axis
  _last_gyroscope.Ax = _lsm6ds3.readAccelerationByAxis('X');
  _last_gyroscope.Ay = _lsm6ds3.readAccelerationByAxis('Y');
  _last_gyroscope.Az = _lsm6ds3.readAccelerationByAxis('Z');
}

void DeviceToCloud::dump()
{
  LE_INFO
    ("DUMP: %f %f %f %f",
     this->_last_temperature,
     this->_last_gyroscope.Ax,
     this->_last_gyroscope.Ay,
     this->_last_gyroscope.Az);
}

void DeviceToCloud::send()
{
  // reference to the modem data connection profile (here: default profile definition)
  le_mdc_ProfileRef_t profile = le_mdc_GetProfile(LE_MDC_DEFAULT_PROFILE);

  // connection state value
  le_mdc_ConState_t mdc_state;

  char json_payload[2048];
  int32_t json_err_code;

  // get WAN connected info
  le_mdc_GetSessionState(profile, &mdc_state);

  // check that we have WAN
  if(mdc_state == LE_MDC_DISCONNECTED) {
    le_data_Request();
    LE_INFO("WAN CONNECTED");
  }
  else if(!_is_mqtt_connected) {
    // check that mqttService is up
    mqtt_Connect(MQTT_BROKER_PASSWORD);
    LE_INFO("MQTT CONNECTED");
  }
  else {
    // format our data into json
    sprintf
      (json_payload,
       "{"
       "\"sensor.temperature\":%f,"
       "\"sensor.ax\":%f,"
       "\"sensor.ay\":%f,"
       "\"sensor.az\":%f"
       "}",
       _last_temperature,
       _last_gyroscope.Ax,
       _last_gyroscope.Ay,
       _last_gyroscope.Az);

    // send playload to AirVantage
    mqtt_SendJson(json_payload, &json_err_code);
  }
}

void DeviceToCloud::appTerminationCallback(int sig)
{
  DeviceToCloud::appCleanUp();
}

void DeviceToCloud::appCleanUp()
{
  // close any open connection
  mqtt_Disconnect();
  LE_INFO("MQTT DISCONNECTED");
}

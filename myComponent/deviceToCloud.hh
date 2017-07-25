// deviceToCloud.hh ---
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

#ifndef __DEVICE_TO_CLOUD_HH__
# define __DEVICE_TO_CLOUD_HH__

#include "legato.h"
#include "interfaces.h"
#include "lsm6ds3.hh"

# define MQTT_BROKER_URL	"eu.airvantage.net"
# define MQTT_BROKER_PASSWORD	"*****"
# define MQTT_PORT_NUMBER	1883
# define MQTT_KEEP_ALIVE	(1*60)
# define MQTT_QOS		0

/**
 * @CLASS DeviceToCloud
 * @brief our main application class
 * @dp    #singleton
 */
class DeviceToCloud
{
  //
  // METHODS
  //

public:

  /**
   * @method getInstance
   * @brief  instantiate the class
   *
   * @return [DeviceToCloud*] a class instance
   */
  static DeviceToCloud* getInstance();

  /**
   * @method appTerminationCallback
   * @brief  Callback catched with the sending signal
   *
   * @param [sig] Unix system signal
   *
   * @return nothing
   *
   * Example Usage:
   * @code
   * appTerminationCallback(SIGTERM);
   */
  static void appTerminationCallback(int sig);

  /**
   * @method appCleanUp
   * @brief  cleaning method called before ending application
   *
   * @return nothing
   */
  static void appCleanUp();

  /**
   * @method start
   * @brief  main loop acquisition process
   *
   * @return nothing
   */
  void start();

  /**
   * @method setMqttSession
   * @brief  mqtt session connection setter
   *
   * @param [v] connection state value
   *
   * @return nothing
   */
  void setMqttSession(bool v);

private:

  // Private constructor to prevent instancing.
  DeviceToCloud();

  /**
   * @method configureMQTT
   * @brief  mqtt service configuration
   *
   * @return nothing
   */
  void configureMQTT();

  /**
   * @method fetch
   * @brief  sensors data acquisition
   *
   * @return nothing
   */
  void fetch();

  /**
   * @method dump
   * @brief  log our data
   *
   * @return nothing
   */
  void dump();

  /**
   * @method send
   * @brief  format and send data to the cloud
   *
   * @return nothing
   */
  void send();

  //
  // ATTRIBUTES
  //

private:

  // our main instance application
  static DeviceToCloud* _instance;

  // native sensors module (3D digital accelerometer, 3D digital gyroscope, etc.)
  Lsm6ds3 _lsm6ds3;

  // timer reference object
  le_timer_Ref_t _timer_ref;

  // last captured temperature (in °Celsius)
  float	_last_temperature;

  // last captured gyroscope data
  struct gyroscope {
    float Ax;
    float Ay;
    float Az;
  } _last_gyroscope;


  // MQTT state
  bool _is_mqtt_connected;
};

#endif // !__DEVICE_TO_CLOUD_HH__

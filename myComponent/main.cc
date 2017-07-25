// main.cc ---
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

#include "legato.h"
#include "deviceToCloud.hh"

// Application entry point
COMPONENT_INIT
{
  LE_INFO("Hi, from deviceToCloud app!");

  // instantiate our application as a singleton
  DeviceToCloud* d = DeviceToCloud::getInstance();

  d->start();
}

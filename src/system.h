#pragma once
#ifndef SYSTEM_H
#define SYSTEM_H
#include <Arduino.h>

#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
  static const BaseType_t request_cpu = 1;
#else
  static const BaseType_t app_cpu = 1;
  static const BaseType_t request_cpu = 0;
#endif

#endif
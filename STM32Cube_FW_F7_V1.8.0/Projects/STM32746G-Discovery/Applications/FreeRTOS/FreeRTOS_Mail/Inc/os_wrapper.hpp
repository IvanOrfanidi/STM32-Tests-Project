/**
 * @file os_wrapper.hpp
 * @brief Заголовочный файл обертки API операционной системы
 */

#ifndef __OS_WRAPPER_HPP
#define __OS_WRAPPER_HPP

#include "FreeRTOS.h"
#include "task.h"

/// API для работы с операционной системой
class OS
{
 public:
   static void Init();   // Инициализация

   /**
    * @brief Запуск планировщика
    */
   static void StartScheduler()
   {
      vTaskStartScheduler();
   };

   /**
    * @brief Вход в критическую секцию кода
    */
   static void EnterCritical()
   {
      taskENTER_CRITICAL();
   };

   /**
    * @brief Выход из критической секции
    */
   static void ExitCritical()
   {
      taskEXIT_CRITICAL();
   };

   /**
    * @brief Сделать задержку
    */
   static void Delay_ms(uint32_t delay_ms)
   {
      vTaskDelay((TickType_t)delay_ms / portTICK_PERIOD_MS);
   };

   /**
    * @brief Запрос времени, прошедшего с момента запуска планировщика
    * @return время, прошедшее с момента запуска планировщика, мс
    */
   static uint32_t GetTickCount()
   {
      return xTaskGetTickCount();
   };
};

#endif

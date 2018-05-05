/**
 * @file thread.hpp
 * @brief C++ обертка для задач FreeRTOS
 */
#ifndef __THREAD_HPP
#define __THREAD_HPP

#include "FreeRTOS.h"
#include "task.h"
#include "queue.hpp"

/// Приоритеты потоков
enum ThreadPriority_t
{
   PRIORITY_IDLE = tskIDLE_PRIORITY,
   PRIORITY_LOW,
   PRIORITY_NORMAL,
   PRIORITY_ABOVE_NORMAL,
   PRIORITY_HIGH,
   PRIORITY_CRITICAL,
};

/**
 * @brief Базовый класс потока
 * @note Реализация метода run в классе - наследнике должна содержать бесконечный цикл
 */
class ThreadBase
{
 public:
   /// Запрос указателя на поток по указателю задачи
   static ThreadBase* GetThread(TaskHandle_t taskHandle)
   {
      return (ThreadBase*)xTaskGetApplicationTaskTag(taskHandle);
   }

   /**
    * @brief Засыпание потока
    * @param delay - таймаут сна, мс
    */
   static void Sleep(uint32_t delay_ms)
   {
      vTaskDelay((TickType_t)delay_ms / portTICK_PERIOD_MS);
   }

 protected:
   /**
    * @brief Исполнимый метод (работа потока)
    */
   virtual void Run() = 0;

   /**
    * @brief Конструктор
    * @param name - наименование потока
    * @param stackDepth - размер стека потока
    * @param priority - приоритет потока
    */
   ThreadBase(const char* name, unsigned stackDepth, ThreadPriority_t priority)
   {
      xTaskCreate(SchedulerCallBack, name, stackDepth, this, priority, &TaskHandle);
      vTaskSetApplicationTaskTag(TaskHandle, (TaskHookFunction_t)this);
   };

   /**
    * @brief Деструктор
    */
   ~ThreadBase()
   {
      vTaskDelete(TaskHandle);
   }

 private:
   /// Функция обратного вызова для планировщика
   static void SchedulerCallBack(void* thread)
   {
      (static_cast<ThreadBase*>(thread))->Run();
   }

   xTaskHandle TaskHandle = nullptr;   ///< Задача FreeRTOS, соответствующая потоку
};

#endif

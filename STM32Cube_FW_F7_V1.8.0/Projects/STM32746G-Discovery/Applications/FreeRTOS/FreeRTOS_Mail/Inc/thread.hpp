/**
 * @file thread.hpp
 * @brief C++ ������� ��� ����� FreeRTOS
 */
#ifndef __THREAD_HPP
#define __THREAD_HPP

#include "FreeRTOS.h"
#include "task.h"
#include "queue.hpp"

/// ���������� �������
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
 * @brief ������� ����� ������
 * @note ���������� ������ run � ������ - ���������� ������ ��������� ����������� ����
 */
class ThreadBase
{
 public:
   /// ������ ��������� �� ����� �� ��������� ������
   static ThreadBase* GetThread(TaskHandle_t taskHandle)
   {
      return (ThreadBase*)xTaskGetApplicationTaskTag(taskHandle);
   }

   /**
    * @brief ��������� ������
    * @param delay - ������� ���, ��
    */
   static void Sleep(uint32_t delay_ms)
   {
      vTaskDelay((TickType_t)delay_ms / portTICK_PERIOD_MS);
   }

 protected:
   /**
    * @brief ���������� ����� (������ ������)
    */
   virtual void Run() = 0;

   /**
    * @brief �����������
    * @param name - ������������ ������
    * @param stackDepth - ������ ����� ������
    * @param priority - ��������� ������
    */
   ThreadBase(const char* name, unsigned stackDepth, ThreadPriority_t priority)
   {
      xTaskCreate(SchedulerCallBack, name, stackDepth, this, priority, &TaskHandle);
      vTaskSetApplicationTaskTag(TaskHandle, (TaskHookFunction_t)this);
   };

   /**
    * @brief ����������
    */
   ~ThreadBase()
   {
      vTaskDelete(TaskHandle);
   }

 private:
   /// ������� ��������� ������ ��� ������������
   static void SchedulerCallBack(void* thread)
   {
      (static_cast<ThreadBase*>(thread))->Run();
   }

   xTaskHandle TaskHandle = nullptr;   ///< ������ FreeRTOS, ��������������� ������
};

#endif

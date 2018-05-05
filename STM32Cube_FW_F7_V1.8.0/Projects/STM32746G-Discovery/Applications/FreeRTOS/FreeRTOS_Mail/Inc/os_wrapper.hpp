/**
 * @file os_wrapper.hpp
 * @brief ������������ ���� ������� API ������������ �������
 */

#ifndef __OS_WRAPPER_HPP
#define __OS_WRAPPER_HPP

#include "FreeRTOS.h"
#include "task.h"

/// API ��� ������ � ������������ ��������
class OS
{
 public:
   static void Init();   // �������������

   /**
    * @brief ������ ������������
    */
   static void StartScheduler()
   {
      vTaskStartScheduler();
   };

   /**
    * @brief ���� � ����������� ������ ����
    */
   static void EnterCritical()
   {
      taskENTER_CRITICAL();
   };

   /**
    * @brief ����� �� ����������� ������
    */
   static void ExitCritical()
   {
      taskEXIT_CRITICAL();
   };

   /**
    * @brief ������� ��������
    */
   static void Delay_ms(uint32_t delay_ms)
   {
      vTaskDelay((TickType_t)delay_ms / portTICK_PERIOD_MS);
   };

   /**
    * @brief ������ �������, ���������� � ������� ������� ������������
    * @return �����, ��������� � ������� ������� ������������, ��
    */
   static uint32_t GetTickCount()
   {
      return xTaskGetTickCount();
   };
};

#endif

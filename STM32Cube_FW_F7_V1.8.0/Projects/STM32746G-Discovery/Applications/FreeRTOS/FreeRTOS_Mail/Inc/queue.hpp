/**
 * @file queue.hpp
 * @brief C++ ������� ��� �������� FreeRTOS
 */

#ifndef __QUEUE_HPP
#define __QUEUE_HPP

#include "FreeRTOS.h"
#include "queue.h"

/**
 * @brief �������
 */
template<class T>
class Queue
{
 public:
   /**
    * @brief �����������
    * @param itemMaxCount - ������������ ����� ��������� � �������
    */
   Queue(uint32_t itemMaxCount)
   {
      MessageQueue = xQueueCreate(itemMaxCount, sizeof(T));
   }

   /**
    * @brief ����������
    */
   ~Queue()
   {
      vQueueDelete(MessageQueue);
   }

   /**
    * @brief ��������� �������� � ����� �������.
    * @param item - ��������� �� �������, ������� ���� ���������
    * @param blockTimeout_ms - ������� ���������� � [��], 0 - ��� ����������,
    *        -1 - ����� �� ��� ���, ���� �� �������� ��������� ����� � �������
    * @return true, ���� ������� ������� � �������, ����� false
    */
   bool Put(const T* item, int32_t blockTimeout_ms = 0)
   {
      return xQueueSend(MessageQueue, (const void*)item, blockTimeout_ms / portTICK_PERIOD_MS);
   }

   /**
    * @brief ��������� �������� � �������, ����������� ��������� �������
    * @param item - ��������� �� �������, ������� ���� ���������
    * @return true, ���� ������� ������� � �������, ����� false
    * @note ������ ������� �������� ������ � ��������� ��������� �����!
    */
   bool PutOverwrite(const T* item)
   {
      return xQueueOverwrite(MessageQueue, (const void*)item);
   }

   /**
    * @brief ��������� �������� � ����� ������� �� ����������� ����������
    * @param item - ��������� �� �������, ������� ���� ���������
    * @return true, ���� ������� ������� � �������, ����� false
    */
   bool PutFromISR(const T* item)
   {
      BaseType_t higherPriorityTaskWoken = pdFALSE;
      bool result = xQueueSendFromISR(MessageQueue, (const void*)item, &higherPriorityTaskWoken);
      portEND_SWITCHING_ISR(higherPriorityTaskWoken);
      return result;
   }

   /**
    * @brief ���������� �������� �� ������ �������
    * @param item - ��������� ����������, � ������� ����� ���������� �������
    * @param blockTimeout_ms - ������� ���������� � [��], 0 - ��� ����������,
    * -1 -����� �� ��� ���, ���� �� �������� ���� �� ���� ������� � �������
    * @return true, ���� ������� ��������, ����� false
    */
   bool Get(T* item, int32_t blockTimeout_ms = 0)
   {
      return xQueueReceive(MessageQueue, (void*)item, blockTimeout_ms / portTICK_PERIOD_MS);
   }

   /**
    * @brief ���������� �������� �� ������ ������� � ����������� ����������
    * @param item - ��������� ����������, � ������� ����� ���������� �������
    * @return true, ���� ������� ��������, ����� false
    */
   bool GetFromISR(T* item)
   {
      BaseType_t higherPriorityTaskWoken = pdFALSE;
      bool result = xQueueReceiveFromISR(MessageQueue, (void*)item, &higherPriorityTaskWoken);
      portEND_SWITCHING_ISR(higherPriorityTaskWoken);
      return result;
   }

   /**
    * @brief ����������� ������� �� ������ �������.
    * @param item - ��������� ����������, � ������� ����� ���������� �������
    * @param blockTimeout_ms - ������� ���������� � [��], 0 - ��� ����������,
    * -1 -����� �� ��� ���, ���� �� �������� ���� �� ���� ������� � �������
    * @return true, ���� ������� ����������, ����� false
    */
   bool Peek(T* item, int32_t blockTimeout_ms = 0)
   {
      return xQueuePeek(MessageQueue, (void*)item, blockTimeout_ms / portTICK_PERIOD_MS);
   }

   /**
    * @brief ������ ����� ���������, ����������� � �������
    * @return ����� ���������
    */
   BaseType_t GetMessageCount()
   {
      return uxQueueMessagesWaiting(MessageQueue);
   }

   /**
    * @brief ������ ����� ���������, ����������� � ������� (�� ����������� ����������)
    * @return ����� ���������
    */
   BaseType_t GetMessageCountFromISR()
   {
      return uxQueueMessagesWaitingFromISR(MessageQueue);
   }

   /**
    * @brief ������ ����� ���������, ������� ��� ����� ����������� � �������
    * @return ����� ���������
    */
   BaseType_t GetSpaceAvailable()
   {
      return uxQueueSpacesAvailable(MessageQueue);
   }

   /**
    * @brief ����� �������
    */
   bool Reset()
   {
      return xQueueReset(MessageQueue);
   }

   /**
    * @brief �������� ������� ������� �� ����������� ����������
    */
   bool EmptyFromISR()
   {
      return xQueueIsQueueEmptyFromISR(MessageQueue);
   };

   /**
    * @brief �������� ������������� ������� �� ����������� ����������
    */
   bool FullFromISR()
   {
      return xQueueIsQueueFullFromISR(MessageQueue);
   };

 private:
   xQueueHandle MessageQueue;   ///< ��������� �� ������� FreeRTOS
};

#endif

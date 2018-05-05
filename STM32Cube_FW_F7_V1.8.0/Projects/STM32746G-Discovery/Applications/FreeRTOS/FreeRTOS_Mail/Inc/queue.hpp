/**
 * @file queue.hpp
 * @brief C++ обертка для очередей FreeRTOS
 */

#ifndef __QUEUE_HPP
#define __QUEUE_HPP

#include "FreeRTOS.h"
#include "queue.h"

/**
 * @brief Очередь
 */
template<class T>
class Queue
{
 public:
   /**
    * @brief Конструктор
    * @param itemMaxCount - максимальное число элементов в очереди
    */
   Queue(uint32_t itemMaxCount)
   {
      MessageQueue = xQueueCreate(itemMaxCount, sizeof(T));
   }

   /**
    * @brief Деструктор
    */
   ~Queue()
   {
      vQueueDelete(MessageQueue);
   }

   /**
    * @brief Помещение элемента в хвост очереди.
    * @param item - указатель на элемент, который надо поместить
    * @param blockTimeout_ms - таймаут блокировки в [мс], 0 - без блокировки,
    *        -1 - ждать до тех пор, пока не появится свободное место в очереди
    * @return true, если элемент помещен в очередь, иначе false
    */
   bool Put(const T* item, int32_t blockTimeout_ms = 0)
   {
      return xQueueSend(MessageQueue, (const void*)item, blockTimeout_ms / portTICK_PERIOD_MS);
   }

   /**
    * @brief Помещение элемента в очередь, перезаписав имеющийся элемент
    * @param item - указатель на элемент, который надо поместить
    * @return true, если элемент помещен в очередь, иначе false
    * @note Данная функция работает только с очередями единичной длины!
    */
   bool PutOverwrite(const T* item)
   {
      return xQueueOverwrite(MessageQueue, (const void*)item);
   }

   /**
    * @brief Помещение элемента в хвост очереди из обработчика прерывания
    * @param item - указатель на элемент, который надо поместить
    * @return true, если элемент помещен в очередь, иначе false
    */
   bool PutFromISR(const T* item)
   {
      BaseType_t higherPriorityTaskWoken = pdFALSE;
      bool result = xQueueSendFromISR(MessageQueue, (const void*)item, &higherPriorityTaskWoken);
      portEND_SWITCHING_ISR(higherPriorityTaskWoken);
      return result;
   }

   /**
    * @brief Извлечение элемента из головы очереди
    * @param item - указатель переменную, в которую будет скопирован элемент
    * @param blockTimeout_ms - таймаут блокировки в [мс], 0 - без блокировки,
    * -1 -ждать до тех пор, пока не появится хотя бы один элемент в очереди
    * @return true, если элемент извлечен, иначе false
    */
   bool Get(T* item, int32_t blockTimeout_ms = 0)
   {
      return xQueueReceive(MessageQueue, (void*)item, blockTimeout_ms / portTICK_PERIOD_MS);
   }

   /**
    * @brief Извлечение элемента из головы очереди в обработчике прерывания
    * @param item - указатель переменную, в которую будет скопирован элемент
    * @return true, если элемент извлечен, иначе false
    */
   bool GetFromISR(T* item)
   {
      BaseType_t higherPriorityTaskWoken = pdFALSE;
      bool result = xQueueReceiveFromISR(MessageQueue, (void*)item, &higherPriorityTaskWoken);
      portEND_SWITCHING_ISR(higherPriorityTaskWoken);
      return result;
   }

   /**
    * @brief Скопировать элемент из хвоста очереди.
    * @param item - указатель переменную, в которую будет скопирован элемент
    * @param blockTimeout_ms - таймаут блокировки в [мс], 0 - без блокировки,
    * -1 -ждать до тех пор, пока не появится хотя бы один элемент в очереди
    * @return true, если элемент скопирован, иначе false
    */
   bool Peek(T* item, int32_t blockTimeout_ms = 0)
   {
      return xQueuePeek(MessageQueue, (void*)item, blockTimeout_ms / portTICK_PERIOD_MS);
   }

   /**
    * @brief Запрос числа элементов, находящихся в очереди
    * @return число элементов
    */
   BaseType_t GetMessageCount()
   {
      return uxQueueMessagesWaiting(MessageQueue);
   }

   /**
    * @brief Запрос числа элементов, находящихся в очереди (Из обработчика прерывания)
    * @return число элементов
    */
   BaseType_t GetMessageCountFromISR()
   {
      return uxQueueMessagesWaitingFromISR(MessageQueue);
   }

   /**
    * @brief Запрос числа элементов, которое еще может поместиться в очередь
    * @return число элементов
    */
   BaseType_t GetSpaceAvailable()
   {
      return uxQueueSpacesAvailable(MessageQueue);
   }

   /**
    * @brief Сброс очереди
    */
   bool Reset()
   {
      return xQueueReset(MessageQueue);
   }

   /**
    * @brief Проверка пустоты очереди из обработчика прерывания
    */
   bool EmptyFromISR()
   {
      return xQueueIsQueueEmptyFromISR(MessageQueue);
   };

   /**
    * @brief Проверка заполненности очереди из обработчика прерывания
    */
   bool FullFromISR()
   {
      return xQueueIsQueueFullFromISR(MessageQueue);
   };

 private:
   xQueueHandle MessageQueue;   ///< Указатель на очередь FreeRTOS
};

#endif

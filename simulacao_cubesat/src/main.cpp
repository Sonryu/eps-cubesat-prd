#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

// BITS DO GRUPO DE EVENTOS
#define BATTERY_OK        (1 << 0)
#define BATTERY_LOW       (1 << 1)
#define BATTERY_CRITICAL  (1 << 2)

EventGroupHandle_t batteryEventGroup;
SemaphoreHandle_t batteryMutex;
TaskHandle_t payloadTaskHandle = NULL;

float batteryLevel = 100.0; 

// ======================
// TAREFA: Monitor de Bateria
// ======================
void TaskBatteryMonitor(void *pvParameters) {
  while (true) {
    xSemaphoreTake(batteryMutex, portMAX_DELAY);
    batteryLevel -= random(4, 8) * 0.5; // Simulação de descarga
    if (batteryLevel < 0) batteryLevel = 0;

    // TODO 1: Limpar os bits antigos do EventGroup
    // TODO 2: Implementar a lógica de decisão:
    // Se > 50%: Setar BATTERY_OK
    // Se entre 20% e 50%: Setar BATTERY_LOW
    // Se < 20%: Setar BATTERY_CRITICAL

    xSemaphoreGive(batteryMutex);
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

// ======================
// TAREFA: Telemetria
// ======================
void TaskTelemetry(void *pvParameters) {
  while (true) {
    // TODO 3: Obter o valor de 'batteryLevel' de forma segura
    float level = 0; // Substituir pela lógica correta

    // TODO 4: Ler os bits atuais do EventGroup para exibir o estado
    Serial.print("[TELEMETRIA] Bateria: ");
    Serial.print(level);
    Serial.println("%");

    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}

// ======================
// TAREFA: Sistema Crítico (Gerencia o Payload)
// ======================
void TaskCritical(void *pvParameters) {
  while (true) {
    // TODO 5: Esperar por QUALQUER um dos bits (OK, LOW ou CRITICAL)
    // Usar xEventGroupWaitBits com lógica 'OU'
    EventBits_t bits; 

    // TODO 6: Implementar a reação do sistema:
    // Se BATTERY_CRITICAL: vTaskSuspend(payloadTaskHandle)
    // Se BATTERY_OK: vTaskResume(payloadTaskHandle)

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void TaskPayload(void *pvParameters) {
  while (true) {
    Serial.println("[PAYLOAD] Realizando experimentos científicos...");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup() {
  Serial.begin(115200);
  batteryEventGroup = xEventGroupCreate();
  batteryMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(TaskBatteryMonitor, "BatMon", 4096, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(TaskTelemetry, "Telem", 4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(TaskPayload, "Payload", 4096, NULL, 1, &payloadTaskHandle, 1);
  xTaskCreatePinnedToCore(TaskCritical, "Critical", 4096, NULL, 3, NULL, 1);
}

void loop() {}
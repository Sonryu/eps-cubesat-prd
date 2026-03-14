#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

// BITS DO GRUPO DE EVENTOS
#define BATTERY_OK        (1 << 0)
#define BATTERY_LOW       (1 << 1)
#define BATTERY_CRITICAL  (1 << 2)

EventGroupHandle_t EventGroup_BATERIA;
SemaphoreHandle_t Mutex_BATERIA;
TaskHandle_t controle_tarefa_Payload = NULL;

float nivel_BATERIA = 100.0; 

// ======================
// TAREFA: Monitor de Bateria
// ======================
void TaskBatteryMonitor(void *pvParameters) {
  while (true) {
    xSemaphoreTake(Mutex_BATERIA, portMAX_DELAY);
    nivel_BATERIA -= random(4, 8) * 0.5; // Simulação de descarga
    if (nivel_BATERIA < 0) nivel_BATERIA = 0;

    
    
    if( nivel_BATERIA >= 50.0 ){
        xEventGroupSetBits(EventGroup_BATERIA, BATTERY_OK);
    
      } else if ( nivel_BATERIA >= 20.0 ) {
        xEventGroupSetBits(EventGroup_BATERIA, BATTERY_LOW);
    
      } else {
        xEventGroupSetBits(EventGroup_BATERIA, BATTERY_CRITICAL);
    }
    // TODO 1: Limpar os bits antigos do EventGroup
    // TODO 2: Implementar a lógica de decisão:
    // Se > 50%: Setar BATTERY_OK
    // Se entre 20% e 50%: Setar BATTERY_LOW
    // Se < 20%: Setar BATTERY_CRITICAL

    xSemaphoreGive(Mutex_BATERIA);
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

// ======================
// TAREFA: Telemetria
// ======================
void TaskTelemetry(void *pvParameters) {
  while (true) {
    // TODO 3: Obter o valor de 'batteryLevel' de forma segura
      xSemaphoreTake(Mutex_BATERIA, portMAX_DELAY);
    float level = nivel_BATERIA; // Substituir pela lógica correta
      xSemaphoreGive(Mutex_BATERIA);
    // TODO 4: Ler os bits atuais do EventGroup para exibir o estado
      EventBits_t statusBits = xEventGroupGetBits(EventGroup_BATERIA);
    Serial.print("[TELEMETRIA] Bateria: ");
    Serial.print(level);
    Serial.println("%");
      
      if ( statusBits & BATTERY_OK ) Serial.print("OK");
      else if ( statusBits & BATTERY_LOW ) Serial.print("LOW");
      else if (statusBits & BATTERY_CRITICAL) Serial.print("CRITICAL"); 

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
    
      bits = xEventGroupWaitBits(
          EventGroup_BATERIA,
          BATTERY_OK | BATTERY_LOW | BATTERY_CRITICAL,
          pdFALSE,
          pdFALSE,
          portMAX_DELAY
      );

    // TODO 6: Implementar a reação do sistema:
    // Se BATTERY_CRITICAL: vTaskSuspend(payloadTaskHandle)
    // Se BATTERY_OK: vTaskResume(payloadTaskHandle)
      if ( bits & BATTERY_CRITICAL ) {
        Serial.print("! ALERTA ! :: Suspendendo Payload ! ");
          vTaskSuspend(controle_tarefa_Payload);
      
      } else if ( bits & BATTERY_OK ) {
          vTaskResume(controle_tarefa_Payload);
      }
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
  EventGroup_BATERIA = xEventGroupCreate();
  Mutex_BATERIA = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(TaskBatteryMonitor, "BatMon", 4096, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(TaskTelemetry, "Telem", 4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(TaskPayload, "Payload", 4096, NULL, 1, &controle_tarefa_Payload, 1);
  xTaskCreatePinnedToCore(TaskCritical, "Critical", 4096, NULL, 3, NULL, 1);
}

void loop() {}

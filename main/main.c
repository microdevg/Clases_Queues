// ESP-IDF - ESP32
// Ejemplo:
// - 3 LEDs definidos con #define
// - 3 tareas productoras (source_a, source_b, source_c)
// - Cola compartida
// - app_main recibe datos y crea tareas dinámicas
// - tarea working imprime, genera pulso y se autodestruye

#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"

// ==========================
// DEFINES LEDS
// ==========================
#define LED_A 18
#define LED_B 19
#define LED_C 5

// ==========================
// ESTRUCTURA DE DATOS
// ==========================
typedef struct
{
    int led_gpio;
    int value;
    char source_name[16];

} work_data_t;

// ==========================
// QUEUE GLOBAL
// ==========================
QueueHandle_t work_queue;

// ==========================
// FUNCION WORKING
// ==========================
void working(void *pvParameters)
{
    work_data_t data = *(work_data_t *)pvParameters;

    printf("WORKING START -> source=%s value=%d gpio=%d\n",
           data.source_name,
           data.value,
           data.led_gpio);

    // Pulso LED
    gpio_set_level(data.led_gpio, 1);

    printf("LED GPIO %d ON\n", data.led_gpio);

    vTaskDelay(pdMS_TO_TICKS(300));

    gpio_set_level(data.led_gpio, 0);

    printf("LED GPIO %d OFF\n", data.led_gpio);

    printf("WORKING END -> deleting task\n\n");

    free(pvParameters);

    // Mata la tarea actual
    vTaskDelete(NULL);
}

// ==========================
// SOURCE A
// ==========================
void source_a(void *pvParameters)
{
    int counter = 0;

    while (1)
    {
        work_data_t data;

        data.led_gpio = LED_A;
        data.value = counter++;

        snprintf(data.source_name,
                 sizeof(data.source_name),
                 "SOURCE_A");

        xQueueSend(work_queue, &data, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// ==========================
// SOURCE B
// ==========================
void source_b(void *pvParameters)
{
    int counter = 100;

    while (1)
    {
        work_data_t data;

        data.led_gpio = LED_B;
        data.value = counter++;

        snprintf(data.source_name,
                 sizeof(data.source_name),
                 "SOURCE_B");

        xQueueSend(work_queue, &data, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

// ==========================
// SOURCE C
// ==========================
void source_c(void *pvParameters)
{
    int counter = 200;

    while (1)
    {
        work_data_t data;

        data.led_gpio = LED_C;
        data.value = counter++;

        snprintf(data.source_name,
                 sizeof(data.source_name),
                 "SOURCE_C");

        xQueueSend(work_queue, &data, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// ==========================
// APP MAIN
// ==========================
void app_main(void)
{
    // Config GPIO
    gpio_reset_pin(LED_A);
    gpio_reset_pin(LED_B);
    gpio_reset_pin(LED_C);

    gpio_set_direction(LED_A, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_B, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_C, GPIO_MODE_OUTPUT);

    // Crear queue comun
    work_queue = xQueueCreate(10, sizeof(work_data_t));

    // Crear tareas productoras
    xTaskCreate(source_a,
                "source_a",
                2048,
                NULL,
                5,
                NULL);

    xTaskCreate(source_b,
                "source_b",
                2048,
                NULL,
                5,
                NULL);

    xTaskCreate(source_c,
                "source_c",
                2048,
                NULL,
                5,
                NULL);

    // Receiver loop
    while (1)
    {
        work_data_t rx_data;

        if (xQueueReceive(work_queue,
                          &rx_data,
                          portMAX_DELAY))
        {
            printf("APP_MAIN RX -> source=%s value=%d\n",
                   rx_data.source_name,
                   rx_data.value);

            // Reservar memoria para nueva tarea
            work_data_t *task_data =
                malloc(sizeof(work_data_t));

            *task_data = rx_data;

            // Crear tarea dinamica
            xTaskCreate(working,
                        "working",
                        2048,
                        task_data,
                        6,
                        NULL);
        }
    }
}
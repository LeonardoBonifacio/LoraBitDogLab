#include <stdio.h>
#include "pico/stdlib.h"
#include "lib/rfm95w/rfm95w.h"

// Configurações dos pinos
const int LORA_SPI_PORT_NUM = 0;
const int LORA_CS_PIN = 17;
const int LORA_SCK_PIN = 18;
const int LORA_MOSI_PIN = 19;
const int LORA_MISO_PIN = 16;
const int LORA_RST_PIN = 22;
const int LORA_DIO0_PIN = 21;

// Frequência para o Brasil
const long LORA_FREQUENCY = 915E6; // 915 MHz

lora_t lora_device;
volatile bool packet_received = false;

// Callback da interrupção do pino DIO0
void dio0_callback(uint gpio, uint32_t events) {
    if (gpio == LORA_DIO0_PIN) {
        packet_received = true;
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("Inicializando Receptor LoRa...\n");

    // Inicializa o SPI
    spi_init(spi0, 1000000);
    gpio_set_function(LORA_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(LORA_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(LORA_MISO_PIN, GPIO_FUNC_SPI);

    // Inicializa o dispositivo LoRa
    if (!lora_init(&lora_device, spi0, LORA_FREQUENCY, LORA_CS_PIN, LORA_RST_PIN, LORA_DIO0_PIN)) {
        printf("Falha ao inicializar o LoRa. Verifique a conexão.\n");
        while(1);
    }
    printf("LoRa inicializado com sucesso! Aguardando pacotes...\n");

    // Configura a interrupção para o pino DIO0
    gpio_set_irq_enabled_with_callback(LORA_DIO0_PIN, GPIO_IRQ_EDGE_RISE, true, &dio0_callback);

    // Coloca o rádio em modo de recepção
    lora_receive_mode(&lora_device);

    while (1) {
        if (packet_received) {
            packet_received = false; // Reseta a flag

            uint8_t buffer[256];
            int packet_size = lora_receive_packet(&lora_device, buffer, sizeof(buffer));

            if (packet_size > 0) {
                buffer[packet_size] = '\0'; // Adiciona terminador nulo para imprimir

                int rssi = lora_packet_rssi(&lora_device);
                printf("Pacote recebido! Tamanho: %d, RSSI: %d dBm\n", packet_size, rssi);
                printf("Mensagem: '%s'\n\n", (char *)buffer);
            } else {
                printf("Erro na recepção do pacote (CRC inválido?).\n");
            }

            // Volta para o modo de recepção para o próximo pacote
            lora_receive_mode(&lora_device);
        }

        // O processador pode fazer outras coisas ou dormir aqui
        // tight_loop_contents();
    }
    return 0;
}

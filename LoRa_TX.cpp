/*
  LoRa TX - Exemplo de transmissor LoRa
  
  Este código configura um módulo LoRa como transmissor com parâmetros otimizados.
  Utiliza a biblioteca pico-lora para Raspberry Pi Pico com módulo RFM95W.
  
  Conexões:
  - CS: GPIO 8
  - RESET: GPIO 9
  - DIO0/IRQ: GPIO 7
  - MISO: GPIO 16
  - MOSI: GPIO 19
  - SCK: GPIO 18
*/

#include "stdlib.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "stdio.h"
#include "string.h"
#include <string>

// Incluir biblioteca LoRa
#include "Lora-RP2040.h"

using std::string;

// Definir pinos para o módulo LoRa
const int csPin = 8;          // LoRa radio chip select
const int resetPin = 9;       // LoRa radio reset
const int irqPin = 4;         // LoRa radio IRQ/DIO0

// Parâmetros de configuração LoRa
const long frequency = 915E6;  // Frequência em Hz (915MHz)
const int txPower = 17;        // Potência de transmissão (dBm)
const int spreadingFactor = 7; // Fator de espalhamento (7-12)
const long signalBandwidth = 125E3; // Largura de banda (Hz)
const int codingRate = 5;      // Taxa de codificação (5-8 para 4/5 até 4/8)
const int preambleLength = 8;  // Comprimento do preâmbulo
const int syncWord = 0x34;     // Palavra de sincronização (0x34 é o padrão)

// Variáveis para controle de envio
uint8_t msgCount = 0;          // Contador de mensagens enviadas
int interval = 2000;           // Intervalo entre envios (ms)
long lastSendTime = 0;         // Timestamp do último envio

// Função para enviar mensagem
void sendMessage(string message) {
  // Colocar o rádio em modo de transmissão
  LoRa.idle();
  LoRa.disableInvertIQ();      // Modo normal para transmissão
  
  // Iniciar pacote
  LoRa.beginPacket();
  
  // Adicionar payload
  LoRa.print(message.c_str());
  
  // Finalizar e enviar pacote
  LoRa.endPacket(true);  // true para envio assíncrono
  
  printf("Mensagem enviada: %s\n", message.c_str());
}

// Callback quando a transmissão for concluída
void onTxDone() {
  printf("Transmissão concluída!\n");
}

int main() {
  // Inicializar stdio
  stdio_init_all();
  
  printf("\nIniciando Transmissor LoRa...\n");
  
  // Configurar pinos do LoRa
  LoRa.setPins(csPin, resetPin, irqPin);
  
  // Inicializar o rádio LoRa
  if (!LoRa.begin(frequency)) {
    printf("Falha na inicialização do LoRa. Verifique as conexões.\n");
    while (true);  // Se falhar, não continua
  }
  
  // Configurar parâmetros do LoRa
  LoRa.setTxPower(txPower, PA_OUTPUT_PA_BOOST_PIN);
  LoRa.setSpreadingFactor(spreadingFactor);
  LoRa.setSignalBandwidth(signalBandwidth);
  LoRa.setCodingRate4(codingRate);
  LoRa.setPreambleLength(preambleLength);
  LoRa.setSyncWord(syncWord);
  LoRa.enableCrc();
  
  // Configurar callback para quando a transmissão for concluída
  LoRa.onTxDone(onTxDone);
  
  printf("Inicialização do LoRa concluída com sucesso!\n");
  printf("Configuração:\n");
  printf("- Frequência: %ld Hz\n", frequency);
  printf("- Potência TX: %d dBm\n", txPower);
  printf("- Fator de Espalhamento: %d\n", spreadingFactor);
  printf("- Largura de Banda: %ld Hz\n", signalBandwidth);
  printf("- Taxa de Codificação: 4/%d\n", codingRate);
  printf("- Comprimento do Preâmbulo: %d\n", preambleLength);
  printf("- Palavra de Sincronização: 0x%02X\n", syncWord);
  printf("\nIniciando transmissão de mensagens...\n\n");
  
  // Loop principal
  while (true) {
    // Verificar se é hora de enviar uma nova mensagem
    if (to_ms_since_boot(get_absolute_time()) - lastSendTime > interval) {
      // Criar mensagem
      string message = "Transmissor LoRa - Mensagem #";
      message += std::to_string(msgCount);
      
      // Enviar mensagem
      sendMessage(message);
      
      // Atualizar timestamp e contador
      lastSendTime = to_ms_since_boot(get_absolute_time());
      msgCount++;
      
      // Variar o intervalo entre 2-3 segundos
      interval = rand() % 1000 + 2000;
    }
  }
  
  return 0;
}
/*
  LoRa RX - Exemplo de receptor LoRa
  
  Este código configura um módulo LoRa como receptor com parâmetros otimizados.
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
const int irqPin = 7;         // LoRa radio IRQ/DIO0

// Parâmetros de configuração LoRa - DEVEM SER IGUAIS AO TRANSMISSOR
const long frequency = 915E6;  // Frequência em Hz (915MHz)
const int spreadingFactor = 7; // Fator de espalhamento (7-12)
const long signalBandwidth = 125E3; // Largura de banda (Hz)
const int codingRate = 5;      // Taxa de codificação (5-8 para 4/5 até 4/8)
const int preambleLength = 8;  // Comprimento do preâmbulo
const int syncWord = 0x34;     // Palavra de sincronização (0x34 é o padrão)

// Função para processar pacotes recebidos
void onReceive(int packetSize) {
  // Ignorar pacotes vazios
  if (packetSize == 0) return;
  
  // Criar buffer para a mensagem
  string message = "";
  
  // Ler bytes do pacote
  while (LoRa.available()) {
    message += (char)LoRa.read();
  }
  
  // Exibir informações do pacote recebido
  printf("\nPacote recebido:\n");
  printf("Mensagem: %s\n", message.c_str());
  printf("RSSI: %d dBm\n", LoRa.packetRssi());
  printf("SNR: %.2f dB\n", LoRa.packetSnr());
  printf("Erro de frequência: %ld Hz\n", LoRa.packetFrequencyError());
}

int main() {
  // Inicializar stdio
  stdio_init_all();
  
  printf("\nIniciando Receptor LoRa...\n");
  
  // Configurar pinos do LoRa
  LoRa.setPins(csPin, resetPin, irqPin);
  
  // Inicializar o rádio LoRa
  if (!LoRa.begin(frequency)) {
    printf("Falha na inicialização do LoRa. Verifique as conexões.\n");
    while (true);  // Se falhar, não continua
  }
  
  // Configurar parâmetros do LoRa - DEVEM SER IGUAIS AO TRANSMISSOR
  LoRa.setSpreadingFactor(spreadingFactor);
  LoRa.setSignalBandwidth(signalBandwidth);
  LoRa.setCodingRate4(codingRate);
  LoRa.setPreambleLength(preambleLength);
  LoRa.setSyncWord(syncWord);
  LoRa.enableCrc();
  
  // Configurar ganho do LNA (Low Noise Amplifier)
  // 0 = automático (AGC), 1-6 = ganho manual (1=mínimo, 6=máximo)
  LoRa.setGain(0);  // Usar AGC (recomendado para a maioria dos casos)
  
  printf("Inicialização do LoRa concluída com sucesso!\n");
  printf("Configuração:\n");
  printf("- Frequência: %ld Hz\n", frequency);
  printf("- Fator de Espalhamento: %d\n", spreadingFactor);
  printf("- Largura de Banda: %ld Hz\n", signalBandwidth);
  printf("- Taxa de Codificação: 4/%d\n", codingRate);
  printf("- Comprimento do Preâmbulo: %d\n", preambleLength);
  printf("- Palavra de Sincronização: 0x%02X\n", syncWord);
  printf("\nAguardando pacotes...\n");
  
  // Configurar callback para recepção de pacotes
  LoRa.onReceive(onReceive);
  
  // Colocar o rádio em modo de recepção contínua
  LoRa.receive();
  
  // Loop principal - apenas mantém o programa rodando
  while (true) {
    // O processamento de pacotes é feito no callback onReceive
    sleep_ms(100);  // Pequena pausa para economizar CPU
  }
  
  return 0;
}
/*
  LoRa Duplex - Exemplo de comunicação bidirecional LoRa
  
  Este código configura um módulo LoRa para comunicação bidirecional (duplex),
  permitindo que o dispositivo envie e receba mensagens.
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

// Definir o tipo byte como uint8_t
typedef uint8_t byte;

using std::string;

// Definir pinos para o módulo LoRa
const int csPin = 8;          // LoRa radio chip select
const int resetPin = 9;       // LoRa radio reset
const int irqPin = 7;         // LoRa radio IRQ/DIO0

// Parâmetros de configuração LoRa
const long frequency = 915E6;  // Frequência em Hz (915MHz)
const int txPower = 17;        // Potência de transmissão (dBm)
const int spreadingFactor = 7; // Fator de espalhamento (7-12)
const long signalBandwidth = 125E3; // Largura de banda (Hz)
const int codingRate = 5;      // Taxa de codificação (5-8 para 4/5 até 4/8)
const int preambleLength = 8;  // Comprimento do preâmbulo
const int syncWord = 0x34;     // Palavra de sincronização (0x34 é o padrão)

// Endereços dos dispositivos
const byte localAddress = 0xBB;     // Endereço deste dispositivo
const byte destinationAddress = 0xAA; // Endereço do dispositivo de destino

// Variáveis para controle de envio
uint8_t msgCount = 0;          // Contador de mensagens enviadas
int interval = 2000;           // Intervalo entre envios (ms)
long lastSendTime = 0;         // Timestamp do último envio

// Função para enviar mensagem
void sendMessage(string message) {
  // Colocar o rádio em modo de transmissão
  LoRa.idle();
  
  // Iniciar pacote
  LoRa.beginPacket();
  
  // Adicionar cabeçalho
  LoRa.write(destinationAddress);  // Endereço de destino
  LoRa.write(localAddress);        // Endereço do remetente
  LoRa.write(msgCount);            // ID da mensagem
  LoRa.write(message.length());    // Comprimento do payload
  
  // Adicionar payload
  LoRa.print(message.c_str());
  
  // Finalizar e enviar pacote
  LoRa.endPacket(true);  // true para envio assíncrono
  
  printf("Mensagem enviada para 0x%02X: %s\n", destinationAddress, message.c_str());
}

// Função para processar pacotes recebidos
void onReceive(int packetSize) {
  // Ignorar pacotes vazios
  if (packetSize == 0) return;
  
  // Ler cabeçalho
  byte recipient = LoRa.read();      // Endereço de destino
  byte sender = LoRa.read();         // Endereço do remetente
  byte incomingMsgId = LoRa.read();  // ID da mensagem
  byte incomingLength = LoRa.read(); // Comprimento do payload
  
  // Criar buffer para a mensagem
  string message = "";
  
  // Ler payload
  while (LoRa.available()) {
    message += (char)LoRa.read();
  }
  
  // Verificar se o comprimento está correto
  if (incomingLength != message.length()) {
    printf("Erro: comprimento da mensagem não corresponde.\n");
    return;
  }
  
  // Verificar se a mensagem é para este dispositivo
  if (recipient != localAddress && recipient != 0xFF) {
    printf("Mensagem não é para este dispositivo.\n");
    return;
  }
  
  // Exibir informações do pacote recebido
  printf("\nPacote recebido de: 0x%02X\n", sender);
  printf("ID da mensagem: %d\n", incomingMsgId);
  printf("Comprimento: %d\n", incomingLength);
  printf("Mensagem: %s\n", message.c_str());
  printf("RSSI: %d dBm\n", LoRa.packetRssi());
  printf("SNR: %.2f dB\n", LoRa.packetSnr());
  
  // Após receber, voltar ao modo de recepção
  LoRa.receive();
}

// Callback quando a transmissão for concluída
void onTxDone() {
  printf("Transmissão concluída!\n");
  // Voltar ao modo de recepção após transmitir
  LoRa.receive();
}

int main() {
  // Inicializar stdio
  stdio_init_all();
  
  printf("\nIniciando Dispositivo LoRa Duplex...\n");
  printf("Endereço local: 0x%02X\n", localAddress);
  printf("Endereço de destino: 0x%02X\n", destinationAddress);
  
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
  
  // Configurar callbacks
  LoRa.onReceive(onReceive);
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
  
  // Iniciar em modo de recepção
  LoRa.receive();
  printf("\nDispositivo pronto para enviar e receber mensagens...\n\n");
  
  // Loop principal
  while (true) {
    // Verificar se é hora de enviar uma nova mensagem
    if (to_ms_since_boot(get_absolute_time()) - lastSendTime > interval) {
      // Criar mensagem
      string message = "Olá do dispositivo 0x";
      char hexAddr[3];
      sprintf(hexAddr, "%02X", localAddress);
      message += hexAddr;
      message += " - Msg #";
      message += std::to_string(msgCount);
      
      // Enviar mensagem
      sendMessage(message);
      
      // Atualizar timestamp e contador
      lastSendTime = to_ms_since_boot(get_absolute_time());
      msgCount++;
      
      // Variar o intervalo entre 2-3 segundos
      interval = rand() % 1000 + 2000;
    }
    
    // Pequena pausa para economizar CPU
    sleep_ms(100);
  }
  
  return 0;
}
/*
  LoRa CAD - Exemplo de Detecção de Atividade de Canal (Channel Activity Detection)
  
  Este código demonstra como usar a funcionalidade CAD (Channel Activity Detection)
  do módulo LoRa para verificar se o canal está ocupado antes de transmitir.
  Isso é útil para implementar mecanismos simples de CSMA (Carrier Sense Multiple Access).
  
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

// Variáveis para controle de CAD
bool channelBusy = false;      // Flag para indicar se o canal está ocupado
int cadAttempts = 0;           // Contador de tentativas de CAD
const int maxCadAttempts = 10; // Número máximo de tentativas de CAD

// Função para enviar mensagem
void sendMessage(string message) {
  printf("Verificando atividade no canal...\n");
  
  // Resetar variáveis de controle de CAD
  channelBusy = false;
  cadAttempts = 0;
  
  // Executar CAD (Channel Activity Detection)
  LoRa.channelActivityDetection();
  
  // Aguardar resultado do CAD (será processado no callback onCadDone)
  while (cadAttempts == 0) {
    sleep_ms(10);
  }
  
  // Se o canal estiver ocupado após todas as tentativas, abortar
  if (channelBusy) {
    printf("Canal ocupado após %d tentativas. Abortando transmissão.\n", cadAttempts);
    return;
  }
  
  // Canal livre, prosseguir com a transmissão
  printf("Canal livre após %d tentativa(s). Iniciando transmissão...\n", cadAttempts);
  
  // Colocar o rádio em modo de transmissão
  LoRa.idle();
  
  // Iniciar pacote
  LoRa.beginPacket();
  
  // Adicionar payload
  LoRa.print(message.c_str());
  
  // Finalizar e enviar pacote
  LoRa.endPacket(true);  // true para envio assíncrono
  
  printf("Mensagem enviada: %s\n", message.c_str());
}

// Callback para resultado do CAD
void onCadDone(bool cadResult) {
  cadAttempts++;
  
  if (cadResult) {
    // Sinal LoRa detectado no canal
    printf("CAD: Sinal detectado no canal (tentativa %d)\n", cadAttempts);
    
    if (cadAttempts < maxCadAttempts) {
      // Tentar novamente após um tempo aleatório (backoff)
      int backoff = rand() % 200 + 100;  // 100-300ms
      printf("Aguardando %d ms antes da próxima tentativa...\n", backoff);
      sleep_ms(backoff);
      LoRa.channelActivityDetection();
    } else {
      // Número máximo de tentativas atingido
      channelBusy = true;
    }
  } else {
    // Nenhum sinal detectado, canal livre
    printf("CAD: Canal livre (tentativa %d)\n", cadAttempts);
    channelBusy = false;
  }
}

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
  
  // Voltar ao modo de recepção
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
  
  printf("\nIniciando Exemplo LoRa CAD (Channel Activity Detection)...\n");
  
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
  LoRa.onCadDone(onCadDone);
  
  printf("Inicialização do LoRa concluída com sucesso!\n");
  printf("Configuração:\n");
  printf("- Frequência: %ld Hz\n", frequency);
  printf("- Potência TX: %d dBm\n", txPower);
  printf("- Fator de Espalhamento: %d\n", spreadingFactor);
  printf("- Largura de Banda: %ld Hz\n", signalBandwidth);
  printf("- Taxa de Codificação: 4/%d\n", codingRate);
  printf("- Comprimento do Preâmbulo: %d\n", preambleLength);
  printf("- Palavra de Sincronização: 0x%02X\n", syncWord);
  printf("- Máximo de tentativas CAD: %d\n", maxCadAttempts);
  
  // Iniciar em modo de recepção
  LoRa.receive();
  printf("\nDispositivo pronto. Iniciando ciclo de transmissão/recepção...\n\n");
  
  // Loop principal
  while (true) {
    // Verificar se é hora de enviar uma nova mensagem
    if (to_ms_since_boot(get_absolute_time()) - lastSendTime > interval) {
      // Criar mensagem
      string message = "Mensagem CAD #";
      message += std::to_string(msgCount);
      
      // Enviar mensagem (com verificação de CAD)
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
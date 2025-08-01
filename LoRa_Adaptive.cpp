/*
  LoRa Adaptive - Exemplo de ajuste adaptativo de parâmetros LoRa
  
  Este código demonstra como ajustar dinamicamente os parâmetros LoRa
  com base nas condições do canal (RSSI e SNR), implementando um
  mecanismo de adaptação automática para otimizar a comunicação.
  
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

// Parâmetros iniciais de configuração LoRa
const long frequency = 915E6;  // Frequência em Hz (915MHz)
int txPower = 17;              // Potência de transmissão (dBm)
int spreadingFactor = 7;       // Fator de espalhamento (7-12)
long signalBandwidth = 125E3;  // Largura de banda (Hz)
int codingRate = 5;            // Taxa de codificação (5-8 para 4/5 até 4/8)
int preambleLength = 8;        // Comprimento do preâmbulo
const int syncWord = 0x34;     // Palavra de sincronização (0x34 é o padrão)

// Endereços dos dispositivos
const byte localAddress = 0xBB;     // Endereço deste dispositivo
const byte destinationAddress = 0xAA; // Endereço do dispositivo de destino

// Variáveis para controle de envio
uint8_t msgCount = 0;          // Contador de mensagens enviadas
int interval = 2000;           // Intervalo entre envios (ms)
long lastSendTime = 0;         // Timestamp do último envio

// Variáveis para adaptação de parâmetros
int lastRssi = 0;              // Último RSSI recebido
float lastSnr = 0.0;           // Último SNR recebido
int consecutiveFails = 0;      // Contador de falhas consecutivas
int consecutiveSuccess = 0;    // Contador de sucessos consecutivos
const int adaptationThreshold = 3; // Limiar para adaptação
bool ackReceived = false;      // Flag para confirmação de recebimento
long ackTimeout = 1000;        // Timeout para aguardar ACK (ms)
long lastAdaptationTime = 0;   // Timestamp da última adaptação
const long adaptationCooldown = 10000; // Período mínimo entre adaptações (ms)

// Estrutura para armazenar configurações LoRa
struct LoRaConfig {
  int sf;
  long bw;
  int cr;
  int txPower;
};

// Diferentes perfis de configuração
const LoRaConfig CONFIG_LONG_RANGE = {12, (long)62500, 8, 20}; // Máximo alcance
const LoRaConfig CONFIG_BALANCED = {9, (long)125000, 6, 17};     // Equilibrado
const LoRaConfig CONFIG_HIGH_DATA = {7, (long)250000, 5, 15};    // Alta taxa de dados

// Configuração atual
LoRaConfig currentConfig = CONFIG_BALANCED;

// Função para aplicar configuração
void applyConfig(LoRaConfig config) {
  printf("Aplicando nova configuração:\n");
  printf("- SF: %d -> %d\n", spreadingFactor, config.sf);
  printf("- BW: %ld -> %ld Hz\n", signalBandwidth, config.bw);
  printf("- CR: 4/%d -> 4/%d\n", codingRate, config.cr);
  printf("- TX Power: %d -> %d dBm\n", txPower, config.txPower);
  
  // Atualizar variáveis globais
  spreadingFactor = config.sf;
  signalBandwidth = config.bw;
  codingRate = config.cr;
  txPower = config.txPower;
  
  // Aplicar configuração ao rádio
  LoRa.idle();
  LoRa.setSpreadingFactor(spreadingFactor);
  LoRa.setSignalBandwidth(signalBandwidth);
  LoRa.setCodingRate4(codingRate);
  LoRa.setTxPower(txPower, PA_OUTPUT_PA_BOOST_PIN);
  LoRa.receive();
  
  // Registrar timestamp da adaptação
  lastAdaptationTime = to_ms_since_boot(get_absolute_time());
  
  // Resetar contadores
  consecutiveFails = 0;
  consecutiveSuccess = 0;
}

// Função para adaptar parâmetros com base nas condições do canal
void adaptParameters() {
  // Verificar se está no período de cooldown
  if (to_ms_since_boot(get_absolute_time()) - lastAdaptationTime < adaptationCooldown) {
    return;
  }
  
  // Adaptar com base em falhas consecutivas (sem ACK)
  if (consecutiveFails >= adaptationThreshold) {
    printf("\nDetectadas %d falhas consecutivas. Adaptando para maior alcance...\n", consecutiveFails);
    
    // Aumentar alcance gradualmente
    LoRaConfig newConfig = currentConfig;
    
    // Aumentar SF (até o máximo)
    if (newConfig.sf < 12) {
      newConfig.sf++;
    }
    // Reduzir BW se necessário
    else if (newConfig.bw > 62.5E3) {
      if (newConfig.bw == 250E3) newConfig.bw = 125E3;
      else if (newConfig.bw == 125E3) newConfig.bw = 62.5E3;
    }
    // Aumentar CR se necessário
    else if (newConfig.cr < 8) {
      newConfig.cr++;
    }
    // Aumentar potência se necessário
    else if (newConfig.txPower < 20) {
      newConfig.txPower++;
    }
    // Se já estiver no máximo, usar configuração de longo alcance
    else {
      newConfig = CONFIG_LONG_RANGE;
    }
    
    // Aplicar nova configuração
    currentConfig = newConfig;
    applyConfig(currentConfig);
  }
  // Adaptar com base em sucessos consecutivos (com boa qualidade de sinal)
  else if (consecutiveSuccess >= adaptationThreshold && lastSnr > 10.0 && lastRssi > -80) {
    printf("\nDetectados %d sucessos consecutivos com boa qualidade de sinal. Otimizando para taxa de dados...\n", consecutiveSuccess);
    
    // Otimizar para taxa de dados gradualmente
    LoRaConfig newConfig = currentConfig;
    
    // Reduzir SF (até o mínimo)
    if (newConfig.sf > 7) {
      newConfig.sf--;
    }
    // Aumentar BW se possível
    else if (newConfig.bw < 250E3) {
      if (newConfig.bw == 62.5E3) newConfig.bw = 125E3;
      else if (newConfig.bw == 125E3) newConfig.bw = 250E3;
    }
    // Reduzir CR se possível
    else if (newConfig.cr > 5) {
      newConfig.cr--;
    }
    // Reduzir potência para economizar energia
    else if (newConfig.txPower > 15 && lastRssi > -70) {
      newConfig.txPower--;
    }
    // Se já estiver otimizado, usar configuração de alta taxa de dados
    else {
      newConfig = CONFIG_HIGH_DATA;
    }
    
    // Aplicar nova configuração
    currentConfig = newConfig;
    applyConfig(currentConfig);
  }
}

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
  printf("Configuração atual: SF=%d, BW=%ld Hz, CR=4/%d, TX Power=%d dBm\n", 
         spreadingFactor, signalBandwidth, codingRate, txPower);
  
  // Resetar flag de ACK
  ackReceived = false;
  
  // Aguardar ACK
  long ackStartTime = to_ms_since_boot(get_absolute_time());
  printf("Aguardando ACK...");
  
  while (!ackReceived && (to_ms_since_boot(get_absolute_time()) - ackStartTime < ackTimeout)) {
    sleep_ms(10);
  }
  
  if (ackReceived) {
    printf(" ACK recebido!\n");
    consecutiveSuccess++;
    consecutiveFails = 0;
  } else {
    printf(" Timeout de ACK!\n");
    consecutiveFails++;
    consecutiveSuccess = 0;
  }
  
  // Verificar se é necessário adaptar parâmetros
  adaptParameters();
}

// Função para enviar ACK
void sendAck(byte toAddress, byte msgId) {
  // Colocar o rádio em modo de transmissão
  LoRa.idle();
  
  // Iniciar pacote
  LoRa.beginPacket();
  
  // Adicionar cabeçalho
  LoRa.write(toAddress);     // Endereço de destino
  LoRa.write(localAddress);  // Endereço do remetente
  LoRa.write(msgId);         // ID da mensagem original
  LoRa.write(3);             // Comprimento do payload
  
  // Adicionar payload ("ACK")
  LoRa.print("ACK");
  
  // Finalizar e enviar pacote
  LoRa.endPacket(true);
  
  printf("ACK enviado para 0x%02X (MsgID: %d)\n", toAddress, msgId);
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
  
  // Armazenar informações de qualidade do sinal
  lastRssi = LoRa.packetRssi();
  lastSnr = LoRa.packetSnr();
  
  // Verificar se a mensagem é para este dispositivo
  if (recipient == localAddress) {
    // Exibir informações do pacote recebido
    printf("\nPacote recebido de: 0x%02X\n", sender);
    printf("ID da mensagem: %d\n", incomingMsgId);
    printf("Comprimento: %d\n", incomingLength);
    printf("Mensagem: %s\n", message.c_str());
    printf("RSSI: %d dBm\n", lastRssi);
    printf("SNR: %.2f dB\n", lastSnr);
    
    // Verificar se é um ACK
    if (message == "ACK") {
      ackReceived = true;
    } else {
      // Enviar ACK para mensagens normais
      sendAck(sender, incomingMsgId);
    }
  } else if (recipient == 0xFF) {
    // Mensagem de broadcast
    printf("\nMensagem de broadcast recebida de: 0x%02X\n", sender);
    printf("ID da mensagem: %d\n", incomingMsgId);
    printf("Mensagem: %s\n", message.c_str());
    printf("RSSI: %d dBm\n", lastRssi);
    printf("SNR: %.2f dB\n", lastSnr);
  }
  
  // Após receber, voltar ao modo de recepção
  LoRa.receive();
}

// Callback quando a transmissão for concluída
void onTxDone() {
  // Voltar ao modo de recepção após transmitir
  LoRa.receive();
}

int main() {
  // Inicializar stdio
  stdio_init_all();
  
  printf("\nIniciando Dispositivo LoRa Adaptativo...\n");
  printf("Endereço local: 0x%02X\n", localAddress);
  printf("Endereço de destino: 0x%02X\n", destinationAddress);
  
  // Configurar pinos do LoRa
  LoRa.setPins(csPin, resetPin, irqPin);
  
  // Inicializar o rádio LoRa
  if (!LoRa.begin(frequency)) {
    printf("Falha na inicialização do LoRa. Verifique as conexões.\n");
    while (true);  // Se falhar, não continua
  }
  
  // Aplicar configuração inicial
  currentConfig = CONFIG_BALANCED;
  applyConfig(currentConfig);
  
  // Outras configurações
  LoRa.setPreambleLength(preambleLength);
  LoRa.setSyncWord(syncWord);
  LoRa.enableCrc();
  
  // Configurar callbacks
  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  
  printf("Inicialização do LoRa concluída com sucesso!\n");
  printf("Configuração inicial:\n");
  printf("- Frequência: %ld Hz\n", frequency);
  printf("- Fator de Espalhamento: %d\n", spreadingFactor);
  printf("- Largura de Banda: %ld Hz\n", signalBandwidth);
  printf("- Taxa de Codificação: 4/%d\n", codingRate);
  printf("- Potência TX: %d dBm\n", txPower);
  printf("- Comprimento do Preâmbulo: %d\n", preambleLength);
  printf("- Palavra de Sincronização: 0x%02X\n", syncWord);
  printf("- Limiar de adaptação: %d\n", adaptationThreshold);
  printf("- Timeout de ACK: %ld ms\n", ackTimeout);
  printf("- Período de cooldown: %ld ms\n", adaptationCooldown);
  
  // Iniciar em modo de recepção
  LoRa.receive();
  printf("\nDispositivo pronto para enviar e receber mensagens...\n\n");
  
  // Loop principal
  while (true) {
    // Verificar se é hora de enviar uma nova mensagem
    if (to_ms_since_boot(get_absolute_time()) - lastSendTime > interval) {
      // Criar mensagem
      string message = "Msg adaptativa #";
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
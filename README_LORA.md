# Comunicação LoRa com Raspberry Pi Pico e RFM95W

Este projeto demonstra a comunicação sem fio utilizando a tecnologia LoRa (Long Range) com módulos RFM95W conectados a placas Raspberry Pi Pico. São fornecidos dois exemplos:

1. **LoRa_TX.ino**: Transmissor que envia mensagens periodicamente
2. **LoRa_RX.ino**: Receptor que aguarda e processa mensagens recebidas

## Conexões de Hardware

Conecte o módulo RFM95W ao Raspberry Pi Pico conforme a tabela abaixo:

| Pino RFM95W | Pino Raspberry Pi Pico |
|-------------|------------------------|
| NSS/CS      | GPIO 8                 |
| RESET       | GPIO 9                 |
| DIO0/IRQ    | GPIO 7                 |
| MISO        | GPIO 16                |
| MOSI        | GPIO 19                |
| SCK         | GPIO 18                |
| VCC         | 3.3V                   |
| GND         | GND                    |

## Parâmetros de Configuração LoRa

Ambos os códigos (transmissor e receptor) utilizam os seguintes parâmetros de configuração:

| Parâmetro           | Valor      | Descrição                                       |
|---------------------|------------|------------------------------------------------|
| Frequência          | 915 MHz    | Frequência de operação (ajuste conforme regulamentação local) |
| Potência TX         | 17 dBm     | Potência de transmissão (apenas no transmissor) |
| Fator de Espalhamento | 7        | Determina a sensibilidade e alcance (7-12)     |
| Largura de Banda    | 125 kHz    | Largura do canal de comunicação                |
| Taxa de Codificação | 4/5        | Taxa de correção de erros                      |
| Comprimento do Preâmbulo | 8     | Tamanho do preâmbulo em símbolos               |
| Palavra de Sincronização | 0x34  | Identificador de rede (dispositivos com palavras diferentes não se comunicam) |

### Explicação dos Parâmetros

- **Frequência**: Deve ser ajustada conforme regulamentação local. Valores comuns: 433 MHz, 868 MHz, 915 MHz.

- **Fator de Espalhamento (SF)**: Determina a sensibilidade e alcance da comunicação.
  - SF mais alto (10-12): Maior alcance, menor taxa de dados, maior consumo de energia
  - SF mais baixo (7-9): Menor alcance, maior taxa de dados, menor consumo de energia

- **Largura de Banda (BW)**: Determina a largura do canal de comunicação.
  - BW menor (62.5 kHz, 41.7 kHz): Maior sensibilidade, menor taxa de dados
  - BW maior (250 kHz, 500 kHz): Menor sensibilidade, maior taxa de dados

- **Taxa de Codificação (CR)**: Define a quantidade de correção de erros.
  - 4/5: Menor redundância, maior taxa de dados úteis
  - 4/8: Maior redundância, menor taxa de dados úteis, mais robustez

- **Palavra de Sincronização**: Funciona como um identificador de rede. Dispositivos com palavras de sincronização diferentes não se comunicam entre si.

## Compilação e Upload

Para compilar e fazer upload dos códigos para o Raspberry Pi Pico:

1. Configure o ambiente de desenvolvimento para Raspberry Pi Pico com suporte à biblioteca pico-lora.

2. Adicione os arquivos LoRa_TX.ino e LoRa_RX.ino ao seu projeto.

3. Compile e faça upload do código LoRa_TX.ino para o Pico que funcionará como transmissor.

4. Compile e faça upload do código LoRa_RX.ino para o Pico que funcionará como receptor.

## Otimização de Parâmetros

Para otimizar a comunicação LoRa para diferentes cenários:

### Maximizar Alcance
```c
const int spreadingFactor = 12;     // Máximo SF
const long signalBandwidth = 62.5E3; // Menor BW
const int codingRate = 8;           // Máxima redundância (4/8)
const int txPower = 20;             // Máxima potência (apenas TX)
```

### Maximizar Taxa de Dados
```c
const int spreadingFactor = 7;      // Mínimo SF
const long signalBandwidth = 500E3;  // Maior BW
const int codingRate = 5;           // Mínima redundância (4/5)
```

### Equilíbrio entre Alcance e Taxa de Dados
```c
const int spreadingFactor = 9;      // SF intermediário
const long signalBandwidth = 125E3;  // BW intermediário
const int codingRate = 6;           // CR intermediário (4/6)
```

## Depuração

Ambos os códigos imprimem informações detalhadas via UART. Conecte o Raspberry Pi Pico ao computador e abra um terminal serial (115200 baud) para visualizar:

- No transmissor: Confirmações de envio e detalhes da configuração
- No receptor: Mensagens recebidas, RSSI (intensidade do sinal), SNR (relação sinal-ruído) e erro de frequência

## Notas Importantes

1. **Compatibilidade**: Para que a comunicação funcione, os parâmetros de configuração LoRa devem ser idênticos em ambos os dispositivos (exceto a potência de transmissão).

2. **Regulamentação**: Verifique as regulamentações locais para uso de frequências ISM antes de operar o dispositivo.

3. **Consumo de Energia**: Fatores de espalhamento mais altos e maior potência de transmissão aumentam significativamente o consumo de energia.

4. **Antena**: Utilize uma antena adequada para a frequência escolhida para obter o melhor desempenho.
# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.12)

# Importar o SDK do Pico
include(pico_sdk_import.cmake)

project(lora_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Inicializar o SDK
pico_sdk_init()

# Adicionar biblioteca Print
add_library(LoRa_print Print.cpp Print.h)

# Adicionar biblioteca LoRa
add_library(LoRa_lib Lora-RP2040.cpp Lora-RP2040.h)
target_link_libraries(LoRa_lib 
    pico_stdlib 
    hardware_spi 
    hardware_gpio 
    hardware_irq 
    LoRa_print
)

# Adicionar executável para o transmissor
add_executable(LoRa_TX
    LoRa_TX.cpp
)

target_link_libraries(LoRa_TX 
    pico_stdlib
    hardware_irq
    hardware_spi
    hardware_gpio
    LoRa_lib
)

# Configurar saída USB
pico_enable_stdio_usb(LoRa_TX 1)
pico_enable_stdio_uart(LoRa_TX 0)

# Gerar arquivos adicionais (UF2, etc.)
pico_add_extra_outputs(LoRa_TX)

# Adicionar executável para o receptor
add_executable(LoRa_RX
    Lora_RX.cpp
)

target_link_libraries(LoRa_RX 
    pico_stdlib
    hardware_irq
    hardware_spi
    hardware_gpio
    LoRa_lib
)

# Configurar saída USB
pico_enable_stdio_usb(LoRa_RX 1)
pico_enable_stdio_uart(LoRa_RX 0)

# Gerar arquivos adicionais (UF2, etc.)
pico_add_extra_outputs(LoRa_RX)

# Adicionar executável para comunicação duplex
add_executable(LoRa_Duplex
    LoRa_Duplex.cpp
)

target_link_libraries(LoRa_Duplex 
    pico_stdlib
    hardware_irq
    hardware_spi
    hardware_gpio
    LoRa_lib
)

# Configurar saída USB
pico_enable_stdio_usb(LoRa_Duplex 1)
pico_enable_stdio_uart(LoRa_Duplex 0)

# Gerar arquivos adicionais (UF2, etc.)
pico_add_extra_outputs(LoRa_Duplex)

# Adicionar executável para detecção de atividade de canal
add_executable(LoRa_CAD
    LoRa_CAD.cpp
)

target_link_libraries(LoRa_CAD 
    pico_stdlib
    hardware_irq
    hardware_spi
    hardware_gpio
    LoRa_lib
)

# Configurar saída USB
pico_enable_stdio_usb(LoRa_CAD 1)
pico_enable_stdio_uart(LoRa_CAD 0)

# Gerar arquivos adicionais (UF2, etc.)
pico_add_extra_outputs(LoRa_CAD)

# Adicionar executável para adaptação dinâmica
add_executable(LoRa_Adaptive
    LoRa_Adaptive.cpp
)

target_link_libraries(LoRa_Adaptive 
    pico_stdlib
    hardware_irq
    hardware_spi
    hardware_gpio
    LoRa_lib
)

# Configurar saída USB
pico_enable_stdio_usb(LoRa_Adaptive 1)
pico_enable_stdio_uart(LoRa_Adaptive 0)

# Gerar arquivos adicionais (UF2, etc.)
pico_add_extra_outputs(LoRa_Adaptive)
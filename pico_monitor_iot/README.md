# 📟 Pico Monitor IoT (BitDogLab)

![Status](https://img.shields.io/badge/status-completo-green) ![C](https://img.shields.io/badge/language-C-blue) ![Pico_SDK](https://img.shields.io/badge/Pico_SDK-%3E%3D1.5-orange)

## 🎯 Motivação
Emular um fluxo completo em sistemas embarcados: ler sensores em um microcontrolador, transmitir via Wi-Fi e exibir em um servidor. Ideal para aprendizado de IoT ponta-a-ponta e prototipagem rápida.

## 🚀 Funcionalidades Principais
- **Wi-Fi STA**: conexão WPA2 (credenciais em `inc/wifi_opts.h`).  
- **Heartbeat LED**: LED vermelho (GPIO 13) pisca a cada 1 s para sinalizar o laço ativo.  
- **Leitura de Botões**: BTN1 (GPIO 22) & BTN2 (GPIO 21) → JSON `{ "btn1":0/1, "btn2":0/1 }`.  
- **Leitura de Joystick**: ADC-0 (Y) & ADC-1 (X) → normalização (–1…+1) + rosa-dos-ventos (8 direções).  
- **Sensor Extra (Temperatura)**: sensor interno ADC-4 → JSON `{ "temp": XX.XX }`.  
- **Envio HTTP**: POST manual via LWIP TCP para rotas `/botoes`, `/joystick`, `/temp` (configurável em `inc/server_opts.h`).

## 📦 Estrutura de Pastas
```text
pico_monitor_iot/
├── inc/                # Arquivos de configuração
│   ├── hw_config.h     # Mapeamento de pinos, constantes gerais
│   ├── wifi_opts.h     # SSID e senha (adicione em .gitignore)
│   └── server_opts.h   # IP e porta do backend (local ou nuvem)
├── pico_monitor_iot.c  # Firmware principal (loop, ADC, GPIO, JSON, HTTP)
├── CMakeLists.txt      # Configuração do build via Pico SDK
└── README.md           # Este documento
```
## ⚙️ Pré-requisitos
- Pico SDK ≥ 1.5 (recomendado 1.5+ / 2.x)
- Tool-chain ARM GCC `(arm-none-eabi-gcc)` ou VS Code + extensão Pico W
- Python 3.10+ (somente para backend de testes local)
- Rede Wi-Fi 2.4 GHz – Pico W e servidor devem estar na mesma sub-rede
## 🔧 Configuração
1. Crie `inc/wifi_opts.h:`
```sh
#define WIFI_SSID     "<SuaRedeWiFi>"
#define WIFI_PASSWORD "<SuaSenha>"
```
2. Crie `inc/server_opts.h`:
```sh
#define SERVER_IP   "<IP_DO_SERVIDOR>"  // ex: "192.168.0.52" ou domínio
#define SERVER_PORT 3000                // porta do backend
```
3. Ajustes opcionais:
- Alterar pino do LED em `hw_config.h` (`LED_RED_PIN`).
- Ajustar intervalo de envio mudando o temporizador de 1e6 µs no código.
## 🛠️ Build & Flash
```sh
# 1. Crie diretório de build
mkdir build && cd build

# 2. Configure com CMake
cmake ..

# 3. Compile
make          # ou ninja

# 4. Flash na Pico W
#    - Coloque a placa em modo BOOTSEL
#    - Copie o .uf2 gerado para a unidade
```
## 🔍 Verificação & Logs
Abra o Serial Monitor (115200 Bd):
```sh
Wi-Fi OK – IP 192.168.0.52
JSON botões: { "btn1": 1, "btn2": 0 }
JSON joystick: { "x": 0.123, "y": -0.456, "rosa": "Nordeste" }
JSON temperatura: { "temp": 29.87 }
```
## 🌐 Integração com Backend
- Ajuste `SERVER_IP`/`SERVER_PORT` para apontar ao seu servidor (FastAPI, Flask ou similar).
- O backend deve expor HTTP 204 No Content e WebSocket `/ws` para visualização em tempo real.
## 🌟 Personalizações Rápidas
- Novo sensor: crie `post_<sensor>()` seguindo o padrão existente.
- Debounce: implemente amostragem múltipla em `buttons_get_json()`.
- OTA: adicione rotina de atualização de firmware via HTTP/UF2.
## 📖 Referências
- Datasheet RP2040 – Sensor de temperatura §4.9
- lwIP API – `tcp_new()`, `tcp_write()`, `tcp_recv()`
- BitDogLab Examples – uso do driver CYW43

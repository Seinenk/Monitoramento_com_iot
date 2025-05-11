Pico Monitor IoT (BitDogLab)

Status: Firmware pronto – Backend local em Flask em desenvolvimento

Este repositório contém o firmware C para a placa Raspberry Pi Pico W (BitDogLab) capaz de:

Função

Detalhes

Wi‑Fi STA

Conexão WPA2 (credenciais em inc/wifi_opts.h)

Heartbeat LED

LED vermelho (GPIO 13) pisca a cada 1 s para indicar a execução do loop

Leitura de Botões

BTN1 (GPIO 22) & BTN2 (GPIO 21) – envia JSON { "btn1":0/1, "btn2":0/1 }

Leitura de Joystick

ADC‑0 (Y) & ADC‑1 (X) – envia posição normalizada e rosa‑dos‑ventos

Temperatura interna

Sensor on‑chip (ADC‑4) – envia JSON { "temp": XX.XX }

POST HTTP

Todos os dados são enviados a um servidor Flask definido em inc/server_opts.h

📁 Estrutura do repositório

pico_monitor_iot/
├── inc/ # Cabeçalhos de configuração
│ ├── hw_config.h # Pinos, LED, etc.
│ ├── wifi_opts.h # ⇦ Você cria aqui (SSID/Senha)
│ └── server_opts.h # ⇦ Você cria aqui (IP/Porta)
├── pico_monitor_iot.c # Firmware principal
├── CMakeLists.txt # Build com pico‑sdk
└── ...

Um segundo diretório backend/ (fora desta pasta) conterá o servidor Flask – instruções mais abaixo.

⚙️ Pré‑requisitos

Pico SDK ≥ 1.5 (usamos 2.1.1) – consulte a documentação oficial

Tool‑chain GCC ARM (ou VS Code + extensão Raspberry Pi Pico‑W)

Python 3.10+ (para o backend Flask)

🚀 Passo‑a‑passo para compilar o firmware

Clone este repositório

Crie inc/wifi_opts.h e inc/server_opts.h:

// wifi_opts.h
#define WIFI_SSID "MinhaRede"
#define WIFI_PASSWORD "senha123"

// server_opts.h
#define SERVER_IP "192.168.0.100" // ou domínio na nuvem
#define SERVER_PORT 5000 // porta do Flask

Abra um terminal na pasta pico_monitor_iot:

mkdir build && cd build
cmake ..
ninja # ou make

Coloque a Pico W em BOOTSEL mode, copie o pico_monitor_iot.uf2 gerado.

Abra o Serial Monitor (115200 Bd). Você deverá ver logs como:

Wi‑Fi OK – IP 192.168.0.42
JSON botões: { "btn1": 0, "btn2": 1 }
JSON joystick: { "x": 0.123, "y": -0.456, "rosa": "Nordeste" }
JSON temperatura: { "temp": 29.87 }

🌐 Backend Flask (próximo passo)

Criaremos uma pasta irmã backend/ com app.py, requirements.txt e um index.html simples para visualizar os dados em tempo real. Quando pronta, bastará ajustar SERVER_IP/PORT.

Observação: durante o desenvolvimento local, você pode usar o IP do computador (ou 10.0.0.x). Para deploy em nuvem, bastará apontar o hostname público e porta 80/443.

🛠️ Personalização rápida

LED: mude LED_RED_PIN em hw_config.h caso utilize outro pino/LED.

Intervalo de envio: altere o timeout de 1e6 µs no main para qualquer valor.

Sensores adicionais: basta criar uma função post\_<sensor>() seguindo o mesmo padrão.

📋 Referências úteis

Datasheet RP2040 – Sensor de temperatura §4.9

lwIP API – tcp_write / tcp_recv

Raspberry Pi Pico W &cyw43 driver examples

MIT License — à vontade para usar/alterar.

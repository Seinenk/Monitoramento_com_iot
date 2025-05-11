# 🚀 Monitoramento IoT com Raspberry Pi Pico W (BitDogLab)

![Status](https://img.shields.io/badge/status-done-green)  
![Python](https://img.shields.io/badge/python-3.12-blue) ![CMake](https://img.shields.io/badge/cmake-v3.20-blue)  

---

## 🎯 Motivação

Em ambientes de prototipagem e ensino de **Sistemas Embarcados**, é fundamental mostrar o fluxo completo de dados:
1. 👇 Leitura de **hardware** (botões, joystick, temperatura)  
2. 📡 Transmissão via **Wi-Fi**/HTTP  
3. 🌐 Recepção e **redistribuição** em tempo real via **WebSocket**  
4. ☁️ Desafio extra: deploy em **nuvem** (AWS/GCP)

---

## 📋 Requisitos

- **Hardware**  
  - Raspberry Pi Pico W (BitDogLab) com 2 botões e joystick  
- **Ferramentas Firmware**  
  - Pico SDK + CMake + `arm-none-eabi-gcc`  
  - LWIP + CYW43 (biblioteca de Wi-Fi)  
- **Ferramentas Backend**  
  - Python 3.10+  
  - `fastapi`, `uvicorn`  
- **Rede**  
  - Wi-Fi 2.4 GHz para Pico W e servidor na mesma sub-rede (porta TCP **3000** aberta)  

---

## 📂 Estrutura de Pastas

```text
monitoramento_com_iot/
├─ pico_monitor_iot/         📦 Firmware C para Pico W
└─ bitdoglab_server/         🌐 Backend FastAPI + Uvicorn
```

## 🖥 pico_monitor_iot/

inc/
├─ hw_config.h       ← SSID, senha e pinos
└─ server_opts.h     ← IP + porta do backend
pico_monitor_iot.c   ← Loop principal, ADC, GPIO, JSON, HTTP via LWIP
CMakeLists.txt       ← Configura build com Pico SDK


## 🌐 bitdoglab_server/

app/
└─ main.py           ← Rotas POST /botoes, /joystick, /temp + WebSocket /ws
requirements.txt     ← Dependências Python
Dockerfile           ← (Opcional) container Python slim + Uvicorn
docker-compose.yml   ← (Opcional) orquestra container
README.md            ← Instruções específicas do backend

## 🔧 Como Rodar

### 1️⃣ Firmware (Pico W)
1. Edite pico_monitor_iot/inc/hw_config.h com SSID e senha da sua rede.
2. Edite pico_monitor_iot/inc/server_opts.h com IP e porta do backend (ex.: "192.168.0.52", 3000).
3. Compile via CMake:
```sh
mkdir build && cd build
cmake ..
make
```
4. Copie o `pico_monitor_iot.uf2` gerado para a Pico W em modo BOOTSEL.

### 2️⃣ Backend (Local)
```sh
cd bitdoglab_server
python -m venv .venv && source .venv/bin/activate      # Windows: .venv\Scripts\Activate
pip install -r requirements.txt
uvicorn app.main:app --reload --port 3000
```
- 📑 Swagger UI: http://localhost:3000/docs
- 🔌 WebSocket: `wscat -c ws://localhost:3000/ws` ou extensão de navegador

## 📌 Observações Finais

- 🕒 O firmware envia 3 POSTs/s (botões, joystick, temperatura).
- 📶 O backend responde `204 No Content` e empurra os eventos em tempo real para todos os clientes WebSocket.

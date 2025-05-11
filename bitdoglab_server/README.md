# Servidor BitDogLab para Monitor de Joystick + Botões + Temperatura

Backend leve em **FastAPI** compatível com o firmware do projeto *Pico Monitor IoT*.

Recebe três tipos de POST vindos da placa (a cada 1 s) e retransmite todos os
eventos via WebSocket `/ws` para dashboards ou testes em tempo real.

## Rotas

| Método | Rota        | Payload (exemplo)                               | Descrição |
| ------ | ----------- | ----------------------------------------------- | --------- |
| POST   | `/botoes`   | `{ "btn1": 1, "btn2": 0 }`                      | Estado dos botões físicos |
| POST   | `/joystick` | `{ "x": 0.12, "y": -0.87, "rosa": "Noroeste" }` | Posição normalizada + rosa-dos-ventos |
| POST   | `/temp`     | `{ "temp": 42.7 }`                              | Temperatura do RP2040 |
| WS     | `/ws`       | –                                               | Fluxo contínuo de todos os eventos em JSON |

Cada mensagem enviada ao WebSocket recebe:

```jsonc
{
  "type": "botoes", // ou "joystick" / "temp"
  "ts": 1713219600.123, // timestamp epoch segundos
  // …demais campos originais
}
```

## Execução local

```bash
python -m venv .venv && source .venv/bin/activate
pip install -r requirements.txt
uvicorn app.main:app --reload --port 3000
# Swagger em http://localhost:3000/docs
```

## Docker

```bash
docker compose up -d --build
```

Depois disso, aponte o seu firmware para `SERVER_IP` = IP da máquina
(porta `3000`) em `inc/server_opts.h`.

## Deploy rápido (AWS Lightsail)

1. Crie instância Ubuntu 22.04, libere a porta 3000 no firewall Lightsail.
2. Instale Docker & Git:
   ```bash
   sudo apt update && sudo apt install -y docker.io docker-compose git
   ```
3. Faça clone do repositório ou envie o ZIP:
   ```bash
   git clone <seu-repo> bitdoglab_server && cd bitdoglab_server
   docker compose up -d --build
   ```
4. Atualize `SERVER_IP` no firmware com o IP público da instância.



from fastapi import FastAPI, WebSocket
from pydantic import BaseModel
from typing import List
import datetime

app = FastAPI(
    title="Servidor BitDogLab",
    description="Recebe JSON do firmware Pico W (botões, joystick e temperatura) "
                "e retransmite via WebSocket para dashboards.",
    version="1.0.0",
    docs_url="/docs",
)

# ----------------------------- MODELOS -----------------------------
class BotoesPayload(BaseModel):
    btn1: int
    btn2: int
    ts: float | None = None  # opcional (epoch segundos)

class JoystickPayload(BaseModel):
    x: float
    y: float
    rosa: str
    ts: float | None = None

class TempPayload(BaseModel):
    temp: float
    ts: float | None = None

# -------------------------- GESTÃO DE CLIENTES ---------------------
clients: List[WebSocket] = []

async def _broadcast(message: dict):
    vivos = []
    for ws in clients:
        try:
            await ws.send_json(message)
            vivos.append(ws)
        except Exception:
            # cliente desconectou
            pass
    clients[:] = vivos

# ----------------------------- ROTAS -------------------------------
@app.post("/botoes", status_code=204)
async def botoes(payload: BotoesPayload):
    data = payload.dict()
    data["type"] = "botoes"
    data["ts"] = data["ts"] or datetime.datetime.utcnow().timestamp()
    await _broadcast(data)

@app.post("/joystick", status_code=204)
async def joystick(payload: JoystickPayload):
    data = payload.dict()
    data["type"] = "joystick"
    data["ts"] = data["ts"] or datetime.datetime.utcnow().timestamp()
    await _broadcast(data)

@app.post("/temp", status_code=204)
async def temperatura(payload: TempPayload):
    data = payload.dict()
    data["type"] = "temp"
    data["ts"] = data["ts"] or datetime.datetime.utcnow().timestamp()
    await _broadcast(data)

# ---------------------------- WEBSOCKET ----------------------------
@app.websocket("/ws")
async def ws_endpoint(ws: WebSocket):
    await ws.accept()
    clients.append(ws)
    try:
        while True:
            # Mantém conexão viva; descarta mensagens do cliente
            await ws.receive_text()
    except Exception:
        pass
    finally:
        if ws in clients:
            clients.remove(ws)

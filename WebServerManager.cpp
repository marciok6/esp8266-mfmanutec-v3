/**
 * @file WebServerManager.cpp
 * @brief Implementação do servidor web de configuração
 *
 * @author  MFMANUTEC
 * @version 1.0.0
 */

#include "WebServerManager.h"

// ============================================================
// HTML DA PÁGINA DE CONFIGURAÇÃO (PROGMEM)
// ============================================================

const char WebServerManager::INDEX_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>MFMANUTEC – Configuração IoT</title>
  <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css">
  <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap-icons@1.11.2/font/bootstrap-icons.css">
  <style>
    :root {
      --mf-blue:  #1a2e4a;
      --mf-blue2: #2d4a73;
      --mf-gray:  #6c757d;
      --mf-light: #f4f6f9;
    }
    body        { background: var(--mf-light); font-family: 'Segoe UI', sans-serif; }
    .navbar     { background: var(--mf-blue) !important; }
    .navbar-brand span { color: #f0a500; font-weight: 700; font-size: 1.4rem; }
    .navbar-brand small { color: #adb5bd; font-size: 0.7rem; display: block; line-height: 1; }
    .card       { border: none; box-shadow: 0 2px 12px rgba(0,0,0,.10); border-radius: 12px; }
    .card-header { background: var(--mf-blue2); color: #fff; border-radius: 12px 12px 0 0 !important; }
    .btn-primary { background: var(--mf-blue2); border-color: var(--mf-blue2); }
    .btn-primary:hover { background: var(--mf-blue); border-color: var(--mf-blue); }
    .badge-info-item { background: #e9ecef; border-radius: 8px; padding: 6px 12px; margin-bottom: 6px; font-size: .85rem; }
    .signal-icon { font-size: 1.2rem; }
    #toast-container { position: fixed; top: 70px; right: 16px; z-index: 9999; }
    .spinner-border-sm { width: 1rem; height: 1rem; }
  </style>
</head>
<body>

<!-- NAVBAR / LOGO -->
<nav class="navbar navbar-dark sticky-top px-3 py-2">
  <a class="navbar-brand d-flex align-items-center gap-2" href="#">
    <div>
      <span>MF<span style="color:#f0a500">MANUTEC</span></span>
      <small>Soluções que Movem Conforto e Eficiência</small>
    </div>
  </a>
</nav>

<!-- TOAST -->
<div id="toast-container"></div>

<div class="container py-4" style="max-width:620px">

  <!-- INFORMAÇÕES DO DISPOSITIVO -->
  <div class="card mb-4">
    <div class="card-header d-flex align-items-center gap-2">
      <i class="bi bi-cpu"></i> Informações do Dispositivo
    </div>
    <div class="card-body" id="device-info">
      <div class="text-center text-muted py-3">
        <div class="spinner-border spinner-border-sm" role="status"></div>
        Carregando...
      </div>
    </div>
  </div>

  <!-- CONFIGURAÇÃO WIFI -->
  <div class="card mb-4">
    <div class="card-header d-flex align-items-center gap-2">
      <i class="bi bi-wifi"></i> Configuração WiFi
    </div>
    <div class="card-body">
      <div class="mb-3">
        <label class="form-label fw-semibold">Rede WiFi (SSID)</label>
        <div class="input-group">
          <select class="form-select" id="ssid-select">
            <option value="">-- Selecione ou busque --</option>
          </select>
          <button class="btn btn-outline-secondary" type="button" id="btn-scan" title="Buscar redes">
            <i class="bi bi-arrow-clockwise"></i>
          </button>
        </div>
        <div class="form-text text-muted" id="scan-status">Clique em <i class="bi bi-arrow-clockwise"></i> para buscar redes.</div>
      </div>
      <div class="mb-3">
        <label class="form-label fw-semibold">Senha</label>
        <div class="input-group">
          <input type="password" class="form-control" id="wifi-pass" placeholder="Senha da rede WiFi" maxlength="63">
          <button class="btn btn-outline-secondary" type="button" id="btn-show-pass">
            <i class="bi bi-eye"></i>
          </button>
        </div>
      </div>
      <div class="d-grid">
        <button class="btn btn-primary" id="btn-save">
          <i class="bi bi-floppy"></i> Salvar e Conectar
        </button>
      </div>
    </div>
  </div>

  <!-- AÇÕES -->
  <div class="card mb-4">
    <div class="card-header d-flex align-items-center gap-2">
      <i class="bi bi-gear"></i> Ações do Dispositivo
    </div>
    <div class="card-body">
      <div class="d-flex gap-2 flex-wrap">
        <button class="btn btn-warning flex-fill" id="btn-restart">
          <i class="bi bi-arrow-counterclockwise"></i> Reiniciar
        </button>
        <button class="btn btn-danger flex-fill" id="btn-factory">
          <i class="bi bi-trash"></i> Restaurar Padrão
        </button>
      </div>
    </div>
  </div>

  <p class="text-center text-muted" style="font-size:.8rem">
    MFMANUTEC IoT v<span id="fw-version">–</span> &nbsp;|&nbsp;
    &copy; 2026 MFMANUTEC
  </p>
</div>

<script>
// ============================================================
// UTILITÁRIOS
// ============================================================
function toast(msg, type='success') {
  const el = document.createElement('div');
  el.className = `alert alert-${type} alert-dismissible fade show shadow`;
  el.role = 'alert';
  el.innerHTML = msg + '<button type="button" class="btn-close" data-bs-dismiss="alert"></button>';
  document.getElementById('toast-container').appendChild(el);
  setTimeout(() => el.remove(), 4000);
}

function signalIcon(rssi) {
  if (rssi >= -55) return '<i class="bi bi-wifi signal-icon text-success"></i>';
  if (rssi >= -65) return '<i class="bi bi-wifi-2 signal-icon text-success"></i>';
  if (rssi >= -75) return '<i class="bi bi-wifi-1 signal-icon text-warning"></i>';
  return '<i class="bi bi-wifi signal-icon text-danger"></i>';
}

// ============================================================
// INFORMAÇÕES DO DISPOSITIVO
// ============================================================
async function loadDeviceInfo() {
  try {
    const r = await fetch('/info');
    const d = await r.json();
    document.getElementById('fw-version').textContent = d.firmware || '–';
    document.getElementById('device-info').innerHTML = `
      <div class="row g-2">
        <div class="col-6"><div class="badge-info-item"><b>MAC:</b><br>${d.mac}</div></div>
        <div class="col-6"><div class="badge-info-item"><b>IP:</b><br>${d.ip}</div></div>
        <div class="col-6"><div class="badge-info-item"><b>Firmware:</b><br>v${d.firmware}</div></div>
        <div class="col-6"><div class="badge-info-item"><b>Chip ID:</b><br>${d.chip_id}</div></div>
        <div class="col-12"><div class="badge-info-item"><b>SSID atual:</b> ${d.ssid || '<em>Não configurado</em>'}</div></div>
      </div>`;
  } catch(e) {
    document.getElementById('device-info').innerHTML =
      '<div class="alert alert-warning mb-0">Não foi possível carregar as informações.</div>';
  }
}

// ============================================================
// SCAN DE REDES
// ============================================================
document.getElementById('btn-scan').addEventListener('click', async () => {
  const btn = document.getElementById('btn-scan');
  const sel  = document.getElementById('ssid-select');
  const status = document.getElementById('scan-status');

  btn.disabled = true;
  btn.innerHTML = '<span class="spinner-border spinner-border-sm"></span>';
  status.textContent = 'Buscando redes...';
  sel.innerHTML = '<option value="">Aguarde...</option>';

  try {
    const r = await fetch('/scan');
    const nets = await r.json();
    sel.innerHTML = '<option value="">-- Selecione a rede --</option>';
    nets.forEach(n => {
      const opt = document.createElement('option');
      opt.value = n.ssid;
      opt.textContent = `${n.ssid}  (${n.rssi} dBm)`;
      sel.appendChild(opt);
    });
    status.textContent = `${nets.length} rede(s) encontrada(s).`;
  } catch(e) {
    status.textContent = 'Erro ao buscar redes.';
    toast('Erro ao buscar redes WiFi.', 'danger');
  } finally {
    btn.disabled = false;
    btn.innerHTML = '<i class="bi bi-arrow-clockwise"></i>';
  }
});

// ============================================================
// MOSTRAR / OCULTAR SENHA
// ============================================================
document.getElementById('btn-show-pass').addEventListener('click', () => {
  const inp = document.getElementById('wifi-pass');
  const btn = document.getElementById('btn-show-pass');
  if (inp.type === 'password') {
    inp.type = 'text';
    btn.innerHTML = '<i class="bi bi-eye-slash"></i>';
  } else {
    inp.type = 'password';
    btn.innerHTML = '<i class="bi bi-eye"></i>';
  }
});

// ============================================================
// SALVAR CONFIGURAÇÃO
// ============================================================
document.getElementById('btn-save').addEventListener('click', async () => {
  const ssid = document.getElementById('ssid-select').value.trim();
  const pass = document.getElementById('wifi-pass').value;

  if (!ssid) { toast('Selecione ou informe uma rede WiFi.', 'warning'); return; }

  const btn = document.getElementById('btn-save');
  btn.disabled = true;
  btn.innerHTML = '<span class="spinner-border spinner-border-sm"></span> Salvando...';

  try {
    const r = await fetch('/save', {
      method: 'POST',
      headers: {'Content-Type':'application/json'},
      body: JSON.stringify({ ssid, password: pass })
    });
    const d = await r.json();
    if (d.ok) {
      toast('Configuração salva! O dispositivo irá reiniciar e conectar à rede.', 'success');
      setTimeout(() => window.location.reload(), 4000);
    } else {
      toast('Erro ao salvar: ' + (d.error || 'desconhecido'), 'danger');
    }
  } catch(e) {
    toast('Falha na comunicação com o dispositivo.', 'danger');
  } finally {
    btn.disabled = false;
    btn.innerHTML = '<i class="bi bi-floppy"></i> Salvar e Conectar';
  }
});

// ============================================================
// REINICIAR
// ============================================================
document.getElementById('btn-restart').addEventListener('click', async () => {
  if (!confirm('Reiniciar o dispositivo agora?')) return;
  await fetch('/restart', { method: 'POST' });
  toast('Dispositivo reiniciando...', 'info');
});

// ============================================================
// RESTAURAR PADRÃO
// ============================================================
document.getElementById('btn-factory').addEventListener('click', async () => {
  if (!confirm('Restaurar configurações de fábrica? Todas as configurações serão apagadas.')) return;
  const r = await fetch('/factory', { method: 'POST' });
  const d = await r.json();
  if (d.ok) {
    toast('Configurações restauradas! O dispositivo irá reiniciar.', 'warning');
    setTimeout(() => window.location.reload(), 3000);
  }
});

// ============================================================
// INICIALIZAÇÃO
// ============================================================
loadDeviceInfo();
</script>

<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js"></script>
</body>
</html>
)rawhtml";

// ============================================================
// CONSTRUTOR
// ============================================================

WebServerManager::WebServerManager(ConfigManager& cfg)
    : _server(80), _cfg(cfg), _running(false)
{
}

// ============================================================
// INICIALIZAÇÃO
// ============================================================

void WebServerManager::begin() {
    // Registra as rotas
    _server.on("/",        HTTP_GET,  [this]() { _handleRoot();    });
    _server.on("/info",    HTTP_GET,  [this]() { _handleInfo();    });
    _server.on("/scan",    HTTP_GET,  [this]() { _handleScan();    });
    _server.on("/save",    HTTP_POST, [this]() { _handleSave();    });
    _server.on("/restart", HTTP_POST, [this]() { _handleRestart(); });
    _server.on("/factory", HTTP_POST, [this]() { _handleReset();   });
    _server.onNotFound(               [this]() { _handleNotFound();});

    _server.begin();
    _running = true;
    Serial.println(F("[WebServer] Servidor HTTP iniciado na porta 80."));
}

void WebServerManager::handle() {
    if (_running) _server.handleClient();
}

void WebServerManager::stop() {
    if (_running) {
        _server.stop();
        _running = false;
        Serial.println(F("[WebServer] Servidor HTTP parado."));
    }
}

bool WebServerManager::isRunning() const { return _running; }

// ============================================================
// HANDLERS
// ============================================================

void WebServerManager::_handleRoot() {
    _server.send_P(200, "text/html", INDEX_HTML);
}

void WebServerManager::_handleInfo() {
    DynamicJsonDocument doc(256);
    doc["mac"]      = WiFi.macAddress();
    doc["ip"]       = WiFi.softAPIP().toString();
    doc["firmware"] = FIRMWARE_VERSION;

    char chipBuf[9];
    snprintf(chipBuf, sizeof(chipBuf), "%08X", ESP.getChipId());
    doc["chip_id"]  = chipBuf;
    doc["ssid"]     = _cfg.config.ssid;

    String out;
    serializeJson(doc, out);
    _server.send(200, F("application/json"), out);
}

void WebServerManager::_handleScan() {
    // Scan síncrono (bloqueante, mas rápido – aceitável no setup AP)
    int n = WiFi.scanNetworks();

    DynamicJsonDocument doc(1024);
    JsonArray arr = doc.to<JsonArray>();

    for (int i = 0; i < n; i++) {
        JsonObject net = arr.createNestedObject();
        net["ssid"]    = WiFi.SSID(i);
        net["rssi"]    = WiFi.RSSI(i);
        net["enc"]     = (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? 0 : 1;
    }

    WiFi.scanDelete();  // Libera memória do scan

    String out;
    serializeJson(doc, out);
    _server.send(200, F("application/json"), out);
}

void WebServerManager::_handleSave() {
    if (!_server.hasArg("plain")) {
        _server.send(400, F("application/json"), F("{\"ok\":false,\"error\":\"Payload ausente\"}"));
        return;
    }

    DynamicJsonDocument doc(256);
    DeserializationError err = deserializeJson(doc, _server.arg("plain"));
    if (err) {
        _server.send(400, F("application/json"), F("{\"ok\":false,\"error\":\"JSON inválido\"}"));
        return;
    }

    String ssid     = doc["ssid"]     | "";
    String password = doc["password"] | "";

    if (ssid.isEmpty()) {
        _server.send(400, F("application/json"), F("{\"ok\":false,\"error\":\"SSID obrigatório\"}"));
        return;
    }

    _cfg.config.ssid     = ssid;
    _cfg.config.password = password;

    if (!_cfg.save()) {
        _server.send(500, F("application/json"), F("{\"ok\":false,\"error\":\"Erro ao salvar\"}"));
        return;
    }

    _server.send(200, F("application/json"), F("{\"ok\":true}"));
    Serial.println(F("[WebServer] Credenciais WiFi salvas. Reiniciando..."));

    delay(1000);
    ESP.restart();
}

void WebServerManager::_handleRestart() {
    _server.send(200, F("application/json"), F("{\"ok\":true}"));
    Serial.println(F("[WebServer] Reinicialização solicitada pelo usuário."));
    delay(500);
    ESP.restart();
}

void WebServerManager::_handleReset() {
    _cfg.reset();
    _server.send(200, F("application/json"), F("{\"ok\":true}"));
    Serial.println(F("[WebServer] Reset de fábrica realizado. Reiniciando..."));
    delay(1000);
    ESP.restart();
}

void WebServerManager::_handleNotFound() {
    _server.send(404, F("text/plain"), F("404 - Not Found"));
}

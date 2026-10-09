# Kinetix — Especificações Técnicas de Hardware e Comunicação (Arduino UNO R4 WiFi)

Este documento detalha a adaptação da arquitetura **Kinetix** para o microcontrolador **Arduino UNO R4 WiFi**, utilizando um conjunto de sensores modulares para monitoramento contínuo da cadeia de suprimentos (temperatura, umidade, luminosidade, impactos mecânicos e inclinação).

---

## 1. Visão Geral do Hardware Adaptado

Para este protótipo, a medição espacial e ambiental foi configurada através de módulos discretos no **Arduino UNO R4 WiFi**:
* **Microcontrolador**: Arduino UNO R4 WiFi (Arquitetura Renesas RA4M1 com módulo ESP32-S3 dedicado para Wi-Fi/BLE e RTC integrado).
* **Módulo DHT11**: Leitura de temperatura e umidade relativa do ar.
* **Módulo LDR**: Detecção digital de incidência de luz (violação de lacre/abertura).
* **Módulo Sensor de Vibração**: Detecção de choques mecânicos e impactos em substituição ao acelerômetro.
* **Módulo Sensor de Inclinação (Mercúrio)**: Detecção de alteração brusca de orientação/posição da carga.
* **Display LCD 16x2 I2C**: Interface visual local de status ("OK", "ALERTA", "VIOLADO").

---

## 2. Mapeamento e Especificação de Pinos (Pinout)

A tabela abaixo resume a alocação de pinos no Arduino UNO R4 WiFi:

| Periférico / Sensor | Tipo de Sinal | Pino Arduino R4 | Descrição / Função |
| :--- | :--- | :--- | :--- |
| **Módulo Sensor de Inclinação** | Entrada Digital (Interrupt) | **Pino 2** | Identifica se a caixa tombou ou mudou de posição vertical. |
| **Módulo Sensor de Vibração** | Entrada Digital (Interrupt) | **Pino 3** | Captura picos de impacto ou trepidação severa no transporte. |
| **Módulo LDR** | Entrada Digital | **Pino 10** | Identifica presença de luz (abertura não autorizada/violação). |
| **Módulo DHT11** | Sinal Digital (1-Wire) | **Pino 11** | Envia dados seriais de temperatura e umidade. |
| **Display LCD 16x2 (I2C)** | Comunicação I2C (SDA) | **Pino SDA** | Linha de dados para o display LCD. |
| **Display LCD 16x2 (I2C)** | Comunicação I2C (SCL) | **Pino SCL** | Linha de clock para o display LCD. |
| **Alimentação dos Sensores** | VCC | **5V / 3.3V** | Barramento de energia para os módulos. |
| **GND** | Terra | **GND** | Ponto de referência comum para todos os sensores. |

---

## 3. Funcionamento Detalhado dos Sensores

### 3.1. Módulo DHT11 (Pino 11 - PWM/Digital)
* **Função no Kinetix**: Garantir a conformidade térmica e hídrica de produtos sensíveis (medicamentos, alimentos, eletrônicos).
* **Modo de Operação**: Transmite via barramento digital de fio único os valores calculados de temperatura (°C) e umidade (%).
* **Lógica de Evento**:
  * Se $Temp > Temp_{máx}$ ou $Temp < Temp_{mín}$, ativa flag de **Alerta Térmico**.
  * Atualiza as medições no LCD e no pacote de telemetria periodicamente (ex: a cada 2 a 5 segundos).

### 3.2. Módulo LDR com Comparador (Pino 10 - PWM/Digital)
* **Função no Kinetix**: Identificar invasão de luz no interior do contêiner ou embalagem (Lacre Violado).
* **Modo de Operação**: O módulo possui um potenciômetro de ajuste de sensibilidade acoplado a um comparador (LM393). Quando a luz ultrapassa o limiar, o pino digital transiciona de estado (HIGH/LOW).
* **Lógica de Evento**:
  * Estado normal (Caixa fechada): Sinal Escuro.
  * Transição (Caixa aberta): Sinal de Luz aciona flag instantâneo de **Violação de Lacre**.

### 3.3. Módulo Sensor de Vibração (Pino 3 - PWM/Digital com Interrupção)
* **Função no Kinetix**: Substituir a detecção de picos do acelerômetro MPU6050 para acusar quedas e choques mecânicos brutais.
* **Modo de Operação**: Contém um elemento elástico/mola interna. Quando submetido a um impacto, os contatos internos fecham momentaneamente, gerando pulso elétrico no Pino 3.
* **Lógica de Evento**:
  * Configurado via Interrupção Externa (`attachInterrupt` no Pino 3 em modo `RISING` ou `FALLING`).
  * Ao detectar impacto, incrementa o contador de impactos brutos e penaliza o **Score de Integridade** da carga.

### 3.4. Módulo Sensor de Inclinação com Mercúrio (Pino 2 - Digital com Interrupção)
* **Função no Kinetix**: Determinar se a carga perdeu a orientação correta ("Este lado para cima").
* **Modo de Operação**: Uma gota de mercúrio fecha ou abre o circuito interno conforme o ângulo em relação à gravidade.
* **Lógica de Evento**:
  * Configurado via Interrupção Externa (`attachInterrupt` no Pino 2).
  * Se a posição mudar por tempo superior a um *debounce* estipulado (evitando falsos positivos por pequenas vibrações), registra o status de **Carga Inclinada / Tombada**.

### 3.5. Display LCD 16x2 I2C (Pinos SDA / SCL)
* **Função no Kinetix**: Fornecer feedback local direto para o operador/entregador sem a necessidade imediata do aplicativo móvel.
* **Exibição de Dados**:
  * **Linha 1**: Status da Carga (`STATUS: OK`, `STATUS: ALERTA`, `STATUS: VIOLADO`) e Wi-Fi (`[WIFI ON]`/`[OFF]`).
  * **Linha 2**: Alternância entre Temperatura/Umidade (`T:24C U:60%`) e Contagem de Eventos (`Imp: 2 | Inc: SIM`).

---

## 4. Funcionamento do Arduino UNO R4 WiFi e Integração com a Arquitetura Kinetix

Abaixo está o fluxo completo de funcionamento interno do Arduino R4 e a sua integração com a nuvem **Wegnology** e o ecossistema Kinetix.

```
+-----------------------------------------------------------------------+
|                       CAMADA EDGE (ARDUINO R4)                        |
|                                                                       |
|  [DHT11]  [LDR]  [Vibração]  [Inclinação]                             |
|     │       │        │            │                                   |
|     ▼       ▼        ▼            ▼                                   |
|  ┌──────────────────────────────────────┐     ┌────────────────────┐  |
|  │      Arduino UNO R4 WiFi Core        │───> │ Display LCD 16x2   │  |
|  │  - Pré-processamento & Debounce     │     └────────────────────┘  |
|  │  - Cálculo prévio do Score local     │                             |
|  │  - JSON Payload Formatting           │                             |
|  └──────────────────────────────────────┘                             |
|                     │                                                 |
|                     ▼ (Wi-Fi Native / WiFiS3)                         |
|              [Protocolo MQTT]                                         |
+---------------------+-------------------------------------------------+
                      │
                      ▼
+-----------------------------------------------------------------------+
|             CAMADA DE ORQUESTRAÇÃO E IIOT (WEGNOLOGY)                 |
|                                                                       |
|  - MQTT Broker / Ingestão de Dados                                   |
|  - Workflows Low-Code & Algoritmo Oficial de Score de Integridade     |
|  - Disparo de Webhooks de Emergência                                  |
+---------------------+-------------------------------------------------+
                      │
                      ▼
+-----------------------------------------------------------------------+
|                    BACK-END & FRONT-END MOBILE                        |
|                                                                       |
|  - API REST + Banco de Dados Relacional                               │
|  - App Flutter: Exibição do Status, Alertas e Leitura via QR Code     │
+-----------------------------------------------------------------------+
```

### 4.1. Ciclo de Processamento Embarcado (Edge Computing)

1. **Inicialização (`setup`)**:
   * Inicialização do barramento I2C e do Display LCD.
   * Configuração das entradas com resistores de Pull-Up/Pull-Down e vinculação das interrupções nos pinos 2 (Inclinação) e 3 (Vibração).
   * Conexão do módulo de rede com a biblioteca `WiFiS3` ao ponto de acesso Wi-Fi local.
   * Conexão ao Broker MQTT da **Wegnology**.

2. **Loop de Execução (`loop`)**:
   * **Varredura Contínua**: Leitura do DHT11 e do estado do pino 10 (LDR).
   * **Atendimento às Interrupções**: As alterações nos pinos de vibração (3) e inclinação (2) acionam funções assíncronas que registram instantaneamente a ocorrência do evento e o carimbo de tempo (*timestamp* via RTC interno do R4).
   * **Atualização da Interface Local**: Atualiza as mensagens no LCD 16x2.
   * **Publicação Telemétrica**: A cada intervalo configurado (ex: 3 segundos), gera um payload JSON e envia via MQTT para o tópico configurado na Wegnology.

### 4.2. Formato do Payload JSON Enviado para a Wegnology

O Arduino R4 formata os dados brutos e eventos da seguinte forma:

```json
{
  "device_id": "kinetix_ard_r4_01",
  "data": {
    "temperatura": 24.5,
    "umidade": 58.0,
    "lacre_violado": false,
    "impacto_detectado": true,
    "qtd_impactos": 3,
    "carga_inclinada": false,
    "status_geral": "ALERTA"
  }
}
```

### 4.3. Fluxo de Comunicação com o Restante do Projeto

1. **Ingestão MQTT na Wegnology**:
   * O Arduino R4 publica a telemetria no *Data Input* MQTT da Wegnology.
   * Caso o sinal de Wi-Fi caia temporariamente, o Arduino pode utilizar uma estrutura de *Buffer* na memória flash/SRAM para reenviar a lista de ocorrências assim que a conexão restabelecer.

2. **Execução de Workflows e Algoritmo de Score**:
   * A plataforma **Wegnology** recebe o JSON, processa as regras de negócio e aplica as penalidades no **Score de Integridade** (0 a 100%):
     * Violação de Lacre (LDR): Penalização severa imediata.
     * Impactos (Vibração): Penalização proporcional à contagem de pulsos.
     * Inclinação (Mercúrio): Penalização por desvio de posição.
     * Desvio Térmico (DHT11): Penalização contínua por tempo de exposição fora do limite.

3. **Distribuição para Back-End e Mobile (Flutter)**:
   * A Wegnology dispara um **Webhook** para a API Back-End para persistência no Banco de Dados.
   * O **Aplicativo Flutter** consulta o status via API REST / WebSocket da Wegnology e atualiza o estado da entrega para o cliente final.

---

## 5. Recomendações e Considerações de Implementação

1. **Tratamento de Ruídos e Debounce**:
   * Sensores de vibração mecânica e mercúrio sofrem com *bouncing* (múltiplos acionamentos falsos em milissegundos). Deve-se aplicar um filtro de *debounce* via software no código do Arduino (ex: ignorar novos acionamentos dentro de uma janela de 200ms).
2. **Bibliotecas Recomendadas para o Arduino UNO R4 WiFi**:
   * `<WiFiS3.h>`: Conectividade sem fio nativa do Arduino R4.
   * `<PubSubClient.h>` ou `<ArduinoMqttClient.h>`: Gerenciamento da comunicação MQTT.
   * `<DHT.h>`: Leitura do sensor DHT11.
   * `<LiquidCrystal_I2C.h>`: Controle do LCD 16x2 via I2C.
   * `<RTC.h>`: RTC nativo do Arduino UNO R4 para marcação temporal interna.
3. **Alimentação**:
   * Garantir alimentação estável de 5V via USB-C ou fonte externa pino VIN para evitar reinicializações do módulo Wi-Fi durante surtos de corrente.
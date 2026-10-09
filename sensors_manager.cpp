#include "sensors_manager.h"
#include "config.h"
#include <DHT.h>

// =============================================================================
// VARIÁVEIS INTERNAS E OBJETOS DE HARDWARE
// =============================================================================
static DHT dht(PIN_DHT, DHT_TYPE);
static SensorData currentData;

// Variáveis voláteis controladas pelas rotinas de interrupção (ISRs)
static volatile unsigned long lastVibInterruptTime = 0;
static volatile unsigned long lastTiltInterruptTime = 0;
static volatile int volatileImpactCount = 0;
static volatile bool volatileImpactDetected = false;
static volatile bool volatileTiltFlag = false;

// Controle de tempo para leitura do DHT11 (mínimo de 1.5 a 2 segundos)
static unsigned long lastDHTReadTime = 0;
static const unsigned long DHT_SAMPLE_INTERVAL_MS = 2000;

// =============================================================================
// ROTINAS DE INTERRUPÇÃO (ISRs) COM DEBOUNCE POR SOFTWARE
// =============================================================================

// ISR do Sensor de Vibração (Pino 3)
// Captura picos de choque mecânico e incrementa o contador
void handleVibrationISR() {
    unsigned long currentTime = millis();
    if (currentTime - lastVibInterruptTime >= DEBOUNCE_TIME_MS) {
        lastVibInterruptTime = currentTime;
        volatileImpactCount++;
        volatileImpactDetected = true;
    }
}

// ISR do Sensor de Inclinação (Pino 2)
// Captura variações bruscas de orientação física do pacote
void handleTiltISR() {
    unsigned long currentTime = millis();
    if (currentTime - lastTiltInterruptTime >= DEBOUNCE_TIME_MS) {
        lastTiltInterruptTime = currentTime;
        volatileTiltFlag = true;
    }
}

// =============================================================================
// INICIALIZAÇÃO DOS SENSORES
// =============================================================================
void sensorsInit() {
    // Inicializa valores padrão da estrutura de dados
    currentData.temperatura = 22.0f;
    currentData.umidade = 50.0f;
    currentData.lacre_violado = false;
    currentData.impacto_detectado = false;
    currentData.qtd_impactos = 0;
    currentData.carga_inclinada = false;
    strcpy(currentData.status_geral, "OK");

    // Configura entradas digitais com INPUT_PULLUP onde aplicável para estabilidade
    pinMode(PIN_LDR, INPUT);
    pinMode(PIN_VIBRATION, INPUT_PULLUP);
    pinMode(PIN_TILT, INPUT_PULLUP);

    // Inicializa o barramento 1-Wire do DHT11
    dht.begin();

    // Vincula interrupções externas aos pinos 2 e 3
    // Arduino Uno R4 WiFi suporta interrupções em praticamente todos os pinos digitais
    attachInterrupt(digitalPinToInterrupt(PIN_VIBRATION), handleVibrationISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_TILT), handleTiltISR, CHANGE);

    // Primeira leitura do estado atual de inclinação
    currentData.carga_inclinada = (digitalRead(PIN_TILT) == HIGH);
}

// =============================================================================
// LEITURA PERIÓDICA E ATUALIZAÇÃO DO STATUS
// =============================================================================
void sensorsUpdate() {
    unsigned long now = millis();

    // 1. Leitura do Sensor DHT11 com taxa de amostragem segura
    if (now - lastDHTReadTime >= DHT_SAMPLE_INTERVAL_MS) {
        lastDHTReadTime = now;
        float t = dht.readTemperature();
        float h = dht.readHumidity();

        // Só atualiza se o valor não for NaN (evita ruídos na linha de dados)
        if (!isnan(t)) {
            currentData.temperatura = t;
        }
        if (!isnan(h)) {
            currentData.umidade = h;
        }
    }

    // 2. Leitura do LDR (Detecção de Abertura não Autorizada)
    int ldrState = digitalRead(PIN_LDR);
    if (ldrState == LDR_LIGHT_TRIGGER) {
        // Uma vez detectada a abertura da caixa, mantém o flag de violação ativo
        currentData.lacre_violado = true;
    }

    // 3. Sincronização atômica das variáveis de interrupção
    noInterrupts();
    currentData.qtd_impactos = volatileImpactCount;
    currentData.impacto_detectado = volatileImpactDetected;
    bool tiltTriggered = volatileTiltFlag;
    volatileTiltFlag = false; // consome o evento de transição
    interrupts();

    // Verifica a inclinação pelo pino em tempo real e pelo evento da interrupção
    if (digitalRead(PIN_TILT) == HIGH || tiltTriggered) {
        currentData.carga_inclinada = true;
    } else {
        currentData.carga_inclinada = false;
    }

    // 4. Avaliação do Status Geral do Pacote
    // Lógica de prioridade: VIOLADO > ALERTA > OK
    if (currentData.lacre_violado) {
        strcpy(currentData.status_geral, "VIOLADO");
    } else if (currentData.carga_inclinada || 
               currentData.qtd_impactos > 0 || 
               currentData.temperatura > TEMP_MAX_LIMIT || 
               currentData.temperatura < TEMP_MIN_LIMIT) {
        strcpy(currentData.status_geral, "ALERTA");
    } else {
        strcpy(currentData.status_geral, "OK");
    }
}

// Retorna os dados consolidados
const SensorData& sensorsGetData() {
    return currentData;
}

// Reseta o indicador de impacto momentâneo
void sensorsClearRecentImpact() {
    noInterrupts();
    volatileImpactDetected = false;
    currentData.impacto_detectado = false;
    interrupts();
}

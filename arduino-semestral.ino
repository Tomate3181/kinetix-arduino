/**
 * =============================================================================
 * PROJETO KINETIX - MONITORAMENTO DE CARGA E CADEIA DE SUPRIMENTOS
 * PLATAFORMA: Arduino UNO R4 WiFi
 * =============================================================================
 * 
 * ARQUIVO PRINCIPAL DO SKETCH: arduino-semestral.ino
 * 
 * Este arquivo orquestra os módulos do sistema:
 *  - Gerenciador de Sensores (DHT11, LDR, Vibração, Inclinação e Interrupções)
 *  - Gerenciador do Display LCD 16x2 I2C (Alternância de telas sem piscar)
 *  - Gerenciador de Conectividade (Wi-Fi S3 nativo e Protocolo MQTT / Wegnology)
 * 
 * Todas as temporizações são controladas de forma assíncrona/não-bloqueante via
 * millis(), garantindo alta precisão no atendimento de interrupções e debounce.
 */

#include "config.h"
#include "sensors_manager.h"
#include "display_manager.h"
#include "network_manager.h"

// =============================================================================
// VARIÁVEIS DE TEMPORIZAÇÃO (CRONÔMETROS NÃO-BLOQUEANTES)
// =============================================================================
static unsigned long lastSensorsUpdateTime   = 0;
static unsigned long lastDisplayUpdateTime   = 0;
static unsigned long lastTelemetryPublishTime = 0;

// Intervalos de execução das tarefas
static const unsigned long SENSORS_INTERVAL_MS = 200;  // Atualiza lógica dos sensores a cada 200ms
static const unsigned long DISPLAY_INTERVAL_MS = 250;  // Atualiza dados no LCD a cada 250ms

// =============================================================================
// FUNÇÃO DE CONFIGURAÇÃO (SETUP)
// =============================================================================
void setup() {
    // 1. Inicializa comunicação serial para telemetria local e depuração
    Serial.begin(SERIAL_BAUD_RATE);
    delay(500); // Aguarda estabilização da porta serial nativa do R4

    Serial.println(F("\n=============================================="));
    Serial.println(F("    INICIANDO SISTEMA KINETIX (UNO R4 WIFI)   "));
    Serial.println(F("=============================================="));

    // 2. Inicializa o Display LCD e apresenta a tela de inicialização
    Serial.println(F("[INIT] Inicializando Display LCD 16x2 I2C..."));
    displayInit();
    displayShowBootMessage();

    // 3. Inicializa os Sensores, Pinos e Interrupções Externas
    Serial.println(F("[INIT] Configurando Sensores e Interrupcoes..."));
    sensorsInit();

    // 4. Inicializa o Subsistema Wi-Fi (WiFiS3) e MQTT
    Serial.println(F("[INIT] Inicializando Modulo de Rede..."));
    networkInit();

    // Pequena pausa para exibição da tela de boas-vindas no LCD
    delay(1200);

    Serial.println(F("[INIT] Setup concluido com sucesso. Iniciando operacao.\n"));
}

// =============================================================================
// LOOP PRINCIPAL (EXECUÇÃO CÍCLICA NÃO-BLOQUEANTE)
// =============================================================================
void loop() {
    unsigned long currentMillis = millis();

    // -------------------------------------------------------------------------
    // 1. MÁQUINA DE ESTADOS DA CONEXÃO DE REDE
    // Processa reconexões de Wi-Fi, reconexões de MQTT e mantém o keep-alive
    // -------------------------------------------------------------------------
    networkLoop();

    // -------------------------------------------------------------------------
    // 2. ATUALIZAÇÃO DA LEITURA DOS SENSORES
    // -------------------------------------------------------------------------
    if (currentMillis - lastSensorsUpdateTime >= SENSORS_INTERVAL_MS) {
        lastSensorsUpdateTime = currentMillis;
        sensorsUpdate();
    }

    // -------------------------------------------------------------------------
    // 3. ATUALIZAÇÃO DO DISPLAY LCD LOCAL
    // -------------------------------------------------------------------------
    if (currentMillis - lastDisplayUpdateTime >= DISPLAY_INTERVAL_MS) {
        lastDisplayUpdateTime = currentMillis;
        const SensorData& data = sensorsGetData();
        bool isWifiOk = networkIsWifiConnected();
        displayUpdate(data, isWifiOk);
    }

    // -------------------------------------------------------------------------
    // 4. ENVIO PERIÓDICO DA TELEMETRIA PARA A WEGNOLOGY / BROKER MQTT
    // -------------------------------------------------------------------------
    if (currentMillis - lastTelemetryPublishTime >= TELEMETRY_INTERVAL_MS) {
        lastTelemetryPublishTime = currentMillis;
        
        const SensorData& data = sensorsGetData();
        
        // Publica dados via MQTT em formato JSON
        bool sent = networkPublishTelemetry(data);
        
        if (sent) {
            // Limpa o indicador de impacto recente (a contagem total é preservada)
            sensorsClearRecentImpact();
        }
    }
}

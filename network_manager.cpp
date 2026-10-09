#include "network_manager.h"
#include "config.h"
#include <WiFiS3.h>
#include <PubSubClient.h>

// =============================================================================
// OBJETOS DE REDE E CLIENTE MQTT
// =============================================================================
static WiFiClient wifiClient;
static PubSubClient mqttClient(wifiClient);

// Temporizadores de reconexão não-bloqueantes
static unsigned long lastWifiRetry = 0;
static unsigned long lastMqttRetry = 0;
static const unsigned long WIFI_RETRY_INTERVAL_MS = 10000; // Tenta reconectar Wi-Fi a cada 10s
static const unsigned long MQTT_RETRY_INTERVAL_MS = 5000;  // Tenta reconectar MQTT a cada 5s

// =============================================================================
// INICIALIZAÇÃO DE REDE
// =============================================================================
void networkInit() {
    // Configura tamanho do buffer MQTT para suportar payloads JSON maiores que 128 bytes
    mqttClient.setBufferSize(512);
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);

    // Verifica presença do módulo de rádio ESP32-S3 no Arduino UNO R4 WiFi
    if (WiFi.status() == WL_NO_MODULE) {
        Serial.println(F("[REDE] ERRO: Modulo WiFi S3 nao detectado!"));
        return;
    }

    Serial.print(F("[REDE] Conectando ao Wi-Fi: "));
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

// =============================================================================
// MÁQUINA DE ESTADOS / LOOP DE CONEXÃO NÃO-BLOQUEANTE
// =============================================================================
void networkLoop() {
    unsigned long now = millis();

    // 1. Manutenção da Conexão Wi-Fi
    if (WiFi.status() != WL_CONNECTED) {
        if (now - lastWifiRetry >= WIFI_RETRY_INTERVAL_MS) {
            lastWifiRetry = now;
            Serial.println(F("[REDE] Wi-Fi desconectado. Tentando reconexao..."));
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        }
        return; // Aguarda Wi-Fi conectar antes de tentar o Broker MQTT
    }

    // 2. Manutenção da Conexão MQTT
    if (!mqttClient.connected()) {
        if (now - lastMqttRetry >= MQTT_RETRY_INTERVAL_MS) {
            lastMqttRetry = now;
            Serial.print(F("[MQTT] Conectando ao Broker "));
            Serial.print(MQTT_BROKER);
            Serial.println(F("..."));

            // Conexão com Client ID, Username e Senha (suporta Wegnology Access Keys)
            bool connected = false;
            if (strlen(MQTT_USERNAME) > 0) {
                connected = mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);
            } else {
                connected = mqttClient.connect(MQTT_CLIENT_ID);
            }

            if (connected) {
                Serial.println(F("[MQTT] Conectado com sucesso!"));
            } else {
                Serial.print(F("[MQTT] Falha na conexao. Codigo de erro: "));
                Serial.println(mqttClient.state());
            }
        }
    } else {
        // Processa mensagens recebidas e pacotes keep-alive
        mqttClient.loop();
    }
}

// =============================================================================
// VERIFICADORES DE STATUS
// =============================================================================
bool networkIsWifiConnected() {
    return (WiFi.status() == WL_CONNECTED);
}

bool networkIsMqttConnected() {
    return mqttClient.connected();
}

// =============================================================================
// ENVIO DA TELEMETRIA VIA MQTT (PAYLOAD FORMATADO SEÇÃO 4.2)
// =============================================================================
bool networkPublishTelemetry(const SensorData& data) {
    if (!mqttClient.connected()) {
        Serial.println(F("[MQTT] Nao publicado: Broker desconectado."));
        return false;
    }

    // Criação do buffer e montagem do JSON estrito exigido pelo backend Kinetix / Wegnology
    char payload[384];
    snprintf(payload, sizeof(payload),
        "{\n"
        "  \"device_id\": \"%s\",\n"
        "  \"data\": {\n"
        "    \"temperatura\": %.1f,\n"
        "    \"umidade\": %.1f,\n"
        "    \"lacre_violado\": %s,\n"
        "    \"impacto_detectado\": %s,\n"
        "    \"qtd_impactos\": %d,\n"
        "    \"carga_inclinada\": %s,\n"
        "    \"status_geral\": \"%s\"\n"
        "  }\n"
        "}",
        DEVICE_ID,
        data.temperatura,
        data.umidade,
        data.lacre_violado ? "true" : "false",
        data.impacto_detectado ? "true" : "false",
        data.qtd_impactos,
        data.carga_inclinada ? "true" : "false",
        data.status_geral
    );

    Serial.println(F("[MQTT] Publicando Telemetria:"));
    Serial.println(payload);

    bool ok = mqttClient.publish(MQTT_PUB_TOPIC, payload);
    if (ok) {
        Serial.println(F("[MQTT] Pacote enviado com sucesso!"));
    } else {
        Serial.println(F("[MQTT] Falha ao enviar pacote!"));
    }

    return ok;
}

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =============================================================================
// KINETIX - ARDUINO UNO R4 WIFI CONFIGURAÇÃO GERAL
// =============================================================================

// -----------------------------------------------------------------------------
// 1. MAPEAMENTO DE PINOS (PINOUT CONFORME SEÇÃO 2 DA ESPECIFICAÇÃO)
// -----------------------------------------------------------------------------
#define PIN_TILT        2   // Pino 2: Sensor de Inclinação (Mercúrio) - Interrupção
#define PIN_VIBRATION   3   // Pino 3: Sensor de Vibração - Interrupção
#define PIN_LDR         10  // Pino 10: Sensor LDR (Comparador LM393) - Digital
#define PIN_DHT         11  // Pino 11: Sensor DHT11 (1-Wire) - Digital

// Pinos I2C do Display LCD 16x2:
// No Arduino UNO R4 WiFi, SDA e SCL possuem pinos dedicados no conector superior
// ou compartilham A4 (SDA) e A5 (SCL).

// -----------------------------------------------------------------------------
// 2. CONFIGURAÇÕES DO DISPLAY LCD 16x2 I2C
// -----------------------------------------------------------------------------
#define LCD_I2C_ADDRESS     0x27  // Endereço padrão comum (ou 0x3F)
#define LCD_COLUMNS         16
#define LCD_ROWS            2
#define LCD_CYCLE_TIME_MS   2500  // Alterna a tela inferior a cada 2.5 segundos

// -----------------------------------------------------------------------------
// 3. CONFIGURAÇÕES DOS SENSORES E LIMITES TÉRMICOS
// -----------------------------------------------------------------------------
#define DHT_TYPE            DHT11 // Tipo do sensor DHT

// Limites de Temperatura para Disparo de Alerta (°C)
#define TEMP_MIN_LIMIT      15.0f // Temperatura mínima aceitável
#define TEMP_MAX_LIMIT      30.0f // Temperatura máxima aceitável

// Configurações do LDR (Lacre da embalagem)
// A maioria dos módulos com LM393 coloca saída em HIGH (ou LOW) quando há luz.
// Defina como HIGH se o módulo enviar HIGH ao receber luz:
#define LDR_LIGHT_TRIGGER   HIGH

// Janela de Debounce para Sensores Mecânicos (ms)
#define DEBOUNCE_TIME_MS    200

// -----------------------------------------------------------------------------
// 4. CONFIGURAÇÕES DE REDE (WIFI & MQTT / WEGNOLOGY)
// -----------------------------------------------------------------------------
// Credenciais da rede Wi-Fi local
#define WIFI_SSID           "SUA_REDE_WIFI"
#define WIFI_PASSWORD       "SUA_SENHA_WIFI"

// Credenciais do Broker MQTT (Wegnology ou Broker genérico)
#define MQTT_BROKER         "broker.app.wnology.io" // Servidor Wegnology
#define MQTT_PORT           1883                    // Porta padrão não-TLS
#define MQTT_CLIENT_ID      "kinetix_ard_r4_01"
#define MQTT_USERNAME       "SEU_ACCESS_KEY"        // Wegnology Access Key
#define MQTT_PASSWORD       "SEU_ACCESS_SECRET"     // Wegnology Access Secret

// Tópico MQTT para envio de telemetria (Data Ingestion Wegnology)
// Exemplo Wegnology: "wnology/<DEVICE_ID>/state"
#define MQTT_PUB_TOPIC      "wnology/kinetix_ard_r4_01/state"

// Intervalo de publicação de telemetria periódica (ms)
#define TELEMETRY_INTERVAL_MS 3000

// -----------------------------------------------------------------------------
// 5. IDENTIFICADOR ÚNICO DO DISPOSITIVO
// -----------------------------------------------------------------------------
#define DEVICE_ID           "kinetix_ard_r4_01"

// -----------------------------------------------------------------------------
// 6. VELOCIDADE DA SERIAL (DEBUG)
// -----------------------------------------------------------------------------
#define SERIAL_BAUD_RATE    115200

#endif // CONFIG_H

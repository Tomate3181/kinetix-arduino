#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include "sensors_manager.h"

// =============================================================================
// FUNÇÕES PÚBLICAS DO GERENCIADOR DE REDE (WIFI & MQTT)
// =============================================================================

// Inicializa a conexão Wi-Fi (WiFiS3) e configura o cliente MQTT
void networkInit();

// Mantém a máquina de estados de rede (reconectividade não-bloqueante e mqttClient.loop)
void networkLoop();

// Retorna se o Wi-Fi está atualmente conectado
bool networkIsWifiConnected();

// Retorna se o cliente MQTT está atualmente conectado ao Broker
bool networkIsMqttConnected();

// Constrói o payload JSON e publica no tópico MQTT configurado (Wegnology Data Input)
bool networkPublishTelemetry(const SensorData& data);

#endif // NETWORK_MANAGER_H

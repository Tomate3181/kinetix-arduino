#ifndef SENSORS_MANAGER_H
#define SENSORS_MANAGER_H

#include <Arduino.h>

// =============================================================================
// ESTRUTURA DE DADOS DOS SENSORES (TELEMETRIA KINETIX)
// Corresponde exatamente ao payload JSON exigido pela Wegnology
// =============================================================================
struct SensorData {
    float temperatura;          // Temperatura atual em °C
    float umidade;              // Umidade relativa do ar em %
    bool  lacre_violado;        // True se houver incidência de luz no LDR
    bool  impacto_detectado;    // True se ocorreu impacto recente
    int   qtd_impactos;         // Contador acumulado de impactos/choques
    bool  carga_inclinada;      // True se a carga estiver fora da posição vertical
    char  status_geral[10];     // "OK", "ALERTA" ou "VIOLADO"
};

// =============================================================================
// FUNÇÕES PÚBLICAS DO MÓDULO DE SENSORES
// =============================================================================

// Inicializa os pinos, interrupções e o sensor DHT11
void sensorsInit();

// Atualiza as leituras periódicas (DHT11 e LDR) e reavalia o status geral
void sensorsUpdate();

// Retorna a estrutura atualizada com todos os dados dos sensores
const SensorData& sensorsGetData();

// Reseta o sinalizador de impacto recente (mantém a quantidade acumulada)
void sensorsClearRecentImpact();

#endif // SENSORS_MANAGER_H

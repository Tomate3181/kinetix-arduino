#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include "sensors_manager.h"

// =============================================================================
// FUNÇÕES PÚBLICAS DO GERENCIADOR DE DISPLAY LCD 16X2 I2C
// =============================================================================

// Inicializa a interface I2C, display e backlight
void displayInit();

// Exibe tela inicial de inicialização/boot
void displayShowBootMessage();

// Atualiza o display periodicamente com base nos dados dos sensores e status da rede
// Gerencia a alternância automática entre a Tela 1 (T/U) e a Tela 2 (Impactos/Inclinação)
void displayUpdate(const SensorData& data, bool wifiConnected);

// Exibe uma mensagem customizada estática de 2 linhas
void displayShowCustom(const char* line1, const char* line2);

#endif // DISPLAY_MANAGER_H

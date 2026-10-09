#include "display_manager.h"
#include "config.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// =============================================================================
// INSTÂNCIA DO LCD E VARIÁVEIS DE ESTADO
// =============================================================================
static LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS);

static unsigned long lastScreenSwitchTime = 0;
static bool currentScreenIsAlt = false;

// =============================================================================
// FUNÇÃO AUXILIAR: ESCREVE LINHA COM PREENCHIMENTO EXATO DE 16 CARACTERES
// Evita o uso de lcd.clear(), prevenindo flickering (efeito de piscar do LCD)
// =============================================================================
static void lcdPrintPadded(uint8_t row, const char* text) {
    char buffer[17];
    snprintf(buffer, sizeof(buffer), "%-16.16s", text);
    lcd.setCursor(0, row);
    lcd.print(buffer);
}

// =============================================================================
// INICIALIZAÇÃO DO DISPLAY LCD
// =============================================================================
void displayInit() {
    Wire.begin();
    lcd.init();
    lcd.backlight();
    lcd.clear();
}

// =============================================================================
// TELA DE BOOT / INICIALIZAÇÃO
// =============================================================================
void displayShowBootMessage() {
    lcdPrintPadded(0, "KINETIX CARGO");
    lcdPrintPadded(1, "INICIALIZANDO...");
}

// =============================================================================
// ATUALIZAÇÃO PERIÓDICA DO DISPLAY
// =============================================================================
void displayUpdate(const SensorData& data, bool wifiConnected) {
    unsigned long now = millis();

    // Alterna a segunda linha a cada LCD_CYCLE_TIME_MS
    if (now - lastScreenSwitchTime >= LCD_CYCLE_TIME_MS) {
        lastScreenSwitchTime = now;
        currentScreenIsAlt = !currentScreenIsAlt;
    }

    // -------------------------------------------------------------------------
    // LINHA 1: STATUS DA CARGA + STATUS DO WI-FI
    // Ex: "OK       [W:ON]" ou "ALERTA   [W:OFF]" ou "VIOLADO  [W:ON]"
    // -------------------------------------------------------------------------
    char line1[20];
    const char* wifiTag = wifiConnected ? "ON" : "--";
    snprintf(line1, sizeof(line1), "%-8s [W:%s]", data.status_geral, wifiTag);
    lcdPrintPadded(0, line1);

    // -------------------------------------------------------------------------
    // LINHA 2: TELA DINÂMICA
    // Tela A: Temperatura e Umidade (ex: "T:24.5C  U:58%")
    // Tela B: Impactos e Inclinação (ex: "Imp:3 | Inc:SIM")
    // -------------------------------------------------------------------------
    char line2[20];
    if (!currentScreenIsAlt) {
        // Exibição Ambiental
        snprintf(line2, sizeof(line2), "T:%.1fC  U:%.0f%%", data.temperatura, data.umidade);
    } else {
        // Exibição Física / Mecânica
        const char* tiltStr = data.carga_inclinada ? "SIM" : "NAO";
        snprintf(line2, sizeof(line2), "Imp:%d | Inc:%s", data.qtd_impactos, tiltStr);
    }
    lcdPrintPadded(1, line2);
}

// =============================================================================
// EXIBIÇÃO DE MENSAGEM CUSTOMIZADA
// =============================================================================
void displayShowCustom(const char* line1, const char* line2) {
    lcdPrintPadded(0, line1);
    lcdPrintPadded(1, line2);
}

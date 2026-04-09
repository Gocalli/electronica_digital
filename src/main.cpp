#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fuzzy.h>
#include "config.h"

// --- OBJETOS GLOBALES ---
Adafruit_BME280 bme;
RTC_DS3231 rtc;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Fuzzy *fuzzy = new Fuzzy();

// --- VARIABLES GLOBALES ---
unsigned long lastMillis = 0;
float tempAire, humAire, humSuelo;
int tiempoRiego = 0;
DateTime now;

#if DEBUG_MODE
DateTime virtualTime(2026, 4, 7, 10, 0, 0); // Empieza a las 10:00 AM (Día soleado)
#endif

// --- PROTOTIPOS DE FUNCIONES ---
void leerSensores();
void evaluarLogica();
void guardarSD();
void activarBomba(int segundos);
void actualizarOLED(String estado);
void setupFuzzy();

void setup() {
    Serial.begin(115200);
    
    if (DEBUG_MODE) {
        Serial.println("--- MODO DEBUG (PEREIRA/LECHUGA) ACTIVADO ---");
    } else {
        Wire.begin(I2C_SDA, I2C_SCL);
        if (!bme.begin(0x76)) Serial.println("Error: BME280");
        if (!rtc.begin()) Serial.println("Error: RTC");
        if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) Serial.println("Error: OLED");
        if (!SD.begin(PIN_SD_CS)) Serial.println("Error: SD");
    }

    pinMode(PIN_RELE_BOMBA, OUTPUT);
    digitalWrite(PIN_RELE_BOMBA, LOW);

    setupFuzzy();
    Serial.println("Sistema configurado para optimización de Lechuga.");
}

void loop() {
    if (millis() - lastMillis >= INTERVALO_LECTURA || lastMillis == 0) {
        lastMillis = millis();
        
        leerSensores();
        evaluarLogica();
        guardarSD();
        
        if (tiempoRiego > 0) {
            actualizarOLED("REGANDO (PULSO)");
            activarBomba(tiempoRiego);
        } else {
            actualizarOLED("MONITOREO...");
        }
    }
}

void leerSensores() {
    if (DEBUG_MODE) {
        // Simulación de clima de Pereira (Caluroso al mediodía)
        tempAire = random(140, 280) / 10.0; // 14.0 - 28.0 C
        humAire = random(60, 85);           // 60% - 85%
        humSuelo = random(45, 75);          // 45% - 75%
        
        virtualTime = virtualTime + TimeSpan(0, 0, SIMULAR_AVANCE_TIEMPO, 0);
        now = virtualTime;

        Serial.print("\n[DEBUG] --- Análisis Pereira ---");
        Serial.print("\nHora: "); Serial.print(now.timestamp(DateTime::TIMESTAMP_TIME));
        Serial.print(" | Temp: "); Serial.print(tempAire); Serial.print("C");
        Serial.print(" | HR Aire: "); Serial.print(humAire); Serial.print("%");
        Serial.print(" | Hum.Suelo: "); Serial.print(humSuelo); Serial.println("%");
    } else {
        long suma = 0;
        for(int i = 0; i < MUESTRAS_PROMEDIO; i++) {
            suma += analogRead(PIN_HUMEDAD_SUELO);
            delay(100);
        }
        humSuelo = map(suma / MUESTRAS_PROMEDIO, 4095, 0, 0, 100);
        tempAire = bme.readTemperature();
        humAire = bme.readHumidity();
        now = rtc.now();
    }
}

void evaluarLogica() {
    // 1. REGLA DE PREVENCIÓN FÚNGICA (Nocturna)
    int horaActual = now.hour();
    if (horaActual >= HORA_INICIO_PROHIBIDO || horaActual < HORA_FIN_PROHIBIDO) {
        tiempoRiego = 0;
        if (DEBUG_MODE) Serial.println("[INFO] Bloqueo Nocturno: Evitando humedad en la hoja.");
        return;
    }

    // 2. REGLA DE SEGURIDAD (Humedad de Aire crítica para Botrytis)
    if (humAire > 80.0) {
        tiempoRiego = 0;
        if (DEBUG_MODE) Serial.println("[ALERTA] HR Aire > 80%: Riesgo de hongos. Riego suspendido.");
        return;
    }

    // 3. MOTOR DE INFERENCIA DIFUSA (Pereira/Lechuga)
    fuzzy->setInput(1, tempAire);
    fuzzy->setInput(2, humSuelo);
    fuzzy->fuzzify();
    tiempoRiego = fuzzy->defuzzify(1);

    if (DEBUG_MODE) {
        Serial.print("[INFO] Tiempo de riego calculado: ");
        Serial.print(tiempoRiego); Serial.println(" segundos.");
    }
}

void activarBomba(int segundos) {
    if (DEBUG_MODE) {
        Serial.print("[BOMBA] >> Pulsando agua por "); Serial.print(segundos); Serial.println("s.");
    }
    digitalWrite(PIN_RELE_BOMBA, HIGH);
    delay(segundos * 100); // 100ms por seg en modo debug
    digitalWrite(PIN_RELE_BOMBA, LOW);
}

void guardarSD() {
    if (DEBUG_MODE) {
        Serial.println("[SD] Datos registrados en log_cultivo.csv.");
        return;
    }
    File logFile = SD.open("/log_cultivo.csv", FILE_WRITE);
    if (logFile) {
        logFile.print(now.timestamp(DateTime::TIMESTAMP_DATE));
        logFile.print(", ");
        logFile.print(now.timestamp(DateTime::TIMESTAMP_TIME));
        logFile.print(", ");
        logFile.print(tempAire);
        logFile.print(", ");
        logFile.print(humAire);
        logFile.print(", ");
        logFile.print(humSuelo);
        logFile.print(", ");
        logFile.println(tiempoRiego);
        logFile.close();
    }
}

void actualizarOLED(String estado) {
    if (!DEBUG_MODE) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.println("SISTEMA INTELIGENTE");
        display.print("H. Suelo: "); display.print(humSuelo); display.println("%");
        display.print("Temp: "); display.print(tempAire); display.println("C");
        display.print("HR Aire: "); display.print(humAire); display.println("%");
        display.println("---------------------");
        display.println(estado);
        display.display();
    }
}

void setupFuzzy() {
    // ENTRADA 1: TEMPERATURA (Optimizado para clima Pereira)
    FuzzyInput *temperatura = new FuzzyInput(1);
    FuzzySet *frio = new FuzzySet(0, 0, 10, 14);           // < 14C
    FuzzySet *idealTemp = new FuzzySet(14, 15, 17, 18);    // 14-18C (Rango óptimo)
    FuzzySet *caliente = new FuzzySet(18, 22, 24, 28);     // > 18C (Estrés térmico)
    temperatura->addFuzzySet(frio);
    temperatura->addFuzzySet(idealTemp);
    temperatura->addFuzzySet(caliente);
    fuzzy->addFuzzyInput(temperatura);

    // ENTRADA 2: HUMEDAD SUELO (Optimizado para Lechuga)
    FuzzyInput *humedadSuelo = new FuzzyInput(2);
    FuzzySet *muySeco = new FuzzySet(0, 0, 45, 55);        // < 55% (Punto de activación estándar)
    FuzzySet *bajo = new FuzzySet(50, 58, 62, 68);         // 55-65% (Sensible en calor)
    FuzzySet *optimoHum = new FuzzySet(60, 65, 70, 75);    // 60-70% (Rango ideal)
    FuzzySet *saturado = new FuzzySet(75, 80, 100, 100);   // > 80% (Peligro asfixia)
    humedadSuelo->addFuzzySet(muySeco);
    humedadSuelo->addFuzzySet(bajo);
    humedadSuelo->addFuzzySet(optimoHum);
    humedadSuelo->addFuzzySet(saturado);
    fuzzy->addFuzzyInput(humedadSuelo);

    // SALIDA: TIEMPO DE RIEGO (Enfoque por pulsos cortos)
    FuzzyOutput *riego = new FuzzyOutput(1);
    FuzzySet *nada = new FuzzySet(0, 0, 0, 0);
    FuzzySet *pulso = new FuzzySet(20, 30, 30, 40);        // Pulso estándar de 30s
    FuzzySet *reforzado = new FuzzySet(45, 60, 60, 75);    // Pulso de 60s (emergencia calor)
    riego->addFuzzySet(nada);
    riego->addFuzzySet(pulso);
    riego->addFuzzySet(reforzado);
    fuzzy->addFuzzyOutput(riego);

    // REGLAS BASADAS EN INVESTIGACIÓN PEREIRA
    
    // Regla 1: Humedad muy baja (<55%) -> Riego estándar
    FuzzyRuleAntecedent *sueloMuySeco = new FuzzyRuleAntecedent();
    sueloMuySeco->joinSingle(muySeco);
    FuzzyRuleConsequent *activarPulso = new FuzzyRuleConsequent();
    activarPulso->addOutput(pulso);
    fuzzy->addFuzzyRule(new FuzzyRule(1, sueloMuySeco, activarPulso));

    // Regla 2: Estrés hídrico por calor (>22C) y suelo bajando (<65%)
    FuzzyRuleAntecedent *calorYBajo = new FuzzyRuleAntecedent();
    calorYBajo->joinWithAND(caliente, bajo);
    FuzzyRuleConsequent *activarRefuerzo = new FuzzyRuleConsequent();
    activarRefuerzo->addOutput(reforzado);
    fuzzy->addFuzzyRule(new FuzzyRule(2, calorYBajo, activarRefuerzo));

    // Regla 3: Suelo óptimo -> No regar (mantener capacidad de campo)
    FuzzyRuleAntecedent *sueloOptimo = new FuzzyRuleAntecedent();
    sueloOptimo->joinSingle(optimoHum);
    FuzzyRuleConsequent *pararRiego = new FuzzyRuleConsequent();
    pararRiego->addOutput(nada);
    fuzzy->addFuzzyRule(new FuzzyRule(3, sueloOptimo, pararRiego));

    // Regla 4: Suelo saturado -> Bloqueo total (evitar asfixia radicular)
    FuzzyRuleAntecedent *sueloSaturado = new FuzzyRuleAntecedent();
    sueloSaturado->joinSingle(saturado);
    fuzzy->addFuzzyRule(new FuzzyRule(4, sueloSaturado, pararRiego));
}

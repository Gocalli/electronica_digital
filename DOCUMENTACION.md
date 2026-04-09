# Documentación Técnica: Sistema Inteligente de Riego para Lechuga (Pereira)

Esta documentación detalla el funcionamiento del software desarrollado para el ESP32, incluyendo librerías, funciones y métodos de adquisición de datos.

---

## 1. Librerías Utilizadas y su Funcionamiento

### 1.1 `Adafruit_BME280.h` (Sensor Ambiental)
*   **Propósito:** Gestionar la comunicación con el sensor BME280 para obtener Temperatura y Humedad Relativa.
*   **Funcionamiento:** Utiliza el protocolo **I2C**. La librería accede a los registros internos del sensor, donde se almacenan valores de capacitancia y resistencia que luego convierte en unidades físicas (Celsius y %).
*   **Uso en el proyecto:** Proporciona la variable crítica de temperatura para detectar el "estrés térmico" en la lechuga.

### 1.2 `RTClib.h` (Reloj de Tiempo Real DS3231)
*   **Propósito:** Mantener la hora y fecha exacta, incluso si el ESP32 se reinicia o pierde energía.
*   **Funcionamiento:** Se comunica vía **I2C**. El módulo DS3231 tiene su propio oscilador de cristal compensado por temperatura, lo que garantiza una precisión de segundos por año.
*   **Uso en el proyecto:** Permite la "Restricción Nocturna" (18:00 - 06:00) para evitar hongos.

### 1.3 `Adafruit_SSD1306.h` & `Adafruit_GFX.h` (Pantalla OLED)
*   **Propósito:** Interfaz Hombre-Máquina (HMI).
*   **Funcionamiento:** La librería crea un **buffer de memoria** en la RAM del ESP32 donde "dibuja" los píxeles. Al llamar a `display()`, envía todo el buffer vía I2C a la pantalla.
*   **Uso en el proyecto:** Visualización en tiempo real de humedad, temperatura y estado de la bomba.

### 1.4 `SD.h` & `SPI.h` (Almacenamiento MicroSD)
*   **Propósito:** Data Logger persistente.
*   **Funcionamiento:** Utiliza el protocolo **SPI** (Bus de alta velocidad). La librería gestiona el sistema de archivos FAT32, permitiendo crear, escribir y cerrar archivos `.csv`.
*   **Uso en el proyecto:** Registro histórico para análisis posterior de la producción.

### 1.5 `eFLL.h` (Embedded Fuzzy Logic Library)
*   **Propósito:** Motor de Inteligencia Artificial (Lógica Difusa).
*   **Funcionamiento:** Implementa el proceso de **Fuzzificación** (convertir números en conceptos como "Frío" o "Seco"), **Inferencia** (aplicar reglas "SI... ENTONCES...") y **Defuzzificación** (convertir el resultado en segundos de riego).
*   **Uso en el proyecto:** Toma de decisiones complejas basadas en el clima de Pereira.

---

## 2. Documentación de Funciones Principales

### `void setup()`
Inicializa los puertos serie, los buses de comunicación (I2C/SPI) y los periféricos. Si el `DEBUG_MODE` está activo, omite los errores de hardware para permitir pruebas en el PC.

### `void loop()`
Implementa una **temporización no bloqueante** usando `millis()`. Esto evita el uso de `delay()`, permitiendo que el procesador esté libre para otras tareas mientras espera el siguiente ciclo de lectura (cada 15 minutos).

### `void leerSensores()`
Es la función encargada de la adquisición de datos brutos.
1.  **Humedad del Suelo (ADC):** Lee el pin analógico 34. Debido a que los sensores capacitivos pueden tener ruido, implementa un **Filtro de Promedios Móviles** (toma 10 muestras y saca el promedio). Luego usa `map()` para convertir valores de voltaje (0-4095) a porcentaje (0-100%).
2.  **BME280:** Llama a `readTemperature()` y `readHumidity()`.
3.  **RTC:** Captura la estampa de tiempo actual.

### `void evaluarLogica()`
El cerebro del sistema. Aplica tres niveles de filtrado:
1.  **Nivel 1 (Horario):** Bloquea el riego si es de noche.
2.  **Nivel 2 (Sanitario):** Bloquea el riego si hay riesgo de hongos (Humedad Aire > 80%).
3.  **Nivel 3 (Difuso):** Si pasa los filtros anteriores, entrega los datos al motor `eFLL` para calcular el tiempo exacto de riego.

### `void activarBomba(int segundos)`
Genera un pulso digital `HIGH` en el pin 26 para activar el relé. En modo real, espera exactamente el tiempo dictado por la IA. En modo debug, acelera el tiempo para agilizar las pruebas.

### `void guardarSD()`
Ensambla una cadena de texto (String) con formato CSV: `Fecha, Hora, Temp, HR_Aire, Hum_Suelo, Riego`. Abre el archivo, añade la línea al final y lo cierra inmediatamente para asegurar que los datos se guarden físicamente.

---

## 3. Obtención de Valores de los Sensores (Física y Software)

### 3.1 Sensor de Humedad de Suelo (Capacitivo)
*   **Método:** Transducción de Humedad a Voltaje.
*   **Cálculo en Software:**
    ```cpp
    lectura = analogRead(PIN); // Obtiene 0 a 4095
    porcentaje = map(lectura, SECO_VAL, HUMEDO_VAL, 0, 100);
    ```
*   **Nota:** Se usa un sensor capacitivo porque no se corroe con el tiempo (a diferencia de los resistivos), garantizando años de operación en la lechuga.

### 3.2 Sensor Ambiental BME280
*   **Método:** Sensores MEMS (Sistemas Micro-Electro-Mecánicos).
*   **Cálculo:** El sensor tiene tablas de compensación de fábrica. La librería lee estas tablas y aplica una fórmula polinómica para corregir el error por temperatura, entregando un valor digital preciso.

### 3.3 Reloj RTC DS3231
*   **Método:** Conteo de ciclos de un cristal de cuarzo.
*   **Cálculo:** El chip mantiene registros de segundos, minutos, horas, días, meses y años. El ESP32 solo "pide" el objeto `DateTime` completo cada vez que lo necesita.

---

**Desarrollado para:** Proyecto de Electrónica Digital - UTP
**Objetivo:** Optimización de Cultivo de Lechuga en Invernadero.

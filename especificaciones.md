Aquí tienes el **Documento de Especificaciones de Software** estructurado formalmente. Este documento es tu "hoja de ruta" para la programación; define exactamente qué vas a programar, con qué herramientas y cómo el código va a interactuar con el hardware que ya definimos.

Teniendo en cuenta que estamos en abril y la entrega final es a finales de mayo, este diseño de software está optimizado para ser modular: puedes programar y probar cada bloque por separado antes de unirlos.

***

# Especificaciones de Software: Sistema Inteligente de Riego (MVP)

## 1. Entorno de Desarrollo y Lenguajes
Para aprovechar al máximo los dos núcleos del ESP32 y garantizar la velocidad de ejecución de la lógica difusa, el sistema se programará a bajo nivel.

* **Lenguaje Principal:** C++ (Framework de Arduino para ESP32).
* **Entorno de Desarrollo Integrado (IDE):** Se recomienda **PlatformIO** (como extensión dentro de Visual Studio Code) en lugar del Arduino IDE tradicional. Esto permite una mejor gestión de las librerías, autocompletado de código profesional y un control de versiones (Git) más limpio.
* **Librerías Clave a Instalar:**
    * `Adafruit_BME280.h` (Para leer el sensor ambiental).
    * `RTClib.h` (Para la lectura de la hora del módulo DS3231).
    * `SD.h` y `SPI.h` (Para el manejo del sistema de archivos en la MicroSD).
    * `Adafruit_SSD1306.h` (Para la interfaz gráfica en la pantalla OLED).
    * `eFLL.h` (Embedded Fuzzy Logic Library - Para el motor de inferencia/IA).

## 2. Mapa de Periféricos (Perspectiva de Software)
El código deberá inicializar y gestionar diferentes protocolos de comunicación. La arquitectura del software debe contemplar las siguientes lecturas y escrituras:

* **Bus I2C (Pines 21 y 22):** El código debe solicitar datos al **BME280** (Temperatura, Humedad del aire), solicitar la hora actual al **RTC DS3231**, y enviar los textos/gráficos a la **Pantalla OLED**.
* **Bus SPI (Pines 5, 18, 19, 23):** El código debe abrir el archivo `.csv` en la **MicroSD**, escribir una nueva línea de datos y cerrar el archivo de forma segura para no corromper la memoria.
* **ADC (Convertidor Analógico-Digital - Pin 34):** El código leerá un valor de voltaje (de 0 a 4095 en el ESP32) proveniente del **Sensor Capacitivo** y lo mapeará a un porcentaje (0% a 100% de humedad del suelo).
* **GPIO (Salida Digital - Pin 26):** El código enviará un estado `HIGH` o `LOW` al **Relé** para encender o apagar la bomba de agua.



## 3. Requerimientos Funcionales del Software
Estas son las tareas obligatorias que el programa debe ejecutar de forma cíclica (en el `loop` principal):

1.  **Temporización y Ciclo de Despertar:** El software no ejecutará el ciclo continuamente. Implementará un temporizador no bloqueante (usando la función `millis()`) para ejecutar las lecturas cada 15 o 30 minutos, ahorrando energía el resto del tiempo.
2.  **Lectura y Acondicionamiento de Datos:**
    * Adquirir la humedad del suelo y aplicar un filtro de promedios (leer 10 veces en 1 segundo y sacar el promedio) para eliminar el "ruido" o lecturas falsas del ADC.
    * Leer la temperatura y humedad relativa del aire.
    * Leer la hora militar (00:00 a 23:59) desde el RTC.
3.  **Procesamiento Lógico (Reglas de Control):**
    * **Restricción Horaria:** Si la hora leída está entre las 18:00 y las 06:00, la variable de riego se fuerza a `FALSE` (No regar de noche para evitar hongos en la lechuga).
    * **Motor de Inferencia:** Si es de día, pasar la Temperatura y la Humedad del Suelo por el algoritmo de Lógica Difusa. El algoritmo retornará un valor de "Tiempo de Riego" (ej. 0 segundos, 30 segundos, 60 segundos).
4.  **Acción y Actuación:**
    * Si el "Tiempo de Riego" es mayor a 0, el software cambia el pin del relé a `HIGH`.
    * Mantiene el estado `HIGH` mediante un temporizador, y luego lo devuelve a `LOW` (cierre de válvula).
5.  **Registro de Datos (Data Logger):**
    * Ensamblar un *String* con el formato: `DD/MM/AAAA, HH:MM, Temp, Hum_Aire, Hum_Suelo, Tiempo_Riego_Activado`.
    * Escribir ese *String* en el archivo `log_cultivo.csv` de la MicroSD.
6.  **Interfaz Gráfica (HMI):**
    * Actualizar la pantalla OLED mostrando la hora actual, la humedad del suelo y un mensaje de estado ("Regando..." o "En Espera").

## 4. Requerimientos No Funcionales (Restricciones y Calidad)
* **Determinismo (Offline):** El software no puede contener ninguna función bloqueante que dependa de respuesta de internet (como `WiFi.begin()`). Todo debe ejecutarse localmente.
* **Gestión de Errores (Watchdog):** El código debe incluir rutinas de prevención de fallos. Si, por ejemplo, el sensor I2C se desconecta accidentalmente, el código no debe quedarse en un bucle infinito ("colgado"). Debe imprimir un error en la pantalla OLED, escribir el error en la MicroSD y mantener la bomba apagada por seguridad.
* **Modularidad:** El código fuente estará dividido en funciones específicas (`leerSensores()`, `evaluarLogica()`, `guardarSD()`, `activarBomba()`) para facilitar la depuración (debugging) en el laboratorio y permitir escalabilidad a futuro.

***

**Sugerencia para empezar a programar:**
Dado el nivel que exige la UTP en electrónica digital, te recomiendo empezar programando un script muy sencillo que **solo** lea el sensor de humedad y lo imprima en la terminal. Cuando eso funcione perfecto, creas otro script que **solo** guarde datos en la SD. La última semana unes todas las piezas. 

¿Quieres que te genere la estructura base del código en C++ (el cascarón con las funciones vacías) para que la copies a tu editor y empieces a rellenar la lógica?

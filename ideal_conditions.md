# Condiciones Óptimas para el Cultivo de Lechuga en Invernadero (Pereira, Risaralda)

## 1. Contexto Geográfico y Referente de Investigación
La presente especificación técnica se fundamenta en estudios de telemetría agrícola desarrollados en la región andina colombiana y en el Eje Cafetero. El municipio de Pereira (Risaralda) presenta una altitud aproximada de 1.411 msnm y una temperatura ambiente promedio de 21°C. 

Sin embargo, **dentro de un invernadero en esta zona, la temperatura puede elevarse entre 5°C y 8°C por encima de la temperatura exterior**, y la humedad relativa puede saturarse. Dado que la lechuga (*Lactuca sativa*) es una hortaliza de clima frío/templado, la implementación de un sistema de control basado en IoT y lógica local (ESP32) es obligatoria para evitar la pérdida del cultivo.

## 2. Variables Climáticas (Sensor BME280)

Para garantizar la calidad de la hoja y evitar enfermedades fitosanitarias, el agente inteligente debe monitorear y reaccionar a los siguientes umbrales:

* **Temperatura Diurna:** El rango óptimo de crecimiento es de **14°C a 18°C**.
* **Temperatura Nocturna:** El rango óptimo es de **10°C a 14°C**.
* **Límite Crítico Superior (Peligro):** Si la temperatura sostenida supera los **24°C**, la lechuga sufre estrés térmico y activa un proceso fisiológico llamado "espigado" (floración prematura), volviendo la hoja amarga y no comercializable. En este punto, el sistema debe alertar para forzar ventilación.
* **Humedad Relativa (HR):** El rango óptimo es de **65% a 75%**. En Pereira, la humedad natural suele ser alta. Si dentro del invernadero la humedad supera el **80%** y se combina con calor, existe un riesgo inminente de proliferación de hongos mortales para la planta, como la *Botrytis cinerea* y el *Sclerotium*.

## 3. Variables Edáficas / Sustrato (Sensor Capacitivo)

El sistema radicular de la lechuga es pivotante, corto y muy superficial (los primeros 25-30 cm del suelo), lo que la hace extremadamente sensible tanto a la sequía como a la asfixia por agua.

* **Humedad del Sustrato (Capacidad de Campo):** Se debe mantener la humedad del suelo constante entre un **60% y un 70%**.
* **Técnica de Riego Obligatoria:** Riego por goteo (pulsos). El sistema inteligente nunca debe permitir el "encharcamiento". Si el sensor capacitivo marca un valor superior al 80%, se produce asfixia radicular.
* **pH del Suelo (Dato de control agronómico):** Ideal entre **5.8 y 6.5**. (Esta variable se controlará manualmente en la preparación de la tierra, pero la frecuencia de riego dictada por el ESP32 ayudará a no lavar los nutrientes).

## 4. Parámetros de Configuración para el Agente (Software ESP32)

Basados en la investigación anterior, el algoritmo de control (Lógica Difusa) a programar en el microcontrolador adoptará las siguientes reglas base para la zona de Pereira:

1.  **Regla de Prevención Fúngica (Nocturna):** Si la hora dictada por el RTC está entre las 18:00 y las 06:00, **NO REGAR**, sin importar la lectura del suelo, para evitar elevar la humedad relativa nocturna.
2.  **Regla de Riego Estándar:** Si la Humedad del Suelo cae al **55%**, activar la bomba de agua por pulsos cortos (ej. 30 segundos) y esperar a que el agua se filtre antes de volver a medir.
3.  **Regla de Estrés Hídrico (Días Soleados):** Si la Temperatura del aire supera los **22°C** al mediodía (alta evapotranspiración), el umbral de activación del riego sube al **65%** para mantener la planta fresca y compensar la pérdida de agua por las hojas.

import random
import time

# --- CONFIGURACIÓN DE LA SIMULACIÓN (Mismos parámetros que el ESP32) ---
HORA_INICIO_PROHIBIDO = 18
HORA_FIN_PROHIBIDO = 6
HR_AIRE_CRITICA = 80
TEMP_PELIGRO_LECHUGA = 24

def logica_difusa_simulada(temp, hum_suelo):
    """Replicación de la lógica programada en C++"""
    # Regla 4: Suelo Saturado
    if hum_suelo > 75:
        return 0, "Suelo Saturado (Asfixia)"
    
    # Regla 1: Suelo Muy Seco
    if hum_suelo < 55:
        return 30, "Riego Estándar (Seco)"
    
    # Regla 2: Estrés Hídrico por Calor
    if temp > 22 and hum_suelo < 65:
        return 60, "RIEGO REFORZADO (Calor/Evaporación)"
    
    # Regla 3: Suelo Óptimo
    if 60 <= hum_suelo <= 75:
        return 0, "Condición Óptima"
    
    return 0, "En Espera"

def simulador_invernadero_pereira():
    print("="*60)
    print("SIMULADOR DE CULTIVO DE LECHUGA - PEREIRA (INVERNADERO)")
    print("="*60)
    
    # Estado inicial
    hum_suelo = 70.0  # Empezamos en capacidad de campo
    agua_total = 0
    horas_estres = 0
    riesgo_hongos = 0
    
    print(f"{'Hora':<6} | {'Temp':<6} | {'HR Aire':<8} | {'Hum Suelo':<10} | {'Acción'}")
    print("-" * 65)

    # Simulación de 24 horas (Ciclo de 1 hora)
    for hora in range(24):
        # Modelo climático de Pereira con efecto invernadero (+8°C al mediodía)
        if 6 <= hora <= 18:
            # Diurno: Sube la temperatura
            temp_ext = 18 + (6 * (1 - abs(hora - 13)/7)) # Campana de calor
            temp = temp_ext + random.uniform(4, 8)       # Efecto invernadero
            hr_aire = random.uniform(60, 75)
            hum_suelo -= random.uniform(2, 4)           # Evaporación diurna
        else:
            # Nocturno: Baja la temperatura, sube la humedad
            temp = random.uniform(12, 16)
            hr_aire = random.uniform(75, 90)
            hum_suelo -= random.uniform(0.5, 1.5)       # Drenaje lento

        # --- APLICAR LÓGICA DEL ESP32 ---
        riego = 0
        mensaje = "Monitoreo"

        # 1. Restricción Nocturna
        if hora >= HORA_INICIO_PROHIBIDO or hora < HORA_FIN_PROHIBIDO:
            mensaje = "Bloqueo Nocturno (Anti-Hongos)"
        # 2. Riesgo de Botrytis
        elif hr_aire > HR_AIRE_CRITICA:
            mensaje = "ALERTA HONGOS: Riego suspendido"
            riesgo_hongos += 1
        # 3. Lógica Difusa
        else:
            riego, mensaje = logica_difusa_simulada(temp, hum_suelo)
            if riego > 0:
                hum_suelo += (riego / 2) # El riego sube la humedad del suelo
                agua_total += riego

        # --- EVALUACIÓN DE SALUD DE LA PLANTA ---
        if temp > TEMP_PELIGRO_LECHUGA:
            horas_estres += 1
        
        print(f"{hora:02d}:00  | {temp:>5.1f}C | {hr_aire:>7.1f}% | {hum_suelo:>9.1f}% | {mensaje}")

    # --- REPORTE FINAL ---
    print("="*60)
    print("REPORTE DE PRODUCCIÓN (24 HORAS):")
    print(f"- Consumo de Agua: {agua_total} segundos de bomba.")
    print(f"- Horas de Estrés Térmico (>24C): {horas_estres}h")
    print(f"- Alertas de Riesgo de Hongos: {riesgo_hongos}")
    
    if horas_estres > 4:
        print("RESULTADO: CALIDAD BAJA (Riesgo de espigado/amargor)")
    elif riesgo_hongos > 3:
        print("RESULTADO: RIESGO FITOSANITARIO (Posible Botrytis)")
    else:
        print("RESULTADO: PRODUCCIÓN ÓPTIMA (Lechuga de alta calidad)")
    print("="*60)

if __name__ == "__main__":
    simulador_invernadero_pereira()

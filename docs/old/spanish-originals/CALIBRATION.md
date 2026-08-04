# Caracterización y calibración del MTX-D

## Por qué es obligatoria

Innovate identifica el sensor de presión como `12-0074` (0–150 PSI) y el
de temperatura como termistor `15-0049`, pero el manual no publica:

- tensión de excitación;
- orden de pines del transductor;
- función voltios→PSI;
- resistencia nominal/Beta o tabla del termistor.

Una suposición típica como “0.5–4.5 V” puede producir lecturas falsas o
dañar el ADC. El firmware no habilitará valores reales hasta completar
esta caracterización.

## Regla principal

El MTX-D permanece conectado y operativo durante la toma de datos. Se
añade una medición de alta impedancia en paralelo. Solo cuando las curvas
coincidan se pasa a alimentar los sensores desde la nueva electrónica.

## 1. Inventario y fotos

Con el coche apagado:

1. Fotografiar etiqueta trasera del MTX-D y todos los conectores.
2. Fotografiar ambos lados de cada conector, pestaña y colores.
3. Anotar qué cable negro es la masa principal y cuál es la masa adicional
   del sensor de presión.
4. Medir continuidad de la carcasa roscada de cada sensor con sus cables,
   siempre desconectados.
5. Localizar cable Innovate 38400 y, si existe, el USB-RS232.

No medir resistencia con el sensor conectado a un circuito alimentado.

## 2. Identificar el sensor de presión

Con el MTX-D alimentado en banco o en el coche, motor parado:

1. Confirmar 0 PSI en el MTX-D.
2. Usar agujas de back-probe; no perforar el aislamiento.
3. Medir cada pin del conector respecto a la masa común.
4. Identificar:
   - masa: aproximadamente 0 V;
   - excitación: tensión estable;
   - señal: cambia con la presión.
5. Repetir con motor al ralentí solo tras aislar y sujetar las puntas.

Registrar todas las medidas; no asignar función por color.

## 3. Curva de presión

### Método mínimo sin desmontar el sensor

Registrar simultáneamente:

- presión indicada/registrada por MTX-D/LogWorks;
- tensión bruta A0;
- tensión de excitación A2;
- régimen/estado del motor.

Puntos mínimos:

1. contacto dado, motor parado: 0 PSI;
2. arranque en frío;
3. ralentí frío;
4. 2000 y 3000 rpm frío;
5. ralentí con aceite caliente;
6. 2000 y 3000 rpm caliente.

Se ajustará una recta solo si los residuos de todos los puntos son
pequeños y no muestran curvatura. La ecuación será:

```text
PSI = pendiente × V_sensor + offset
```

La calibración debe reservar margen hasta 150 PSI, aunque el reloj muestre
0–145 PSI.

### Método preferido de banco

Si se puede desmontar el transductor con seguridad, usar una bomba/calibrador
0–10 bar y manómetro de referencia en al menos 0, 2, 4, 6, 8 y 10 bar.
No usar una fuente de presión sin limitación ni accesorios no clasificados.

## 4. Curva del termistor

### Entrada del MTX-D como patrón

Con el termistor desconectado y el MTX-D alimentado:

1. conectar una caja de décadas a la entrada de temperatura;
2. variar resistencia sin cortocircuitar la entrada;
3. registrar resistencia y temperatura mostrada/LogWorks;
4. concentrar puntos entre 49 y 138 °C;
5. repetir algunos puntos en ambos sentidos para detectar histéresis.

Esto revela la curva que espera el MTX-D sin calentar aceite.

### Sensor real

Medir el termistor desconectado:

- a temperatura ambiente con termómetro de referencia;
- con motor completamente frío;
- en varios estados de calentamiento, comparando MTX-D;
- idealmente, si hay sensor de repuesto, en baño de agua controlado.

No aplicar llama. Para superar 100 °C hace falta un procedimiento de
laboratorio adecuado; no improvisar un baño de aceite caliente.

El firmware admite coeficientes Steinhart-Hart:

```text
1 / T_kelvin = A + B·ln(R) + C·ln(R)^3
```

También se conservarán los puntos originales para validación. La precisión
se evaluará con puntos que no hayan entrado en el ajuste.

## 5. MTS y LogWorks

El kit incluye el cable serie `38400`. El puerto OUT permite que LogWorks
registre presión y temperatura. Procedimiento:

1. conectar OUT→38400→USB-RS232→PC;
2. confirmar ambos canales en LogWorks;
3. exportar el registro con tiempo;
4. registrar en paralelo CSV del ESP32;
5. alinear por un evento claro, por ejemplo contacto/arranque.

En paralelo se puede observar el RS-232 con un MAX3232, inicialmente solo
en recepción. No conectar el nivel RS-232 directamente a GPIO44.

## 6. Criterios para retirar el MTX-D

- Pinout confirmado por medida, no por color.
- 0 PSI correcto con motor parado.
- Presión nueva frente a MTX-D: error objetivo ≤2 PSI en la zona normal.
- Temperatura nueva frente a MTX-D: error objetivo ≤2 °C en 60–130 °C.
- Detecta circuito abierto/corto en ambos sensores.
- Ensayo frío/caliente repetido en al menos tres trayectos.
- Alarmas probadas con datos simulados, no provocando una avería real.
- Reinicio por arranque del motor sin valores falsos persistentes.

Hasta entonces el MTX-D sigue siendo el instrumento de referencia.

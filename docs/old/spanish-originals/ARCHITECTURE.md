# Arquitectura de hardware

## Decisión

Se preparan dos rutas, pero la meta es la **ruta A**.

| Ruta | Qué se conserva | Hardware extra | Ventaja | Inconveniente |
|---|---|---|---|---|
| A. Sensores directos | Sensores y cableado | ADS1115 y acondicionamiento | El MTX-D desaparece por completo | Hay que caracterizar ambos sensores |
| B. MTS | Sensores y electrónica del MTX-D oculta | Receptor RS-232 | Conserva las conversiones de Innovate | Hay que ocultar el reloj y decodificar MTS |

La ruta B sirve también como herramienta de desarrollo. Si se recibe por
MTS exactamente presión y temperatura, proporciona una referencia digital
sincronizada para comprobar la ruta A.

## Ruta A: lectura directa

```text
ACC 12 V
  │
  ├── fusible 2 A
  │
  ├── TVS de carga + protección de polaridad
  │
  └── buck 5 V / ≥3 A
           ├── VBUS de Waveshare
           └── excitación sensor presión (valor por confirmar)

Waveshare 3V3 ── ADS1115 (solo prototipo; Q1 en PCB final)
GPIO15 SDA ───── ADS1115 SDA
GPIO14 SCL ───── ADS1115 SCL
GND estrella ─── ESP32 + ADC + sensores

Presión ── divisor/protección/RC ── A0
Termistor ─ pull-up seleccionable/RC ─ A1
Excitación presión monitorizada /2 ── A2
Luces 12 V, entrada protegida ─────── A3
```

### ADC

Para la ruta A, el ADC externo es **prácticamente obligatorio**. La placa
solo expone cómodamente I²C y UART: GPIO14/15 comparten el bus de los
periféricos integrados y llevan pull-ups de 2.2 kΩ, GPIO19/20 son el USB
nativo y GPIO43/44 no son entradas ADC. Usar el ADC interno implicaría
invadir señales de la propia placa y seguiría dando peor repetibilidad para
dos sensores analógicos.

El **ADS1115-Q1** es adecuado porque ofrece cuatro entradas, 16 bits,
PGA, referencia interna, hasta 860 muestras/s, I²C y calificación
automotriz de −40 a 125 °C. El aceite cambia despacio; 128 muestras/s y
filtrado digital dejan margen suficiente.

Se alimentará a **3.3 V**. Aunque el PGA tenga una escala de ±4.096 o
±6.144 V, ninguna entrada puede superar VDD + 0.3 V. Por eso toda señal
potencialmente de 5/12 V llevará divisor y protección.

La dirección elegida es `0x48`. No coincide con los periféricos conocidos
de la Waveshare. El escaneo de llegada deberá confirmarlo.

### Frente analógico provisional

Estos valores permiten prototipar, pero no se convertirán en PCB final
hasta medir el MTX-D real:

| Canal | Circuito de banco provisional | Uso |
|---|---|---|
| A0 | 33 kΩ / 33 kΩ, 0.1 %, 100 nF, clamp de baja fuga | señal de presión hasta 5 V |
| A1 | pull-up enchufable 2.49/4.99/10 kΩ, 0.1 %, 100 nF | termistor |
| A2 | 33 kΩ / 33 kΩ, 0.1 %, 100 nF | supervisar excitación del sensor |
| A3 | 150 kΩ / 22 kΩ, 0.1 %, 100 nF, clamp | detectar luces encendidas |

Antes de conectar A0 se medirá su mínimo/máximo con multímetro. Antes de
conectar A1 se medirá la resistencia del sensor desconectado. Los valores
de pull-up se eligen para situar el rango útil cerca del centro del ADC y
limitar el autocalentamiento.

### Tierras

El manual Innovate exige que el cable negro adicional del sensor de
presión use la misma masa que el reloj. La sustitución replicará una masa
en estrella: sensor, ADC, ESP32 y entrada del convertidor convergen en un
punto limpio, separado de radio, encendido y bomba de combustible.

## Ruta B: conservar el MTX-D como acondicionador

```text
Sensores ── MTX-D ── puerto OUT ── cable Innovate 38400
                                      │ RS-232
                                      ▼
                              MAX3232E-Q1 / TRS3232E-Q1
                                      │ 3.3 V UART RX
                                      ▼
                              GPIO44 de Waveshare
```

La documentación pública de MTS describe RS-232 a 19200, 8N1 y paquetes
de palabras de 16 bits en big-endian. Esto se considera una hipótesis de
trabajo: primero se capturarán las tramas reales del MTX-D y se
compararán con LogWorks. Inicialmente se conectará solo la recepción del
ESP32 para evitar enviar comandos accidentales al reloj.

Si la ruta B resulta estable, no necesita ADS1115 para las variables de
aceite. Sí necesita mantener alimentado y escondido el cuerpo de 52 mm
del MTX-D.

## Alimentación

### Banco

- USB-C de datos y fuente 5 V / 3 A de calidad.
- Sensores sin conectar durante el primer encendido.
- ADS1115 alimentado desde el pad 3V3.

### Prototipo en coche

- Fusible dedicado de 2 A lo más cerca posible de ACC.
- TVS automotriz de carga.
- Convertidor de 5 V con entrada amplia.
- Como módulo de prototipo se propone Pololu D36V28F5: 5 V, 3.2 A,
  entrada hasta 50 V y protección de polaridad inversa.
- No es una validación de automoción completa. El diseño final deberá
  cubrir transitorios, arranque con pinzas, carga del alternador, EMI y
  temperatura.

### PCB final

La selección final se hará tras medir el consumo real. Candidatos:

- ADC `ADS1115-Q1` en VSSOP-10.
- Convertidor automotriz de 60/65 V como familia
  `LM76003-Q1`/`LM65635-Q1`.
- TVS `SLD8S24A` para sistema de 12 V, coordinada con fusible y buck.
- Transceptor `TRS3232E-Q1` solo si se adopta MTS.

## Mecánica

La carcasa oficial Waveshare mide 46 × 46 mm y unos 8.3 mm de fondo; el
frontal útil del display ronda 39 × 39 mm. El MTX-D ocupa un hueco de
52 mm, por lo que no existe sustitución geométrica directa: las esquinas
de la carcasa cuadrada no caben dentro de un círculo de 52 mm.

Se diseñará una pieza con:

- espiga cilíndrica para el hueco de 52 mm;
- frontal cuadrado mayor que el hueco;
- salida lateral/inferior para USB-C y cableado;
- acceso de servicio a PWR/BOOT;
- material resistente al calor (ASA/PETG técnico, no PLA).

No se cerrarán las cotas CAD hasta medir la unidad recibida y el soporte
real del coche.

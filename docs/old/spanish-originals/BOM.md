# Lista de materiales

La compra se separa para no fabricar una PCB final basada en curvas que
todavía no conocemos.

## Comprar o localizar ahora

| Cant. | Material | Referencia recomendada | Para qué |
|---:|---|---|---|
| 1 | ADC de banco ADS1115 | Adafruit Product 1085 o módulo de marca reconocida con TI ADS1115 | capturar las señales directas |
| 1 | Conversor RS-232↔TTL 3.3 V | módulo con MAX3232E/TRS3232E, **no MAX232 de 5 V** | capturar el puerto MTS |
| 1 | Adaptador DB9 a bornas | género por confirmar con el cable 38400 | conexión reversible al MAX3232 |
| 1 | Caja de décadas 0–100 kΩ | preferible 0.1 %, aceptable 1 % para exploración | mapear el termistor contra el MTX-D |
| 1 lote | Resistencias 0.1 %, ≤25 ppm/°C | 1 k, 2.49 k, 4.99 k, 10 k, 22 k, 33 k, 47 k, 100 k, 150 k | divisores y pull-ups |
| 1 lote | Condensadores X7R | 100 nF, 1 µF, 10 µF | filtros y desacoplo |
| 1 | Protoboard sin soldar + jumpers | 2.54 mm, cables cortos | caracterización inicial |
| 1 | Protoboard soldable | paso 2.54 mm | frente analógico de banco |
| 1 lote | Bornas, tiras de pin y conectores Dupont | paso 2.54/5.08 mm | conexiones reversibles de banco |
| 1 | Cable USB-C de datos | certificado, corto | programación y medidas |
| 1 | Fuente USB 5 V / 3 A | de marca | pruebas de banco |
| 1 | Multímetro de alta impedancia | DC V, Ω y continuidad | identificar pines y curvas |
| 1 | Juego de back-probes finos | puntas aisladas, sin perforar cable | medir el MTX-D conectado |
| 1 | Termómetro de referencia | sonda de contacto/termopar | validar temperatura |
| 1 | Cable apantallado 4 conductores | 0.22–0.35 mm² | señales entre ADC y sensores |
| 1 lote | Tubo termorretráctil, funda trenzada, ferrules y terminales | automoción | mazo reversible y protegido |

### Material Innovate que debería existir con el kit

Comprobar antes de comprar:

| Pieza | P/N Innovate | Acción |
|---|---:|---|
| Cable de programación serie MTX | 38400 | localizarlo; es muy útil para LogWorks/MTS |
| Cable reloj→sensor de presión | 08-0256C | conservar intacto |
| Sensor de presión 0–150 PSI | 12-0074 | ya instalado |
| Termistor | 15-0049 | ya instalado |

Si el ordenador no dispone de RS-232, hace falta además el adaptador
USB-serie Innovate `37330` o uno compatible de calidad.

## Alimentación para el prototipo en coche

| Cant. | Material | Referencia | Nota |
|---:|---|---|---|
| 1 | Buck 12 V→5 V | Pololu D36V28F5, #3782 | 5 V/3.2 A, 5.3–50 V, protección inversa; prototipo |
| 1 | Portafusible mini/ATO en línea | automoción | instalar cerca de ACC |
| 3 | Fusible 2 A | mini/ATO | uno instalado y dos de repuesto |
| 1 | TVS automotriz load-dump | Littelfuse SLD8S24A | para red de 12 V; montar tras el fusible |
| 1 | PCB/perfboard de potencia y caja | por elegir | aislar TVS, buck y terminales |

El Pololu es una plataforma de prototipo, no convierte por sí solo todo el
conjunto en equipo homologado. La coordinación fusible/TVS, temperatura y
ruido se validará antes de dejarlo fijo.

## Opcional pero útil

| Material | Motivo |
|---|---|
| Analizador lógico u osciloscopio | verificar niveles y tramas MTS sin arriesgar GPIO44 |
| Sensor de temperatura Innovate 15-0049 de repuesto | curva en baño controlado sin desmontar el instalado |
| Bomba/calibrador 0–10 bar con manómetro de referencia | calibración de presión en banco |
| Segundo cable 08-0256C | fabricar un adaptador sin tocar el mazo del coche |
| Medidor USB-C de corriente | dimensionar fuente, fusible y temperatura |

## No comprar todavía

- Conectores “parecidos” a los de Innovate: se identificarán con fotos,
  medidas y forma de las llaves.
- ADS1115-Q1 suelto, referencias de clamps o una PCB a medida: esperar a
  las mediciones.
- Resistencias fijas definitivas del termistor.
- Pieza 3D de 52 mm o insertos: esperar a medir pantalla, tornillos y
  soporte del coche.
- Batería LiPo para la Waveshare: no es necesaria para una instalación
  alimentada por ACC.

## Enlaces oficiales de compra/datos

- ADS1115 de banco:
  https://www.adafruit.com/product/1085
- ADS1115-Q1 final:
  https://www.ti.com/product/ADS1115-Q1
- Pololu D36V28F5:
  https://www.pololu.com/product/3782
- TVS SLD8S24A:
  https://www.littelfuse.com/products/overvoltage-protection/tvs-diodes/automotive-tvs-diodes/sld8s/sld8s24a
- Accesorios MTX-D:
  https://www.innovatemotorsports.com/mtx-d-oil-pressure-temperature.html

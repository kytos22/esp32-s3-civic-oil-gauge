# Lista de comprobación cuando llegue la Waveshare

## Sin conectar al coche

- [ ] Confirmar modelo exacto y versión de PCB.
- [ ] Fotografiar placa, carcasa, etiqueta y contenido del paquete.
- [ ] Si incluye LiPo, dejarla desconectada durante las pruebas de banco.
- [ ] Medir carcasa 46 × 46 mm y fondo; comprobar tolerancias reales.
- [ ] Medir tornillos y separación aproximada 37 × 34 mm.
- [ ] Comprobar acceso a USB-C, microSD, PWR y BOOT.
- [ ] Revisar si los pads VBUS/3V3/GND/14/15/43/44 son soldables con la
  carcasa montada.

## Primer encendido

- [ ] Usar solo USB-C 5 V.
- [ ] Ejecutar primero el firmware/factory demo oficial.
- [ ] Guardar foto de pantalla y log serie.
- [ ] Medir consumo con brillo 64, 128 y 255.
- [ ] Comprobar toque en esquinas y centro.
- [ ] Verificar que no hay píxeles anómalos ni reinicios.

## Firmware del proyecto

- [ ] Compilar antes de flashear.
- [ ] Mantener `OIL_GAUGE_DEMO_MODE=1`.
- [ ] Comprobar que aparece la marca `DEMO`.
- [ ] Ejecutar escaneo I²C.
- [ ] Confirmar direcciones de periféricos y ausencia de conflicto en 0x48.
- [ ] Conectar ADS1115 solo a 3V3/GND/SDA15/SCL14.
- [ ] Verificar cuatro tensiones ADC con entradas a masa y a 3.3 V mediante
  divisor seguro.

## MTX-D, todavía instalado

- [ ] Localizar P/N 38400 y adaptador USB-RS232.
- [ ] Confirmar que LogWorks ve presión y temperatura.
- [ ] Fotografiar conectores antes de desconectar nada.
- [ ] Medir excitación y señal del sensor de presión con back-probe.
- [ ] Medir resistencia del termistor únicamente desconectado.
- [ ] Construir un mazo adaptador reversible; no cortar el original.

## Antes de una prueba en el coche

- [ ] Fusible 2 A junto a ACC.
- [ ] TVS orientada y aislada.
- [ ] Buck verificado a 5.0 V con carga.
- [ ] Sin conexión simultánea USB/external VBUS.
- [ ] Caja cerrada, sin cobre ni terminales expuestos.
- [ ] Cableado alejado de encendido, bomba, alternador y audio.
- [ ] Una segunda persona supervisa datos; el conductor no opera el equipo.

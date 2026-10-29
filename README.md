# POLISHED MCX Compressor (VST3 para FL Studio)

Compresor estilo "R-Compressor" con estetica alien/OVNI.

## Controles
- Threshold, Ratio (hasta inf:1), Attack, Release, Knee (rodilla suave), Gain (makeup), Mix (compresion paralela)
- Mode: Electro (clasico) / Opto (ataque suave y release que depende del programa)
- ARC: Auto Release Control (release se adapta al material)
- Detector: Peak / RMS, Sidechain HPF (20-500 Hz), Stereo Link 0-100 %
- Auto Gain (compensacion automatica)
- Curva de transferencia con punto en vivo, historial de Gain Reduction y medidores IN / GR / OUT

## Compilar (igual que el EQ)
1. Sube esta carpeta a un repo nuevo de GitHub.
2. Si falta `.github/workflows/build.yml`, crealo a mano (Add file -> Create new file).
3. Actions -> espera el check verde -> Summary -> Artifacts -> POLISHED-MCX-VST3-Windows.
4. Copia `POLISHED MCX.vst3` a `C:\Program Files\Common Files\VST3\` y en FL Studio: Options -> Manage plugins -> Find installed plugins.

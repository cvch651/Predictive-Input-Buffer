# Predictive Input Buffer (recreación experimental)

Reimplementación basada en el código y comportamiento descritos en la conversación; no es el código original.

## Etapas
- Stage 1: 2 ticks todo / 2 wave
- Stage 2: 4 / 4
- Stage 3: 8 / 8
- Stage 4: 16 / 8
- OFF: input inmediato

1 tick = 1/240 s. F6 cicla las etapas.

## Compilación
1. Sube el contenido de esta carpeta a un repositorio GitHub.
2. Abre **Actions** y ejecuta **Build Geode Mod**.
3. Si el trabajo finaliza correctamente, descarga **Build Output**.
4. Instala el `.geode` en `Geometry Dash/geode/mods/`.

## Importante
Este proyecto no se ha compilado ni probado en el juego. La firma de `processCommands`, el acceso a `m_fields`, la firma de `dispatchKeyboardMSG` y la detección de wave deben verificarse contra los bindings del SDK exacto. La lógica actual descuenta un tick por llamada a `processCommands`; si el motor llama esta función más de una vez por tick físico, habrá que ajustar esa parte. Cambia `tuusuario` y `TuUsuario` en `mod.json` antes de publicar.

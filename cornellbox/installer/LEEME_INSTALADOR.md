# Cómo generar el instalador de Windows (`CornellBox-Setup.exe`)

Este instalador empaqueta el programa, sus shaders y las DLL necesarias, y al
instalarlo crea accesos directos en el menú Inicio y un desinstalador. El proceso
se hace **en una máquina Windows** (no se puede generar el `.exe` desde Linux).

Tiempo aproximado: ~10 minutos la primera vez.

> **Requisito de la PC que ejecutará el programa:** una GPU con **OpenGL 4.3+**
> (cualquier tarjeta de la última década, incluso gráficos Intel integrados
> recientes). El instalador no instala drivers.

---

## Paso 1 — Compilar el programa con MSYS2

1. Instala **[MSYS2](https://www.msys2.org/)** y abre la terminal
   **“MSYS2 MinGW x64”** (la del icono azul).
2. Instala el compilador y las librerías:
   ```sh
   pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-make \
     mingw-w64-x86_64-pkgconf mingw-w64-x86_64-glfw mingw-w64-x86_64-glew
   ```
3. Ve a la carpeta del proyecto y compílalo (ajusta la ruta a donde lo tengas):
   ```sh
   cd /c/Users/TU_USUARIO/.../ModelosDeIluminacionGlobal/cornellbox
   mingw32-make
   ```

## Paso 2 — Armar el paquete (binario + shaders + DLL)

```sh
mingw32-make dist
```
Esto crea `cornellbox\dist\cornellbox\` con:
- `cornellbox.exe`
- la carpeta `shaders\`
- **automáticamente** los DLL de MinGW que el programa necesita
  (`glfw3.dll`, `glew32.dll`, `libstdc++-6.dll`, `libgcc_s_seh-1.dll`,
  `libwinpthread-1.dll`, …) detectados con `ldd`.
- `README.md` y `LICENSE`.

> Verificación rápida: abre `dist\cornellbox\` en el Explorador y haz doble clic
> en `cornellbox.exe`. Si abre la ventana del programa, el paquete está completo.
> (Si falta algún DLL, Windows dirá cuál; cópialo desde `C:\msys64\mingw64\bin`.)

## Paso 3 — Instalar Inno Setup

Descarga e instala **[Inno Setup 6.3 o superior](https://jrsoftware.org/isdl.php)**
(gratuito; el enlace ofrece la última versión). Si usaras una versión anterior a
la 6.3, cambia en `cornellbox.iss` las dos líneas `x64compatible` por `x64`.

## Paso 4 — Compilar el instalador

**Opción A (gráfica):** abre `cornellbox\installer\cornellbox.iss` con Inno Setup
y pulsa el botón **Compilar** (o menú *Build → Compile*).

**Opción B (línea de comandos)**, desde `cornellbox\installer\`:
```bat
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" cornellbox.iss
```

El instalador queda en:
```
cornellbox\installer\Output\CornellBox-Setup.exe
```

## Paso 5 — Distribuir

Comparte **`CornellBox-Setup.exe`**. En la PC destino, al ejecutarlo:
- Se instala en `C:\Program Files\CornellBox\` (o donde el usuario elija).
- Crea un acceso directo en el menú Inicio (y opcionalmente en el escritorio).
- Incluye un **desinstalador** (Configuración → Aplicaciones → CornellBox).

---

## Notas

- **No hace falta tener MSYS2 ni nada instalado en la PC destino**: las DLL van
  dentro del instalador.
- Si compilaste con **Visual Studio** en vez de MSYS2, simplemente apunta el
  script a tu carpeta de salida (que contenga `cornellbox.exe`, `shaders\` y los
  DLL `glew32.dll`/`glfw3.dll`):
  ```bat
  ISCC.exe /DSourceDir="C:\ruta\a\tu\salida" cornellbox.iss
  ```
- Para cambiar versión, nombre o autor, edita las líneas `#define` al inicio de
  `cornellbox.iss`.
- La carpeta `installer\Output\` (el instalador generado) está en `.gitignore`;
  no se sube al repositorio.

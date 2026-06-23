# Caja de Cornell — Modelos Globales de Iluminación (C++ / OpenGL)

Programa de demostración del informe *Modelos Globales de Iluminación en la
Computación Gráfica*. Renderiza la caja de Cornell con **seis algoritmos de
iluminación conmutables en vivo** mediante un *path tracer* progresivo en GPU
(GLSL) y un menú interactivo (Dear ImGui). Resolución hasta **1080p**.

La documentación completa de diseño está en
[`../Implementacion_OpenGL_Cornell_Box.md`](../Implementacion_OpenGL_Cornell_Box.md).

## Algoritmos
| # | Algoritmo | Sección del informe |
|---|-----------|---------------------|
| 0 | Iluminación local (Phong) | 2.1 |
| 1 | Trazado de rayos (Whitted) | 3.1 |
| 2 | Radiosidad (GI difusa) | 3.2 |
| 3 | Path tracing | 3.3 |
| 4 | Mapeo de fotones (cáusticas) | 3.4 |
| 5 | Oclusión ambiental / GI tiempo real | 3.5 |

## Galería (renders a 1080p)

La misma caja de Cornell con cada algoritmo (generados con el modo headless):

| Iluminación local (Phong) | Trazado de rayos (Whitted) |
|:---:|:---:|
| ![local](capturas/00_local.png) | ![whitted](capturas/01_whitted.png) |
| **Radiosidad (GI difusa)** | **Path tracing** |
| ![radiosidad](capturas/02_radiosidad.png) | ![path](capturas/03_pathtracing.png) |
| **Mapeo de fotones (cáusticas)** | **Oclusión ambiental** |
| ![fotones](capturas/04_fotones.png) | ![ao](capturas/05_ao.png) |

Vista “solo oclusión” del modo de tiempo real (lo que realmente calcula el AO):

![ao solo oclusión](capturas/05_ao_solo.png)

## Requisitos
- Compilador C++17 (g++ / clang / MSVC).
- **OpenGL 4.3+** (la app usa *compute shaders*). Cualquier GPU de la última
  década sirve.
- Bibliotecas **GLFW 3** y **GLEW**.
- Dear ImGui y `stb_image_write.h` **ya vienen incluidos** en `third_party/`.

---

## 🐧 Linux (Arch / CachyOS, Ubuntu/Debian, Fedora)

### 1. Instalar dependencias
```sh
# Arch / CachyOS
sudo pacman -S --needed glfw glew mesa gcc make

# Ubuntu / Debian
sudo apt install build-essential libglfw3-dev libglew-dev libgl1-mesa-dev

# Fedora
sudo dnf install gcc-c++ make glfw-devel glew-devel mesa-libGL-devel
```

### 2. Compilar y ejecutar
```sh
cd cornellbox
make
./cornellbox          # o:  make run
```

> Sesiones **Wayland**: el programa ya solicita el backend X11 de GLFW
> (funciona sobre XWayland) porque GLEW usa GLX. No hay que hacer nada extra.

---

## 🪟 Windows

Hay dos caminos. El **A (MSYS2)** es el más sencillo porque reutiliza el mismo
`Makefile` que en Linux.

### Opción A — MSYS2 / MinGW-w64 (recomendada)

1. Instala **[MSYS2](https://www.msys2.org/)** y abre la terminal
   **“MSYS2 MinGW x64”** (la azul, no la del entorno UCRT/MSYS).
2. Instala el *toolchain* y las dependencias:
   ```sh
   pacman -S --needed \
     mingw-w64-x86_64-gcc \
     mingw-w64-x86_64-make \
     mingw-w64-x86_64-pkgconf \
     mingw-w64-x86_64-glfw \
     mingw-w64-x86_64-glew
   ```
3. Compila y ejecuta (el `Makefile` detecta Windows y enlaza `opengl32`/`gdi32`):
   ```sh
   cd cornellbox
   mingw32-make            # genera cornellbox.exe
   ./cornellbox.exe        # o:  mingw32-make run
   ```
   > Si `mingw32-make` no existe, usa `make`. Para ejecutar el `.exe` desde el
   > Explorador de Windows, copia junto a él los DLL
   > `glfw3.dll`, `glew32.dll` (están en `C:\msys64\mingw64\bin`), o lánzalo
   > siempre desde la terminal MinGW.

### Opción B — Visual Studio + vcpkg

1. Instala **[vcpkg](https://vcpkg.io/)** y las librerías:
   ```bat
   vcpkg install glfw3 glew
   vcpkg integrate install
   ```
2. Crea un proyecto **C++ de consola** en Visual Studio y añade al proyecto:
   - Todos los `.cpp` de `cornellbox/src/`.
   - De `cornellbox/third_party/imgui/`: `imgui.cpp`, `imgui_draw.cpp`,
     `imgui_tables.cpp`, `imgui_widgets.cpp`.
   - De `cornellbox/third_party/imgui/backends/`: `imgui_impl_glfw.cpp` e
     `imgui_impl_opengl3.cpp`.
3. En *Propiedades del proyecto*:
   - **C/C++ → General → Directorios de inclusión adicionales:**
     `cornellbox\src; cornellbox\third_party; cornellbox\third_party\imgui; cornellbox\third_party\imgui\backends`
   - **C/C++ → Preprocesador → Definiciones:** añade
     `SHADER_DIR="C:/ruta/a/cornellbox/shaders"` (usa `/` y deja las comillas).
   - **Enlazador → Entrada → Dependencias adicionales:** `glfw3.lib;glew32.lib;opengl32.lib`
   - Estándar de lenguaje: **C++17**.
4. Compila en **x64** y ejecuta. Copia `glew32.dll` junto al `.exe` si hace falta.

> En cualquiera de las dos opciones, ejecuta el programa con la carpeta `shaders/`
> accesible en la ruta indicada por `SHADER_DIR`.

---

## Ejecutar en otra computadora

Este programa es una aplicación **GUI con GPU** (OpenGL 4.3 + *compute shaders*).
Su rendimiento depende de la GPU de la máquina destino, **no** del método de
empaquetado. Requisito mínimo: una GPU con **OpenGL 4.3+** (cualquiera de la
última década, incluidas las integradas Intel HD 500+). Opciones, de más a menos
recomendable:

### 1. Compilar en la PC destino (recomendado)
Es lo más fiable y da el mejor rendimiento. Sigue las instrucciones de *Linux* o
*Windows* de arriba; compila en segundos. Como el binario busca los shaders junto
a sí mismo, no hay rutas que ajustar.

### 2. Instalador de Windows (`CornellBox-Setup.exe`)
Para entregar a usuarios de Windows un instalador con un clic (menú Inicio +
desinstalador, sin necesidad de instalar nada más). El script de **Inno Setup**
y la guía paso a paso están en
[`installer/LEEME_INSTALADOR.md`](installer/LEEME_INSTALADOR.md). En resumen, en
una máquina Windows: `mingw32-make dist` → compilar `installer/cornellbox.iss`
con Inno Setup → `CornellBox-Setup.exe`.

### 3. Carpeta portable (sin compilar en destino)
En la máquina donde ya compila, genera un paquete:
```sh
make dist          # crea dist/cornellbox/ con el binario + shaders/ (+ DLLs en Windows)
```
Copia esa carpeta a la otra PC y ejecuta el binario desde dentro. Condiciones:
- **Misma familia de SO/arquitectura** (un binario de Linux x64 no corre en
  Windows y viceversa: hay que generar `dist` en cada plataforma).
- En **Windows**, `make dist` ya copia los DLL necesarios automáticamente.
- En **Linux**, la PC destino necesita las libs del sistema instaladas
  (`glfw`, `glew`); si no, ver la opción AppImage abajo.

### 4. Linux: archivo único con AppImage (opcional)
Para un único archivo que corra en casi cualquier distro sin instalar nada,
empaqueta con [`linuxdeploy`](https://github.com/linuxdeploy/linuxdeploy)
(incluye las libs dentro del AppImage). Útil si no quieres pedir que instalen
`glfw`/`glew` en la PD destino. Sigue necesitando una GPU con OpenGL 4.3+.

### ¿Y Docker? — No recomendado aquí
Docker está pensado para servicios/CLI, no para apps gráficas con GPU. Para
correr esto en un contenedor necesitarías **pasar la GPU** (NVIDIA Container
Toolkit, solo en host Linux + NVIDIA) **y el display** (montar el socket X11):
es frágil, atado al host y **no mejora el rendimiento**. Para una exposición es
justo lo que conviene evitar. Usa la opción 1 o 2.

### Rendimiento y plan B
El render es progresivo: en una GPU más lenta **no se rompe**, solo tarda más en
limpiarse el ruido. Si la PC destino es floja, baja la **resolución** y las
**muestras/frame** desde el menú. Como respaldo para la exposición, lleva también
las **capturas/video pregrabados** (carpeta `capturas/` o el modo headless), tal
como aconseja el informe.

## Modo headless (genera PNG sin abrir ventana)
Útil para las figuras del informe y la tabla de métricas:
```sh
# ./cornellbox --render <modo> <escena> <spp/frame> <frames> <resIdx> [salida.png] [aoView]
./cornellbox --render 3 3 4 200 2 path.png        # path tracing, escena completa, 720p
./cornellbox --render 5 3 4 80  2 ao_gris.png 1   # AO en escala de grises
```
`resIdx`: 0=360p 1=540p 2=720p 3=900p 4=1080p · `aoView`: 0=combinada 1=solo oclusión.

## Controles (modo interactivo)
- **Arrastrar** con el ratón: orbitar la cámara.
- **Rueda**: acercar / alejar.
- Menú: algoritmo, escena, resolución, muestras, rebotes, luz, materiales,
  tone mapping, captura PNG, reinicio de acumulación.

## Licencia
Código bajo licencia **MIT** (ver [`../LICENSE`](../LICENSE)). Las dependencias
de `third_party/` conservan sus propias licencias (Dear ImGui: MIT; stb: dominio
público).

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

# Makefile - Cornell Box: Modelos Globales de Iluminacion (C++ / OpenGL)
# Linux  : probado en CachyOS/Arch (g++ 16, GLFW 3.4, GLEW 2.3, OpenGL 4.6 core).
# Windows: usar MSYS2/MinGW-w64 (ver README.md). Detecta la plataforma abajo.

BUILD_DIR  := build
SRC_DIR    := src
IMGUI_DIR  := third_party/imgui

# --- Deteccion de plataforma ---
UNAME_S := $(shell uname -s 2>/dev/null)
ifneq (,$(filter MINGW% MSYS% CYGWIN% Windows%,$(UNAME_S)))
    # Windows con MSYS2/MinGW: OpenGL es opengl32; ventana via gdi32.
    PLATFORM_LIBS := -lopengl32 -lgdi32
    APP := cornellbox.exe
    RUN := ./cornellbox.exe
    RM  := rm -rf
else
    # Linux (y similares): GLX/X11.
    PLATFORM_LIBS := -lGL -ldl -lpthread
    APP := cornellbox
    RUN := ./cornellbox
    RM  := rm -rf
endif

# Ruta absoluta a la carpeta de shaders (para poder ejecutar desde cualquier sitio)
SHADER_DIR := $(abspath shaders)

CXX        := g++
CXXSTD     := -std=c++17
WARN       := -Wall -Wextra -Wno-unused-parameter
OPT        := -O2

# Dependencias del sistema via pkg-config
PKGS       := glfw3 glew
PKG_CFLAGS := $(shell pkg-config --cflags $(PKGS))
PKG_LIBS   := $(shell pkg-config --libs   $(PKGS))

CXXFLAGS := $(CXXSTD) $(WARN) $(OPT) \
            -I$(SRC_DIR) -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -Ithird_party \
            $(PKG_CFLAGS) \
            -DSHADER_DIR=\"$(SHADER_DIR)\"

LDLIBS   := $(PKG_LIBS) $(PLATFORM_LIBS)

# --- Fuentes ---
APP_SRC   := $(wildcard $(SRC_DIR)/*.cpp)
IMGUI_SRC := $(IMGUI_DIR)/imgui.cpp \
             $(IMGUI_DIR)/imgui_draw.cpp \
             $(IMGUI_DIR)/imgui_tables.cpp \
             $(IMGUI_DIR)/imgui_widgets.cpp \
             $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp \
             $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp

ALL_SRC := $(APP_SRC) $(IMGUI_SRC)
OBJ     := $(addprefix $(BUILD_DIR)/,$(notdir $(ALL_SRC:.cpp=.o)))

# Permite encontrar los .cpp de imgui ademas de los de src
VPATH := $(SRC_DIR):$(IMGUI_DIR):$(IMGUI_DIR)/backends

.PHONY: all clean run dist

all: $(APP)

$(APP): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)
	@echo "==> Compilado: ./$(APP)"

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Recompila si cambia algun header (.d generados con -MMD).
-include $(wildcard $(BUILD_DIR)/*.d)

run: $(APP)
	$(RUN)

# Empaqueta una carpeta portable (binario + shaders) para copiar a otra PC.
# El binario busca los shaders junto a si mismo, asi que basta copiar dist/cornellbox/.
# En Windows (MSYS2) ademas copia automaticamente los DLL de MinGW necesarios,
# dejando la carpeta lista para el instalador de Inno Setup.
DIST_DIR := dist/cornellbox
dist: $(APP)
	@mkdir -p $(DIST_DIR)/shaders
	@cp $(APP) $(DIST_DIR)/
	@cp shaders/* $(DIST_DIR)/shaders/
	@cp README.md $(DIST_DIR)/ 2>/dev/null || true
	@cp ../LICENSE $(DIST_DIR)/ 2>/dev/null || true
	@if echo "$(UNAME_S)" | grep -qiE 'mingw|msys'; then \
	  echo "    Copiando DLLs de MinGW (ldd)..."; \
	  for dll in $$(ldd $(APP) | grep -iE 'mingw64|/ucrt64' | awk '{print $$3}'); do \
	    [ -f "$$dll" ] && cp "$$dll" $(DIST_DIR)/ || true; \
	  done; \
	fi
	@echo "==> Bundle listo en cornellbox/$(DIST_DIR)/"
	@echo "    Para el instalador de Windows: ver cornellbox/installer/LEEME_INSTALADOR.md"

clean:
	$(RM) $(BUILD_DIR) $(APP) dist

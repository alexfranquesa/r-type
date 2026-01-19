#!/bin/bash

# =============================================================================
# Script de Verificación Local del Pipeline CI
# =============================================================================
# Este script reproduce localmente los pasos del CI de GitHub Actions
# para detectar problemas antes de hacer push.
# =============================================================================

set -e  # Exit on error

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Función para logging
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[✓]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[⚠]${NC} $1"
}

log_error() {
    echo -e "${RED}[✗]${NC} $1"
}

# Banner
echo ""
echo "═══════════════════════════════════════════════════════════"
echo "  R-Type CI/CD - Verificación Local (Linux)"
echo "═══════════════════════════════════════════════════════════"
echo ""

# Verificar que estamos en el directorio correcto
if [ ! -f "CMakeLists.txt" ]; then
    log_error "No se encontró CMakeLists.txt. Ejecuta este script desde el root del proyecto."
    exit 1
fi

log_success "Directorio del proyecto: $(pwd)"
echo ""

# =============================================================================
# PASO 1: Verificar herramientas necesarias
# =============================================================================
log_info "Paso 1: Verificando herramientas necesarias..."

command -v cmake >/dev/null 2>&1 || { log_error "cmake no está instalado. Abortando."; exit 1; }
command -v git >/dev/null 2>&1 || { log_error "git no está instalado. Abortando."; exit 1; }
command -v pkg-config >/dev/null 2>&1 || { log_warning "pkg-config no está instalado (recomendado)."; }

CMAKE_VERSION=$(cmake --version | head -n1 | awk '{print $3}')
log_success "cmake versión: $CMAKE_VERSION"

GCC_VERSION=$(gcc --version | head -n1 | awk '{print $3}')
log_success "gcc versión: $GCC_VERSION"

echo ""

# =============================================================================
# PASO 2: Verificar/instalar dependencias del sistema
# =============================================================================
log_info "Paso 2: Verificando dependencias del sistema..."

MISSING_DEPS=""

# Lista de paquetes necesarios para SFML
REQUIRED_PACKAGES=(
    "libx11-dev"
    "libxrandr-dev"
    "libxcursor-dev"
    "libxi-dev"
    "libudev-dev"
    "libgl1-mesa-dev"
    "libflac-dev"
    "libogg-dev"
    "libvorbis-dev"
    "libopenal-dev"
    "libfreetype-dev"
)

for pkg in "${REQUIRED_PACKAGES[@]}"; do
    if ! dpkg -l | grep -qw "$pkg"; then
        MISSING_DEPS="$MISSING_DEPS $pkg"
    fi
done

if [ -n "$MISSING_DEPS" ]; then
    log_warning "Faltan dependencias:$MISSING_DEPS"
    log_info "Para instalarlas, ejecuta:"
    echo "  sudo apt-get update && sudo apt-get install -y$MISSING_DEPS"
    echo ""
    read -p "¿Continuar de todas formas? (y/n): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
else
    log_success "Todas las dependencias del sistema están instaladas"
fi

echo ""

# =============================================================================
# PASO 3: Clonar/actualizar vcpkg
# =============================================================================
log_info "Paso 3: Configurando vcpkg..."

if [ ! -d "vcpkg" ]; then
    log_info "Clonando vcpkg (puede tardar unos minutos)..."
    git clone https://github.com/microsoft/vcpkg.git
    log_success "vcpkg clonado"
else
    log_success "vcpkg ya existe"
fi

# Bootstrap vcpkg si no está compilado
if [ ! -f "vcpkg/vcpkg" ]; then
    log_info "Bootstrapping vcpkg (primera vez, puede tardar)..."
    ./vcpkg/bootstrap-vcpkg.sh -disableMetrics
    log_success "vcpkg bootstrap completado"
else
    log_success "vcpkg ya está compilado"
fi

VCPKG_VERSION=$(./vcpkg/vcpkg version | head -n1)
log_success "vcpkg versión: $VCPKG_VERSION"

echo ""

# =============================================================================
# PASO 4: Instalar dependencias (manifest mode)
# =============================================================================
log_info "Paso 4: Instalando dependencias via vcpkg..."
log_info "Esto puede tardar varios minutos en la primera ejecución..."

./vcpkg/vcpkg install --triplet=x64-linux

log_success "Dependencias instaladas"
echo ""

# =============================================================================
# PASO 5: Configurar con CMake
# =============================================================================
log_info "Paso 5: Configurando proyecto con CMake (preset: linux-release)..."

cmake --preset linux-release

log_success "Configuración completada"
echo ""

# =============================================================================
# PASO 6: Compilar
# =============================================================================
log_info "Paso 6: Compilando proyecto..."

NUM_CORES=$(nproc)
log_info "Usando $NUM_CORES cores..."

cmake --build --preset linux-release --parallel $NUM_CORES

log_success "Compilación completada"
echo ""

# =============================================================================
# PASO 7: Ejecutar tests
# =============================================================================
log_info "Paso 7: Ejecutando tests..."

# Verificar si xvfb está disponible (para tests gráficos sin GUI)
if command -v xvfb-run >/dev/null 2>&1; then
    log_info "Usando xvfb-run para tests gráficos..."
    xvfb-run -a ctest --preset linux-release --output-on-failure --verbose
else
    log_warning "xvfb no disponible, ejecutando tests directamente (puede fallar sin GUI)"
    ctest --preset linux-release --output-on-failure --verbose
fi

log_success "Tests completados"
echo ""

# =============================================================================
# PASO 8: Verificar binarios
# =============================================================================
log_info "Paso 8: Verificando binarios generados..."

if [ -f "build/linux-release/rtype_server" ]; then
    log_success "✓ rtype_server compilado"
    ls -lh build/linux-release/rtype_server
else
    log_error "✗ rtype_server NO encontrado"
fi

if [ -f "build/linux-release/rtype_client" ]; then
    log_success "✓ rtype_client compilado"
    ls -lh build/linux-release/rtype_client
else
    log_error "✗ rtype_client NO encontrado"
fi

if [ -f "build/linux-release/rtype_tests" ]; then
    log_success "✓ rtype_tests compilado"
    ls -lh build/linux-release/rtype_tests
else
    log_warning "⚠ rtype_tests NO encontrado (puede ser normal si tests están deshabilitados)"
fi

echo ""

# =============================================================================
# PASO 9: Verificar dependencias dinámicas
# =============================================================================
log_info "Paso 9: Verificando dependencias SFML..."

log_info "Dependencias de rtype_client:"
ldd build/linux-release/rtype_client | grep -i sfml || log_warning "No se encontraron libs SFML (puede ser estático)"

echo ""

# =============================================================================
# RESUMEN FINAL
# =============================================================================
echo "═══════════════════════════════════════════════════════════"
echo -e "${GREEN}  ✓ VERIFICACIÓN COMPLETADA EXITOSAMENTE${NC}"
echo "═══════════════════════════════════════════════════════════"
echo ""
echo "Próximos pasos:"
echo "  1. Revisar output de tests arriba"
echo "  2. (Opcional) Ejecutar: ./build/linux-release/rtype_tests"
echo "  3. Si todo está bien, hacer commit y push"
echo ""
echo "El CI de GitHub Actions ejecutará los mismos pasos en Ubuntu y Windows."
echo ""

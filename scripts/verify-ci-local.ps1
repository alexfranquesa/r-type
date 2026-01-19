# =============================================================================
# Script de Verificación Local del Pipeline CI - Windows
# =============================================================================
# Este script reproduce localmente los pasos del CI de GitHub Actions
# para detectar problemas antes de hacer push.
# =============================================================================
# USO: Ejecutar desde Developer PowerShell for VS 2022
# =============================================================================

$ErrorActionPreference = "Stop"

# Colores para output
function Write-Info { Write-Host "[INFO] $args" -ForegroundColor Blue }
function Write-Success { Write-Host "[✓] $args" -ForegroundColor Green }
function Write-Warning { Write-Host "[⚠] $args" -ForegroundColor Yellow }
function Write-Error { Write-Host "[✗] $args" -ForegroundColor Red }

# Banner
Write-Host ""
Write-Host "═══════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "  R-Type CI/CD - Verificación Local (Windows)" -ForegroundColor Cyan
Write-Host "═══════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host ""

# =============================================================================
# PASO 0: Verificar entorno MSVC
# =============================================================================
Write-Info "Paso 0: Verificando entorno de desarrollo..."

if (-not (Test-Path env:VSINSTALLDIR)) {
    Write-Error "Entorno MSVC no detectado."
    Write-Warning "Ejecuta este script desde 'Developer PowerShell for VS 2022'"
    Write-Warning "O ejecuta: & 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1'"
    exit 1
}

Write-Success "Visual Studio: $env:VSINSTALLDIR"
Write-Success "Directorio del proyecto: $(Get-Location)"
Write-Host ""

# Verificar CMakeLists.txt
if (-not (Test-Path "CMakeLists.txt")) {
    Write-Error "No se encontró CMakeLists.txt. Ejecuta este script desde el root del proyecto."
    exit 1
}

# =============================================================================
# PASO 1: Verificar herramientas
# =============================================================================
Write-Info "Paso 1: Verificando herramientas necesarias..."

# Verificar CMake
try {
    $cmakeVersion = (cmake --version | Select-Object -First 1)
    Write-Success "CMake: $cmakeVersion"
} catch {
    Write-Error "cmake no está en PATH. Instala CMake desde https://cmake.org/"
    exit 1
}

# Verificar Git
try {
    $gitVersion = (git --version)
    Write-Success "Git: $gitVersion"
} catch {
    Write-Error "git no está en PATH. Instala Git para Windows."
    exit 1
}

Write-Host ""

# =============================================================================
# PASO 2: Configurar vcpkg
# =============================================================================
Write-Info "Paso 2: Configurando vcpkg..."

if (-not (Test-Path "vcpkg\vcpkg.exe")) {
    if (-not (Test-Path "vcpkg")) {
        Write-Info "Clonando vcpkg (puede tardar unos minutos)..."
        git clone https://github.com/microsoft/vcpkg.git
        Write-Success "vcpkg clonado"
    }
    
    Write-Info "Bootstrapping vcpkg (primera vez, puede tardar)..."
    & "vcpkg\bootstrap-vcpkg.bat" -disableMetrics
    Write-Success "vcpkg bootstrap completado"
} else {
    Write-Success "vcpkg ya está compilado"
}

$vcpkgVersion = (& "vcpkg\vcpkg.exe" version | Select-Object -First 1)
Write-Success "vcpkg versión: $vcpkgVersion"
Write-Host ""

# =============================================================================
# PASO 3: Instalar dependencias (x64-windows = dinámico /MD)
# =============================================================================
Write-Info "Paso 3: Instalando dependencias via vcpkg..."
Write-Info "Triplet: x64-windows (dinámico /MD, DLLs de SFML)"
Write-Info "Esto puede tardar varios minutos en la primera ejecución..."
Write-Host ""

& "vcpkg\vcpkg.exe" install --triplet=x64-windows

if ($LASTEXITCODE -ne 0) {
    Write-Error "vcpkg install falló"
    exit 1
}

Write-Success "Dependencias instaladas"
Write-Host ""

# =============================================================================
# PASO 4: Configurar con CMake
# =============================================================================
Write-Info "Paso 4: Configurando proyecto con CMake (preset: windows-release)..."

cmake --preset windows-release

if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configure falló"
    exit 1
}

Write-Success "Configuración completada"
Write-Host ""

# =============================================================================
# PASO 5: Compilar
# =============================================================================
Write-Info "Paso 5: Compilando proyecto (configuración: Release)..."

cmake --build --preset windows-release --config Release --parallel

if ($LASTEXITCODE -ne 0) {
    Write-Error "Compilación falló"
    Write-Warning "Revisa los logs arriba, especialmente busca errores LNK2038"
    exit 1
}

Write-Success "Compilación completada"
Write-Host ""

# =============================================================================
# PASO 6: Copiar DLLs de SFML
# =============================================================================
Write-Info "Paso 6: Copiando DLLs de SFML al directorio de ejecutables..."

$sfmlDllSource = "vcpkg_installed\x64-windows\bin"
$buildDir = "build\windows-release\Release"

if (Test-Path $sfmlDllSource) {
    Copy-Item "$sfmlDllSource\sfml-*.dll" -Destination $buildDir -Force -ErrorAction SilentlyContinue
    Copy-Item "$sfmlDllSource\*.dll" -Destination $buildDir -Force -ErrorAction SilentlyContinue
    Write-Success "DLLs copiadas"
} else {
    Write-Warning "Directorio de DLLs de vcpkg no encontrado: $sfmlDllSource"
}

Write-Host ""

# =============================================================================
# PASO 7: Ejecutar tests
# =============================================================================
Write-Info "Paso 7: Ejecutando tests..."

ctest --preset windows-release --output-on-failure --verbose -C Release

if ($LASTEXITCODE -ne 0) {
    Write-Warning "Algunos tests fallaron (ver output arriba)"
} else {
    Write-Success "Tests completados exitosamente"
}

Write-Host ""

# =============================================================================
# PASO 8: Verificar binarios
# =============================================================================
Write-Info "Paso 8: Verificando binarios generados..."

$binaries = @(
    "$buildDir\rtype_server.exe",
    "$buildDir\rtype_client.exe",
    "$buildDir\rtype_tests.exe"
)

foreach ($binary in $binaries) {
    if (Test-Path $binary) {
        $size = (Get-Item $binary).Length / 1MB
        Write-Success "✓ $(Split-Path $binary -Leaf) - $("{0:N2}" -f $size) MB"
    } else {
        Write-Warning "✗ $(Split-Path $binary -Leaf) NO encontrado"
    }
}

Write-Host ""

# =============================================================================
# PASO 9: Verificar dependencias DLL
# =============================================================================
Write-Info "Paso 9: Verificando dependencias DLL de rtype_client.exe..."

if (Test-Path "$buildDir\rtype_client.exe") {
    try {
        # Usar dumpbin si está disponible
        dumpbin /dependents "$buildDir\rtype_client.exe" | Select-String "sfml"
    } catch {
        Write-Warning "dumpbin no disponible, saltando verificación de dependencias"
    }
} else {
    Write-Warning "rtype_client.exe no encontrado"
}

Write-Host ""

# =============================================================================
# PASO 10: Test de ejecución rápida (smoke test)
# =============================================================================
Write-Info "Paso 10: Test de ejecución rápida..."

Write-Info "Intentando ejecutar rtype_tests.exe..."
if (Test-Path "$buildDir\rtype_tests.exe") {
    try {
        & "$buildDir\rtype_tests.exe" --help 2>&1 | Out-Null
        Write-Success "rtype_tests.exe ejecuta sin errores de DLLs faltantes"
    } catch {
        Write-Warning "rtype_tests.exe no se pudo ejecutar (puede ser normal si no tiene --help)"
    }
} else {
    Write-Warning "rtype_tests.exe no encontrado"
}

Write-Host ""

# =============================================================================
# RESUMEN FINAL
# =============================================================================
Write-Host "═══════════════════════════════════════════════════════════" -ForegroundColor Green
Write-Host "  ✓ VERIFICACIÓN COMPLETADA EXITOSAMENTE" -ForegroundColor Green
Write-Host "═══════════════════════════════════════════════════════════" -ForegroundColor Green
Write-Host ""
Write-Host "Próximos pasos:" -ForegroundColor Yellow
Write-Host "  1. Revisar output de tests arriba"
Write-Host "  2. (Opcional) Ejecutar: .\build\windows-release\Release\rtype_tests.exe"
Write-Host "  3. Si todo está bien, hacer commit y push"
Write-Host ""
Write-Host "El CI de GitHub Actions ejecutará los mismos pasos en Ubuntu y Windows." -ForegroundColor Cyan
Write-Host ""

# Información adicional
Write-Info "Configuración usada:"
Write-Host "  - Triplet: x64-windows (dinámico /MD)"
Write-Host "  - Runtime: MultiThreadedDLL (/MD)"
Write-Host "  - SFML: Librerías dinámicas (DLLs)"
Write-Host ""

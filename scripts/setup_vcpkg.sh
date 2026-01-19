#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd -- "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VCPKG_DIR="${ROOT_DIR}/vcpkg"

if [[ -d "${VCPKG_DIR}/.git" ]]; then
    echo "[setup_vcpkg] Existing vcpkg checkout detected at ${VCPKG_DIR}"
else
    echo "[setup_vcpkg] Cloning vcpkg..."
    git clone https://github.com/microsoft/vcpkg.git "${VCPKG_DIR}"
fi

echo "[setup_vcpkg] Bootstrapping..."
"${VCPKG_DIR}/bootstrap-vcpkg.sh"

echo "[setup_vcpkg] Done. You can now run: cmake --preset linux-debug"

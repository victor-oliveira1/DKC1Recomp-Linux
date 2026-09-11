#!/usr/bin/env bash
set -euo pipefail

if [[ "$(uname -s)" != "Linux" ]]; then
  echo "error: build_linux.sh must run on Linux" >&2
  exit 2
fi

repo_dir="$(cd "$(dirname "$0")" && pwd)"
build_dir="${DKC1_BUILD_DIR:-$repo_dir/build/linux}"
rom_path="${1:-${DKC1_ROM:-}}"

for tool in cmake ninja python3; do
  if ! command -v "$tool" >/dev/null 2>&1; then
    echo "error: missing required tool: $tool" >&2
    echo "Install CMake, Ninja, and SDL2 development libraries (e.g., sudo apt install cmake ninja-build libsdl2-dev)." >&2
    exit 2
  fi
done

if ! pkg-config --exists sdl2; then
  echo "error: missing required library: SDL2" >&2
  echo "Install SDL2 development libraries (e.g., sudo apt install libsdl2-dev)." >&2
  exit 2
fi

if ! compgen -G "$repo_dir/generated/snesrecomp/*.c" >/dev/null; then
  if [[ -z "$rom_path" ]]; then
    echo "error: private generated sources are missing" >&2
    echo "usage: ./build_linux.sh '/path/to/Donkey Kong Country (USA).sfc'" >&2
    exit 2
  fi
  python3 "$repo_dir/scripts/generate_snesrecomp.py" --rom "$rom_path"
fi

cmake -S "$repo_dir" -B "$build_dir" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DSNESRECOMP_SDL_BACKEND=SDL2

cmake --build "$build_dir" --target dkc1_linux --parallel

echo "LINUX_BUILD_OK"
echo "$build_dir"

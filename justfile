# Default recipe — lists all available recipes
default:
    @just --list

# ---- Helpers ---------------------------------------------------------------

# Map a preset name to its CMake build type
[private]
build-type preset:
    #!/usr/bin/env sh
    case "{{ preset }}" in
        *-dbg*) echo "Debug"          ;;
        *-rdi*) echo "RelWithDebInfo" ;;
        *-rel*) echo "Release"        ;;
        *-min*) echo "MinSizeRel"     ;;
        *) echo "Unknown preset pattern: {{ preset }}" >&2; exit 1 ;;
    esac

# ---- Core workflow ---------------------------------------------------------

# Configure via CMake preset
configure preset:
    cmake --preset {{ preset }}

# Build via CMake preset (runs configure first)
build preset: (configure preset)
    cmake --build --preset build-{{ preset }}

# Configure + build in one shot (alias for build)
all preset: (build preset)

# Run a built executable from build/bin/ (uses fzf picker if no binary given)
run *args:
    #!/usr/bin/env sh
    if [ -z "$(ls -A build/bin 2>/dev/null)" ]; then
        echo "No binaries found in build/bin/" >&2; exit 1
    fi
    if [ "{{ args }}" = "" ] && command -v fzf >/dev/null 2>&1; then
        binary=$(ls build/bin/ | fzf --prompt="Select binary: ")
        [ -z "$binary" ] && exit 0
        ./build/bin/"$binary"
    elif [ "{{ args }}" = "" ]; then
        echo "Available binaries (install fzf for interactive picker):" >&2
        ls build/bin/ >&2
        echo "" >&2
        echo "Usage: just run <binary> [args...]" >&2
        exit 1
    else
        ./build/bin/{{ args }}
    fi

# Build then immediately run a binary (uses fzf picker if no binary given)
build-run preset *args: (build preset)
    just run {{ args }}

# ---- Utility ---------------------------------------------------------------

# Remove the build directory
clean:
    rm -rf build

# Remove build and the local Conan cache for this project
clean-all: clean
    conan remove "*" --confirm

# List all available CMake configure presets
list-presets:
    cmake --list-presets

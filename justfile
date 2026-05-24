# PebbleConduct Justfile

# Default recipe - show available commands
default:
    @just --list

# Build the application for all platforms
build:
    pebble build

# Build for a specific platform (aplite, basalt, chalk, diorite, emery)
build-platform platform:
    pebble build --platform {{platform}}

# Clean build artifacts
clean:
    pebble clean

# Install on emulator (default: basalt for Pebble Time)
install-emulator platform="basalt": build
    pebble install --emulator {{platform}}

# Install on physical watch via CloudPebble
install-watch: build
    pebble install --cloudpebble

# Install and log on physical watch via CloudPebble
debug-watch: install-watch
    pebble logs --cloudpebble

# Download and convert an SVG icon for use in the app
# Usage: just add-icon <url> [name] [size]
add-icon url name="" size="18":
    @if [ -z "{{name}}" ]; then \
        ./tools/download-and-convert-icon.sh "{{url}}" "{{size}}"; \
    else \
        ./tools/download-and-convert-icon.sh "{{url}}" "{{name}}" "{{size}}"; \
    fi

# Convert a local SVG icon to PNG
# Usage: just convert-icon <input.svg> [output.png] [size]
convert-icon input output="" size="18":
    @if [ -z "{{output}}" ]; then \
        node ./tools/convert-icon.js "{{input}}" "{{size}}"; \
    else \
        node ./tools/convert-icon.js "{{input}}" "{{output}}" "{{size}}"; \
    fi

# Install on phone via USB (requires connected phone)
install-phone: build
    pebble install --phone

# Login to Pebble account
login:
    pebble login

# Logout from Pebble account
logout:
    pebble logout

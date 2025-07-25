default: help

##@ Helpers
.PHONY: help
help:  ## Display this help
	@awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n  make \033[36m<target>\033[0m\n"} /^[a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)

##@ Build
.PHONY: build
build:  ## Build the project
	make -C src

##@ WebAssembly
.PHONY: wasm
wasm:  ## Build WebAssembly version for web
	@echo "Building WebAssembly version..."
	@if ! command -v emcc >/dev/null 2>&1; then \
		echo "Error: Emscripten (emcc) not found. Please install Emscripten first:"; \
		echo "  git clone https://github.com/emscripten-core/emsdk.git"; \
		echo "  cd emsdk"; \
		echo "  ./emsdk install latest"; \
		echo "  ./emsdk activate latest"; \
		echo "  source ./emsdk_env.sh"; \
		exit 1; \
	fi
	@mkdir -p web
	emcc -std=c++17 \
		-Iimgui -Iimgui/backends -Iinclude \
		-D__EMSCRIPTEN__ \
		-O2 -s WASM=1 -s USE_WEBGL2=1 -s USE_GLFW=3 \
		-s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
		-s EXPORTED_FUNCTIONS='["_main","_malloc","_free"]' \
		-s USE_WEBGL2=1 \
		-s FULL_ES3=1 \
		--preload-file assets@/assets \
		--use-preload-plugins \
		-o web/NotARhythmGame.js \
		src/*.cpp imgui/*.cpp imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp \
		-lm
	@echo "WebAssembly build complete! Files created in web/ directory"
	@echo "To run: serve the web/ directory with a web server"

.PHONY: wasm-debug
wasm-debug:  ## Build WebAssembly version with debug symbols
	@echo "Building WebAssembly version (debug)..."
	@if ! command -v emcc >/dev/null 2>&1; then \
		echo "Error: Emscripten (emcc) not found. Please install Emscripten first."; \
		exit 1; \
	fi
	@mkdir -p web
	emcc -std=c++17 \
		-Iimgui -Iimgui/backends -Iinclude \
		-D__EMSCRIPTEN__ \
		-g -O0 -s WASM=1 -s USE_WEBGL2=1 -s USE_GLFW=3 \
		-s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
		-s EXPORTED_FUNCTIONS='["_main","_malloc","_free"]' \
		-s EXPORT_NAME="NotARhythmGame" \
		-s MODULARIZE=1 \
		-s USE_WEBGL2=1 \
		-s FULL_ES3=1 \
		--preload-file assets@/assets \
		--use-preload-plugins \
		-o web/NotARhythmGame.js \
		src/*.cpp imgui/*.cpp imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp \
		-lm
	@echo "WebAssembly debug build complete!"

.PHONY: serve-wasm
serve-wasm: wasm  ## Build and serve WebAssembly version locally
	@echo "Starting local web server..."
	@if command -v python3 >/dev/null 2>&1; then \
		cd web && python3 -m http.server 8079; \
	elif command -v python >/dev/null 2>&1; then \
		cd web && python -m SimpleHTTPServer 8079; \
	elif command -v node >/dev/null 2>&1; then \
		npx http-server web -p 8079; \
	else \
		echo "No suitable web server found. Please install python3, python, or node.js"; \
		echo "Or manually serve the web/ directory with any web server"; \
	fi

.PHONY: serve-wasm-prod
serve-wasm-prod:  ## Serve WebAssembly version with production server
	@echo "Checking WebAssembly files..."
	@if [ ! -f web/NotARhythmGame.wasm ]; then \
		echo "Error: WebAssembly file not found: web/NotARhythmGame.wasm"; \
		echo "Please build the WebAssembly version first with: make wasm"; \
		exit 1; \
	fi
	@if [ ! -f web/NotARhythmGame.js ]; then \
		echo "Error: JavaScript file not found: web/NotARhythmGame.js"; \
		echo "Please build the WebAssembly version first with: make wasm"; \
		exit 1; \
	fi
	@echo "Starting production web server..."
	@if command -v nginx >/dev/null 2>&1; then \
		echo "Using nginx for production serving..."; \
		echo "Creating temporary nginx config..."; \
		echo "server { listen 8079; root $(PWD)/web; index index.html; location ~* \.wasm$ { add_header Content-Type application/wasm; } }" > /tmp/notarhythmgame_nginx.conf; \
		echo "Starting nginx on port 8079..."; \
		nginx -c /tmp/notarhythmgame_nginx.conf -g "daemon off;" & \
		echo "Nginx started on http://localhost:8079"; \
		echo "Press Ctrl+C to stop"; \
		wait; \
	elif command -v lighttpd >/dev/null 2>&1; then \
		echo "Using lighttpd for production serving..."; \
		echo "server.document-root = \"$(PWD)/web\"" > /tmp/lighttpd.conf; \
		echo "server.port = 8079" >> /tmp/lighttpd.conf; \
		echo "mimetype.assign = ( \".wasm\" => \"application/wasm\" )" >> /tmp/lighttpd.conf; \
		cd web && lighttpd -D -f /tmp/lighttpd.conf; \
	elif command -v node >/dev/null 2>&1; then \
		echo "Using Node.js http-server for production serving..."; \
		npx http-server web -p 8079 -a 0.0.0.0 --cors -c-1; \
	elif command -v python3 >/dev/null 2>&1; then \
		echo "Using Python3 for production serving..."; \
		cd web && python3 -m http.server 8079; \
	else \
		echo "No production web server found. Please install nginx, lighttpd, or node.js"; \
		echo "Available options:"; \
		echo "  nginx: sudo apt install nginx"; \
		echo "  lighttpd: sudo apt install lighttpd"; \
		echo "  node.js: curl -fsSL https://deb.nodesource.com/setup_lts.x | sudo -E bash - && sudo apt-get install -y nodejs"; \
	fi

##@ Package
.PHONY: package
package:  ## Create distribution package
	./package.sh

##@ Clean
.PHONY: clean
clean:  ## Clean the project
	rm -f NotARhythmGame
	rm -f NotARhythmGame.exe
	rm -rf web/*.wasm
	rm -rf web/*.data
	make -C src clean

##@ Full Clean
.PHONY: fclean
fclean: clean ## Full clean the project
	rm -f src/*.o
	rm -f NotARhythmGame
	rm -f NotARhythmGame.exe
	rm -f NotARhythmGame-Linux.tar.gz
	rm -rf NotARhythmGame-Linux
	rm -rf NotARhythmGame-Windows
	rm -rf NotARhythmGame-Windows.zip
	rm -rf NotARhythmGame-Dist
	rm -rf web/*.wasm
	rm -rf web/*.data

##@ Rebuild
.PHONY: re
re: fclean build  ## Rebuild the project

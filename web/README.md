# Not A Rhythm Game - WebAssembly

This is the WebAssembly version of Not A Rhythm Game, allowing you to run the rhythm game directly in your web browser.

## Prerequisites

### Install Emscripten SDK

1. **Clone the Emscripten repository:**
   ```bash
   git clone https://github.com/emscripten-core/emsdk.git
   cd emsdk
   ```

2. **Install the latest Emscripten version:**
   ```bash
   ./emsdk install latest
   ./emsdk activate latest
   ```

3. **Set up the environment:**
   ```bash
   source ./emsdk_env.sh
   ```

4. **Verify installation:**
   ```bash
   emcc --version
   ```

## Building the WebAssembly Version

### From the project root:

1. **Build the WebAssembly version:**
   ```bash
   make wasm
   ```

2. **Build with debug symbols (for development):**
   ```bash
   make wasm-debug
   ```

3. **Build and serve locally:**
   ```bash
   make serve-wasm
   ```

## Running the Application

### Local Development

After running `make serve-wasm`, open your browser and go to:
```
http://localhost:8000
```

### Manual Server Setup

If you prefer to use your own web server:

1. **Using Python 3:**
   ```bash
   cd web
   python3 -m http.server 8000
   ```

2. **Using Python 2:**
   ```bash
   cd web
   python -m SimpleHTTPServer 8000
   ```

3. **Using Node.js:**
   ```bash
   cd web
   npx http-server -p 8000
   ```

## Production Deployment

### Using Apache (with the provided configuration)

1. **Copy the Apache configuration:**
   ```bash
   sudo cp ../notarhythmgame.maelrabot.com.conf /etc/apache2/sites-available/
   ```

2. **Enable the site:**
   ```bash
   sudo a2ensite notarhythmgame.maelrabot.com.conf
   sudo systemctl reload apache2
   ```

3. **Deploy your files:**
   ```bash
   sudo cp -r * /var/www/notarhythmgame/
   sudo chown -R www-data:www-data /var/www/notarhythmgame
   ```

### Using any web server

Simply copy the contents of the `web/` directory to your web server's document root.

## Browser Compatibility

- **Chrome/Chromium**: 67+
- **Firefox**: 60+
- **Safari**: 11.1+
- **Edge**: 79+

## Features

- ✅ **Full ImGui interface** in the browser
- ✅ **Audio playback** (Web Audio API)
- ✅ **Mouse and keyboard input**
- ✅ **Fullscreen support**
- ✅ **Responsive design**
- ✅ **Loading progress indicator**

## Troubleshooting

### Common Issues

1. **"emcc not found"**
   - Make sure Emscripten is installed and the environment is sourced
   - Run: `source ./emsdk_env.sh`

2. **Canvas not displaying**
   - Check browser console for errors
   - Ensure WebGL2 is supported by your browser
   - Try refreshing the page

3. **Audio not working**
   - WebAssembly version uses stub audio (no actual audio playback)
   - Audio features are simulated for demonstration

4. **Performance issues**
   - Use the optimized build (`make wasm`) for production
   - Use debug build (`make wasm-debug`) for development

### Build Errors

- **Missing dependencies**: Ensure all source files are present
- **Memory issues**: The build includes `ALLOW_MEMORY_GROWTH=1` for dynamic memory allocation
- **WebGL errors**: Check browser WebGL2 support

## Development

### File Structure

```
web/
├── index.html          # Main HTML file
├── NotARhythmGame.js   # Emscripten JavaScript glue
├── NotARhythmGame.wasm # WebAssembly binary
├── assets/             # Preloaded assets
└── README.md          # This file
```

### Customization

- **Styling**: Modify the CSS in `index.html`
- **Loading screen**: Update the loading progress and text
- **Fullscreen**: Customize the fullscreen button behavior

## License

Same as the main project. See the main README for license information.
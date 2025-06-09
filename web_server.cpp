#include "web_server.h"
#include "matrix_display.h"
#include "wifi_manager.h"

void initializeWebServer()
{
    // Server initialization happens in wifi_manager
    server.begin();
    Serial.println("Web server ready");
}

void handleWebClients()
{
    if (wifiStatus == WL_CONNECTED)
    {
        WiFiClient client = server.available();
        if (client)
        {
            handleWebClient(client);
        }
    }
}

void handleWebClient(WiFiClient client)
{
    Serial.println("Client connected");
    String request = "";
    String currentLine = "";

    while (client.connected())
    {
        if (client.available())
        {
            char c = client.read();

            if (c == '\n')
            {
                if (currentLine.length() == 0)
                {
                    processRequest(client, request);
                    break;
                }
                else
                {
                    if (currentLine.startsWith("GET "))
                    {
                        request = currentLine;
                    }
                    currentLine = "";
                }
            }
            else if (c != '\r')
            {
                currentLine += c;
            }
        }
    }

    client.stop();
    Serial.println("Client disconnected");
}

// helper function to extract a parameter value from the request
String extractCodeFromURL(String url) {
    // Handle both full URLs and just the code parameter
    String code = "";

    // First, try to find ?code= in the URL
    int codeStart = url.indexOf("?code=");
    if (codeStart == -1) {
        // Maybe it's after other parameters, try &code=
        codeStart = url.indexOf("&code=");
    }

    if (codeStart >= 0) {
        // Found the code parameter, extract it
        codeStart += (url.charAt(codeStart) == '?') ? 6 : 6; // Skip "?code=" or "&code="
        int codeEnd = url.indexOf('&', codeStart);
        if (codeEnd == -1) {
            codeEnd = url.length();
        }
        code = url.substring(codeStart, codeEnd);

        Serial.println("Extracted code from URL: " + code);
        return code;
    }

    // If no ?code= found, maybe the user just pasted the code directly
    // Check if it looks like a Spotify auth code (alphanumeric, reasonable length)
    url.trim();
    if (url.length() > 20 && url.length() < 200 && url.indexOf(' ') == -1) {
        Serial.println("Treating input as direct code: " + url);
        return url;
    }

    Serial.println("Could not extract code from: " + url);
    return "";
}

void processRequest(WiFiClient client, String request)
{
    Serial.println("DEBUG: Received request: " + request);

    // Send headers
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html; charset=UTF-8");
    client.println("Connection: close");
    client.println();

    // Route requests
    if (request.indexOf("GET / ") >= 0)
    {
        sendControlPage(client);
    }
    else if (request.indexOf("GET /color?c=") >= 0)
    {
        int colorIndex = extractParameter(request, "c=");
        setAnimationColor(colorIndex);
        client.println("Color changed");
    }
    else if (request.indexOf("GET /pattern") >= 0)
    {
        setAnimationPattern();
        client.println("Pattern activated");
    }
    else if (request.indexOf("GET /truck") >= 0)
    {
        setTruckAnimation();
        client.println("Truck animation activated");
    }
    else if (request.indexOf("GET /text?msg=") >= 0)
    {
        String message = extractString(request, "msg=");
        setAnimationText(message);
        client.println("Text set: " + message);
    }
    else if (request.indexOf("GET /clear") >= 0)
    {
        clearAnimationZone();
        client.println("Display cleared");
    }
    else if (request.indexOf("GET /widget?w=") >= 0)
    {
        int widgetType = extractParameter(request, "w=");
        setWidget((WidgetType)widgetType);
        client.println("Widget changed");
    }
        // Weather debug routes
    else if (request.indexOf("GET /weather_debug_on") >= 0) {
        setWeatherDebugMode(true);
        client.println("Weather debug mode enabled - cycling through conditions");
    }
    else if (request.indexOf("GET /weather_debug_off") >= 0) {
        setWeatherDebugMode(false);
        client.println("Weather debug mode disabled");
    }
    else if (request.indexOf("GET /weather_debug_next") >= 0) {
        advanceDebugWeather();
        client.println("Advanced to next debug weather condition");
    }
    else if (request.indexOf("GET /weather_debug_status") >= 0) {
        client.println(getDebugWeatherInfo());
    }
        // Spotify routes
    else if (request.indexOf("GET /spotify_auth") >= 0) {
        String authURL = getSpotifyAuthURL();
        client.println("Spotify Auth URL: " + authURL);
        client.println("Visit this URL to authorize the app, then use /spotify_token to set tokens");
    }
    else if (request.indexOf("GET /spotify_token?access=") >= 0) {
        String accessToken = extractString(request, "access=");
        String refreshToken = extractString(request, "refresh=");
        setSpotifyTokens(accessToken, refreshToken);
        client.println("Spotify tokens set successfully");
    }
//    else if (request.indexOf("GET /spotify_code?code=") >= 0) {
//        String authCode = extractString(request, "code=");
//        Serial.println("Received authorization code: " + authCode);
//
//        if (exchangeCodeForTokens(authCode)) {
//            client.println("Success! Spotify tokens obtained and saved.");
//            client.println("You can now select the Spotify widget.");
//        } else {
//            client.println("Failed to exchange authorization code for tokens.");
//        }
//    }
    else if (request.indexOf("GET /spotify_url?url=") >= 0) {
        String fullURL = extractString(request, "url=");

        // URL decode the input
        fullURL.replace("%3A", ":");
        fullURL.replace("%2F", "/");
        fullURL.replace("%3F", "?");
        fullURL.replace("%3D", "=");
        fullURL.replace("%26", "&");

        String authCode = extractCodeFromURL(fullURL);

        if (authCode.length() > 0) {
            Serial.println("Processing extracted authorization code: " + authCode);

            if (exchangeCodeForTokens(authCode)) {
                client.println("Success! Spotify tokens obtained from URL.");
                client.println("Code extracted: " + authCode.substring(0, 10) + "...");
                client.println("You can now select the Spotify widget.");
            } else {
                client.println("Failed to exchange authorization code for tokens.");
                client.println("Extracted code: " + authCode.substring(0, 10) + "...");
            }
        } else {
            client.println("Error: Could not extract authorization code from URL.");
            client.println("Please make sure you're pasting the full redirect URL that contains '?code='");
            client.println("Example: https://spotify.com/?code=AQC1234567890...");
        }
    }

    else {
        Serial.println("DEBUG: Unknown command: " + request);
        client.println("Unknown command");
    }
}

void sendControlPage(WiFiClient client)
{
    client.println("<!DOCTYPE html>");
    client.println("<html>");
    client.println("<head>");
    client.println("<title>LED Matrix Control</title>");
    client.println("<style>");
    client.println("body { font-family: Arial; margin: 20px; background: #222; color: #fff; }");
    client.println("h1 { color: #4CAF50; }");
    client.println("button { padding: 10px 20px; margin: 5px; font-size: 16px; border: none; border-radius: 5px; cursor: pointer; }");
    client.println(".color-btn { width: 50px; height: 50px; margin: 3px; border: 2px solid #666; }");
    client.println(".control-btn { background: #4CAF50; color: white; }");
    client.println(".control-btn:hover { background: #45a049; }");
    client.println(".truck-btn { background: #FF6B35; color: white; }");
    client.println(".truck-btn:hover { background: #E55A2B; }");
    client.println(".widget-btn { background: #2196F3; color: white; }");
    client.println(".widget-btn:hover { background: #1976D2; }");
    client.println(".debug-btn { background: #9C27B0; color: white; }");
    client.println(".debug-btn:hover { background: #7B1FA2; }");
    client.println(".spotify-btn { background: #1DB954; color: white; }");
    client.println(".spotify-btn:hover { background: #1AAE4C; }");
    client.println("input[type='text'] { padding: 8px; font-size: 16px; width: 200px; border: 1px solid #666; border-radius: 4px; background: #444; color: #fff; }");
    client.println("select { padding: 8px; font-size: 16px; border: 1px solid #666; border-radius: 4px; background: #444; color: #fff; }");
    client.println(".section { margin: 20px 0; padding: 15px; background: #333; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.3); }");
    client.println(".widget-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin: 10px 0; }");
    client.println("</style>");
    client.println("</head>");
    client.println("<body>");

    client.println("<h1>🎨 LED Matrix Control Panel</h1>");

    client.println("<div class='section'>");
    client.println("<h3>Colors:</h3>");
    client.println("<button class='color-btn' style='background: black' onclick='setColor(0)'></button>");
    client.println("<button class='color-btn' style='background: red' onclick='setColor(1)'></button>");
    client.println("<button class='color-btn' style='background: green' onclick='setColor(2)'></button>");
    client.println("<button class='color-btn' style='background: blue' onclick='setColor(3)'></button>");
    client.println("<button class='color-btn' style='background: yellow' onclick='setColor(4)'></button>");
    client.println("<button class='color-btn' style='background: magenta' onclick='setColor(5)'></button>");
    client.println("<button class='color-btn' style='background: cyan' onclick='setColor(6)'></button>");
    client.println("<button class='color-btn' style='background: white' onclick='setColor(7)'></button>");
    client.println("</div>");

    // Animation Selection Section
    client.println("<div class='section'>");
    client.println("<h3>Display Modes:</h3>");
    client.println("<button class='control-btn' onclick='setPattern()'>🌈 Rainbow Pattern</button>");
    client.println("<button class='truck-btn' onclick='setTruck()'>🚛 Truck Animation</button>");
    client.println("<button class='control-btn' onclick='clearDisplay()'>⚫ Clear Display</button>");
    client.println("</div>");

    // Widget Selector Section
    client.println("<div class='section'>");
    client.println("<h3>Smart Widgets:</h3>");
    client.println("<div class='widget-grid'>");
    client.println("<div>");
    client.println("<label>Widget:</label><br>");
    client.println("<select id='widget'>");
    client.println("<option value='0'>None</option>");
    client.println("<option value='1' selected>Clock</option>");
    client.println("<option value='2'>Weather</option>");
    client.println("<option value='3'>Teams Status</option>");
    client.println("<option value='4'>Stock Ticker</option>");
    client.println("<option value='5'>🎵 Spotify</option>");
    client.println("</select>");
    client.println("<button class='widget-btn' onclick='setWidget()'>Set</button>");
    client.println("</div>");
    client.println("</div>");
    client.println("</div>");

    // Text Section
    client.println("<div class='section'>");
    client.println("<h3>Text Display:</h3>");
    client.println("<input type='text' id='textInput' placeholder='Enter text to display' value='Hello Matrix!'>");
    client.println("<button class='control-btn' onclick='setText()'>📝 Show Text</button>");
    client.println("</div>");

    // Weather Debug Section
    client.println("<div class='section'>");
    client.println("<h3>🌤️ Weather Debug Mode:</h3>");
    client.println("<p>Test all weather conditions and animations:</p>");
    client.println("<button class='debug-btn' onclick='enableWeatherDebug()'>🔄 Enable Auto-Cycle</button>");
    client.println("<button class='debug-btn' onclick='nextWeatherDebug()'>⏭️ Next Condition</button>");
    client.println("<button class='control-btn' onclick='disableWeatherDebug()'>⏹️ Disable Debug</button>");
    client.println("<button class='widget-btn' onclick='checkDebugStatus()'>📊 Status</button>");
    client.println("<div id='debugStatus' style='margin-top: 10px; padding: 10px; background: #444; border-radius: 4px;'>");
    client.println("Debug status will appear here");
    client.println("</div>");
    client.println("</div>");

    // Spotify Setup Section - UPDATED
    client.println("<div class='section'>");
    client.println("<h3>🎵 Spotify Setup:</h3>");
    client.println("<p><strong>Step 1:</strong> Get authorization URL</p>");
    client.println("<button class='spotify-btn' onclick='spotifyAuth()'>🔑 Get Auth URL</button>");
    client.println("<div id='authUrlDisplay' style='margin: 10px 0; padding: 10px; background: #444; border-radius: 4px; word-break: break-all;'></div>");

    client.println("<p><strong>Step 2:</strong> Visit the URL above, authorize, then paste the FULL redirect URL below</p>");
    client.println("<label>Redirect URL (paste the complete URL you get redirected to):</label><br>");
    client.println("<input type='text' id='redirectUrl' placeholder='https://spotify.com/?code=AQC1234567890...' style='width: 400px;'><br>");
    client.println("<button class='control-btn' onclick='submitRedirectURL()'>🔄 Extract Code & Get Tokens</button>");

    client.println("<p style='font-size: 12px; color: #888;'>");
    client.println("💡 Tip: After clicking authorize on Spotify, you'll be redirected to a page that might show an error. ");
    client.println("That's normal! Just copy the ENTIRE URL from your browser's address bar and paste it above.");
    client.println("</p>");

    client.println("<div id='spotifyStatus' style='margin-top: 10px; padding: 10px; background: #444; border-radius: 4px;'>");
    client.println("Click 'Get Auth URL' to start setup");
    client.println("</div>");
    client.println("</div>");


    client.println("<script>");
    client.println("function setColor(colorIndex) { fetch('/color?c=' + colorIndex); }");
    client.println("function setPattern() { fetch('/pattern'); }");
    client.println("function setTruck() { fetch('/truck'); }");
    client.println("function clearDisplay() { fetch('/clear'); }");
    client.println("function setText() { const text = document.getElementById('textInput').value; fetch('/text?msg=' + encodeURIComponent(text)); }");
    client.println("function setWidget() { const w = document.getElementById('widget').value; fetch('/widget?w=' + w); }");

    // Weather debug functions
    client.println("function enableWeatherDebug() { ");
    client.println("  fetch('/weather_debug_on').then(r => r.text()).then(data => {");
    client.println("    document.getElementById('debugStatus').innerHTML = data;");
    client.println("    setTimeout(checkDebugStatus, 1000);");
    client.println("  });");
    client.println("}");

    client.println("function disableWeatherDebug() { ");
    client.println("  fetch('/weather_debug_off').then(r => r.text()).then(data => {");
    client.println("    document.getElementById('debugStatus').innerHTML = data;");
    client.println("  });");
    client.println("}");

    client.println("function nextWeatherDebug() { ");
    client.println("  fetch('/weather_debug_next').then(r => r.text()).then(data => {");
    client.println("    document.getElementById('debugStatus').innerHTML = data;");
    client.println("    setTimeout(checkDebugStatus, 500);");
    client.println("  });");
    client.println("}");

    client.println("function checkDebugStatus() { ");
    client.println("  fetch('/weather_debug_status').then(r => r.text()).then(data => {");
    client.println("    document.getElementById('debugStatus').innerHTML = data;");
    client.println("  });");
    client.println("}");

    // Spotify functions

    client.println("function spotifyAuth() {");
    client.println("  fetch('/spotify_auth').then(r => r.text()).then(data => {");
    client.println("    const url = data.split('Spotify Auth URL: ')[1].split('\\n')[0];");
    client.println("    document.getElementById('authUrlDisplay').innerHTML = ");
    client.println("      '<a href=\"' + url + '\" target=\"_blank\" style=\"color: #1DB954; text-decoration: none;\">🔗 Click here to authorize Spotify</a><br><small style=\"color: #888;\">' + url + '</small>';");
    client.println("    document.getElementById('spotifyStatus').innerHTML = ");
    client.println("      '1. Click the link above to authorize<br>2. Copy the complete redirect URL<br>3. Paste it in the text box below';");
    client.println("  });");
    client.println("}");

    client.println("function submitRedirectURL() {");
    client.println("  const url = document.getElementById('redirectUrl').value.trim();");
    client.println("  if (url) {");
    client.println("    if (url.includes('code=')) {");
    client.println("      document.getElementById('spotifyStatus').innerHTML = 'Extracting code and exchanging for tokens...';");
    client.println("      fetch('/spotify_url?url=' + encodeURIComponent(url))");
    client.println("        .then(r => r.text()).then(data => {");
    client.println("          document.getElementById('spotifyStatus').innerHTML = data.replace(/\\n/g, '<br>');");
    client.println("          if (data.includes('Success!')) {");
    client.println("            document.getElementById('redirectUrl').value = '';");
    client.println("            document.getElementById('spotifyStatus').innerHTML += '<br><br>✅ <strong>Setup complete! You can now use the Spotify widget.</strong>';");
    client.println("          }");
    client.println("        });");
    client.println("    } else {");
    client.println("      document.getElementById('spotifyStatus').innerHTML = ");
    client.println("        '❌ Error: The URL should contain \"code=\". Please make sure you\\'re pasting the full redirect URL from Spotify.';");
    client.println("    }");
    client.println("  } else {");
    client.println("    document.getElementById('spotifyStatus').innerHTML = 'Please paste the redirect URL from Spotify';");
    client.println("  }");
    client.println("}");


    client.println("// Auto-refresh debug status every 3 seconds when in debug mode");
    client.println("setInterval(() => {");
    client.println("  if (document.getElementById('debugStatus').innerHTML.includes('Debug:')) {");
    client.println("    checkDebugStatus();");
    client.println("  }");
    client.println("}, 3000);");

    client.println("</script>");

    client.println("</body>");
    client.println("</html>");
}

int extractParameter(String request, String param)
{
    int start = request.indexOf(param);
    if (start >= 0)
    {
        start += param.length();
        int end = request.indexOf(' ', start);
        if (end < 0) end = request.indexOf('&', start);
        if (end < 0) end = request.length();
        return request.substring(start, end).toInt();
    }
    return 0;
}

String extractString(String request, String param)
{
    int start = request.indexOf(param);
    if (start >= 0)
    {
        start += param.length();
        int end = request.indexOf(' ', start);
        if (end < 0) end = request.indexOf('&', start);
        if (end < 0) end = request.length();
        String value = request.substring(start, end);
        value.replace("%20", " ");
        value.replace("+", " ");
        return value;
    }
    return "";
}
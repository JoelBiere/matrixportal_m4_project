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

void processRequest(WiFiClient client, String request)
{
  Serial.println("DEBUG: Received request: " + request);

  // Send headers
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html; charset=UTF-8");
  client.println("Connection: close");
  client.println();

  // Route requests - UPDATED to use new function names
  if (request.indexOf("GET / ") >= 0)
  {
    sendControlPage(client);
  }
  else if (request.indexOf("GET /color?c=") >= 0)
  {
    int colorIndex = extractParameter(request, "c=");
    setAnimationColor(colorIndex); // UPDATED
    client.println("Color changed");
  }
  else if (request.indexOf("GET /pattern") >= 0)
  {
    setAnimationPattern(); // UPDATED
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
    setAnimationText(message); // UPDATED
    client.println("Text set: " + message);
  }
  else if (request.indexOf("GET /clear") >= 0)
  {
    clearAnimationZone(); // UPDATED
    client.println("Display cleared");
  }
  else if (request.indexOf("GET /widget?w=") >= 0)
  {
    int widgetType = extractParameter(request, "w=");
    setWidget((WidgetType)widgetType);
    client.println("Widget changed");
  }
  // NEW: Weather debug routes
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
  else {
    Serial.println("DEBUG: Unknown command: " + request);
    client.println("Unknown command");
  }
}

// Updated sendControlPage() function with widget controls
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
  // client.println("<button class='widget-btn' onclick='setWidgets()'>📊 Smart Widgets</button>");
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

  // NEW: Weather Debug Section
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

  client.println("<script>");
  client.println("function setColor(colorIndex) { fetch('/color?c=' + colorIndex); }");
  client.println("function setPattern() { fetch('/pattern'); }");
  client.println("function setTruck() { fetch('/truck'); }");
  client.println("function clearDisplay() { fetch('/clear'); }");
  client.println(
    "function setText() { const text = document.getElementById('textInput').value; fetch('/text?msg=' + encodeURIComponent(text)); }");
  client.println(
    "function setWidget() { const w = document.getElementById('widget').value; fetch('/widget?w=' + w); }");

  // NEW: Weather debug functions
  client.println("function enableWeatherDebug() { ");
  client.println("  fetch('/weather_debug_on').then(r => r.text()).then(data => {");
  client.println("    document.getElementById('debugStatus').innerHTML = data;");
  client.println("    setTimeout(checkDebugStatus, 1000);"); // Auto-refresh status
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

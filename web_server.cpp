#include "web_server.h"
#include "matrix_display.h"

void initializeWebServer() {
  // Server initialization happens in wifi_manager
  server.begin();
  Serial.println("Web server ready");
}

void handleWebClients() {
  if (wifiStatus == WL_CONNECTED) {
    WiFiClient client = server.available();
    if (client) {
      handleWebClient(client);
    }
  }
}

void handleWebClient(WiFiClient client) {
  Serial.println("Client connected");
  String request = "";
  String currentLine = "";

  while (client.connected()) {
    if (client.available()) {
      char c = client.read();

      if (c == '\n') {
        if (currentLine.length() == 0) {
          processRequest(client, request);
          break;
        } else {
          if (currentLine.startsWith("GET ")) {
            request = currentLine;
          }
          currentLine = "";
        }
      } else if (c != '\r') {
        currentLine += c;
      }
    }
  }

  client.stop();
  Serial.println("Client disconnected");
}

void processRequest(WiFiClient client, String request) {
  // Send headers
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html; charset=UTF-8");
  client.println("Connection: close");
  client.println();

  // Route requests
  if (request.indexOf("GET / ") >= 0) {
    sendControlPage(client);
  } else if (request.indexOf("GET /color?c=") >= 0) {
    int colorIndex = extractParameter(request, "c=");
    setMatrixColor(colorIndex);
    client.println("Color changed");
  } else if (request.indexOf("GET /pattern") >= 0) {
    setMatrixPattern();
    client.println("Pattern activated");
  } else if (request.indexOf("GET /truck") >= 0) {  // Added truck animation route
    setTruckAnimation();
    client.println("Truck animation activated");
  } else if (request.indexOf("GET /text?msg=") >= 0) {
    String message = extractString(request, "msg=");
    setMatrixText(message);
    client.println("Text set: " + message);
  } else if (request.indexOf("GET /clear") >= 0) {
    clearMatrixDisplay();
    client.println("Display cleared");
  } else if (request.indexOf("GET /widgets") >= 0) {
    setSmartWidgetsMode();
    client.println("Smart widgets activated");
  } else if (request.indexOf("GET /widget-left?w=") >= 0) {
    int widgetType = extractParameter(request, "w=");
    setTopLeftWidget((WidgetType)widgetType);
    client.println("Left widget changed");
  } else if (request.indexOf("GET /widget-right?w=") >= 0) {
    int widgetType = extractParameter(request, "w=");
    setTopRightWidget((WidgetType)widgetType);
    client.println("Right widget changed");
  } else if (request.indexOf("GET /toggle-widgets") >= 0) {
    toggleWidgets();
    client.println("Widgets toggled");
  }
  else {
    client.println("Unknown command");
  }
}

// Updated sendControlPage() function with widget controls
void sendControlPage(WiFiClient client) {
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
  
  client.println("<div class='section'>");
  client.println("<h3>Display Modes:</h3>");
  client.println("<button class='control-btn' onclick='setPattern()'>🌈 Rainbow Pattern</button>");
  client.println("<button class='truck-btn' onclick='setTruck()'>🚛 Truck Animation</button>");
  client.println("<button class='widget-btn' onclick='setWidgets()'>📊 Smart Widgets</button>");
  client.println("<button class='control-btn' onclick='clearDisplay()'>⚫ Clear Display</button>");
  client.println("</div>");
  
  client.println("<div class='section'>");
  client.println("<h3>Smart Widgets:</h3>");
  client.println("<div class='widget-grid'>");
  client.println("<div>");
  client.println("<label>Top Left Widget:</label><br>");
  client.println("<select id='leftWidget'>");
  client.println("<option value='0'>None</option>");
  client.println("<option value='1' selected>Clock</option>");
  client.println("<option value='2'>Weather</option>");
  client.println("<option value='3'>Teams Status</option>");
  client.println("<option value='4'>Stock Ticker</option>");
  client.println("</select>");
  client.println("<button class='widget-btn' onclick='setLeftWidget()'>Set</button>");
  client.println("</div>");
  client.println("<div>");
  client.println("<label>Top Right Widget:</label><br>");
  client.println("<select id='rightWidget'>");
  client.println("<option value='0'>None</option>");
  client.println("<option value='1'>Clock</option>");
  client.println("<option value='2' selected>Weather</option>");
  client.println("<option value='3'>Teams Status</option>");
  client.println("<option value='4'>Stock Ticker</option>");
  client.println("</select>");
  client.println("<button class='widget-btn' onclick='setRightWidget()'>Set</button>");
  client.println("</div>");
  client.println("</div>");
  client.println("<button class='widget-btn' onclick='toggleWidgets()'>🔄 Toggle Widgets</button>");
  client.println("</div>");
  
  client.println("<div class='section'>");
  client.println("<h3>Text Display:</h3>");
  client.println("<input type='text' id='textInput' placeholder='Enter text to display' value='Hello Matrix!'>");
  client.println("<button class='control-btn' onclick='setText()'>📝 Show Text</button>");
  client.println("</div>");
  
  client.println("<script>");
  client.println("function setColor(colorIndex) { fetch('/color?c=' + colorIndex); }");
  client.println("function setPattern() { fetch('/pattern'); }");
  client.println("function setTruck() { fetch('/truck'); }");
  client.println("function setWidgets() { fetch('/widgets'); }");
  client.println("function clearDisplay() { fetch('/clear'); }");
  client.println("function setText() { const text = document.getElementById('textInput').value; fetch('/text?msg=' + encodeURIComponent(text)); }");
  client.println("function setLeftWidget() { const w = document.getElementById('leftWidget').value; fetch('/widget-left?w=' + w); }");
  client.println("function setRightWidget() { const w = document.getElementById('rightWidget').value; fetch('/widget-right?w=' + w); }");
  client.println("function toggleWidgets() { fetch('/toggle-widgets'); }");
  client.println("</script>");
  
  client.println("</body>");
  client.println("</html>");
}

int extractParameter(String request, String param) {
  int start = request.indexOf(param);
  if (start >= 0) {
    start += param.length();
    int end = request.indexOf(' ', start);
    if (end < 0) end = request.indexOf('&', start);
    if (end < 0) end = request.length();
    return request.substring(start, end).toInt();
  }
  return 0;
}

String extractString(String request, String param) {
  int start = request.indexOf(param);
  if (start >= 0) {
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
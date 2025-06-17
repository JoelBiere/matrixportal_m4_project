#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WiFiNINA.h>
#include "display_modes.h"
#include "widgets.h"

// Web server object
extern WiFiServer server;

// Web server functions
void initializeWebServer();
void handleWebClients();
void handleWebClient(WiFiClient client);
void processRequest(WiFiClient client, String request);
void sendControlPage(WiFiClient client);
int extractParameter(String request, String param);
String extractString(String request, String param);

// Helper function for URL extraction (used for auth codes)
String extractCodeFromURL(String url);

// IP address to string converter (replacement for toString)
String ipAddressToString(IPAddress ip);

#endif
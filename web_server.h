#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "config.h"
#include "widgets.h"

// Function declarations
void initializeWebServer();
void handleWebClients();
void handleWebClient(WiFiClient client);
void processRequest(WiFiClient client, String request);
void sendControlPage(WiFiClient client);
int extractParameter(String request, String param);
String extractString(String request, String param);

#endif
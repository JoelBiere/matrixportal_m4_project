#ifndef CREDENTIALS_H
#define CREDENTIALS_H

// WiFi credentials - keep this separate for security
extern char ssid[];
extern char wifiPass[];

extern char ssid2[]; // Optional second SSID

// API keys for widgets (if needed)
extern char weatherApiKey[];
extern char timeZone[];

// Spotify API credentials
extern char spotifyClientId[];
extern char spotifyClientSecret[];

// Microsoft Graph API credentials
extern char msGraphClientId[];
extern char msGraphClientSecret[];

#endif
// api_server.h
#ifndef API_SERVER_H
#define API_SERVER_H

#include <microhttpd.h>
#include <json-c/json.h>
#include "safer.h"

// API server configuration
typedef struct {
    int port;
    SafetyManagementSystem *sms;
    Database *db;
} APIServer;

// Function declarations
int start_api_server(APIServer *server);
void stop_api_server(APIServer *server);

#endif // API_SERVER_H

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

// api_server.c
#include "api_server.h"

static struct MHD_Daemon *daemon;

// API endpoint handlers
static int handle_mission_request(struct MHD_Connection *connection, 
                                const char *method,
                                json_object *request_json,
                                SafetyManagementSystem *sms);

static int handle_radio_analysis_request(struct MHD_Connection *connection,
                                       const char *method,
                                       json_object *request_json);

static int send_json_response(struct MHD_Connection *connection,
                            json_object *response,
                            int status_code) {
    const char *json_str = json_object_to_json_string(response);
    struct MHD_Response *response_obj = MHD_create_response_from_buffer(
        strlen(json_str),
        (void*)json_str,
        MHD_RESPMEM_MUST_COPY
    );
    
    MHD_add_response_header(response_obj, "Content-Type", "application/json");
    int ret = MHD_queue_response(connection, status_code, response_obj);
    MHD_destroy_response(response_obj);
    
    return ret;
}

static int request_handler(void *cls,
                         struct MHD_Connection *connection,
                         const char *url,
                         const char *method,
                         const char *version,
                         const char *upload_data,
                         size_t *upload_data_size,
                         void **con_cls) {
    APIServer *server = (APIServer*)cls;
    
    // Handle only POST and GET methods
    if (strcmp(method, "POST") != 0 && strcmp(method, "GET") != 0) {
        return MHD_NO;
    }
    
    // Parse JSON request body for POST requests
    json_object *request_json = NULL;
    if (strcmp(method, "POST") == 0 && upload_data != NULL) {
        request_json = json_tokener_parse(upload_data);
        if (request_json == NULL) {
            json_object *error = json_object_new_object();
            json_object_object_add(error, "error", json_object_new_string("Invalid JSON"));
            return send_json_response(connection, error, MHD_HTTP_BAD_REQUEST);
        }
    }
    
    // Route requests to appropriate handlers
    if (strcmp(url, "/api/mission") == 0) {
        return handle_mission_request(connection, method, request_json, server->sms);
    } else if (strcmp(url, "/api/radio-analysis") == 0) {
        return handle_radio_analysis_request(connection, method, request_json);
    }
    
    // Handle unknown endpoints
    json_object *error = json_object_new_object();
    json_object_object_add(error, "error", json_object_new_string("Endpoint not found"));
    return send_json_response(connection, error, MHD_HTTP_NOT_FOUND);
}

int start_api_server(APIServer *server) {
    daemon = MHD_start_daemon(
        MHD_USE_SELECT_INTERNALLY,
        server->port,
        NULL,
        NULL,
        &request_handler,
        server,
        MHD_OPTION_END
    );
    
    return daemon != NULL ? 0 : -1;
}

void stop_api_server(APIServer *server) {
    if (daemon != NULL) {
        MHD_stop_daemon(daemon);
    }
}

// Example API endpoint implementation
static int handle_mission_request(struct MHD_Connection *connection,
                                const char *method,
                                json_object *request_json,
                                SafetyManagementSystem *sms) {
    if (strcmp(method, "POST") == 0) {
        // Create new mission from JSON
        Mission mission = {0};
        json_object *mission_id_obj;
        if (json_object_object_get_ex(request_json, "id", &mission_id_obj)) {
            strncpy(mission.id, json_object_get_string(mission_id_obj), sizeof(mission.id) - 1);
        }
        
        // Perform risk assessment including radio interference
        RadioEnvironment radio_env = {0};
        // Populate radio environment from JSON...
        RadioInterferenceAnalysis radio_analysis = analyze_radio_interference(&radio_env);
        
        // Combine all risk assessments
        perform_risk_assessment(&mission);
        if (radio_analysis.risk_level > mission.risk_level) {
            mission.risk_level = radio_analysis.risk_level;
        }
        
        // Create response
        json_object *response = json_object_new_object();
        json_object_object_add(response, "mission_id", json_object_new_string(mission.id));
        json_object_object_add(response, "risk_level", json_object_new_int(mission.risk_level));
        json_object_object_add(response, "radio_interference", json_object_new_double(radio_analysis.interference_level));
        
        return send_json_response(connection, response, MHD_HTTP_OK);
    } else if (strcmp(method, "GET") == 0) {
        // Retrieve mission details
        const char *mission_id = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "id");
        if (mission_id == NULL) {
            json_object *error = json_object_new_object();
            json_object_object_add(error, "error", json_object_new_string("Mission ID required"));
            return send_json_response(connection, error, MHD_HTTP_BAD_REQUEST);
        }
        
        // Create response with mission details
        json_object *response = json_object_new_object();
        // Populate response with mission data...
        
        return send_json_response(connection, response, MHD_HTTP_OK);
    }
    
    return MHD_NO;
}

static int handle_radio_analysis_request(struct MHD_Connection *connection,
                                       const char *method,
                                       json_object *request_json) {
    if (strcmp(method, "POST") != 0) {
        json_object *error = json_object_new_object();
        json_object_object_add(error, "error", json_object_new_string("Method not allowed"));
        return send_json_response(connection, error, MHD_HTTP_METHOD_NOT_ALLOWED);
    }
    
    // Parse radio environment from JSON
    RadioEnvironment env = {0};
    json_object *sources_array;
    if (json_object_object_get_ex(request_json, "sources", &sources_array)) {
        env.num_sources = json_object_array_length(sources_array);
        env.sources = malloc(sizeof(RadioSource) * env.num_sources);
        
        for (int i = 0; i < env.num_sources; i++) {
            json_object *source_obj = json_object_array_get_idx(sources_array, i);
            json_object *freq_obj, *power_obj, *distance_obj, *terrain_obj;
            
            if (json_object_object_get_ex(source_obj, "frequency", &freq_obj)) {
                env.sources[i].frequency = json_object_get_double(freq_obj);
            }
            if (json_object_object_get_ex(source_obj, "power", &power_obj)) {
                env.sources[i].power = json_object_get_double(power_obj);
            }
            if (json_object_object_get_ex(source_obj, "distance", &distance_obj)) {
                env.sources[i].distance = json_object_get_double(distance_obj);
            }
            if (json_object_object_get_ex(source_obj, "terrain_factor", &terrain_obj)) {
                env.sources[i].terrain_factor = json_object_get_double(terrain_obj);
            }
        }
    }
    
    // Perform analysis
    RadioInterferenceAnalysis analysis = analyze_radio_interference(&env);
    
    // Create response
    json_object *response = json_object_new_object();
    json_object_object_add(response, "interference_level", json_object_new_double(analysis.interference_level));
    json_object_object_add(response, "signal_to_noise", json_object_new_double(analysis.signal_to_noise));
    json_object_object_add(response, "risk_level", json_object_new_int(analysis.risk_level));
    json_object_object_add(response, "recommendations", json_object_new_string(analysis.recommendations));
    
    // Cleanup
    free(env.sources);
    
    return send_json_response(connection, response, MHD_HTTP_OK);
}

// main.c (updated with API and radio interference)
int main() {
    // Initialize safety management system and database
    SafetyManagementSystem sms = {0};
    Database db = {0};
    
    if (init_database(&db) != SQLITE_OK) {
        fprintf(stderr, "Failed to initialize database\n");
        return 1;
    }
    
    // Initialize and start API server
    APIServer api_server = {
        .port = 8080,
        .sms = &sms,
        .db = &db
    };
    
    if (start_api_server(&api_server) != 0) {
        fprintf(stderr, "Failed to start API server\n");
        return 1;
    }
    
    printf("SAFER API server running on port %d\n", api_server.port);
    
    // Example radio interference analysis
    RadioSource sources[] = {
        {
            .frequency = 430.0,  // 430 MHz
            .power = 30.0,      // 30 dBm
            .distance = 5.0,    // 5 km
            .terrain_factor = 2.5
        },
        {
            .frequency = 435.0,
            .power = 25.0,
            .distance = 3.0,
            .terrain_factor = 2.0
        }
    };
    
    RadioEnvironment env = {
        .sources = sources,
        .num_sources = 2,
        .background_noise = -90.0,  // -90 dBm
        .weather_factor = 1.2
    };
    
    RadioInterferenceAnalysis analysis = analyze_radio_interference(&env);
    
    printf("\nRadio Interference Analysis:\n");
    printf("Interference Level: %.2f dBm\n", analysis.interference_level);
    printf("Signal to Noise Ratio: %.2f dB\n", analysis.signal_to_noise);
    printf("Risk Level: %d\n", analysis.risk_level);
    printf("Recommendations: %s\n", analysis.recommendations);
    
    // Main event loop
    printf("\nPress Enter to exit...\n");
    getchar();
    
    // Cleanup
    stop_api_server(&api_server);
    sqlite3_close(db.db);
    
    return 0;
}

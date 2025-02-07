// database.c - Database implementation
#include "database.h"

int init_database(Database *db) {
    int rc = sqlite3_open("safer.db", &db->db);
    if (rc) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db->db));
        return rc;
    }
    
    // Create tables
    const char *sql = 
        "CREATE TABLE IF NOT EXISTS aircraft ("
        "id TEXT PRIMARY KEY,"
        "model TEXT,"
        "manufacture_date INTEGER,"
        "total_flight_hours INTEGER"
        ");"
        
        "CREATE TABLE IF NOT EXISTS crew_members ("
        "id TEXT PRIMARY KEY,"
        "name TEXT,"
        "role TEXT,"
        "certification TEXT,"
        "flight_hours INTEGER,"
        "last_training INTEGER"
        ");"
        
        "CREATE TABLE IF NOT EXISTS missions ("
        "id TEXT PRIMARY KEY,"
        "aircraft_id TEXT,"
        "departure_time INTEGER,"
        "estimated_duration REAL,"
        "mission_type TEXT,"
        "risk_level INTEGER,"
        "FOREIGN KEY(aircraft_id) REFERENCES aircraft(id)"
        ");";
    
    char *err_msg = 0;
    rc = sqlite3_exec(db->db, sql, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return rc;
    }
    
    return SQLITE_OK;
}

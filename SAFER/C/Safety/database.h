// database.h - Database management header
#ifndef DATABASE_H
#define DATABASE_H

#include "safer.h"
#include <sqlite3.h>

typedef struct {
    sqlite3 *db;
    char *err_msg;
} Database;

// Database function declarations
int init_database(Database *db);
int save_mission(Database *db, Mission *mission);
int save_aircraft(Database *db, Aircraft *aircraft);
int save_crew_member(Database *db, CrewMember *crew);
Mission* load_mission(Database *db, const char *mission_id);
Aircraft* load_aircraft(Database *db, const char *aircraft_id);
CrewMember* load_crew_member(Database *db, const char *crew_id);

#endif // DATABASE_H
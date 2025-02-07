CREATE TYPE risk_level AS ENUM ('Low', 'Medium', 'High', 'Critical');

CREATE TABLE aircraft (
    id TEXT PRIMARY KEY,
    model TEXT NOT NULL,
    manufacture_date TIMESTAMP WITH TIME ZONE NOT NULL,
    total_flight_hours INTEGER NOT NULL
);

CREATE TABLE crew_members (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    role TEXT NOT NULL,
    certification_level TEXT NOT NULL,
    flight_hours INTEGER NOT NULL,
    last_training TIMESTAMP WITH TIME ZONE NOT NULL
);

CREATE TABLE maintenance_records (
    id SERIAL PRIMARY KEY,
    aircraft_id TEXT REFERENCES aircraft(id),
    last_inspection TIMESTAMP WITH TIME ZONE NOT NULL,
    maintenance_due TIMESTAMP WITH TIME ZONE NOT NULL,
    reported_issues TEXT[] NOT NULL DEFAULT '{}'
);

CREATE TABLE missions (
    id TEXT PRIMARY KEY,
    aircraft_id TEXT REFERENCES aircraft(id),
    departure_time TIMESTAMP WITH TIME ZONE NOT NULL,
    estimated_duration DOUBLE PRECISION NOT NULL,
    mission_type TEXT NOT NULL,
    risk_level risk_level NOT NULL,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE mission_crew (
    mission_id TEXT REFERENCES missions(id),
    crew_id TEXT REFERENCES crew_members(id),
    PRIMARY KEY (mission_id, crew_id)
);

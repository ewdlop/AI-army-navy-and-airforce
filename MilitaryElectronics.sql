-- Create database for military electronics performance tracking
CREATE DATABASE MilitaryElectronics;
GO

USE MilitaryElectronics;
GO

-- Base components table
CREATE TABLE components (
    id INT IDENTITY(1,1) PRIMARY KEY,
    component_name NVARCHAR(100) NOT NULL,
    category NVARCHAR(50), -- processor, memory, sensor, etc.
    manufacturer NVARCHAR(100),
    mil_spec_level NVARCHAR(50), -- MIL-STD-883, MIL-PRF-38534, etc.
    radiation_hardening_level DECIMAL(10,2), -- krad
    operating_temp_min INT, -- Celsius
    operating_temp_max INT, -- Celsius
    power_consumption_watts DECIMAL(10,2),
    mtbf_hours INT, -- Mean Time Between Failures
    unit_cost DECIMAL(10,2),
    weight_grams DECIMAL(10,2),
    last_updated DATETIME2 DEFAULT GETDATE()
);

-- Performance characteristics
CREATE TABLE performance_metrics (
    id INT IDENTITY(1,1) PRIMARY KEY,
    component_id INT,
    metric_name NVARCHAR(100),
    metric_value DECIMAL(18,4),
    metric_unit NVARCHAR(50),
    test_condition NVARCHAR(MAX),
    FOREIGN KEY (component_id) REFERENCES components(id)
);

-- Environmental resistance ratings
CREATE TABLE environmental_ratings (
    id INT IDENTITY(1,1) PRIMARY KEY,
    component_id INT,
    vibration_resistance_g DECIMAL(10,2),
    shock_resistance_g DECIMAL(10,2),
    humidity_resistance_percent DECIMAL(5,2),
    salt_spray_resistance_hours INT,
    emp_resistance_vpm DECIMAL(10,2), -- Volts per meter
    FOREIGN KEY (component_id) REFERENCES components(id)
);

-- Cost breakdown
CREATE TABLE cost_factors (
    id INT IDENTITY(1,1) PRIMARY KEY,
    component_id INT,
    raw_material_cost DECIMAL(10,2),
    manufacturing_cost DECIMAL(10,2),
    testing_cost DECIMAL(10,2),
    certification_cost DECIMAL(10,2),
    radiation_hardening_cost DECIMAL(10,2),
    maintenance_cost_yearly DECIMAL(10,2),
    FOREIGN KEY (component_id) REFERENCES components(id)
);

-- Systems integration
CREATE TABLE systems (
    id INT IDENTITY(1,1) PRIMARY KEY,
    system_name NVARCHAR(100),
    platform_type NVARCHAR(50), -- aircraft, vehicle, vessel, etc.
    total_power_budget_watts DECIMAL(10,2),
    total_weight_budget_kg DECIMAL(10,2),
    operational_altitude_max_meters INT,
    operational_speed_max_kmh INT,
    target_mtbf_hours INT
);

-- Component-System relationships
CREATE TABLE system_components (
    id INT IDENTITY(1,1) PRIMARY KEY,
    system_id INT,
    component_id INT,
    quantity INT,
    redundancy_level INT,
    criticality_level INT, -- 1-5 scale
    FOREIGN KEY (system_id) REFERENCES systems(id),
    FOREIGN KEY (component_id) REFERENCES components(id)
);

-- Performance trade-offs
CREATE TABLE performance_tradeoffs (
    id INT IDENTITY(1,1) PRIMARY KEY,
    component_id INT,
    performance_aspect NVARCHAR(100),
    cost_impact DECIMAL(10,2),
    weight_impact_grams DECIMAL(10,2),
    power_impact_watts DECIMAL(10,2),
    reliability_impact_percent DECIMAL(5,2),
    FOREIGN KEY (component_id) REFERENCES components(id)
);

-- Reliability testing
CREATE TABLE reliability_tests (
    id INT IDENTITY(1,1) PRIMARY KEY,
    component_id INT,
    test_type NVARCHAR(100),
    test_duration_hours INT,
    temperature_cycles INT,
    vibration_cycles INT,
    failure_count INT,
    test_cost DECIMAL(10,2),
    FOREIGN KEY (component_id) REFERENCES components(id)
);

-- Create analysis views
CREATE VIEW vw_component_cost_performance AS
SELECT 
    c.component_name,
    c.category,
    c.mil_spec_level,
    c.mtbf_hours,
    c.unit_cost,
    cf.raw_material_cost + cf.manufacturing_cost + cf.testing_cost + 
    cf.certification_cost + cf.radiation_hardening_cost as total_production_cost,
    c.power_consumption_watts,
    er.vibration_resistance_g,
    er.emp_resistance_vpm,
    (c.mtbf_hours * 1.0) / (c.unit_cost * c.power_consumption_watts) as efficiency_score
FROM components c
JOIN cost_factors cf ON c.id = cf.component_id
JOIN environmental_ratings er ON c.id = er.component_id;

-- Create optimization procedures
CREATE PROCEDURE sp_OptimizeSystemConfiguration
    @SystemId INT,
    @MaxBudget DECIMAL(10,2),
    @MaxPowerConsumption DECIMAL(10,2)
AS
BEGIN
    WITH component_metrics AS (
        SELECT 
            sc.system_id,
            c.component_name,
            c.unit_cost * sc.quantity as total_cost,
            c.power_consumption_watts * sc.quantity as total_power,
            c.mtbf_hours / sc.quantity as effective_mtbf,
            sc.criticality_level
        FROM system_components sc
        JOIN components c ON sc.component_id = c.id
        WHERE sc.system_id = @SystemId
    )
    SELECT 
        component_name,
        total_cost,
        total_power,
        effective_mtbf,
        criticality_level,
        CASE 
            WHEN total_cost > @MaxBudget * 0.2 THEN 'Over Budget'
            WHEN total_power > @MaxPowerConsumption * 0.2 THEN 'Power Critical'
            ELSE 'Acceptable'
        END as status
    FROM component_metrics
    ORDER BY criticality_level DESC;
END;
GO

-- Sample data insertion
INSERT INTO components (
    component_name, 
    category, 
    mil_spec_level, 
    radiation_hardening_level,
    operating_temp_min,
    operating_temp_max,
    power_consumption_watts,
    mtbf_hours,
    unit_cost
) VALUES (
    'RAD-HARD Processor XM-5',
    'processor',
    'MIL-STD-883',
    100.00, -- 100 krad
    -55,
    125,
    15.5,
    50000,
    12500.00
);

-- Create performance analysis procedure
CREATE PROCEDURE sp_AnalyzePerformanceImpact
    @ComponentId INT
AS
BEGIN
    SELECT 
        c.component_name,
        pt.performance_aspect,
        pt.cost_impact,
        pt.weight_impact_grams,
        pt.power_impact_watts,
        pt.reliability_impact_percent,
        (pt.cost_impact * 0.3 + 
         pt.power_impact_watts * 0.3 + 
         pt.reliability_impact_percent * 0.4) as total_impact_score
    FROM components c
    JOIN performance_tradeoffs pt ON c.id = pt.component_id
    WHERE c.id = @ComponentId
    ORDER BY total_impact_score DESC;
END;

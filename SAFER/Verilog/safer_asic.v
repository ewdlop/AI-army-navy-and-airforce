// SAFER ASIC Top Module
module safer_asic (
    input wire clk,
    input wire rst_n,
    // Flight Data Interface
    input wire [15:0] altitude_data,
    input wire [15:0] speed_data,
    input wire [15:0] heading_data,
    // Environmental Sensors
    input wire [11:0] temperature,
    input wire [11:0] pressure,
    input wire [7:0] visibility,
    // Maintenance Data
    input wire [31:0] engine_hours,
    input wire [7:0] maintenance_flags,
    // Personnel Status
    input wire [7:0] crew_status,
    // Output Interface
    output reg [7:0] risk_level,
    output reg [15:0] alert_flags,
    output reg system_ok
);

    // Internal registers for processing
    reg [15:0] flight_risk;
    reg [15:0] env_risk;
    reg [15:0] maint_risk;
    reg [15:0] personnel_risk;

    // Flight Data Processing Module
    flight_data_processor flight_proc (
        .clk(clk),
        .rst_n(rst_n),
        .altitude(altitude_data),
        .speed(speed_data),
        .heading(heading_data),
        .risk_score(flight_risk)
    );

    // Environmental Risk Calculator
    environmental_processor env_proc (
        .clk(clk),
        .rst_n(rst_n),
        .temperature(temperature),
        .pressure(pressure),
        .visibility(visibility),
        .risk_score(env_risk)
    );

    // Maintenance Risk Analyzer
    maintenance_processor maint_proc (
        .clk(clk),
        .rst_n(rst_n),
        .engine_hours(engine_hours),
        .maint_flags(maintenance_flags),
        .risk_score(maint_risk)
    );

    // Personnel Status Processor
    personnel_processor pers_proc (
        .clk(clk),
        .rst_n(rst_n),
        .crew_status(crew_status),
        .risk_score(personnel_risk)
    );

    // Risk Integration Logic
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            risk_level <= 8'h00;
            alert_flags <= 16'h0000;
            system_ok <= 1'b0;
        end else begin
            // Combine risk scores
            risk_level <= (flight_risk[15:8] + env_risk[15:8] + 
                          maint_risk[15:8] + personnel_risk[15:8]) >> 2;
            
            // Generate alert flags
            alert_flags <= {
                flight_risk[15],   // Critical flight risk
                env_risk[15],      // Critical environmental risk
                maint_risk[15],    // Critical maintenance risk
                personnel_risk[15],// Critical personnel risk
                flight_risk[14:12],// Flight warnings
                env_risk[14:12],   // Environmental warnings
                maint_risk[14:12], // Maintenance warnings
                personnel_risk[14:12] // Personnel warnings
            };
            
            // System status
            system_ok <= !(|alert_flags[15:12]); // OK if no critical risks
        end
    end

endmodule
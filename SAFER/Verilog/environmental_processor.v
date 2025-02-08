// Environmental Data Processing Module
module environmental_processor (
    input wire clk,
    input wire rst_n,
    input wire [11:0] temperature,
    input wire [11:0] pressure,
    input wire [7:0] visibility,
    output reg [15:0] risk_score
);
    // Environmental thresholds
    parameter TEMP_MIN = 12'h100;  // Minimum safe temperature
    parameter TEMP_MAX = 12'hB00;  // Maximum safe temperature
    parameter PRES_MIN = 12'h200;  // Minimum safe pressure
    parameter VIS_MIN = 8'h40;     // Minimum safe visibility

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            risk_score <= 16'h0000;
        end else begin
            // Calculate environmental risk
            risk_score <=
                ((temperature < TEMP_MIN) ? 16'h8000 :
                 (temperature > TEMP_MAX) ? 16'h4000 :
                 (pressure < PRES_MIN)    ? 16'h2000 :
                 (visibility < VIS_MIN)   ? 16'h1000 : 16'h0000) |
                ((temperature < TEMP_MIN + 12'h080) ? 16'h0800 :
                 (temperature > TEMP_MAX - 12'h080) ? 16'h0400 :
                 (pressure < PRES_MIN + 12'h040)    ? 16'h0200 :
                 (visibility < VIS_MIN + 8'h20)     ? 16'h0100 : 16'h0000);
        end
    end
endmodule
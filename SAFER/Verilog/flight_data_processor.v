// Flight Data Processing Module
module flight_data_processor (
    input wire clk,
    input wire rst_n,
    input wire [15:0] altitude,
    input wire [15:0] speed,
    input wire [15:0] heading,
    output reg [15:0] risk_score
);
    // Flight parameter thresholds
    parameter ALT_MIN = 16'h0200;  // Minimum safe altitude
    parameter ALT_MAX = 16'hF000;  // Maximum operating altitude
    parameter SPD_MAX = 16'hC000;  // Maximum safe speed

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            risk_score <= 16'h0000;
        end else begin
            // Calculate flight risk based on parameters
            risk_score <= 
                ((altitude < ALT_MIN) ? 16'h8000 :
                 (altitude > ALT_MAX) ? 16'h4000 :
                 (speed > SPD_MAX)    ? 16'h2000 : 16'h0000) |
                ((altitude < ALT_MIN + 16'h0100) ? 16'h0800 :
                 (altitude > ALT_MAX - 16'h0100) ? 16'h0400 :
                 (speed > SPD_MAX - 16'h0800)    ? 16'h0200 : 16'h0000);
        end
    end
endmodule

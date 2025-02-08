// Personnel Status Processing Module
module personnel_processor (
    input wire clk,
    input wire rst_n,
    input wire [7:0] crew_status,
    output reg [15:0] risk_score
);
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            risk_score <= 16'h0000;
        end else begin
            // Calculate personnel risk
            risk_score <=
                ((crew_status[7:6] != 2'b11) ? 16'h8000 : // Critical crew issues
                 (crew_status[5:4] != 2'b11) ? 16'h4000 : // Major crew issues
                 (crew_status[3:2] != 2'b11) ? 16'h0800 : // Minor crew issues
                 (crew_status[1:0] != 2'b11) ? 16'h0400 : 16'h0000);
        end
    end
endmodule
module fetch (
    input logic clk,
    input logic reset,
    input logic [7:0] mem_hi,
    input logic [7:0] mem_lo,
    output logic [11:0] pc,
    output logic [15:0] opcode
);
    always_ff @(posedge clk or posedge reset) begin
        if(reset)
            pc <= 12'h200;
        else
           pc <= pc + 12'd2;

        opcode <= {mem_hi, mem_lo};
    end
endmodule
module decode (
    input logic [15:0] opcode,
    output logic [3:0] first_nibble,
    output logic [3:0] x,
    output logic [3:0] y,
    output logic [3:0] n,
    output logic [7:0] nn,
    output logic [11:0] nnn
);
    assign first_nibble = opcode[15:12];
    assign x = opcode[11:8];
    assign y = opcode[7:4];
    assign n = opcode[3:0];
    assign nn = opcode[7:0];
    assign nnn = opcode[11:0];
endmodule
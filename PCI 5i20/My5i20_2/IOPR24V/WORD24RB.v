module WORD24RB(obus,readport,portdata);


input readport;
input [23:0] portdata;
output [23:0] obus;

reg [23:0] obus;

always @(portdata, readport)
begin
	if (readport==1) 
		obus <= portdata;
	else
		obus <= 24'hZZZZZZ;
end

endmodule
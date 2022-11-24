module WORDPR24(clear, clk, ibus, obus, loadport, loadddr, readddr, portdata);

input	clear;
input	clk;


input	loadport;
input	loadddr;
input readddr;

input [23:0]	ibus;

output [23:0]	obus;
output [23:0]	portdata;

reg [23:0] outreg;
reg [23:0] ddrreg;
reg [23:0] tsoutreg;

integer i;

always @(posedge clk)
begin
	if(clear == 1)
		ddrreg <=24'h000000;
	else if (loadddr == 1)
		ddrreg <=ibus;
	else
		ddrreg <=ddrreg;
end
	
always @(posedge clk)
begin
	if (loadport == 1)
		outreg <=ibus;
	else
		outreg <=outreg;	
end


always @(ddrreg, outreg)
for (i = 0; i < 24; i = i+1)
  begin
    if (ddrreg[i]==1)  
				tsoutreg[i] = outreg[i];
			else
				tsoutreg[i] = 1'bZ;
  
end

assign portdata = tsoutreg;
assign obus = (readddr==1)? ddrreg: 24'hZZZZZZ;

endmodule

module test(	LWR,
					ADS,
					LAD,
					LClk,
					LEDS
);


input	LWR;
input	ADS;
inout  [3:0] LAD;
input	LClk;

output [7:0]	LEDS;

reg [3:0] A;
reg [3:0] D;


//AddressLatch: 
always @(posedge LClk)
begin
	if(~ADS)
		A <=LAD;
	else
		A <= A;
end




//Data Latch: 
always @(posedge LClk)
begin
	if(~LWR)
		D <=LAD;
	else
		D <= D;
end

assign LEDS = {D,A};

endmodule

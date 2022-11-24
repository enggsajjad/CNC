module IOPR24(LRD,LWR,LW_R,ALE,ADS,BLAST,WAITO,LOCKO,
					CS0,CS1,
					READY,
					INT,
					LAD,LA,LBE,IOBits,SYNCLK,LClk,LEDS
);


input	LRD;
input	LWR;
input	LW_R;
input	ALE;
input	ADS;
input	BLAST;
input	WAITO;
input	LOCKO;
input	CS0;
input	CS1;
output	READY;
output	INT;
	
inout  [31:0] LAD;
input [8:2]	LA;
input	[3:0] LBE;

inout	[71:0] IOBits;
input	SYNCLK;
input	LClk;
output [7:0]	LEDS;



	reg INT;
	reg READY;

	wire [31:0] D;
	reg [15:0] A;
	reg PreFastRead;
	wire FastRead;
	reg [31:0] test32;

	reg [3:0] WordSel;	
	reg [3:0] LoadPortCmd;
	reg [3:0] ReadPortCmd;
	reg [2:0] DDRSel;	
	reg [2:0] LoadDDRCmd;
	reg [2:0] ReadDDRCmd;
		
	integer i;
	// Instantiate the module
	LEDBLINK gledblink (
    .clk(SYNCLK), 
    .ledx(LEDS)
    );
	 	
	// Instantiate the module
	WORDPR24 oportx0 (
    .clear(1'b0), 
    .clk(LClk), 
    .ibus(LAD[23:0]), 
    .obus(D[23:0]), 
    .loadport(LoadPortCmd[0]), 
    .loadddr(LoadDDRCmd[0]), 
    .readddr(ReadDDRCmd[0]), 
    .portdata(IOBits[23:0])
    );
	// Instantiate the module
	WORDPR24 oportx1 (
    .clear(1'b0), 
    .clk(LClk), 
    .ibus(LAD[23:0]), 
    .obus(D[23:0]), 
    .loadport(LoadPortCmd[1]), 
    .loadddr(LoadDDRCmd[1]), 
    .readddr(ReadDDRCmd[1]), 
    .portdata(IOBits[47:24])
    );
	// Instantiate the module
	WORDPR24 oportx2 (
    .clear(1'b0), 
    .clk(LClk), 
    .ibus(LAD[23:0]), 
    .obus(D[23:0]), 
    .loadport(LoadPortCmd[2]), 
    .loadddr(LoadDDRCmd[2]), 
    .readddr(ReadDDRCmd[2]), 
    .portdata(IOBits[71:48])
    );	



	// Instantiate the module
	WORD24RB iportx0 (
    .obus(D[23:0]), 
    .readport(ReadPortCmd[0]), 
    .portdata(IOBits[23:0])
    );
	// Instantiate the module
	WORD24RB iportx1 (
    .obus(D[23:0]), 
    .readport(ReadPortCmd[1]), 
    .portdata(IOBits[47:24])
    );
	// Instantiate the module
	WORD24RB iportx2 (
    .obus(D[23:0]), 
    .readport(ReadPortCmd[2]), 
    .portdata(IOBits[71:48])
    );

always @(A)
begin
	case (A[4:2])
		3'b000:	WordSel = 4'b0001; 
		3'b001:	WordSel = 4'b0010;
		3'b010:	WordSel = 4'b0100;
		3'b011:	WordSel = 4'b1000;
		default:	WordSel = 4'b0000; 
	endcase
end
always @(A)
begin
	case (A[4:2])
		3'b100:	DDRSel = 3'b001; 
		3'b101:	DDRSel = 3'b010;
		3'b110:	DDRSel = 3'b100;
		default:	DDRSel = 3'b000; 
	endcase
end	

always @(WordSel,DDRSel,LWR,FastRead)
begin
	for (i=0;i<4; i=i+1)
	begin
		if (WordSel[i]==1 && LWR==0)
			LoadPortCmd[i] = 1'b1;
		else
			LoadPortCmd[i] = 1'b0;

		if (WordSel[i]==1 && FastRead==1)
			ReadPortCmd[i] = 1'b1;
		else
			ReadPortCmd[i] = 1'b0;
	end

	for (i=0;i<3; i=i+1)
	begin
		if (DDRSel[i]==1 && LWR==0)
			LoadDDRCmd[i] = 1'b1;
		else
			LoadDDRCmd[i] = 1'b0;

		if (DDRSel[i]==1 && FastRead==1)
			ReadDDRCmd[i] = 1'b1;
		else
			ReadDDRCmd[i] = 1'b0;
	end
end

//LADDrivers: 
assign LAD = (FastRead==1)?D:32'hZZZZZZZZ;
//AddressLatch: 
always @(posedge LClk)
begin
	if(ADS==0)
		A <=LAD[15:0];
	else
		A <= A;
end
//TestLatch:
/* 
always @(posedge LClk)
begin
	if(LoadPortCmd[3]==1)
		test32 <=LAD;
	else
		test32 <= test32;

	if(ReadPortCmd[3]==1)
		D <=test32;
	else
		D <= 32'hZZZZZZZZ;
end
*/

	// we generate an early read from ADS and LR_W
	// since the 10 nS LRD delay and 5 nS setup time
 	// only give us 15 nS to provide data to the PLX chip

//MakeFastRead: 
always @(posedge LClk)
begin
	if(ADS ==0 && LW_R==0)
		PreFastRead<= 1'b1;
	else
		PreFastRead<= 1'b0;
end

assign FastRead = PreFastRead | (~LRD);

endmodule

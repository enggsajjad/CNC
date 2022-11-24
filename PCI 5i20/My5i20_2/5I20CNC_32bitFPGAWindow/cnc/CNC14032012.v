module CNC(LWR,
					ADS,
					LAD,
					LClk,
					LEDS,
					ST_CLK,
					ST_ENB,
					ST_DIS,
					ST_DIR,
					SP_BRK,
					SP_DIS,
					SP_DIR
);


input	LWR;
input	ADS;
//inout  [7:0] LAD;
input  [31:0] LAD;
input	LClk;

output [7:0]	LEDS;
output ST_CLK;
output ST_ENB;
output ST_DIS;
output ST_DIR;
output SP_BRK;
output SP_DIS;
output SP_DIR;

reg [31:0] A;
reg [31:0] D;

reg ST_CLK;
reg ST_DIS;
reg ST_DIR;
reg ST_ENB;
reg SP_DIS;
reg SP_DIR;
reg SP_BRK;
reg [31:0] cnt;
reg [31:0] cnt_val;
wire LWR1;
reg cnt_en;
wire  rst;

assign rst = 1;

IBUF i1(.O(LWR1),.I(LWR));
//AddressLatch: 
always @(posedge LClk)
begin
	if(ADS==0)
		A <=LAD;
	else
		A <= A;
end


//Data Latch: 
always @(posedge LClk)
begin
	if(LWR1==0)
		D <=LAD;
	else
		D <= D;
end

// Address (Command) Decoder

//Stepper Motor Clock SIGNAL
always @(posedge LClk)
begin
   if((ST_DIS==0)&&(ST_ENB==1))
	begin	
	if(cnt==cnt_val)
		ST_CLK <= ~ST_CLK;
	else 
		ST_CLK <=ST_CLK;
	end
	else
		ST_CLK <=0;
end

//Stepper Motor ENABLE SIGNAL
always @(A , LWR1, rst)
begin
	if(rst==0)
	  	cnt_val <= 0;
	else 	if(A[4:2]==1 && LWR1==0)
		cnt_val <= D-1;
	else
		cnt_val <= cnt_val;
end

always @(A , LWR1, rst)
begin
	if(rst==0)
	  	cnt_en <= 0;
	else 	if(A[4:2]==1 && LWR1==0)
		cnt_en <= 1;
	else 
		cnt_en<= 0;
end
	 
//Stepper Motor ENABLE SIGNAL
always @(posedge LClk)
begin
	if(cnt_en)
		cnt <= 0;
	else if (cnt==cnt_val) 
		cnt<= 0;
	 else
		cnt <= cnt+1;
end
//Stepper Motor ENABLE SIGNAL
always @(LWR1 , A, rst)
begin
	if(rst==0)
	  	ST_ENB <= 0;
	else if(A[4:2]==2 && LWR1==0)
		ST_ENB <= D[0];
	else
		ST_ENB <= ST_ENB;
end

// Stepper Motor DISABLE SIGNAL
always @(LWR1 , A, rst)
begin
	if(rst==0)
	  	ST_DIS <= 0;
	else if(A[4:2]==3 && LWR1==0)
		ST_DIS <= D[0];
	else
		ST_DIS <= ST_DIS;
end

// Stepper Motor DIRECTION SIGNAL
always @(LWR1 , A, rst)
begin
	if(rst==0)
	  	ST_DIR <= 0;
	else if(A[4:2]==4 && LWR1==0)
		ST_DIR <= D[0];
	else
		ST_DIR <= ST_DIR;
end


//Spindle Motor BREAK SIGNAL
always @(LWR1 , A, rst)
begin
	if(rst==0)
	  	SP_BRK <= 0;
	else if(A[4:2]==5 && LWR1==0)
		SP_BRK <= D[0];
	else
		SP_BRK <= SP_BRK;
end

//Spindle Motor BREAK SIGNAL
always @(LWR1 , A, rst)
begin
 	if(rst==0)
	  	SP_DIS <= 0;
	else if(A[4:2]==6 && LWR1==0)
		SP_DIS <= D[0];
	else
		SP_DIS <= SP_DIS;
end

//Spindle Motor BREAK SIGNAL
always @(LWR1 , A, rst)
begin
   if(rst==0)
	  	SP_DIR <= 0;
	else	if(A[4:2]==7 && LWR1==0)
		SP_DIR <= D[0];
	else
		SP_DIR <= SP_DIR;
end


//assign LEDS = ~{1'b1,ld_Cnt,SP_BRK,SP_DIR,SP_DIS,ST_ENB,ST_DIR,ST_DIS};
assign LEDS = ~{D[4:0],A[4:2]};
//assign LEDS = ~{D[7:0]};
endmodule

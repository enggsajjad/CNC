module IOPR24(LWR,
					ADS,
					LAD,
					LClk,
					LEDS,
					ST_CLK
);


input	LWR;
input	ADS;
//inout  [7:0] LAD;
input  [7:0] LAD;
input	LClk;

output [7:0]	LEDS;
output ST_CLK;

reg [7:0] A;
reg [7:0] D;

wire st_load_clk;
wire st_dis_hi;
wire st_dis_lo;
wire st_dir_hi;
wire st_dir_lo;
wire st_enb_hi;
wire st_enb_lo;

wire sp_dis_hi;
wire sp_dis_lo;
wire sp_dir_hi;
wire sp_dir_lo;
wire sp_brk_hi;
wire sp_brk_lo;
wire st_clk_hi;
wire st_clk_lo;


reg ST_CLK;
reg ST_DIS;
reg ST_DIR;
reg ST_ENB;
reg SP_DIS;
reg SP_DIR;
reg SP_BRK;
reg [31:0] cnt;
reg [31:0] cnt_val;
reg [15:0] cmds;
reg [3:0] Areg;
reg ld_val;




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

// Address (Command) Decoder

always @(A)
begin
	case (A[3:0])
		//Stepper Motor Signals
		0:	cmds = 16'h0000;
		1:	cmds = 16'h0001; //st_clk_hi
		2:	cmds = 16'h0002;	//st_dis_hi
		3:	cmds = 16'h0004;	//st_dis_lo
		4:	cmds = 16'h0008;	//st_dir_hi
		5:	cmds = 16'h0010;	//st_dir_lo
		6:	cmds = 16'h0020;	//st_enb_hi
		7:	cmds = 16'h0040;	//st_enb_lo
		8:	cmds = 16'h0080;	//sp_dis_hi
		9:	cmds = 16'h0100;	//sp_dis_lo
		10:cmds = 16'h0200;	//sp_dir_hi
		11:cmds = 16'h0400;	//sp_dir_lo
		12:cmds = 16'h0800;	//sp_brk_hi
		13:cmds = 16'h1000;	//sp_brk_lo
		14:cmds = 16'h2000;	//st_clk_lo
		default cmds =16'h0000;
	endcase
end

//Stepper Motor Signals
assign st_clk_hi = cmds[0]; 
assign st_clk_lo = cmds[13]; 

assign st_dis_hi = cmds[1];
assign st_dis_lo = cmds[2];

assign st_dir_hi = cmds[3];
assign st_dir_lo = cmds[4];

assign st_enb_hi = cmds[5];
assign st_enb_lo = cmds[6];


//Spindle Motor Signals
assign sp_dis_hi = cmds[7];
assign sp_dis_lo = cmds[8];

assign sp_dir_hi = cmds[9];
assign sp_dir_lo = cmds[10];

assign sp_brk_hi = cmds[11];
assign sp_brk_lo = cmds[12];


//1. Stepper Motor CLK SIGNAL
always @(posedge LClk)
begin
	if(st_clk_hi)
		ld_val <= 1'b1;
	else if (st_clk_lo)
	   ld_val <= 1'b0;
	else
	   ld_val <= ld_val;
 
end

always @(posedge LClk)
begin
	if(ld_val)//(st_load_clk)
		cnt_val <= 32'h17D7840 - {D[7:0],8'h00};//{D[7:0],16'h0000};
	else
		cnt_val <= cnt_val;
end

//1. Stepper Motor DISABLE SIGNAL
always @(posedge LClk)
begin
	if ((ST_ENB==0)&&(ST_DIS==1))
	   cnt <=32'h00000000;
	else if(cnt==cnt_val)
		cnt <= 0;
	else   
		cnt <= cnt+1;
end

//1. Stepper Motor DISABLE SIGNAL
always @(posedge LClk)
begin
   if((ST_DIS==0)&&(ST_ENB==1))
	begin	
	if(cnt==0)
		ST_CLK <= ~ST_CLK;
	else 
		ST_CLK <=ST_CLK;
	end
	else
		ST_CLK <=0;
end
	 

//2. Stepper Motor DISABLE SIGNAL
always @(posedge LClk)
begin
	if(st_dis_lo)
		ST_DIS <= 1'b0;
	else if (st_dis_hi)
		ST_DIS <= 1'b1;
	else
		ST_DIS <= ST_DIS;
end

//2. Stepper Motor DIRECTION SIGNAL
always @(posedge LClk)
begin
	if(st_dir_lo)
		ST_DIR <= 1'b0;
	else if (st_dir_hi)
		ST_DIR <= 1'b1;
	else
		ST_DIR <= ST_DIR;
end
//3. Stepper Motor ENABLE SIGNAL
always @(posedge LClk)
begin
	if(st_enb_lo)
		ST_ENB <= 1'b0;
	else if (st_enb_hi)
		ST_ENB <= 1'b1;
	else
		ST_ENB <= ST_ENB;
end

//1. Spindle Motor DISABLE SIGNAL
always @(posedge LClk)
begin
	if(sp_dis_lo)
		SP_DIS <= 1'b0;
	else if (sp_dis_hi)
		SP_DIS <= 1'b1;
	else
		SP_DIS <= SP_DIS;
end
//2. Spindle Motor DIRECTION SIGNAL
always @(posedge LClk)
begin
	if(sp_dir_lo)
		SP_DIR <= 1'b0;
	else if (sp_dir_hi)
		SP_DIR <= 1'b1;
	else
		SP_DIR <= SP_DIR;
end
//3. Spindle Motor BRAKE SIGNAL
always @(posedge LClk)
begin
	if(sp_brk_lo)
		SP_BRK <= 1'b0;
	else if (sp_brk_hi)
		SP_BRK <= 1'b1;
	else
		SP_BRK <= SP_BRK;
end



//assign LEDS = ~{2'b11,SP_BRK,SP_DIR,SP_DIS,ST_ENB,ST_DIR,ST_DIS};
assign LEDS = ~{D[7:0]};
endmodule

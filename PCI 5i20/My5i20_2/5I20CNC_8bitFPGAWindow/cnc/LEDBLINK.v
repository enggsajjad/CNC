module LEDBLINK(clk,ledx);

input clk;
output [7:0] ledx;

reg [28:0] count;

always @(posedge clk)
begin
	count <= count+1;
end


//assign	ledx = ~count[28:21];
assign	ledx = ~{count[24:21],4'b1111};

endmodule
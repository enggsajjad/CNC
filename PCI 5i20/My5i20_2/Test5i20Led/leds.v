module leds(led);

output [7:0] led;

assign led = 8'h05;
/*
LED CONNECTIONS:

LED0,CR8		10
LED1,CR7		14
LED2,CR6		15
LED3,CR5		16
LED4,CR4		17
LED5,CR3		18
LED6,CR2		20
LED7,CR1		21
*/

endmodule

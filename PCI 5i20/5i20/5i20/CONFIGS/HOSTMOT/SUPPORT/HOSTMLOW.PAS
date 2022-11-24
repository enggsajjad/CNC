const
{ note: all registers are 16 bit unless noted }

CounterBase = $00;      { 32 bit counter base address }
CCRegBase = $40;

{CCR (Counter Control Register) Bit definitions:}
CCRCounterMode = $200;	 { 0 = quadrature 1 = up/down}
CCRInputFilter = $100;	 { 0 = No filter 1 = ~2 MHz input filter}
CCRInbit = $80;
CCRCountenable = $40;	 { 0 = count disabled 1 = counting enabled}
CCRClearOnIndex = $20;	 { 0 = no COI 1 = clear counter on index}
CCRLatchedindex	= $10;   { 0 = no index	1 = index detected}
CCRIndexPolarity = $08;  { 0 = index active low	1 = index active high}
CCRIndex = $04;          { Real time index status (reads) counter clear (writes)}
CCRB = $02;		 { Real time input A status (read only)}
CCRA = $01;		 { Read time input B status (read only)}

PWMBase = $60;
PCRegBase = $80;

{PCR bit Definitions:}
PCRDir = $10;		{DIR bit (input if in unsigned mode)}
PCRPWMBit = $08;	{Realtime PWM output bit status (read only)}
PCRUnsigned = $04;	{Unsigned mode bit 1 = Unsigned}
{In signed mode, PWMVAL is a signed 16 bit number which is output as 10 bit PWM plus}
{sign on the direction bit.}
{In unsignedmode PWMVAL is an unsigned 16 bit number which is outPut as 10 bit PWM}
{DIR is an input in unsigned mode.}
PCRInterlaceEna = $02;	{Interlace enable (interlaces PWM output for use as analog out)}
PCRPWMEna = $01; 	{Enable -- enables PWM and ENA bit if set}

PortA = $A0;            { 32 bit port a register }
PortADDR = $A4;         { 32 bit port b register }

GContReg = $C0;
{GContReg bit definitions:}

GCClrIRQ = $08;   {0 = NOP 1 = Clear IRQ}
GCClrPWMs = $04;  {0 = NOP 1 = Clear all PWMs}
GCLatchAll = $02; {0 = NOP 1 = Latch all counters}
GCCClrAll = $01;  {0 = NOP 1 = Clear all counters}

{Note GContReg is write only and "transient" you only need to write once to}
{accomplish the desired action, no need to clear the bit later...}

GModeReg = $C2;

{GMode register bit definitions:}

GMIRQstatus = $40;  {Real time IRQ status}
GMIRQMask = $20;    {IRQ mask bit}
GMMissedIRQ = $10;  {Missed interrupt status}
GMSOM = $08;        {Stop on Missed interrupt control bit}
GMLOI = $04;        {Latch on interrupt control bit}
GMPWMEN = $02;      {Global PWM enable}
GMCTREN = $01;      {Global counter enable}

IRQDivReg = $C4;
{8 bit programmable divider for IRQ rate = PWM rate/(IRQDIV+1)}

RateReg = $C6;
{16 bit rate register: multiplies PWM reference counter input rate by}
{RATE/65536 so final PWM rate = 33MHz/1024 * RATE/65536}

LEDPtrReg = $C8;

{LED PTR register determines what channel the on card LEDS monitor.
{LEDs monitor the following signals of the selected channel:

{CR1	IRQLatch}
{CR2	QuadA}
{CR3	QuadB}
{CR4	IDX}
{CR5	DIR}
{CR6	PWM}
{CR7	ENA}
{CR8	OUTBIT}

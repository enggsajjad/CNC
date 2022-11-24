;Copyright (C) 2005 MESA Electronics.  All rights reserved.
;The functional interface to the motor controller is done by a dual port ram
;sample:	the time between iterations of the pid loop, see prescale and postscale
;counts:	encoder counts ppr*4

;UNITS
;despos		encoder counts
;pwm		8.8 signed number, only integer part used for output
;velocity	8.24 counts per sample (127 max motor counts per sample)
;accel		8.24 counts per sample per sample	  B.BBB
;accelfract	is the fractional part of accel           0.000BB
;dela		added to accelfract and accel each sample 0.00000BBBB
;avgvel		8.8 velocity units

;TYPES (contents of ram address)
;	<blank>	number of bits is hardware dependent	
;	ptr	10 bit address pointer
;	flag	16 bit value 0 is false, nonzero is true
;	uint	16 bit unsigned value 0..65535
;	int	16 bit signed value -32768..32767
;	ulong	32 bit unsigned value 0..4294967295
;	long	32 bit signed value -2147483648..2147483647
;	double	64 bit signed value -1.8E19..1.8E19

;PARAMETER	VALUE	TYPE	FUNCTION
null		0	%ptr	uninitialized pointers point to dummy

majorrev	2		updated when parameters change, hi byte
minorrev	4		lo byte

dummy		0	%uint	default pointers point here 
go		1	%flag	set true to start move
pid		2	%flag	set true to enable pid 
profile		3	%flag	set true to enable profile generator
extprofile	4	%flag	set true for host to control velocity
dirinv		5	%flag	set true to invert motor drive polarity
nextmot		6	%flag	set true to copy motion block, becomes false when copy complete
nextfil		7	%flag	set true to copy filter block, becomes false when copy complete
home		8	%flag	becomes true when home has occured
motion		9	%flag	becomes when velocity = 0
slew		10 	%flag	becomes true when maximum velocity is reached
error		11 	%flag	becomes true when excessive position or pwm error occurs
events		12	%uint	number of events 16 max
maxpwm		13	%uint	pwm limited to this value
exposerr	14 	%uint	excessive position error limit <32767
expwmerr	15	%uint	excessive pwm count limit
avgvel		16	%int	average velocity 8.8
maxnegerr	17 	%int	max negative error
maxposerr	18 	%int	max positive error
actvel		19	%int	actual velocity
bnderr		20	%int	short bounded(error_long)
;reserved	21
extvelocity	22	%ptr	pointer to velocity parameter
follow		23	%ptr	pointer to desired position
gearpparam	24	%ptr	pointer to primary geared parameter
gearsparam	25	%ptr	pointer to secondary geared parameter
gearp		26	%long	primary gear ratio 8.24
gears		28	%long	secondary gear ratio 8.24
gearpoffset	30	%long	primary offset
gearsoffset	32	%long	secondary offset
preslewvel	34	%long	velocity before slew starts or maximum velocity
preslewpos	36	%long	position before slew starts or maximum velocity

;reserved	38	
;reserved	39

;32bit user parameters
homeposp	40	%long	position loaded when primary index occurs
homeposs	42	%long	position loaded when seconday index occurs
phasek		44	%long	phase konstant
phasea		46	%long	phase accumulator, phasek is added to phasea each sample
encp		48	%long	primary encoder, read only
encs		50	%long	secondary encoder, read only
velocity	52 	%long	current profile velocity
fixup		54	%long	profile generator errror

;filter param
filterblock	56	begining of filter block
kk		56	%int	pwm offset, initial pwm value
kff		57	%uint	friction in the direction of motion
kp		58	%uint	kpd*error added to pwm
kd		59 	%uint	kdd*actualvelocity added to pwm
ka		60	%uint	unused accel error
ki		61	%ulong	ki*error added to integralq each sample, high word added to pwm
kih		62	%uint	high word of ki
kil		63 	%uint	high word limit of ki, 32767 max
kf0		64	%uint	(unimplemented) spring
kf1		65	%uint	velocity ff
kf2		66	%uint	accel ff
kf3		67	%uint	unused
kdfil		68	%uint	amount of filtering 0..32767
posenc		69	%ptr	pointer to position encoder
velenc		70	%ptr	pointer to velocity encoder
nextfilblock	71	%ptr	pointer to next filter block

;motion parameters with breakpoint registers
motionblock	72		beginning of motion block
nextpos		72	%long	where we want to go next
slewlimit	74	%long	slew speed limit, 2^23-1 max for kf1,kf2
desvel		76 	%long	desired velocity		
accel		78	%long	acceleration, if 0 then no motion
dela		80	%long	delta accel, added to accel each sample
breakpoint	82	%long	local breakpoint value
breaka		84	%ptr 	pointer for compare event 
breakb		85	%ptr	pointer for compare event
nextmotblock	86	%ptr	pointer to next move block
user		87	%uint

;64bit internal use only
desposf		88 	%double	fractional part of profile position
despos		90	%long	profile position

;32bit internal use only
pwm		92	%int	pwm value
;reserved	93 
delta		94	%long	velq - velocityq 
lastvelocity	96	%long	old velocity
checkpos	98 	%long	checking position for done
decelpos	100 	%long	position to start decel

;48bit number internal use only
integral	102 	%long	integral error accumulator
integralovf	104	%int	overflow for integral

;16bit internal use only
pwmcntr		105	%uint	counter of full scale pwm
reset		106	%flag	true when hardware counter is initialized
accelfract	107	%int	fractional part of accel for delaq
oldgo		108	%flag	internal
oldpid		109 	%flag	internal
lastpos		110 	%uint	last profile position low half
skip		111	%flag	skip position update once only
	
;user ram
block9		112
event1		112
event2		120
block8		128
event3		128
event4		136
block7		144
event5		144
event6		152
block6		160
event7		160
event8		168
block5		176
event9		176
event10		184
block4		192
event11		192
event12		200
block3		208
event13		208
event14		216
block2		224
event15		224
event16		232
block1		240
event17		240
event18		248

;global ram
gevent1		256		host events
gevent2		264
gevent3		272
gevent4		280
gevent5		288
gevent6		296
gevent7		304
gevent8		312
gevent9		320
gevent10	328
gevent11	336
gevent12	344
gevent13	352
gevent14	360
gevent15	368
gevent16	376
gevent17	384
gevent18	392
gevent19	400
gevent20	408
gevent21	416
gevent22	424
gevent23	432
gevent24	440

;geared parameter
gearedp0	448	%long	axis 0 primary geared
geareds0	450	%long	axis 0 seconday geared
gearedp1	452	%long	axis 1 primary geared
geareds1	454	%long	axis 1 seconday geared
gearedp2	456	%long	axis 2 primary geared
geareds2	458	%long	axis 2 seconday geared
gearedp3	460	%long	axis 3 primary geared
geareds3	462	%long	axis 3 seconday geared
gearedp4	464	%long	axis 4 primary geared, 8 axis only
geareds4	466	%long	axis 4 seconday geared, 8 axis only
gearedp5	468	%long	axis 5 primary geared, 8 axis only
geareds5	470	%long	axis 5 seconday geared, 8 axis only
gearedp6	472	%long	axis 6 primary geared, 8 axis only
geareds6	474	%long	axis 6 seconday geared, 8 axis only
gearedp7	476	%long	axis 7 primary geared, 8 axis only
geareds7	478	%long	axis 7 seconday geared, 8 axis only

gphasek		480	%long	gphase konstant
gphasea		482	%long	gphase accumulator, gphasek is added to gphasea each sample

ledaxis		484	%ptr	axis for leds to monitor
led		485	%ptr	parameter for leds to monitor, if not zero
beeper		486	%ptr	beep uses what this points to, if not zero
timeout		487	%flag	incremented each time we haven't finished before the next sample
proctimer	488	%uint	read proctimer here
gevents		489	%uint	number of global events 16 max
axis		490	%ptr	the current axis

;these locations are reserved, do not use
greservedstart	491
greservedend	510 

swrevision	511	%uint	majorrev hibyte minorrev lobyte

;************************* PORTS **********************************************
hcomm		512	%uint	16bit register (read only)
clrbusy		513		write clears host busy flag
hdata		514	%ulong	32bit register

axisindex	516		4 bits
statusreg	517		read samplebit and commandpending

counterp	518		primary 9bit relative, latched by sample, read only
cntcntlp	519		primary counter control reg

counters	520		secondary 9bit relative, latch by sample, read only
cntcntls	521		secondary counter control reg

prescale	524		pwm rate = 50mhz/256*(65536/prescale), write only
postscale	525		sample rate = pwm rate/postscale, write only

pwmgen		526		pwm generator per axis msbyte of word
dir		527		per axis msbit of word
ena		528		per axis pwm enable
hproctimer	529		hardware write to latch then read

hcommrw		531	%uint	16bit register (read write)

shiftbox	532		(8.24 * 256) becomes 32.32 out

;ports are 12 bits wide
porta		536	
portaddr	537		bit is 1 for output
portb		538	
portbddr	539		bit is 1 for output
portc		540	
portcddr	541		bit is 1 for output
portd		542	
portdddr	543		bit is 1 for output
porte		544	
porteddr	545		bit is 1 for output
portf		546	
portfddr	547		bit is 1 for output

irqreg		552		irqnumber
irqena		553		write to enable 
irqdis		554		write to disable

;trace buffer			read		write
tbuffpush	560		data		data
tbuffpop	561		data		pop pointer
tbuffmode	562		STF		STF		lsb StopOnFull
tbuffcnt	563		count		reset

;multiplier hardware
mula		576	%int	signed multiplier read/write
mulb		577	%uint	unsigned multiplier read/write
hprod		578	%long	32bit signed product read only
hprodh		580	%uint	32bit unsigned product hi word read only
hacc		582	%long	r/w, adds hprod when write to hprod


leds		624		byte 
beep		625		msb

;hardware constants
bit14		630	%uint	constant
bit13		631	%uint	constant
zero		632	%int
false		632	%flag	
one		633	%int
minusone	634	%int	
true		634	%flag
signbit		635	%int

naxis		637		number of axis's in hardware
hwrevision	638		hardware revision
hwtype		639		harware type

;step motor parameters
step		2	%flag	set true to enable stepping 
gearcopy	21	%flag	enables gearing and copying
microoffset	56	%uint	microstep offset
;reserved	57
motori		58	%uint	current set, 21845/amp hi current (43690/amp lo)
motorioffset	59	%int	phase B current offset

;step motor ports
pwmgena		522		pwm generator per axis lsbyte of word
dira		523		per axis bit 9 of word
pwmgenb		526		pwm generator per axis lsbyte of word
dirb		527		per axis bit 9 of word
lookupadd	556	%uint	sine lookup table address r/w 0..511
lookupdata	557	%uint	sine lookup table data r/w 0..255


;transfer types			axis	parameter	value
;Word 	16bit unsigned uint
;Double 32bit signed long
;Flag	16bit flag  flag
;Prog	Program rom file
;Conf configure eeprom	file

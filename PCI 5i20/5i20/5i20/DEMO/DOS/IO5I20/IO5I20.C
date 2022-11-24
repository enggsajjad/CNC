/*---------------------------------------------------------------------------*/
/*
   io5i20.c

   Simple parallel I/O demonstration program for the Mesa Electronics 5I20
	 I/O card/DOS.  Uses IOPR24.

   Version 1.0, Saturday December 11, 2004 -- 01:21:11.
*/
/*---------------------------------------------------------------------------*/
/*
   Compiler: Borland C++, version 3.1+.  (Use command line options "-B -3".)

   Tabs: 4
*/
/*---------------------------------------------------------------------------*/
/*
   Revision history.

   1) Version 1.0, Saturday December 11, 2004 -- 01:21:11.

	  Code frozen for version 1.0.
*/

/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dos.h>

/*---------------------------------------------------------------------------*/

/* Assorted #defines.
*/
#define PROGNAME      "IO5I20"
#define SIGNONMESSAGE "\n"PROGNAME" version "PROGVERSION"\n"
#define PROGVERSION   "1.0"

/* PCI registers.
*/
#define R_5I20PCICFGIO32 0x1C /* 32-bit I/O window to FPGA. (Base address register 3.) */

/* Port offsets for IOPR24.
*/
#define PBASE_IO        0  /* Port offset to I/O block. */
#define PBASE_DIRECTION 16 /* Port offset to I/O data direction control block. */

/* PCI BIOS services functions.
*/
#define F_PCIBIOSERVICES 0xB1 /* PCI BIOS services escape for interrupt 0x1A. */

#define PCIBIOSFUNC_HAVEBIOSQ   0x01 /* Check for presence of PCI BIOS, get interface particulars. */
#define PCIBIOSFUNC_FINDDEVBYID 0x02 /* Search for a device with the specified vendor I.D., device I.D., and index. */
#define PCIBIOSFUNC_READCFGWORD 0x09 /* Read configuration word. */

/* PCI BIOS support function return codes.
*/
#define PCIBIOSRETCODE_SUCCESS 0x00 /* The service exists, concluded successfully. */

/* Card I.D.
*/
#define VENDORID5I20 0x10B5 /* PCI vendor I.D. */
#define DEVICEID5I20 0x9030 /* PCI device I.D. */

/* Exit codes.
*/
#define EC_OK     0   /* Exit OK. */
#define EC_BADCL  100 /* Bad command line. */
#define EC_USER   101 /* Invalid input. */
#define EC_SYS    104 /* Beyond our scope. */

/*---------------------------------------------------------------------------*/

/* Assorted typedefs.
*/
typedef unsigned char unsigned8, ubyte ;
typedef unsigned short unsigned16, uword ;
typedef unsigned long unsigned32, udword ;

/*---------------------------------------------------------------------------*/

/* Local external variables.
*/
static udword OVal, DirectionMask ;

static int DoOutput /* = !!0 */, SetDirection /* = !!0 */ ;

static unsigned PortIndex ;

static uword P_FPGAIOBase ;

static const char ProgNameMsg[] = { PROGNAME } ;

static unsigned8 PCIBus, PCIDev ;

/*---------------------------------------------------------------------------*/

/* Function: fatalerror
   Purpose: Display a fatal error message and exit.
   Used by: Local functions.
   Returns: Never.
   Notes:
   Revision history:
	 1) Saturday December 11, 2004 -- 01:21:11.
*/

static void fatalerror(int exitcode, const char *fmt, ...)

{
	va_list vp ;


	va_start(vp, fmt) ;
	fprintf(stderr, "\n\n\a%s:\n", ProgNameMsg) ;
	vfprintf(stderr, fmt, vp) ;
	putc('\n', stderr) ;

	exit(exitcode) ;
}

/*---------------------------------------------------------------------------*/

/* Function: dataout32
   Purpose: Write a doubleword to the specified port.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Saturday December 11, 2004 -- 01:21:11.
*/

static void dataout32(unsigned p, udword d)

{
	p += P_FPGAIOBase ;
	asm {
				mov		dx,[p]
				mov		eax,[d]
				out		dx,eax
	}
}

/*---------------------------------------------------------------------------*/

/* Function: datain32
   Purpose: Read a doubleword from the specified port.
   Used by: Local functions.
   Returns: The doubleword.
   Notes:
   Revision history:
	 1) Saturday December 11, 2004 -- 01:21:11.
*/

static udword datain32(unsigned p)

{
	udword retval ;


	p += P_FPGAIOBase ;
	asm {
				mov		dx,[p]
				in		eax,dx
				mov		[retval],eax
	}
	return retval ;
}

/*---------------------------------------------------------------------------*/

/* Function: havepcibiosq
   Purpose: Determine whether or not we have PCI BIOS services available.
   Used by: Local functions.
   Returns: Non-zero if we have PCI BIOS services functionality.
   Notes:
   Revision history:
	 1) Saturday December 11, 2004 -- 01:21:11.
*/

static int havepcibiosq(void)

{
	union REGS r ;


	(r . x . ax) = ((F_PCIBIOSERVICES << 8) | PCIBIOSFUNC_HAVEBIOSQ) ;
	int86(0x1A, &r, &r) ;
	return (!(r . x . cflag) & ((r . h . ah) == PCIBIOSRETCODE_SUCCESS) & ((r . x . dx) == (('C' << 8) | 'P'))) ;
}

/*---------------------------------------------------------------------------*/

/* Function: pcicardsearch
   Purpose: Try to find the specified PCI card.  If found set parameters.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Saturday December 11, 2004 -- 01:21:11.
*/

static signed pcicardsearch(unsigned cardnum)

{
	union REGS r ;


	(r . x . ax) = ((F_PCIBIOSERVICES << 8) | PCIBIOSFUNC_FINDDEVBYID) ;
	(r . x . cx) = DEVICEID5I20 ;
	(r . x . dx) = VENDORID5I20 ;
	(r . x . si) = cardnum ;
	int86(0x1A, &r, &r) ;
	PCIBus = (r . h . bh) ;
	PCIDev = (r . h . bl) ;

	return ((!(r . x . cflag) & ((r . h . ah) == PCIBIOSRETCODE_SUCCESS)) ? 0 : -1) ;
}

/*---------------------------------------------------------------------------*/

/* Function: pcicfgread16
   Purpose: Read a 16-bit parameter from a PCI register file.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Saturday December 11, 2004 -- 01:21:11.
*/

static signed pcicfgread16(unsigned char busnum, unsigned char devnfunc, unsigned regnum, unsigned short *wptr)

{
	union REGS r ;


	(r . x . ax) = ((F_PCIBIOSERVICES << 8) | PCIBIOSFUNC_READCFGWORD) ;
	(r . h . bh) = busnum ;
	(r . h . bl) = devnfunc ;
	(r . x . di) = regnum ;
	int86(0x1A, &r, &r) ;
	*wptr = (r . x . cx) ;

	return ((!(r . x . cflag) & ((r . h . ah) == PCIBIOSRETCODE_SUCCESS)) ? 0 : -1) ;
}

/*---------------------------------------------------------------------------*/

/* Function: findpcicard
   Purpose: Try to find the specified PCI card.  If found set parameters.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Saturday December 11, 2004 -- 01:21:11.
*/

static void findpcicard(unsigned cardnum)

{
	if(!havepcibiosq())
	{
		fatalerror(EC_SYS, "No PCI BIOS services are available on this system.") ;
	}
	if(pcicardsearch(cardnum) < 0)
	{
		fatalerror(EC_USER, "Can't find 5I20 card number %u.\n", cardnum) ;
	}
	printf("\nUsing PCI device at bus %u/device %u.", PCIBus, (PCIDev >> 3)) ;

	/* Get our I/O base address.
	*/
	pcicfgread16(PCIBus, PCIDev, R_5I20PCICFGIO32, &P_FPGAIOBase) ; /* (We don't bother to check for errors.) */
	P_FPGAIOBase &= ~0x3 ;

	printf("\nPLX 9030 32-bit I/O base: 0x%04X", P_FPGAIOBase) ;

	putchar('\n') ;
}

/*---------------------------------------------------------------------------*/

/* Function: signon
   Purpose: Display the signon message.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Saturday December 11, 2004 -- 01:21:11.
*/

static void signon(void)

{
	printf("%s", SIGNONMESSAGE) ;
	printf("Mesa Electronics 5I20 simple I/O demo. for DOS.\n") ;
}

/*---------------------------------------------------------------------------*/

/* Function: pcl
   Purpose: Collect parameters from the command line.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Saturday December 11, 2004 -- 01:21:11.
*/

static const char CardInfo[] = "\n"
"card is the number of the PCI card.  (For use with multiple 5I20 cards.)" ;

static const char PortInfo[] = "\n"
"port is the index of the group of 24 I/O bits to be accessed; 0 for bits\n"
"  0...23, 1 for bits 24...47, 2 for bits 48...71.  (See card silkscreen.)" ;

static const char ValInfo[] = "\n"
"val is a 24-bit hexadecimal number to output to 5I20 GPIO.  If this field is\n"
"  supplied, a write will be performed to the specified port.  Otherwise a read\n"
"  will be performed." ;

static const char MaskInfo[] = "\n"
"mask, if specified, is a 24-bit hexadecimal I/O mask that will be used to\n"
"  configure the I/O directions of the pins of the port specified by port.\n"
"  0-bits switch the the bit direction to 0utput, 1-bits switch the bit\n"
"  direction to 1nput." ;

static const char PurposeInfo[] = "\n"
"Purpose: 5I20 I/O access demo. using IOPR24 configuration.  Note that the\n"
"  IOPR24 configuration must be manually loaded (see SC5I20*) before this\n"
"  program will work." ;


static void pcl(unsigned argc, char *argv[])

{
	unsigned cardnum = 0 ;


	if((argc < 3) || (argc > 5))
	{
		printf("\n%s program usage:\n%s card port[ val][ mask]", ProgNameMsg, ProgNameMsg) ;
		printf("\nWhere:") ;
		printf("%s", CardInfo) ;
		printf("%s", PortInfo) ;
		printf("%s", ValInfo) ;
		printf("%s", MaskInfo) ;
		printf("%s", PurposeInfo) ;
		putchar('\n') ;

		exit(EC_BADCL) ;
	}

	if(sscanf(argv[2], "%u", &PortIndex) != 1)
	{
		badportindex:
		fatalerror(EC_USER, "Invalid port index: %s", argv[2]) ;
	}
	if(PortIndex > 2)
	{
		goto badportindex ;
	}

	if(argc >= 4)
	{
		if(sscanf(argv[3], "%lx", &OVal) != 1)
		{
			fatalerror(EC_USER, "Invalid output value: %s", argv[3]) ;
		}

		DoOutput = !0 ;
	}

	if(argc >= 5)
	{
		if(sscanf(argv[4], "%lx", &DirectionMask) != 1)
		{
			fatalerror(EC_USER, "Invalid direction mask: %s", argv[4]) ;
		}

		SetDirection = !0 ;
	}

	if(sscanf(argv[1], "%u", &cardnum) != 1)
	{
		badcardnum:
		fatalerror(EC_USER, "Invalid card number: %s", argv[1]) ;
	}
	if(cardnum > 15)
	{
		goto badcardnum ;
	}
	findpcicard(cardnum) ;
}

/*---------------------------------------------------------------------------*/

/* Function: saybits24
   Purpose: Display the specified 32-bit number as bits.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Saturday December 11, 2004 -- 01:21:11.
*/

static void saybits24(udword bits, char val0, char val1)

{
	udword m ;


	for(m = 0x00800000UL ; m != 0 ; m >>= 1)
	{
		putchar((bits & m) ? val1 : val0) ;
		if(m & 0x00010100UL)
		{
			putchar('.') ;
		}
	}
}

/*---------------------------------------------------------------------------*/

/* Function: doio
   Purpose: Perform the I/O operation.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Saturday December 11, 2004 -- 01:21:11.
*/

static void doio(void)

{
	unsigned p = (PortIndex * sizeof(udword)) ;


	if(SetDirection)
	{
		printf("\nSetting bits %u...%u I/O direction to: ", (PortIndex * 24), ((PortIndex * 24) + 23)) ;
		saybits24(DirectionMask, 'O', 'i') ; putchar('\n') ;
		DirectionMask = ~DirectionMask ; /* (Switch from user to hardware representation.) */
		dataout32((p + PBASE_DIRECTION), DirectionMask) ;
	}

	if(DoOutput)
	{
		dataout32((p + PBASE_IO), OVal) ;
		printf("\nPort 0x%04X <-- ", (uword)(P_FPGAIOBase + p)) ; saybits24(OVal, '0', '1') ;
	}
	else
	{
		OVal = datain32(p + PBASE_IO) ;
		printf("\nPort 0x%04X bits == ", (uword)(P_FPGAIOBase + p)) ; saybits24(OVal, '0', '1') ;
	}
}

/*---------------------------------------------------------------------------*/

/* function: main
   Purpose: Program entry point.
   Used by: Program launcher.
   Returns: Exit status.
   Notes:
   Revision history:
	 1) Saturday December 11, 2004 -- 01:21:11.
*/

int main(int argc, char *argv[])

{
	signon() ;
	pcl(argc, argv) ;
	doio() ;
	printf("\n\nFunction complete.\n") ;

	return EC_OK ;
}

/*---------------------------------------------------------------------------*/

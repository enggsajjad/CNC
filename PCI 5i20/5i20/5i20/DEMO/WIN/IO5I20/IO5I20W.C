/*---------------------------------------------------------------------------*/
/*
   io5i20w.c

   Simple parallel I/O demonstration program for the Mesa Electronics 5I20
	 I/O card/Windoze.  Uses IOPR24.

   Version 2.0, Saturday December 11, 2004 -- 01:13:43.
*/
/*---------------------------------------------------------------------------*/
/*
   Compiler: Microsoft Visual C++ 6.0.

   Tabs: 4
*/
/*---------------------------------------------------------------------------*/
/*
   Revision history.

   1) Version 2.0, Saturday December 11, 2004 -- 01:13:43.

	  Code frozen for version 1.0.
*/
/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <time.h>
#include "PlxApi.h" /* (From the PLX SDK.) */

/*---------------------------------------------------------------------------*/

/* Assorted #defines.
*/
#define PROGNAME      "IO5I20W"
#define SIGNONMESSAGE "\n"PROGNAME" version "PROGVERSION"\n"
#define PROGVERSION   "2.0"

/* PCI base register indeces.
*/
#define BRNUM_5I20IO32 3 /* 32-bit I/O window to FPGA. (Base address register 3.) */

/* PCI registers.
*/
#define pcibasereg2offset(r) (0x10 + ((r) * 4))

#define R_5I20PCICFGIO32 pcibasereg2offset(BRNUM_5I20IO32) /* 32-bit I/O window to FPGA. */

/* Port offsets for IOPR24.
*/
#define PBASE_IO        0  /* Port offset to I/O block. */
#define PBASE_DIRECTION 16 /* Port offset to I/O data direction control block. */

/* Card I.D.
*/
#define VENDORID5I20 0x10B5 /* PCI vendor I.D. */
#define DEVICEID5I20 0x9030 /* PCI device I.D. */

/* Exit codes.
*/
#define EC_OK     0   /* Exit OK. */
#define EC_BADCL  100 /* Bad command line. */
#define EC_USER   101 /* Invalid input. */
#define EC_PLXAPI 105 /* Some sort of error from the PLX API functions. */

/*---------------------------------------------------------------------------*/

/* Assorted typedefs.
*/
typedef U16 unsigned16, uword ;
typedef U32 unsigned32, udword ;

typedef RETURN_CODE     plxapiretcode ;
typedef DEVICE_LOCATION plxdevloc ;

/*---------------------------------------------------------------------------*/

/* Local external variables.
*/
static HANDLE PCIDevHandle ;

static udword OVal, DirectionMask ;

static int DoOutput /* = !!0 */, SetDirection /* = !!0 */ ;
static int Opened /* = !!0 */ ;

static unsigned PortIndex ;

static uword P_FPGAIOBase ;

static const char ProgNameMsg[] = { PROGNAME } ;

/*---------------------------------------------------------------------------*/

/* Function: cleanup
   Purpose: Release external resources.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
     -Failure to call this function before program exit will cause system
		resource leaks.
   Revision history:
	 1) Saturday December 11, 2004 -- 01:13:43.
*/

static void cleanup(void)

{
	if(Opened)
	{
		PlxPciDeviceClose(PCIDevHandle) ;
	}
}

/*---------------------------------------------------------------------------*/

/* Function: fatalerror
   Purpose: Display a fatal error message and exit.
   Used by: Local functions.
   Returns: Never.
   Notes:
   Revision history:
	 1) Saturday December 11, 2004 -- 01:13:43.
*/

static void fatalerror(int exitcode, const char *fmt, ...)

{
	va_list vp ;


	cleanup() ; /* Maybe free up allocated system resources. */

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
	 1) Saturday December 11, 2004 -- 01:13:43.
*/

static void dataout32(unsigned p, udword d)

{
	if(PlxIoPortWrite(PCIDevHandle, (P_FPGAIOBase + p), BitSize32, &d) != ApiSuccess)
	{
		fatalerror(EC_PLXAPI, "Can't write to 5I20 ports.\n") ;
	}
}

/*---------------------------------------------------------------------------*/

/* Function: datain32
   Purpose: Read a doubleword from the specified port.
   Used by: Local functions.
   Returns: The doubleword.
   Notes:
   Revision history:
	 1) Saturday December 11, 2004 -- 01:13:43.
*/

static udword datain32(unsigned p)

{
	udword d ;


	if(PlxIoPortRead(PCIDevHandle, (P_FPGAIOBase + p), BitSize32, &d) != ApiSuccess)
	{
		fatalerror(EC_PLXAPI, "Can't read from 5I20 ports.\n") ;
	}

	return d ;
}

/*---------------------------------------------------------------------------*/

/* Function: findpcicard
   Purpose: Try to find the specified PCI card.  If found set parameters.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Saturday December 11, 2004 -- 01:13:43.
*/

static void findpcicard(unsigned cardnum)

{
	udword p ;

	unsigned32 devicenum ;

	plxapiretcode rcode ;

	plxdevloc devicelocinfo ;


	/* Find the card.
	*/
	(devicelocinfo . BusNumber) = -1 ;
	(devicelocinfo . SlotNumber) = -1 ;
	(devicelocinfo . VendorId) = VENDORID5I20 ;
	(devicelocinfo . DeviceId) = DEVICEID5I20 ;
	(devicelocinfo . SerialNumber[0]) = '\0' ;
	devicenum = (unsigned32)cardnum ;
	if(PlxPciDeviceFind(&devicelocinfo, &devicenum) != ApiSuccess)
	{
		fatalerror(EC_USER, "Can't find 5I20 card number %u.\n", cardnum) ;
	}
	if(PlxPciDeviceOpen(&devicelocinfo, &PCIDevHandle) != ApiSuccess)
	{
		fatalerror(EC_PLXAPI, "Can't gain access to 5I20 card number %u.\n", cardnum) ;
	}
	Opened = !0 ; /* The API documentation doesn't say that 0 will never be returned in PCIDevHandle, so we use a flag. */
	printf("\nUsing PCI device at bus %u/device %u.", (devicelocinfo . BusNumber), (devicelocinfo . SlotNumber)) ;

	/* Get our I/O base address.
	*/
	p = PlxPciConfigRegisterRead((devicelocinfo . BusNumber), (devicelocinfo . SlotNumber), R_5I20PCICFGIO32, &rcode) ;
	if(rcode != ApiSuccess)
	{
		fatalerror(EC_PLXAPI, "Can't read 5I20 card number %u I/O base address.\n", cardnum) ;
	}
	P_FPGAIOBase = (uword)(p & ~0x3) ;
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
	 1) Saturday December 11, 2004 -- 01:13:43.
*/

static void signon(void)

{
	printf("%s", SIGNONMESSAGE) ;
	printf("Mesa Electronics 5I20 simple I/O demo. for Windows.\n") ;
}

/*---------------------------------------------------------------------------*/

/* Function: pcl
   Purpose: Collect parameters from the command line.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Saturday December 11, 2004 -- 01:13:43.
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
	 1) Saturday December 11, 2004 -- 01:13:43.
*/

static void saybits24(udword bits, char val0, char val1)

{
	udword m ;


	for(m = 0x00800000 ; m != 0 ; m >>= 1)
	{
		putchar((bits & m) ? val1 : val0) ;
		if(m & 0x00010100)
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
	 1) Saturday December 11, 2004 -- 01:13:43.
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
	 1) Saturday December 11, 2004 -- 01:13:43.
*/

int main(int argc, char *argv[])

{
	signon() ;
	pcl(argc, argv) ;
	doio() ;
	cleanup() ;
	printf("\n\nFunction complete.\n") ;

	return EC_OK ;
}

/*---------------------------------------------------------------------------*/

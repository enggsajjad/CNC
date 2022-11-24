/*---------------------------------------------------------------------------*/
/*
   sc9030w.c

   Demonstration program for configuration of the Mesa Electronics 9030 I/O
	 card/Windoze.

   Version 1.0, Thursday December 9, 2004 -- 13:40:27.
*/
/*---------------------------------------------------------------------------*/
/*
   Compiler: Microsoft Visual C++ 6.0.

   Tabs: 4
*/
/*---------------------------------------------------------------------------*/
/*
   Revision history.

   1) Version 1.0, Thursday December 9, 2004 -- 13:40:27.

	  Code frozen for version 1.0.
*/
/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <time.h>
#include "PlxApi.h" /* (From the PLX SDK.) */

/*---------------------------------------------------------------------------*/

/* Assorted #defines.
*/
#define PROGNAME      "SC9030W"
#define SIGNONMESSAGE "\n"PROGNAME" version "PROGVERSION"\n"
#define PROGVERSION   "1.0"

/* I/O register indices.
*/
#define R_9030CONTROLNSTATUS 0x0054 /* 9030 32-bit control/status register. */

/* PCI base register indeces.
*/
#define BRNUM_9030LCLCFGIO 1 /* Local configuration I/O base address register number. */
#define BRNUM_9030IO16     2 /* 16-bit I/O window to FPGA. (Base address register 2.) */
#define BRNUM_9030IO32     3 /* 32-bit I/O window to FPGA. (Base address register 3.) */
#define BRNUM_9030CFGMEM   4 /* Configuration memory window. */

/* PCI registers.
*/
#define pcibasereg2offset(r) (0x10 + ((r) * 4))

#define R_9030PCILCLCFGIO pcibasereg2offset(BRNUM_9030LCLCFGIO) /* Local configuration registers. */
#define R_9030PCICFGIO16  pcibasereg2offset(BRNUM_9030IO16)     /* 16-bit I/O window to FPGA. */
#define R_9030PCICFGIO32  pcibasereg2offset(BRNUM_9030IO32)     /* 32-bit I/O window to FPGA. */
#define R_9030PCICFGMEM   pcibasereg2offset(BRNUM_9030CFGMEM)   /* Configuration memory/32-bit memory window to FPGA. */

/* Masks for R_9030CONTROLNSTATUS.
*/
#define P_9030CFGPRGM         P_FPGAIOControlnStatus32
#define B_9030CFGPRGM         26 /* Program enable. (Output.) */
#define M_9030CFGPRGM         (0x1 << B_9030CFGPRGM)
#define M_9030CFGPRGMDEASSERT (0x0 << B_9030CFGPRGM) /* Reset the FPGA. */
#define M_9030CFGPRGMASSERT   (0x1 << B_9030CFGPRGM) /* Begin programming. */

#define P_9030CFGRW           P_FPGAIOControlnStatus32
#define B_9030CFGRW           23 /* Data direction control. (Output.) */
#define M_9030CFGRW           (0x1 << B_9030CFGRW)
#define M_9030CFGWRITEENABLE  (0x0 << B_9030CFGRW) /* From CPU. */
#define M_9030CFGWRITEDISABLE (0x1 << B_9030CFGRW) /* To CPU. */

#define P_9030LEDONOFF        P_FPGAIOControlnStatus32
#define B_9030LEDONOFF        17 /* Red LED control. (Output.) */
#define M_9030LEDONOFF        (0x1 << B_9030LEDONOFF)
#define M_9030LEDON           (0x0 << B_9030LEDONOFF)
#define M_9030LEDOFF          (0x1 << B_9030LEDONOFF)

#define P_9030PRGMDUN         P_FPGAIOControlnStatus32
#define B_9030PRGMDUN         11 /* Programming-done flag. (Input.) */
#define M_9030PRGMDUN         (0x1 << B_9030PRGMDUN)
#define M_9030PRGMDUNNOW      (0x1 << B_9030PRGMDUN)

/* Card I.D.
*/
#define VENDORID9030 0x10B5 /* PCI vendor I.D. */
#define DEVICEID9030 0x9030 /* PCI device I.D. */

/* Exit codes.
*/
#define EC_OK     0   /* Exit OK. */
#define EC_BADCL  100 /* Bad command line. */
#define EC_USER   101 /* Invalid input. */
#define EC_HDW    102 /* Some sort of hardware failure on the 9030. */
#define EC_FILE   103 /* File error of some sort. */
#define EC_SYS    104 /* Beyond our scope. */
#define EC_PLXAPI 105 /* Some sort of error from the PLX API functions. */

/* Other constants.
*/
#define DOWNLOAD2MEMORY /* Download using mapped memory. Uncomment for fast download. */

/*---------------------------------------------------------------------------*/

/* Assorted typedefs.
*/
typedef U8  unsigned8,  ubyte ;
typedef U16 unsigned16, uword ;
typedef U32 unsigned32, udword ;

typedef RETURN_CODE     plxapiretcode ;
typedef PLX_DEVICE_KEY plxdevloc ;

/*---------------------------------------------------------------------------*/

/* Local external variables.
*/
static void (*DataOutFuncPtr)(ubyte thebyte) ;

static signed long ImageLen ; /* Number of bytes in device image portion of file. */

static int Opened /* = !!0 */ ;
#if defined DOWNLOAD2MEMORY
static int Mapped /* = !!0 */ ;
#endif

static FILE *ConfigFile ;

#if defined DOWNLOAD2MEMORY
static ubyte *A_FPGAData ;
#else
static udword P_FPGAData ;
#endif

static PLX_DEVICE_OBJECT PCIDevHandle ;

static udword P_FPGAIOControlnStatus32 ;

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
     1) Thursday December 9, 2004 -- 13:40:27.
*/

static void cleanup(void)

{
#if defined DOWNLOAD2MEMORY
	if(Mapped)
	{
		PlxPci_PciBarUnmap(&PCIDevHandle, &A_FPGAData) ;
	}
#endif
	if(Opened)
	{
		PlxPci_DeviceClose(&PCIDevHandle) ;
	}
}

/*---------------------------------------------------------------------------*/

/* Function: fatalerror
   Purpose: Display a fatal error message and exit.
   Used by: Local functions.
   Returns: Never.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
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

/* Function: ledon
   Purpose: Turn on the 9030's programming LED.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

static void ledon(void)

{
	udword r ;


	if(PlxPci_IoPortRead(&PCIDevHandle, P_9030LEDONOFF, &r, 4, BitSize32) != ApiSuccess)
	{
		noaccess:
		fatalerror(EC_PLXAPI, "Can't access 9030 card control register.\n") ;
	}
	r = ((r & ~M_9030LEDONOFF) | M_9030LEDON) ;
	if(PlxPci_IoPortWrite(&PCIDevHandle, P_9030LEDONOFF, &r, 4, BitSize32) != ApiSuccess)
	{
		goto noaccess ;
	}
}

/*---------------------------------------------------------------------------*/

/* Function: ledoff
   Purpose: Turn off the 9030's programming LED.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

static void ledoff(void)

{
	udword r ;


	if(PlxPci_IoPortRead(&PCIDevHandle, P_9030LEDONOFF, &r, 4, BitSize32) != ApiSuccess)
	{
		noaccess:
		fatalerror(EC_PLXAPI, "Can't access 9030 card control register.\n") ;
	}
	r = ((r & ~M_9030LEDONOFF) | M_9030LEDOFF) ;
	if(PlxPci_IoPortWrite(&PCIDevHandle, P_9030LEDONOFF, &r, 4, BitSize32) != ApiSuccess)
	{
		goto noaccess ;
	}
}

/*---------------------------------------------------------------------------*/

/* Function: writeenable
   Purpose: Enable configuration data transfer from the CPU to the FPGA.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

static void writeenable(void)

{
	udword r ;


	if(PlxPci_IoPortRead(&PCIDevHandle, P_9030CFGRW, &r, 4, BitSize32) != ApiSuccess)
	{
		noaccess:
		fatalerror(EC_PLXAPI, "Can't access 9030 card control register.\n") ;
	}
	r = ((r & ~M_9030CFGRW) | M_9030CFGWRITEENABLE) ;
	if(PlxPci_IoPortWrite(&PCIDevHandle, P_9030CFGRW, &r, 4, BitSize32) != ApiSuccess)
	{
		goto noaccess ;
	}
	ledon() ;
}

/*---------------------------------------------------------------------------*/

/* Function: writedisable
   Purpose: Disable configuration data transfer from the CPU to the FPGA.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

static void writedisable(void)

{
	udword r ;


	if(PlxPci_IoPortRead(&PCIDevHandle, P_9030CFGRW, &r, 4, BitSize32) != ApiSuccess)
	{
		noaccess:
		fatalerror(EC_PLXAPI, "Can't access 9030 card control register.\n") ;
	}
	r = ((r & ~M_9030CFGRW) | M_9030CFGWRITEDISABLE) ;
	if(PlxPci_IoPortWrite(&PCIDevHandle, P_9030CFGRW, &r, 4, BitSize32) != ApiSuccess)
	{
		goto noaccess ;
	}
	ledoff() ;
}

/*---------------------------------------------------------------------------*/

/* Function: programenable
   Purpose: Enable FPGA programming.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

static void programenable(void)

{
	udword r ;


	writeenable() ;

	if(PlxPci_IoPortRead(&PCIDevHandle, P_9030CFGPRGM, &r, 4, BitSize32) != ApiSuccess)
	{
		noaccess:
		fatalerror(EC_PLXAPI, "Can't access 9030 card control register.\n") ;
	}
	r = ((r & ~M_9030CFGPRGM) | M_9030CFGPRGMASSERT) ;
	if(PlxPci_IoPortWrite(&PCIDevHandle, P_9030CFGPRGM, &r, 4, BitSize32) != ApiSuccess)
	{
		goto noaccess ;
	}
}

/*---------------------------------------------------------------------------*/

/* Function: programdisable
   Purpose: Disable FPGA programming/reset the FPGA.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

static void programdisable(void)

{
	udword r ;


	if(PlxPci_IoPortRead(&PCIDevHandle, P_9030CFGPRGM, &r, 4, BitSize32) != ApiSuccess)
	{
		noaccess:
		fatalerror(EC_PLXAPI, "Can't access 9030 card control register.\n") ;
	}
	r = ((r & ~M_9030CFGPRGM) | M_9030CFGPRGMDEASSERT) ;
	if(PlxPci_IoPortWrite(&PCIDevHandle, P_9030CFGPRGM, &r, 4, BitSize32) != ApiSuccess)
	{
		goto noaccess ;
	}
	writedisable() ;
}

/*---------------------------------------------------------------------------*/

/* Function: writedata8
   Purpose: Send a byte to the FPGA data port.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

static void writedata8(ubyte b)

{
#if defined DOWNLOAD2MEMORY
	*A_FPGAData = b ;
#else
	if(PlxPci_IoPortWrite(PCIDevHandle, P_FPGAData, &b, 4, BitSize32) != ApiSuccess)
	{
		fatalerror(EC_PLXAPI, "Can't access 9030 card data register.\n") ;
	}
#endif
}

/*---------------------------------------------------------------------------*/

/* Function: outdata8wswap
   Purpose: Send a byte to the FPGA data port with bits swapped.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

static void outdata8wswap(ubyte thebyte)

{
	static const ubyte swaptab[256] =
	{
		0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
		0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
		0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
		0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
		0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
		0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
		0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
		0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
		0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
		0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
		0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
		0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
		0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
		0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
		0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
		0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
	} ;


	writedata8(swaptab[thebyte]) ;
}

/*---------------------------------------------------------------------------*/

/* Function: outdata8noswap
   Purpose: Send a byte to the FPGA data port with no bit swapping.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

static void outdata8noswap(ubyte thebyte)

{
	writedata8(thebyte) ;
}

/*---------------------------------------------------------------------------*/

/* Function: findpcicard
   Purpose: Try to find the specified PCI card.  If found set parameters.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

static void findpcicard(unsigned cardnum)

{
	udword p ;

	unsigned8 devicenum ;

	plxapiretcode rcode ;

	plxdevloc devicelocinfo ;


	/* Find the card.
	*/
	/* Clear key structure to find first device */
	memset(&devicelocinfo, PCI_FIELD_IGNORE, sizeof(PLX_DEVICE_KEY));
	(devicelocinfo . VendorId) = VENDORID9030 ;
	(devicelocinfo . DeviceId) = DEVICEID9030 ;
	(devicelocinfo . function) = 0 ;
	/*(devicelocinfo . SerialNumber[0]) = '\0' ;*/
	devicenum = (unsigned8)cardnum ;
	if(PlxPci_DeviceFind(&devicelocinfo, devicenum) != ApiSuccess)
	{
		fatalerror(EC_USER, "Can't find 9030 card number %u.\n", cardnum) ;
	}
	if(PlxPci_DeviceOpen(&devicelocinfo, &PCIDevHandle) != ApiSuccess)
	{
		fatalerror(EC_PLXAPI, "Can't gain access to 9030 card number %u.\n", cardnum) ;
	}
	Opened = !0 ; /* The API documentation doesn't say that 0 will never be returned in PCIDevHandle, so we use a flag. */
	printf("\nUsing PCI device at bus %u/device %u.", (devicelocinfo . bus), (devicelocinfo . slot)) ;
	fflush(stdout) ;
	
		switch (devicelocinfo.SubDeviceId)
		{
		case 0x3131: printf("\nFound Mesa Electronics 5I20 card"); 
			break;
		case 0x3132: printf("\nFound Mesa Electronics 4I65 card"); 
			break;
		default: printf("\nUnknown PLX PCI9030 based card!");
		}
	/* Get the card's configuration I/O addresses.
	*/
	p = PlxPci_PciRegisterRead((devicelocinfo . bus), (devicelocinfo . slot), (devicelocinfo . function), R_9030PCILCLCFGIO, &rcode) ;
	if(rcode != ApiSuccess)
	{
#if !defined DOWNLOAD2MEMORY /* (Frivolous conditional compilation supresses annoying warning message.) */
		nocfgaccess:
#endif
		fatalerror(EC_PLXAPI, "Can't read 9030 card number %u configuration registers.\n", cardnum) ;
	}

	P_FPGAIOControlnStatus32 = (p & ~0x3) ; /* FPGA base port. (We always assume that this base register maps I/O.) */
	printf("\n\nPLX 9030 configuration I/O base port: 0x%04lX", P_FPGAIOControlnStatus32) ;
	P_FPGAIOControlnStatus32 += R_9030CONTROLNSTATUS ;
	printf("\nPLX 9030 control/status port:         0x%04lX", P_FPGAIOControlnStatus32) ;

#if defined DOWNLOAD2MEMORY
	rcode = PlxPci_PciBarMap(&PCIDevHandle, BRNUM_9030CFGMEM, &A_FPGAData) ;
	if(rcode != ApiSuccess)
	{
		fatalerror(EC_PLXAPI, "Can't map 9030 card number %u base registers.\n", cardnum) ;
	}
	Mapped = !0 ;
	if((A_FPGAData == (ubyte *)-1) || (A_FPGAData == 0)) /* PLX example code performs this check, so we do too. */
    {
		fatalerror(EC_PLXAPI, "Invalid 9030 card number %u base register mapping.\n", cardnum) ;
    }
	/* From this point on, cleanup() *must* be called before we exit in order
		 to prevent system resource leaks.
	*/
	printf("\nPLX 9030 memory base:                 0x%p", (void *)A_FPGAData) ;
#else
	p = PlxPci_PciRegisterRead((devicelocinfo . bus), (devicelocinfo . slot), (devicelocinfo . function), R_9030PCICFGIO32, &rcode) ;
	if(rcode != ApiSuccess)
	{
		goto nocfgaccess ;
	}
	P_FPGAData = (uword)(p & ~0x3) ;
	printf("\nPLX 9030 32-bit I/O base:             0x%04lX", P_FPGAData) ;
#endif

	putchar('\n') ;
}

/*---------------------------------------------------------------------------*/

/* Function: sendcompletionclocks
   Purpose: Send completion clocks to finish up programming.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

static void sendcompletionclocks(void)

{
	unsigned count ;


	writedisable() ;

	/* Send configuration completion clocks.  (6 should be enough, but we send
		 lots for good measure.)
	*/
	for(count = 24 ; count != 0 ; --count)
	{
		writedata8(-1) ;
	}
}

/*---------------------------------------------------------------------------*/

/* Function: programdunq
   Purpose: Return whether or not the FPGA is done programming.
   Used by: Local functions.
   Returns: Yes/no status.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

static int programdunq(void)

{
	udword r ;


	if(PlxPci_IoPortRead(&PCIDevHandle, P_9030CFGPRGM, &r, 4, BitSize32) != ApiSuccess)
	{
		fatalerror(EC_PLXAPI, "Can't access 9030 card status register.\n") ;
	}
	return (r & M_9030PRGMDUN) == M_9030PRGMDUNNOW ;
}

/*---------------------------------------------------------------------------*/

/* Function: wait4dun
   Purpose: Wait for the FPGA to finish programming.
   Used by: Local functions.
   Returns: Done/timeout status.
   Notes:
	 -Failure to become DONE may be caused by an attempt to load an invalid
		FPGA code image, or an I/O address conflict.
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

static signed wait4dun(void)

{
	clock_t then ;

	unsigned waitcount ;


	/* Wait at least long enough for the FPGA to finish its post-programming
		 tomfoolery.  This time is generally masked by software overhead, but the
		 programmer needs to be aware of it.
	*/
	then = (clock() + ((CLOCKS_PER_SEC / 10000) + 1)) ; /* 100 uS. should be enough. */
	while(clock() < then)
	{
		;
	}

	/* The DONE bit should now indicate end of programming.
	*/
	if(programdunq())
	{	/* Programming seems to be complete; make sure the state sticks and we're not
			 seeing some sort of I/O conflict-related foo.
		*/
		for(waitcount = 100 ; waitcount != 0 ; --waitcount)
		{
			if(!programdunq())
			{
				return -1 ; /* (Shouldn't.) */
			}
		}

		return 0 ; /* Success. */
	}
	else
	{
		return -1 ; /* Failure -- didn't become DONE. */
	}
}

/*---------------------------------------------------------------------------*/

/* Function: delay100us
   Purpose: Wait for 100 uS.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

static void delay100us(void)

{
	clock_t then ;

	then = (clock() + ((CLOCKS_PER_SEC / 10000) + 1)) ;
	while(clock() < then)
	{
		;
	}
}

/*---------------------------------------------------------------------------*/

/* Function: filelengthq
   Purpose: Determine the length of the specified file.
   Used by: Local functions.
   Returns: The file length, or -1 on failure.
   Notes:
	 -The file's location pointer is preserved.
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

static long filelengthq(FILE *f)

{
	long curpos, len ;


	curpos = ftell(f) ;
	if(curpos < 0)
	{
		return -1 ;
	}
	if(fseek(f, 0, SEEK_END) != 0)
	{
		return -1 ;
	}
	len = ftell(f) ;
	if(len < 0)
	{
		return -1 ;
	}
	if(fseek(f, curpos, SEEK_SET) != 0)
	{
		return -1 ;
	}
	return len ;
}

/*---------------------------------------------------------------------------*/

/* Function: saybytesfromfile
   Purpose: Send the bytes from the specified offset in the specified open
     file to standard out.
   Used by: Local functions.
   Returns: The number of bytes read.
   Notes:
     -It is assumed that numbytes includes the trailing '\0'.
     -The file position is not restored to its original value.
     -Don't try to output 0 bytes.
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

static size_t saybytesfromfile(FILE *f, long fromoffset, size_t numbytes)

{
	int c ;

	size_t len = 0 ;


	if(fseek(f, fromoffset, SEEK_SET) != 0)
	{
		return 0 ;
	}
	while(numbytes-- != 0)
	{
		if((c = getc(f)) == EOF)
		{
			break ;
		}
		++len ;
		if(c == '\0')
		{
			break ;
		}
		putchar(c) ;
	}
	return len ;
}

/*---------------------------------------------------------------------------*/

/* Function: readbytesfromfile
   Purpose: Read the specified number of bytes from the specified offset in
	 the specified open file.
   Used by: Local functions.
   Returns: The number of bytes read.
   Notes:
     -The file position is not restored to its original value.
     -Don't try to read 0 bytes.
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

static size_t readbytesfromfile(FILE *f, long fromoffset, size_t numbytes, ubyte *bufptr)

{
	if(fseek(f, fromoffset, SEEK_SET) != 0)
	{
		return 0 ;
	}
	return fread(bufptr, 1, numbytes, f) ;
}

/*---------------------------------------------------------------------------*/

/* Function: sayfileinfo
   Purpose: Display some bytes from our configuration file on standard out.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
     -It is assumed that numbytes includes the trailing '\0'.
     -The file position is not restored to its original value.
     -Don't try to output 0 bytes.
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

static void sayfileinfo(signed long *infobaseptr)

{
	size_t len ;

	ubyte b[2] ;


	if(readbytesfromfile(ConfigFile, *infobaseptr, sizeof(b), b) != sizeof(b))
	{
		ioerror:
		fatalerror(EC_FILE, "File I/O error.  Unexpected end of file?") ;
	}
	len = ((b[0] << 8) | b[1]) ; /* (Bigendian.) */
	if(saybytesfromfile(ConfigFile, (*infobaseptr + sizeof(b)), len) != len)
	{
		goto ioerror ;
	}
	*infobaseptr += (sizeof(b) + (signed long)len + 1) ; /* Skip over length field, text field, and tag. */
}

/*---------------------------------------------------------------------------*/

/* Function: selectfiletype
   Purpose: Set up for processing the input file.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

static void selectfiletype(void)

{
	ubyte b[14] ;


	/* Read in the interesting parts of the file header.
	*/
	if(readbytesfromfile(ConfigFile, 0, sizeof(b), b) != sizeof(b))
	{
		ioerror:
		fatalerror(EC_FILE, "File I/O error.  Unexpected end of file?") ;
	}

	/* Figure out what kind of file we have.
	*/
	if((b[0] == 0x00) && (b[1] == 0x09) && (b[11] == 0x00) && (b[12] == 0x01) && (b[13] == 'a'))
	{	/* Looks like a .BIT file.
		*/
		signed long base ;


		printf("\nLooks like a .BIT file:") ;
		base = 14 ; /* Offset of design name length field. */

		/* Display file particulars.
		*/
		printf("\n  Design name:          ") ; sayfileinfo(&base) ;
		printf("\n  Part I.D.:            ") ; sayfileinfo(&base) ;
		printf("\n  Design date:          ") ; sayfileinfo(&base) ;
		printf("\n  Design time:          ") ; sayfileinfo(&base) ;

		if(readbytesfromfile(ConfigFile, base, 4, b) != 4)
		{
			goto ioerror ;
		}
		ImageLen = (((unsigned long)b[0] << 24) |
					((unsigned long)b[1] << 16) |
					((unsigned long)b[2] << 8) |
					(unsigned long)b[3]) ;
		printf("\n  Configuration length: %lu bytes", ImageLen) ;

		DataOutFuncPtr = outdata8wswap ; /* We have to swap bits in .BIT files. */

		/* We leave the file position set to the next byte in the file, which should
		     be the first byte of the body of the data image.
		*/
	}
	else if((b[0] == 0xFF) && (b[4] == 0x55) && (b[5] == 0x99) && (b[6] == 0xAA) && (b[7] == 0x66))
	{	/* Looks like a PROM file.
		*/
		printf("\nLooks like a PROM file:") ;
		DataOutFuncPtr = outdata8noswap ; /* No bit swap in PROM files. */

		ImageLen = filelengthq(ConfigFile); ;
		if(ImageLen < 0)
		{
			seekerror:
			fatalerror(EC_SYS, "File seek error.") ;
		}
		printf("\n  Configuration length: %lu bytes", ImageLen) ;

		/* PROM file data starts at offset 0, so we have to back up.
		*/
		if(fseek(ConfigFile, 0, SEEK_SET) != 0)
		{
			goto seekerror ;
		}
	}
	else
	{	/* It isn't something we know about.
		*/
		fatalerror(EC_FILE, "Unknown file type.") ;
	}

	putchar('\n') ;
	fflush(stdout) ; /* (Make sure the user can see all output to date.) */
}

/*---------------------------------------------------------------------------*/

/* Function: signon
   Purpose: Display the signon message.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

static void signon(void)

{
	printf("%s", SIGNONMESSAGE) ;
	printf("Mesa Electronics 9030 configuration utility for Windows.\n") ;
}

/*---------------------------------------------------------------------------*/

/* Function: pcl
   Purpose: Collect parameters from the command line.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Thursday December 9, 2004 -- 13:40:27.
*/

static const char File_Name_Info[] = "\n"
"filename.ext specifies the name of the configuration file to be loaded." ;

static const char CardInfo[] = "\n"
"card is the number of the PCI card.  (For use with multiple 9030 cards.)" ;

static const char PurposeInfo[] = "\n"
"Purpose: 9030 configuration upload." ;


static void pcl(unsigned argc, char *argv[])

{
	unsigned cardnum = 0 ;


	if(argc != 3)
	{
		printf("\n%s program usage:\n%s filename.ext card", ProgNameMsg, ProgNameMsg) ;
		printf("\nWhere:") ;
		printf("%s", File_Name_Info) ;
		printf("%s", CardInfo) ;
		printf("%s", PurposeInfo) ;
		putchar('\n') ;

		exit(EC_BADCL) ;
	}

	if((ConfigFile = fopen(argv[1], "rb")) == 0) /* (Binary mode.) */
	{
		fatalerror(EC_USER, "File not found: %s", argv[1]) ;
	}

	if(sscanf(argv[2], "%u", &cardnum) != 1)
	{
		badcardnum:
		fatalerror(EC_USER, "Invalid card number: %s", argv[2]) ;
	}
	if(cardnum > 15)
	{
		goto badcardnum ;
	}

	findpcicard(cardnum) ;
	selectfiletype() ;
}

/*---------------------------------------------------------------------------*/

/* Function: programfpga
   Purpose: Initialize the FPGA with the contents of file ConfigFile.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Thursday December 9, 2004 -- 13:40:27.
*/

#define FORMATSTRING "%-6lu"

static void programfpga(void)

{
	signed long bytesleft ;
	unsigned long numbytes ;

	size_t count ;

	static ubyte filebuffer[16384] ;
	ubyte *bptr ;


	/* Enable programming.
	*/
	printf("\nProgramming...\n") ;
	fflush(stdout) ;

	programdisable() ; /* Reset the FPGA. */
	if(programdunq())
	{	/* Note that if we see DONE at the start of programming, it's most likely due
			 to an attempt to access the FPGA at the wrong I/O location.
		*/
		fatalerror(EC_HDW, "<DONE> status bit indicates busy at start of programming.") ;
	}
	programenable() ;

	/* Delay for at least 100 uS. to allow the FPGA to finish its reset
		 sequencing.  (In reality, the time taken for the initial file access
		 makes this this delay redundant.)
	*/
	delay100us() ;

	/* Program the card.
	*/
	numbytes = 0 ;
	bytesleft = ImageLen ;
	for( ; ; )
	{	/* Write the file to the FPGA.
		*/
		errno = 0 ;
		count = fread(filebuffer, 1, sizeof(filebuffer), ConfigFile) ;
		if(count == 0)
		{	/* 0 bytes could be end of file or error; must check.
			*/
			if(errno != 0)
			{
				programdisable() ;

				fatalerror(EC_SYS, "File I/O error.") ;
			}
			else
			{
				printf(FORMATSTRING, 0L) ;

				break ; /* Done. */
			}
		}
		else
		{	/* Write the block to the FPGA data port.
			*/
			printf(FORMATSTRING "\r", bytesleft) ;
			fflush(stdout) ;
			numbytes += (unsigned long)count ;
			bytesleft -= (signed long)count ;

			bptr = filebuffer ;
			do
			{
				DataOutFuncPtr(*bptr++) ;
			} while(--count != 0) ;
		}
	}

	fclose(ConfigFile) ;

	/* Wait for completion of programming.
	*/
	if(wait4dun() < 0)
	{
		fatalerror(EC_HDW, "Error: Not <DONE>; programming not completed.") ;
	}

	/* Send configuration completion clocks.
	*/
	sendcompletionclocks() ;

	printf("\nSuccessfully programmed %lu bytes.", numbytes) ;
}

/*---------------------------------------------------------------------------*/

/* Function: main
   Purpose: Program entry point.
   Used by: Program launcher.
   Returns: Exit status.
   Notes:
   Revision history:
	 1) Thursday December 9, 2004 -- 13:40:27.
*/

int main(int argc, char *argv[])

{
	signon() ;
	pcl(argc, argv) ;
	programfpga() ;
	cleanup() ;
	printf("\n\nFunction complete.\n") ;

	return EC_OK ;
}

/*---------------------------------------------------------------------------*/

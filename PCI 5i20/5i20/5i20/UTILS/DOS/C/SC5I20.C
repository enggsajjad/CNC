/*---------------------------------------------------------------------------*/
/*
   sc5i20.c

   Demonstration program for configuration of the Mesa Electronics 5I20 I/O
	 card.

   Version 2.0, Friday December 10, 2004 -- 00:57:45.
*/
/*---------------------------------------------------------------------------*/
/*
   Compiler: Borland C++, version 3.1+.

   Tabs: 4
*/
/*---------------------------------------------------------------------------*/
/*
   Revision history.

   1) Version 2.0, Friday December 10, 2004 -- 00:57:45.

	  Code frozen for version 2.0.
*/
/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <dos.h>

/*---------------------------------------------------------------------------*/

/* Assorted #defines.
*/
#define PROGNAME      "SC5I20"
#define SIGNONMESSAGE "\n"PROGNAME" version "PROGVERSION"\n"
#define PROGVERSION   "2.0"

/* I/O register indices.
*/
#define R_5I20CONTROLNSTATUS 0x0054 /* 5I20 control/status register. */

/* PCI registers.
*/
#define R_5I20PCILCLCFGIO 0x14 /* Local configuration registers. */
#define R_5I20PCICFGIO16  0x18 /* 16-bit I/O window to FPGA. */
#define R_5I20PCICFGIO32  0x1C /* 32-bit I/O window to FPGA. */

/* Masks for R_5I20CONTROLNSTATUS.
*/
#define P_5I20CFGPRGM         P_FPGAIOControlnStatus16MSW
#define B_5I20CFGPRGM         10 /* Program enable. (Output.) */
#define M_5I20CFGPRGM         (0x1 << B_5I20CFGPRGM)
#define M_5I20CFGPRGMDEASSERT (0x0 << B_5I20CFGPRGM) /* Reset the FPGA. */
#define M_5I20CFGPRGMASSERT   (0x1 << B_5I20CFGPRGM) /* Begin programming. */

#define P_5I20CFGRW           P_FPGAIOControlnStatus16MSW
#define B_5I20CFGRW           7 /* Data direction control. (Output.) */
#define M_5I20CFGRW           (0x1 << B_5I20CFGRW)
#define M_5I20CFGWRITEENABLE  (0x0 << B_5I20CFGRW) /* From CPU. */
#define M_5I20CFGWRITEDISABLE (0x1 << B_5I20CFGRW) /* To CPU. */

#define P_5I20LEDONOFF        P_FPGAIOControlnStatus16MSW
#define B_5I20LEDONOFF        1 /* Red LED control. (Output.) */
#define M_5I20LEDONOFF        (0x1 << B_5I20LEDONOFF)
#define M_5I20LEDON           (0x0 << B_5I20LEDONOFF)
#define M_5I20LEDOFF          (0x1 << B_5I20LEDONOFF)

#define P_5I20PRGMDUN         P_FPGAIOControlnStatus16LSW
#define B_5I20PRGMDUN         11 /* Programming-done flag. (Input.) */
#define M_5I20PRGMDUN         (0x1 << B_5I20PRGMDUN)
#define M_5I20PRGMDUNNOW      (0x1 << B_5I20PRGMDUN)

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
#define EC_OK    0   /* Exit OK. */
#define EC_BADCL 100 /* Bad command line. */
#define EC_USER  101 /* Invalid input. */
#define EC_HDW   102 /* Some sort of hardware failure on the 5I20. */
#define EC_FILE  103 /* File error of some sort. */
#define EC_SYS   104 /* Beyond our scope. */

/* Other constants.
*/
#define PROGWAITLOOPCOUNT 20000 /* Wait loop count for programming completion. */

/*---------------------------------------------------------------------------*/

/* typedefs.
*/
typedef unsigned char unsigned8, ubyte ;
typedef unsigned short unsigned16, uword ;

/*---------------------------------------------------------------------------*/

/* Local external variables.
*/
static void (*DataOutFuncPtr)(unsigned thebyte) ;

static signed long ImageLen ; /* Number of bytes in device image portion of file. */

static uword P_FPGAIOControlnStatus16LSW, P_FPGAIOControlnStatus16MSW, P_FPGAData ;

static const char ProgNameMsg[] = { PROGNAME } ;

static unsigned8 PCIBus, PCIDev ;

static FILE *ConfigFile ;

/*---------------------------------------------------------------------------*/

/* Function: fatalerror
   Purpose: Display a fatal error message and exit.
   Used by: Local functions.
   Returns: Never.
   Notes:
   Revision history:
	 1) Friday December 10, 2004 -- 00:57:45.
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

/* Function: ledon
   Purpose: Turn on the 5I20's programming LED.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Friday December 10, 2004 -- 00:57:45.
*/

static void ledon(void)

{
	outport(P_5I20LEDONOFF, (inport(P_5I20LEDONOFF) & ~M_5I20LEDONOFF) | M_5I20LEDON) ;
}

/*---------------------------------------------------------------------------*/

/* Function: ledoff
   Purpose: Turn off the 5I20's programming LED.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Friday December 10, 2004 -- 00:57:45.
*/

static void ledoff(void)

{
	outport(P_5I20LEDONOFF, (inport(P_5I20LEDONOFF) & ~M_5I20LEDONOFF) | M_5I20LEDOFF) ;
}

/*---------------------------------------------------------------------------*/

/* Function: writeenable
   Purpose: Enable configuration data transfer from the CPU to the 5I20.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Friday December 10, 2004 -- 00:57:45.
*/

static void writeenable(void)

{
	outport(P_5I20CFGRW, (inport(P_5I20CFGRW) & ~M_5I20CFGRW) | M_5I20CFGWRITEENABLE) ;
	ledon() ;
}

/*---------------------------------------------------------------------------*/

/* Function: writedisable
   Purpose: Disable configuration data transfer from the CPU to the 5I20.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Friday December 10, 2004 -- 00:57:45.
*/

static void writedisable(void)

{
	outport(P_5I20CFGRW, (inport(P_5I20CFGRW) & ~M_5I20CFGRW) | M_5I20CFGWRITEDISABLE) ;
	ledoff() ;
}

/*---------------------------------------------------------------------------*/

/* Function: programenable
   Purpose: Enable 5I20 programming.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Friday December 10, 2004 -- 00:57:45.
*/

static void programenable(void)

{
	outport(P_5I20CFGPRGM, (inport(P_5I20CFGPRGM) & ~M_5I20CFGPRGM) | M_5I20CFGPRGMASSERT) ;
	writeenable() ;
}

/*---------------------------------------------------------------------------*/

/* Function: programdisable
   Purpose: Disable 5I20 programming/reset the FPGA.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Friday December 10, 2004 -- 00:57:45.
*/

static void programdisable(void)

{
	outport(P_5I20CFGPRGM, (inport(P_5I20CFGPRGM) & ~M_5I20CFGPRGM) | M_5I20CFGPRGMDEASSERT) ;
	writedisable() ;
}

/*---------------------------------------------------------------------------*/

/* Function: outdata8wswap
   Purpose: Send a byte to the 5I20 data port with bits swapped.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Friday December 10, 2004 -- 00:57:45.
*/

static void outdata8wswap(unsigned thebyte)

{
	static const unsigned char swaptab[256] =
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


	outportb(P_FPGAData, swaptab[thebyte]) ;
}

/*---------------------------------------------------------------------------*/

/* Function: outdata8noswap
   Purpose: Send a byte to the 5I20 data port with no bit swapping.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Friday December 10, 2004 -- 00:57:45.
*/

static void outdata8noswap(unsigned thebyte)

{
	outportb(P_FPGAData, thebyte) ;
}

/*---------------------------------------------------------------------------*/

/* Function: havepcibiosq
   Purpose: Determine whether or not we have PCI BIOS services available.
   Used by: Local functions.
   Returns: Non-zero if we have PCI BIOS services functionality.
   Notes:
   Revision history:
	 1) Friday December 10, 2004 -- 00:57:45.
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
	 1) Friday December 10, 2004 -- 00:57:45.
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
	 1) Friday December 10, 2004 -- 00:57:45.
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
	 1) Friday December 10, 2004 -- 00:57:45.
*/

static void findpcicard(unsigned cardnum)

{
	unsigned short portbase ;


	if(!havepcibiosq())
	{
		fatalerror(EC_SYS, "No PCI BIOS services are available on this system.") ;
	}
	if(pcicardsearch(cardnum) < 0)
	{
		fatalerror(EC_USER, "Can't find 5I20 card number %u.\n", cardnum) ;
	}

	printf("\nUsing PCI device at bus %u/device %u.\n", PCIBus, (PCIDev >> 3)) ;

	pcicfgread16(PCIBus, PCIDev, R_5I20PCILCLCFGIO, &P_FPGAIOControlnStatus16LSW) ; /* (We don't bother to check for errors.) */
	P_FPGAIOControlnStatus16LSW &= ~0x3 ; /* 16-bit I/O channel to FPGA chip control register. */
	printf("\nPLX 9030 configuration I/O base port: 0x%04hX", P_FPGAIOControlnStatus16LSW) ;
	P_FPGAIOControlnStatus16LSW += R_5I20CONTROLNSTATUS ;
	P_FPGAIOControlnStatus16MSW = (P_FPGAIOControlnStatus16LSW + sizeof(unsigned short)) ;
	printf("\nPLX 9030 control/status port:         0x%04hX", P_FPGAIOControlnStatus16LSW) ;

	pcicfgread16(PCIBus, PCIDev, R_5I20PCICFGIO16, &P_FPGAData) ;
	P_FPGAData &= ~0x3 ;
	printf("\nPLX 9030 16-bit I/O base:             0x%04hX", P_FPGAData) ;

	pcicfgread16(PCIBus, PCIDev, R_5I20PCICFGIO32, &portbase) ;
	printf("\nPLX 9030 32-bit I/O base:             0x%04hX", (portbase & ~0x3)) ; /* (Informational -- not used in this program.) */

	putchar('\n') ;
	fflush(stdout) ;
}

/*---------------------------------------------------------------------------*/

/* Function: sendcompletionclocks
   Purpose: Send completion clocks to finish up programming.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Friday December 10, 2004 -- 00:57:45.
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
		outport(P_FPGAData, -1) ;
	}
}

/*---------------------------------------------------------------------------*/

/* Function: programdunq
   Purpose: Return whether or not the FPGA is done programming.
   Used by: Local functions.
   Returns: Yes/no status.
   Notes:
	 -Failure to become DONE may be caused by an attempt to load an invalid
		FPGA code image, or an I/O address conflict.
   Revision history:
	 1) Friday December 10, 2004 -- 00:57:45.
*/

static int programdunq(void)

{
	return (inport(P_5I20PRGMDUN) & M_5I20PRGMDUN) == M_5I20PRGMDUNNOW ;
}

/*---------------------------------------------------------------------------*/

/* Function: wait4dun
   Purpose: Wait for the card to finish programming.
   Used by: Local functions.
   Returns: Success/failure status.
   Notes:
	 -Failure to become DONE may be caused by an attempt to load an invalid
		FPGA code image, or an I/O address conflict.
   Revision history:
	 1) Friday December 10, 2004 -- 00:57:45.
*/

static signed wait4dun(void)

{
	unsigned waitcount ;


	/* Wait for the DONE bit to indicate completion of programming.
	*/
	for(waitcount = PROGWAITLOOPCOUNT ; waitcount != 0 ; --waitcount)
	{
		if(programdunq())
		{	/* Programming seems to be complete; make sure the state sticks and we're not
				 seeing some sort of I/O conflict-related foo.
			*/
			for(waitcount = 100 ; waitcount != 0 ; --waitcount)
			{
				if(!programdunq())
				{
					return -1 ; /* Shouldn't. */
				}
			}

			return 0 ; /* Success. */
		}
	}

	return -1 ; /* Failure -- didn't become DONE. */
}

/*---------------------------------------------------------------------------*/

/* Function: delay100us
   Purpose: Wait for 100 uS.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Friday December 10, 2004 -- 00:57:45.
*/

static void delay100us(void)

{
	unsigned count ;


	for(count = 1000 ; count != 0 ; --count)
	{
		inport(P_5I20PRGMDUN) ;
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
	 1) Friday December 10, 2004 -- 00:57:45.
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
	 1) Friday December 10, 2004 -- 00:57:45.
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
	 1) Friday December 10, 2004 -- 00:57:45.
*/

static size_t readbytesfromfile(FILE *f, long fromoffset, size_t numbytes, unsigned char *bufptr)

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
   Returns: The number of bytes read.
   Notes:
     -It is assumed that numbytes includes the trailing '\0'.
     -The file position is not restored to its original value.
     -Don't try to output 0 bytes.
   Revision history:
	 1) Friday December 10, 2004 -- 00:57:45.
*/

static void sayfileinfo(signed long *infobaseptr)

{
	size_t len ;

	unsigned char b[2] ;


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
	 1) Friday December 10, 2004 -- 00:57:45.
*/

static void selectfiletype(void)

{
	unsigned char b[14] ;


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
	 1) Friday December 10, 2004 -- 00:57:45.
*/

static void signon(void)

{
	printf("%s", SIGNONMESSAGE) ;
	printf("Mesa Electronics 5I20 configuration utility.\n") ;
}

/*---------------------------------------------------------------------------*/

/* Function: pcl
   Purpose: Collect parameters from the command line.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
	 1) Friday December 10, 2004 -- 00:57:45.
*/

static const char FileNameInfo[] = "\n"
"filename.ext specifies the name of the configuration file to be loaded." ;

static const char CardInfo[] = "\n"
"card is the number of the PCI card.  (For use with multiple 5I20 cards.)" ;

static const char PurposeInfo[] = "\n"
"Purpose: 5I20 configuration upload." ;


static void pcl(unsigned argc, char *argv[])

{
	unsigned cardnum = 0 ;


	if(argc != 3)
	{
		printf("\n%s program usage:\n%s filename.ext card", ProgNameMsg, ProgNameMsg) ;
		printf("\nWhere:") ;
		printf("%s", FileNameInfo) ;
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
	 1) Friday December 10, 2004 -- 00:57:45.
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
	 1) Friday December 10, 2004 -- 00:57:45.
*/

int main(int argc, char *argv[])

{
	signon() ;
	pcl(argc, argv) ;
	programfpga() ;
	printf("\n\nFunction complete.\n") ;

	return EC_OK ;
}

/*---------------------------------------------------------------------------*/

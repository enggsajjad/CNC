/*---------------------------------------------------------------------------*/
/*
   rio5i20.c

   Simple general-purpose I/O demonstration program for the Mesa Electronics
	 5I20 I/O card/DOS.

   Version 1.0, Friday December 17, 2004 -- 23:40:52.
*/
/*---------------------------------------------------------------------------*/
/*
   Compiler: Borland C++, version 3.1+.  (Use command line options "-B -3".)

   Tabs: 4
*/
/*---------------------------------------------------------------------------*/
/*
   Revision history.

   1) Version 1.0, Friday December 17, 2004 -- 23:40:52.

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
#define PROGNAME      "RIO5I20"
#define SIGNONMESSAGE "\n"PROGNAME" version "PROGVERSION"\n"
#define PROGVERSION   "1.0"

/* PCI registers.
*/
#define R_5I20PCICFGIO16  0x18 /* 16-bit I/O window to FPGA. (Base address register 2.) */
#define R_5I20PCICFGIO32  0x1C /* 32-bit I/O window to FPGA. (Base address register 3.) */

#define NUMPORTINDEXBYTES 256 /* Number of bytes accessible within the specified I/O window. */

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
static udword IOVal ;

static int DoOutput /* = !!0 */, DoIO32 ;

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
     1) Friday December 17, 2004 -- 23:40:52.
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
   Purpose: Write a doubleword to the specified I/O port.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Friday December 17, 2004 -- 23:40:52.
*/

static void dataout32(uword p, udword d)

{
	asm {
				mov		dx,[p]
				mov		eax,[d]
				out		dx,eax
	}
}

/*---------------------------------------------------------------------------*/

/* Function: dataout16
   Purpose: Write a word to the specified I/O port.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Friday December 17, 2004 -- 23:40:52.
*/

static void dataout16(uword p, uword d)

{
	outport(p, d) ;
}

/*---------------------------------------------------------------------------*/

/* Function: datain32
   Purpose: Read a doubleword from the specified I/O port.
   Used by: Local functions.
   Returns: The doubleword.
   Notes:
   Revision history:
     1) Friday December 17, 2004 -- 23:40:52.
*/

static udword datain32(unsigned p)

{
	udword retval ;


	asm {
				mov		dx,[p]
				in		eax,dx
				mov		[retval],eax
	}
	return retval ;
}

/*---------------------------------------------------------------------------*/

/* Function: datain16
   Purpose: Read a word from the specified I/O port.
   Used by: Local functions.
   Returns: The word.
   Notes:
   Revision history:
     1) Friday December 17, 2004 -- 23:40:52.
*/

static uword datain16(uword p)

{
	return inport(p) ;
}

/*---------------------------------------------------------------------------*/

/* Function: havepcibiosq
   Purpose: Determine whether or not we have PCI BIOS services available.
   Used by: Local functions.
   Returns: Non-zero if we have PCI BIOS services functionality.
   Notes:
   Revision history:
     1) Friday December 17, 2004 -- 23:40:52.
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
     1) Friday December 17, 2004 -- 23:40:52.
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
     1) Friday December 17, 2004 -- 23:40:52.
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
     1) Friday December 17, 2004 -- 23:40:52.
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
	if(DoIO32)
	{
		pcicfgread16(PCIBus, PCIDev, R_5I20PCICFGIO32, &P_FPGAIOBase) ; /* (We don't bother to check for errors.) */
		P_FPGAIOBase &= ~0x3 ;

		printf("\nPLX 9030 32-bit I/O base: 0x%04X", P_FPGAIOBase) ;
	}
	else
	{
		pcicfgread16(PCIBus, PCIDev, R_5I20PCICFGIO16, &P_FPGAIOBase) ;
		P_FPGAIOBase &= ~0x3 ;

		printf("\nPLX 9030 16-bit I/O base: 0x%04X", P_FPGAIOBase) ;
	}

	putchar('\n') ;
}

/*---------------------------------------------------------------------------*/

/* Function: signon
   Purpose: Display the signon message.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Friday December 17, 2004 -- 23:40:52.
*/

static void signon(void)

{
	printf("%s", SIGNONMESSAGE) ;
	printf("Mesa Electronics 5I20 general-purpose I/O access demo. for DOS.\n") ;
}

/*---------------------------------------------------------------------------*/

/* Function: pcl
   Purpose: Collect parameters from the command line.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Friday December 17, 2004 -- 23:40:52.
*/

static const char CardInfo[] = "\n"
"card is the number of the PCI card.  (For use with multiple 5I20 cards.)" ;

static const char WidthInfo[] = "\n"
"width is the data width indicator; w for word (16-bit), d for doubleword\n"
"  (32-bit)." ;

static const char PortInfo[] = "\n"
"port is the byte index of the I/O port within the 16- or 32-bit I/O space." ;

static const char OutValInfo[] = "\n"
"outval is a 16- or 32-bit hexadecimal number to output to 5I20 FPGA.  If this\n"
"  field is supplied, a write will be performed to the specified port.\n"
"  Otherwise a read will be performed." ;

static const char PurposeInfo[] = "\n"
"Purpose: 5I20 general-purpose I/O access demo." ;


static void pcl(unsigned argc, char *argv[])

{
	unsigned cardnum = 0 ;

	char widthchar ;


	if((argc < 4) || (argc > 5))
	{
		printf("\n%s program usage:\n%s card width port[ outval]", ProgNameMsg, ProgNameMsg) ;
		printf("\nWhere:") ;
		printf("%s", CardInfo) ;
		printf("%s", WidthInfo) ;
		printf("%s", PortInfo) ;
		printf("%s", OutValInfo) ;
		printf("%s", PurposeInfo) ;
		putchar('\n') ;

		exit(EC_BADCL) ;
	}

	if(sscanf(argv[2], "%c", &widthchar) != 1)
	{
		badwidth:
		fatalerror(EC_USER, "Invalid data width: %s", argv[2]) ;
	}
	switch(widthchar)
	{
		case 'w':
		case 'W':
			DoIO32 = !!0 ;
			break ;

		case 'd':
		case 'D':
			DoIO32 = !0 ;
			break ;

		default:
			goto badwidth ;
	}

	if(sscanf(argv[3], "%x", &PortIndex) != 1)
	{
		badportindex:
		fatalerror(EC_USER, "Invalid port index: %s", argv[3]) ;
	}
	if(DoIO32)
	{
		if((PortIndex & 0x0003) || (PortIndex > (NUMPORTINDEXBYTES - sizeof(udword))))
		{	/* Must be 32-bit aligned...
			*/
			goto badportindex ;
		}
	}
	else
	{
		if((PortIndex & 0x0001) || (PortIndex > (NUMPORTINDEXBYTES - sizeof(uword))))
		{	/* Must be 16-bit aligned...
			*/
			goto badportindex ;
		}
	}

	if(argc == 5)
	{
		if(sscanf(argv[4], "%lx", &IOVal) != 1)
		{
			fatalerror(EC_USER, "Invalid output value: %s", argv[4]) ;
		}

		DoOutput = !0 ;
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

/* Function: words2bitstring
   Purpose: Convert the specified number of words into '1's and '0's in a
     string.
   Used by: Local functions.
   Returns: A pointer to the string.
   Notes:
   Revision history:
     1) Friday December 17, 2004 -- 23:40:52.
*/

static char *words2bitstring(unsigned numwords, udword thebits)

{
	udword m ;

	unsigned i ;

	static char bitstring[32 + 3 + 1] ;


	if(numwords > 2)
	{
		numwords = 2 ;
	}
	i = 0 ;
	for(m = (0x1UL << ((numwords * 16) - 1)) ; m != 0 ; m >>= 1)
	{
		bitstring[i++] = ((thebits & m) ? '1' : '0') ;
		if(m & 0x01010100UL)
		{
			bitstring[i++] = '.' ;
		}
	}
	bitstring[i] = '\0' ;

	return bitstring ;
}

/*---------------------------------------------------------------------------*/

/* Function: doio
   Purpose: Perform the I/O operation.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Friday December 17, 2004 -- 23:40:52.
*/

static void doio(void)

{
	uword p = (P_FPGAIOBase + PortIndex) ;


	if(DoOutput)
	{
		if(DoIO32)
		{
			dataout32(p, IOVal) ;
			printf("\nPort 0x%04X <-- 0x%08lX (%s)", p, IOVal, words2bitstring(2, IOVal)) ;
		}
		else
		{
			dataout16(p, (uword)IOVal) ;
			printf("\nPort 0x%04X <-- 0x%04X (%s)", p, (uword)IOVal, words2bitstring(1, IOVal)) ;
		}
	}
	else
	{
		if(DoIO32)
		{
			IOVal = datain32(p) ;
			printf("\nPort 0x%04X bits == 0x%08lX (%s)", p, IOVal, words2bitstring(2, IOVal)) ;
		}
		else
		{
			IOVal = (udword)datain16(p) ;
			printf("\nPort 0x%04X bits == 0x%04X (%s)", p, (uword)IOVal, words2bitstring(1, IOVal)) ;
		}
	}
}

/*---------------------------------------------------------------------------*/

/* function: main
   Purpose: Program entry point.
   Used by: Program launcher.
   Returns: Exit status.
   Notes:
   Revision history:
     1) Friday December 17, 2004 -- 23:40:52.
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

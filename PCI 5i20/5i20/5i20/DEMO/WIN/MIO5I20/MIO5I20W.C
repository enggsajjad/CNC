/*---------------------------------------------------------------------------*/
/*
   mio5i20w.c

   Simple general-purpose PCI memory access demonstration program for the
	 Mesa Electronics 5I20 I/O card/DOS.

   Version 1.0, Tuesday December 21, 2004 -- 22:02:16
*/
/*---------------------------------------------------------------------------*/
/*
   Compiler: Microsoft Visual C++ 6.0.

   Tabs: 4
*/
/*---------------------------------------------------------------------------*/
/*
   Revision history.

   1) Version 1.0, Tuesday December 21, 2004 -- 22:02:16

	  Code frozen for version 1.0.
*/
/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <time.h>
#include "PlxApi.h" /* (From the PLX SDK.) */

/*---------------------------------------------------------------------------*/

/* Assorted #defines.
*/
#define PROGNAME      "MIO5I20W"
#define SIGNONMESSAGE "\n"PROGNAME" version "PROGVERSION"\n"
#define PROGVERSION   "1.0"

/* PCI base register indeces, etc.
*/
#define BRNUM_5I20MEM16 4 /* 16-bit memory window to FPGA. (Base address register 4.) */
#define BRNUM_5I20MEM32 5 /* 32-bit memory window to FPGA. (Base address register 5.) */

#define NUMMEMWINDOWBYTES 65536UL /* Number of bytes accessible within the specified memory window. */

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
typedef U8 unsigned8, ubyte ;
typedef U16 unsigned16, uword ;
typedef U32 unsigned32, udword ;

typedef RETURN_CODE     plxapiretcode ;
typedef DEVICE_LOCATION plxdevloc ;

/*---------------------------------------------------------------------------*/

/* Local external variables.
*/
static HANDLE PCIDevHandle ;

static udword IOVal ;
static unsigned long MemoryOffset ;

static int DoOutput /* = !!0 */, Opened /* = !!0 */, Mapped /* = !!0 */, DoIO32 ;

static void *A_PCIMemBase ;

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
     1) Tuesday December 21, 2004 -- 22:02:16.
*/

static void cleanup(void)

{
	if(Mapped)
	{
		PlxPciBarUnmap(PCIDevHandle, A_PCIMemBase) ;
	}
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
     1) Tuesday December 21, 2004 -- 22:02:16.
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
   Purpose: Write a doubleword to the specified memory address.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Tuesday December 21, 2004 -- 22:02:16.
*/

static void dataout32(volatile void *p, udword d)

{
	*(udword *)p = d ;
}

/*---------------------------------------------------------------------------*/

/* Function: dataout16
   Purpose: Write a word to the specified memory address.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Tuesday December 21, 2004 -- 22:02:16.
*/

static void dataout16(volatile void *p, uword d)

{
	*(uword *)p = d ;
}

/*---------------------------------------------------------------------------*/

/* Function: datain32
   Purpose: Read a doubleword from the specified memory address.
   Used by: Local functions.
   Returns: The doubleword.
   Notes:
   Revision history:
     1) Tuesday December 21, 2004 -- 22:02:16.
*/

static udword datain32(volatile void *p)

{
	return *(udword *)p ;
}

/*---------------------------------------------------------------------------*/

/* Function: datain16
   Purpose: Read a word from the specified memory address.
   Used by: Local functions.
   Returns: The word.
   Notes:
   Revision history:
     1) Tuesday December 21, 2004 -- 22:02:16.
*/

static uword datain16(volatile void *p)

{
	return *(uword *)p ;
}

/*---------------------------------------------------------------------------*/

/* Function: findpcicard
   Purpose: Try to find the specified PCI card.  If found set parameters.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Tuesday December 21, 2004 -- 22:02:16.
*/

static void findpcicard(unsigned cardnum)

{
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

	/* Get our mapped memory base address.
	*/
	if(DoIO32)
	{
		rcode = PlxPciBarMap(PCIDevHandle, BRNUM_5I20MEM32, &A_PCIMemBase) ;
		if(rcode != ApiSuccess)
		{
			fatalerror(EC_PLXAPI, "Can't map 5I20 card number %u 32-bit base register.\n", cardnum) ;
		}
		Mapped = !0 ;
		if((A_PCIMemBase == (ubyte *)-1) || (A_PCIMemBase == 0)) /* PLX example code performs this check, so we do too. */
		{
			fatalerror(EC_PLXAPI, "Invalid 5I20 card number %u base register mapping.\n", cardnum) ;
		}
		/* From this point on, cleanup() *must* be called before we exit in order
			 to prevent system resource leaks.
		*/
		printf("\nPLX 9030 32-bit memory base: 0x%p", (void *)A_PCIMemBase) ;
	}
	else
	{
		rcode = PlxPciBarMap(PCIDevHandle, BRNUM_5I20MEM16, &A_PCIMemBase) ;
		if(rcode != ApiSuccess)
		{
			fatalerror(EC_PLXAPI, "Can't map 5I20 card number %u 16-bit base register.\n", cardnum) ;
		}
		Mapped = !0 ;
		if((A_PCIMemBase == (ubyte *)-1) || (A_PCIMemBase == 0)) /* PLX example code performs this check, so we do too. */
		{
			fatalerror(EC_PLXAPI, "Invalid 5I20 card number %u base register mapping.\n", cardnum) ;
		}
		/* From this point on, cleanup() *must* be called before we exit in order
			 to prevent system resource leaks.
		*/
		printf("\nPLX 9030 16-bit memory base: 0x%p", (void *)A_PCIMemBase) ;
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
     1) Tuesday December 21, 2004 -- 22:02:16.
*/

static void signon(void)

{
	printf("%s", SIGNONMESSAGE) ;
	printf("Mesa Electronics 5I20 general-purpose PCI memory access demo. for Windows.\n") ;
}

/*---------------------------------------------------------------------------*/

/* Function: pcl
   Purpose: Collect parameters from the command line.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Tuesday December 21, 2004 -- 22:02:16.
*/

static const char CardInfo[] = "\n"
"card is the number of the PCI card.  (For use with multiple 5I20 cards.)" ;

static const char WidthInfo[] = "\n"
"width is the data width indicator; w for word (16-bit), d for doubleword\n"
"  (32-bit)." ;

static const char OffsetInfo[] = "\n"
"offset is the byte offset within the 16- or 32-bit PCI memory to be written." ;

static const char OutValInfo[] = "\n"
"outval is a 16- or 32-bit hexadecimal number to output to 5I20 FPGA.  If this\n"
"  field is supplied, a write will be performed to the specified memory offset.\n"
"  Otherwise a read will be performed." ;

static const char PurposeInfo[] = "\n"
"Purpose: 5I20 general-purpose PCI memory access demo." ;

static void pcl(unsigned argc, char *argv[])

{
	unsigned cardnum = 0 ;

	char widthchar ;


	if((argc < 4) || (argc > 5))
	{
		printf("\n%s program usage:\n%s card width offset[ outval]", ProgNameMsg, ProgNameMsg) ;
		printf("\nWhere:") ;
		printf("%s", CardInfo) ;
		printf("%s", WidthInfo) ;
		printf("%s", OffsetInfo) ;
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

	if(sscanf(argv[3], "%lx", &MemoryOffset) != 1)
	{
		badoffset:
		fatalerror(EC_USER, "Invalid byte offset: %s", argv[3]) ;
	}
	if(DoIO32)
	{
		if(MemoryOffset > (NUMMEMWINDOWBYTES - sizeof(udword)))
		{
			goto badoffset ;
		}
	}
	else
	{
		if(MemoryOffset > (NUMMEMWINDOWBYTES - sizeof(uword)))
		{
			goto badoffset ;
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
     1) Tuesday December 21, 2004 -- 22:02:16.
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
   Purpose: Perform the memory read or write operation.
   Used by: Local functions.
   Returns: Nothing.
   Notes:
   Revision history:
     1) Tuesday December 21, 2004 -- 22:02:16.
*/

static void doio(void)

{
	void *p = &((ubyte *)A_PCIMemBase)[MemoryOffset] ;


	if(DoOutput)
	{
		if(DoIO32)
		{
			dataout32(p, IOVal) ;
			printf("\n0x%08lX[0x%08lX] (%08lX) <-- 0x%08lX (%s)",
				   A_PCIMemBase, MemoryOffset, p, IOVal, words2bitstring(2, IOVal)) ;
		}
		else
		{
			dataout16(p, (uword)IOVal) ;
			printf("\n0x%08lX[0x%08lX] (%08lX) <-- 0x%04X (%s)",
				   A_PCIMemBase, MemoryOffset, p, (uword)IOVal, words2bitstring(1, IOVal)) ;
		}
	}
	else
	{
		if(DoIO32)
		{
			IOVal = datain32(p) ;
			printf("\n0x%08lX[0x%08lX] (%08lX) == 0x%08lX (%s)",
				   A_PCIMemBase, MemoryOffset, p, IOVal, words2bitstring(2, IOVal)) ;
		}
		else
		{
			IOVal = (udword)datain16(p) ;
			printf("\n0x%08lX[0x%08lX] (%08lX) == 0x%04X (%s)",
				   A_PCIMemBase, MemoryOffset, p, (uword)IOVal, words2bitstring(2, IOVal)) ;
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
     1) Tuesday December 21, 2004 -- 22:02:16.
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

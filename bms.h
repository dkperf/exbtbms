

#define BMS_FET_CHARGING     1
#define BMS_FET_DISCHARGING  2

#define BMS_NAME_LEN 32
#define BMS_MAX_TEMPS 8
#define BMS_MAX_CELLS 16


struct bms_info {
   char name[BMS_NAME_LEN];   
   float voltage;
   float current;             // BMS current flowing in Amps
   float balance;             // Battery current capacity in Ah
   float capacity;            // Battery total capacity in Ah
   unsigned char stateOfCharge;
   int status;         
   int ntemps;                // # of temperature probes on BMS
   float temps[BMS_MAX_TEMPS];   
   int cells;                 //  # cells in battery pack
   float cellvolt[BMS_MAX_CELLS]; 
   float cellres[BMS_MAX_CELLS]; // cell resistance
   unsigned char  fetbits;
   uint16_t protectbits;      // Current BMS protection status
   uint32_t balancebits;      // Balance bitmask
};

typedef struct bms_info bms_info_t;


#define _getshort(p) ((short) ((*((p)) << 8) | *((p)+1) ))


/*
   new routine that MUST be added to bottom of btlib.c
   if you use the stock btlib.c

//--------------------------------------------------
int setCallback( int node, int cticn, int (*callback)() )
{
   struct cticdata *cp;  // characteristic info structure
   int ndevice;

   if (( ndevice = devnp(node)) < 0 )
      return(0);

   if ( cp = ctic( ndevice, cticn ))
      cp->callback = callback;

   return 1;
}
*/
int setCallback( int node, int cticn, int (*callback)() );





/*
 
   code available but not used by bms.c

   If you want to get rid of the devices.txt file you can
   add below device information programmatically in your program.

DEVICE = JBD_BMS   TYPE=LE    NODE=9  ADDRESS=A4:C1:38:55:2A:CA
  LECHAR = Name     PERMIT=12 SIZE=22  HANDLE=0003 UUID=2A00  ; index 0
  LECHAR = FF01     PERMIT=12 SIZE=32  HANDLE=0011            ; index 1  
  LECHAR = FF02     PERMIT=06 SIZE=20  HANDLE=0015            ; index 2  


int addDevice( char *name, int type, char *bt_address, int node );
int addCtic( int dn, char *name, int perm, int handle, int size, int cticn );
void dumpCtic( int dn );

-------------------------

Sample usage.

    int dn = addDevice( "JBD_BMS", BTYPE_LE, "A4:C1:38:55:2A:CA", 9 );
    
    int rc;
    rc  = addCtic( dn, "Name", 0x12, 0x03, 22, 0 );
    rc += addCtic( dn, "FF01", 0x12, 0x11, 32, 1 );
    rc += addCtic( dn, "FF02", 0x06, 0x15, 20, 2 );

    if ( rc != 3 )  // add failed
      return 0;
  
    dumpCtic( 0 );

*/



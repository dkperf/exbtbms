/* 
  bms.c
  
  This project is a sample for the Raspberry Pi which accesses a JDB BMS
  via the Bluetooth BLE protocol and requests the basic BMS status 
  information.  This project is ment to be just that, a simple sample
  of how to access the JDB BMS basic information via Bluetooth.

  This code is VERY specific to JBD BMS.  Will not work well with
  other BLE devices without a lot of modification..

  It is a simple base to start coding your BMS project from.  
  Other BMS status projects on the WEB do sooo.. many other
  things it is impossible to extract the basic code stuff from them.

 

  sample devices.txt for a JBD device ;  
  You must set your   local and BMS  addresses

  Sample btlib device file (required for this program) "devices.txt" 5 lines.
  The BMS NODE id seems just arbitary. I selected 9.


  DEVICE = Local_Pi TYPE=MESH  NODE=1  ADDRESS=XX:XX:XX:XX:XX:XX
  DEVICE = JBD_BMS  TYPE=LE    NODE=9  ADDRESS=XX:XX:XX:XX:XX:XX
    LECHAR = Name     PERMIT=12 SIZE=22  HANDLE=0003 UUID=2A00  ; index 0
    LECHAR = FF01     PERMIT=12 SIZE=20  HANDLE=0011            ; index 1  
    LECHAR = FF02     PERMIT=06 SIZE=20  HANDLE=0015            ; index 2  




  The LECHAR above are derived from Characteristics info (btlib style) of JBD BMS
  
      Characteristics info given by JBD BMS when asked.

    Device Name            22 byte Permit 12 rn  Handle=0003 UUID=2A00
    Appearance              2 byte Permit 02 r   Handle=0005 UUID=2A01
    Pref Connection Params  8 byte Permit 02 r   Handle=0007 UUID=2A04
    PnP ID                  7 byte Permit 02 r   Handle=000E UUID=2A50
    FF01 UUID              20 byte Permit 12 rn  Handle=0011 UUID=FF01
    FF02 UUID              20 byte Permit 06 rw  Handle=0015 UUID=FF02
    FA01 UUID               1 byte Permit 06 rw  Handle=0019 UUID=FA01

    This bms progam currently only uses 3 of the characteristics.


  btlib problems:
  Require root ( ie. 'sudo' ) to run.  Accesses HCI directly.
  Not an event driven system so hogs CPU. Must be very careful.
  Uses this funky devices.txt definitions file.

  btlib pluses:
  MUCH simpler than BlueZ.
  works! ie. gets the job done.

*/ 

//  ---- NOTICE ---------
//  These MUST match entries in devices.txt.  ugh...   btlib thing.
//  You can change them, just make sure file and these match.

#define BMS_NODE  9   //  NODE=9  for Your BMS device.txt entry.

#define IDX_2A00  0   //  index 0 (first ) LECHAR line under DEVICE=JBD..
#define IDX_FF01  1   //  index 1 (second) LECHAR line under DEVICE=JBD..
#define IDX_FF02  2   //  index 2 (third ) LECHAR line under DEVICE=JBD..



//  Acknoledgements:
//  Used a few lines from Stephen P. Shoecraft  git_hub :: jbdtool
//  Some lines in routine to decode JBD 03 and 04 messages. some data structs.
//  btlib of course, from git_hub btferret.

//  to make project:    gcc -o bms bms.c btlib.c
//
//  just for fun...   removes unused functions at link time.
//  gcc -Wl,--gc-sections -ffunction-sections -fdata-sections -o bms bms.c btlib.c


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "btlib.h"
#include "bms.h"



//#define DEBUG 1

#ifdef DEBUG
    #define DBUG( block ) block
#else
    #define DBUG( block )
#endif


//  Request messages for JDB BMS 
//  See "JBD BMS protocol info.xls" for more info.
char msg3[] = { 0xDD, 0xA5, 0x03, 0x00, 0xFF, 0xFD, 0x77 };
char msg4[] = { 0xDD, 0xA5, 0x04, 0x00, 0xFF, 0xFC, 0x77 };

bms_info_t g_BmsInfo = {};   // Bms Info and zero to start


//------------------------------------------------------------------
//  output the information in the BMS info structure.
//------------------------------------------------------------------
void printBmsInfo ( bms_info_t *bp )
{
   printf( "%-19s %s\n",   "+  bms_name:",  bp->name);
   printf( "%-19s %.2f\n", "+  voltage:",   bp->voltage);
   printf( "%-19s %.2f\n", "+  current:",   bp->current);
   printf( "%-19s %.2f\n", "+  capacity:",  bp->capacity);
   printf( "%-19s %.2f\n", "+  balance:",   bp->balance);

   printf( "%-19s %d\n",   "+  percent_charged:",(int)bp->stateOfCharge);

   printf( "%-19s %02x\n", "+  fetbits:",        bp->fetbits);
   printf( "%-19s %s\n", "+  Charging__FETs:", 
            ((bp->fetbits & BMS_FET_CHARGING) ? "ON" : "OFF" ));
   printf( "%-19s %s\n", "+  DisCharge_FETs:",
            ((bp->fetbits & BMS_FET_DISCHARGING) ? "ON" : "OFF" ));

   printf( "%-19s %d\n", "+  cells:", bp->cells);

   if ( bp->cellvolt[0] )
   {
      for(int i=0; i < bp->cells; i++) {
         printf( "+  cell_Volt_%d:     %.3f\n", i+1, bp->cellvolt[i] );
      }
   }

   //  List temperatures in degrees F. ( stored in C )
   for ( int i=0; i<bp->ntemps; i++) 
      printf( "+  temp_%d:          %.2f\n", i+1, (bp->temps[i] * 9/5) + 32);

   //  Balance on/off bits.
   {
      char bits[40];
      unsigned short mask = 1;
      int i = 0;
      while(mask) 
      {
         bits[i++] = ((bp->balancebits & mask) != 0 ? '1' : '0');
         mask <<= 1;
      }
      bits[i] = 0;
      printf("%-19s %s\n", "+  balancebits:", bits);
   }
}




//------------------------------------------------------------------
//  Decode the MSG 03 sent by the BMS 
//------------------------------------------------------------------
void processMsg03( bms_info_t *bp, unsigned char *data )
{

   bp->voltage  = (float)_getshort(&data[0]) / 100.0;
   bp->current  = (float)_getshort(&data[2]) / 100.0;
   bp->balance  = (float)_getshort(&data[4]) / 100.0;
   bp->capacity = (float)_getshort(&data[6]) / 100.0;

   bp->balancebits  = _getshort(&data[12]);    // Cells Balance active? 
   bp->balancebits |= _getshort(&data[14]) << 16;

   bp->protectbits = _getshort(&data[16]);  // What BMS protections are on

   bp->stateOfCharge = data[19];
   bp->fetbits = data[20];       //  Input / Output FET state

   bp->cells = data[21];         //  # Battery cells
   bp->ntemps = data[22];

   // Temps of BMS probes  in 0.1 K
   for( int i=0; i < bp->ntemps; i++) {
      bp->temps[i] = (((float)_getshort(&data[23+(i*2)]) - 2731) / 10);
   }
}



//------------------------------------------------------------------
//  Decode the MSG 04 sent by the BMS 
//------------------------------------------------------------------
void processMsg04( bms_info_t *bp, unsigned char *data )
{   
   //  current cell voltages.
   for(int i=0; i < bp->cells; i++) {
      bp->cellvolt[i] = (float)_getshort(&data[i*2]) / 1000;
   }
}



//------------------------------------------------------------------
//    convert byte array to hex: outpur "XX XX XX..."
//------------------------------------------------------------------
char *strToHex( char *buf, int len )
{
   static char hbuf[500];
   int x = 0;

   if ( len > (sizeof(hbuf)/3))
      len = (sizeof(hbuf)/3);

   for (int i = 0; i < len; i++ )
   {
      sprintf(hbuf+x, "%02X ", (unsigned char)*buf++ );
      x +=3;
   }
   if (x > 2) hbuf[x-1] = 0;  // nuke trailing ' '

   return( hbuf ); 
}



//------------------------------------------------------------------
//  Called by btLib when an async (notify) message comes in from BMS
//  JBD BMS sends some BMS messages in multiple parts (Bluetooth max msg)
//  If we don't have the whole BMS message then this routine will 
//  get called again when more parts are received via bluetooth.
//------------------------------------------------------------------
int notify_callback(int lenode,int cticn,char *buf,int nread)
{ 
   static char g_msg_buf[200]; // not the best, but for simplicity...
   static int  g_buf_len =0;

   DBUG( printf( "%s has rec'd %s\n",device_name(lenode),ctic_name(lenode,cticn));)
   DBUG( printf( "buf: %s\n", strToHex( buf, nread ));)

   memmove( &g_msg_buf[g_buf_len], buf, nread );
   g_buf_len += nread;

   if ( *g_msg_buf != 0xDD || g_msg_buf[2]) // Unk msg?  ie. bad header
   {                                        // or ERROR set in msg 
      if ( g_msg_buf[2] )
         printf( "%s : ERROR in received msg\n", g_BmsInfo.name );

      g_buf_len = 0;       // chuck the message ; buf[2] = 0 if no ERROR
      return 0;
   }

   // Have whole message  ??
   // 4+3 is for header & checksum & End of msg 0x77
   // JBD messages have length of data in byte 3
   if( g_buf_len >= g_msg_buf[3] + 4 + 3)
   {
      switch ( g_msg_buf[1] )   // Message ID ( register ID )
      {
         case 0x03:
            processMsg03( &g_BmsInfo, &g_msg_buf[4] );
            break;
         case 0x04:
            processMsg04( &g_BmsInfo, &g_msg_buf[4] );
            break;
         case 0x05:   // Todo ....
            break;
      }

      DBUG( printBmsInfo( &g_BmsInfo );)

      g_buf_len = 0; 
   }

   DBUG( else printf( "waiting for more data... %d %d\n", g_buf_len, nread);)

   return(0);
}  


//------------------------------------------------------------------
//   request BMS status and wait for response msg's to be handled
//------------------------------------------------------------------
void get_BMS_StatusInfo()
{
   //  My BMS seems to often eat first FF02 (handle 0x15) request ? 
   //  So we loop and send requests a couple of times just make sure. no harm
   //  Something to do with Bluetooth dongle going to sleep perhaps?

   write_ctic(BMS_NODE,IDX_FF02,msg4,sizeof(msg4)); // Request BMS MSG 04

   for ( int j =0; j<2; j++)
   {
      // btlib is not an event driven system.   :-(
      // It is a constant read in a loop system that hogs CPU.
      // So to help a little we use usleep instead of using btlib 
      // read_notify(xx) with long timeout. Still VERY bad.
      // read_notify( 3000 ) will use %100 cpu for 3 seconds.
      // you can use top command to check your progam cpu usage.
      // 03 and 04 msg response time seems to be 60-150 ms.

      usleep(100*1000); // milli sec sleep

      // These BMS requests are sent as characteristic FF02 requests
      // and are returned by the BMS as FF01 characteristic info.
      write_ctic(BMS_NODE,IDX_FF02,msg3,sizeof(msg3)); // Request BMS MSG 03

      usleep(200*1000); 
      read_notify(2);   // spin and read any response for x milli sec

      write_ctic(BMS_NODE,IDX_FF02,msg4,sizeof(msg4)); // Request BMS MSG 04

      write_ctic(BMS_NODE,IDX_FF02,msg4,sizeof(msg4)); // Request BMS MSG 04
      usleep(200*1000);
      read_notify(2);  

      DBUG( printBmsInfo( &g_BmsInfo ); ) // Show info read from BMS
      DBUG( printf("\n---------------------------\n\n"); )
   }
}



//------------------------------------------------------------------
//   Main
//------------------------------------------------------------------
int main()
{
   DBUG( set_print_flag(PRINT_VERBOSE); )

   if(init_blue("devices.txt") == 0)
      return(0);

   set_le_wait(150);             // LE connection completion time  ms

   if (connect_node(BMS_NODE,CHANNEL_LE,0)==0)// 3rd param 0 unused on LE devices
      return(0);

   // read Name from BMS  (index IDX_2A00 from devices.txt file)
   read_ctic(BMS_NODE,IDX_2A00,g_BmsInfo.name,sizeof(g_BmsInfo.name));   

   DBUG( printf( "BMS Name = %s\n",g_BmsInfo.name); )

   // inform btlib of callback func for FF01 Msgs that come from BMS
   setCallback( BMS_NODE, IDX_FF01, notify_callback );  

   // Send status requests to BMS and process returned messages.
   get_BMS_StatusInfo();

   disconnect_node(BMS_NODE);
   close_all();

   printBmsInfo( &g_BmsInfo ); // Show info read from BMS

   return(0);
}



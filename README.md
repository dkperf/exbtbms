
# ezbtbms JBD BMS bluetooth interface

JBD BMS Bluetooth BLE status request.

This 'c' project was done on a Raspberry Pi. It's purpose is to access a
JBD BMS using the Bluetooth BLE protocol and request the basic BMS
status information.  This project is meant to be a simple example
of how to access the JBD BMS basic information via Bluetooth.

This code is VERY specific to JBD BMS.  Will not work well with
other BLE devices without modification.

It is a simple code base to start coding your BMS project from.
Other BMS BT status projects on the WEB do sooo.. many other
things it is difficult to extract the basic code set from them.

This project is written all in 'c' using the btlib code provided in the
github petzval/btferret project.  The btferret project has a library
of bluetooth functions provided in btlib.c.  Go read https://github.com/petzval/btferret project docs.

## Installation

To create the binary, download 5 files and compile.(not even a make file).

    btlib.c
    btlib.h
    bms.c
    bms.h
    devices.txt

    gcc -o bms bms.c btlib.c

    edit devices.txt      add bluetooth address of your BMS and local host.
                          See BT ADDRESSes  below if you don't have them.

    sudo setcap 'cap_net_raw,cap_net_admin+eip' bms

    ./bms


Thats it.  No other dependencies.


## BT ADDRESSes
In order to find local Pi bluetooth device address(s) run:

    sudo hcitool dev

In order to find remote bluetooth BLE device addresses run:

    sudo hcitool lescan

## Usage/Output

Stdout output from the execution of  './bms'


    +  bms_name:        xiaoxiang BMS
    +  voltage:         13.33
    +  current:         0.00
    +  capacity:        260.00
    +  balance:         207.91
    +  state_of_charge: %80
    +  fetbits:         03
    +  Charging__FETs:  ON
    +  DisCharge_FETs:  ON
    +  cells:           4
    +  cell_Volt_1:     3.334
    +  cell_Volt_2:     3.333
    +  cell_Volt_3:     3.334
    +  cell_Volt_4:     3.334
    +  temp_1:          78.08
    +  temp_2:          79.88
    +  temp_3:          79.16
    +  balancebits:     0000000000000000


Or if bms.c compliled with #define JSON_OUT 1
then Stdout output from the execution of  './bms' is :

    {
      "bms": {
        "name"        : "xiaoxiang BMS",
        "voltage"     : 13.33,
        "current"     : 0.00,
        "capacity"    : 260.00,
        "balance"     : 207.84,
        "percent"     : 80,
        "fetbits"     : 3,
        "balancebits" : 0,
        "cellvolts"   : [  0.000, 0.000, 0.000, 0.000 ],
        "temps"       : [  22.80, 23.30, 23.00 ]
      }
    }


## More Information
The few samples I have found on the WEB are either way too complicated
or have too many dependencies.

I just want to connect to my JBD BMS via Bluetooth and ask for the
basic BMS status information.

Not to mention that all the documentation ( not much ) I have found
about using the Bluetooth BLE protocol within the 'C' language world
are very limited.

I am trying to keep this simple.  No feature creep. Creep is for you todo.
bms.c only has couple hundred lines of actual code.  btlib.c is another story,
but hopefully you don't have to look at btlib.c for this purpose.
We use just a small subset of btlib.
For more information on btlib see the github project 'petzval/btferret'.
https://github.com/petzval/btferret 


    Using  btlib  problems:
      Requires root ( ie. 'sudo' ) capabilities for network_raw. 
      Accesses HCI directly. See 'info on raw sockets' below.
        Hence the "sudo setcap 'cap_net_raw,cap_net_admin+eip' bms"
       
      Not an event driven system so hogs CPU. Must be very careful.
      Uses this funky devices.txt definitions file.
      Takes over local pi bluetooth device HCI ; no other BT devices can be connected.

    btlib pluses:
      All self contained.  All the source for this project is in two c files.
      Does not use or need BlueZ ( default Pi bluetooth stack )
      MUCH simpler than BlueZ and others.
      works! ie. gets the job done.
      
More info on raw sockets: [Linux Raw sockets](https://squidarth.com/networking/systems/rc/2018/05/28/using-raw-sockets.html)

## Acknowledgements

 - [btlib project/code](https://github.com/petzval/btferret )
 - [jbdtool project](https://github.com/sshoecraft/jbdtool)
 


## Other JBD BMS Info / Documentation

[btlib documentation](http://github.com/petzval/btferret)

[Lots of JBD BMS information](http://overkillsolar.com)

[BMS Comm_Protocol_Documents](https://github.com/FurTrader/OverkillSolarBMS/tree/master/Comm_Protocol_Documentation)

[Project w BMS BLE interface code using BlueZ/Gattlib](http://github.com/sshoecraft/jbdtool)

[JBD BMS BLE Protocol discussion](https://endless-sphere.com/forums/viewtopic.php?t=91672)

## Project details

Acknoledgements:
  Used a few lines from Stephen P. Shoecraft  jbdtool
  From routine to decode JBD 03 and 04 messages. some data structs.
  btlib of course.


The btlib.c in this project has been modified from the stock btlib.c
provided by the github :: btferret project.
Timestamps have been added to output when DEBUG is on and DEBUG is now
off by default.  Also a new routine setCallback() has been added.

btlib requires a devices.txt file:
You must set your   local and BMS  addresses in devices.txt

Sample btlib device file (required for this program) "devices.txt" 5 lines.

    DEVICE = Local_Pi TYPE=MESH  NODE=1  ADDRESS=XX:XX:XX:XX:XX:XX
    DEVICE = JBD_BMS  TYPE=LE    NODE=9  ADDRESS=XX:XX:XX:XX:XX:XX
       LECHAR = Name     PERMIT=12 SIZE=22  HANDLE=0003 UUID=2A00  ; index 0
       LECHAR = FF01     PERMIT=12 SIZE=20  HANDLE=0011            ; index 1
       LECHAR = FF02     PERMIT=06 SIZE=20  HANDLE=0015            ; index 2


  The LECHAR lines above are derived from Characteristics info of JBD BMS.

      Characteristics info given by JBD BMS when requested:

    Device Name            22 byte Permit 12 rn  Handle=0003 UUID=2A00
    Appearance              2 byte Permit 02 r   Handle=0005 UUID=2A01
    Pref Connection Params  8 byte Permit 02 r   Handle=0007 UUID=2A04
    PnP ID                  7 byte Permit 02 r   Handle=000E UUID=2A50
    FF01 UUID              20 byte Permit 12 rn  Handle=0011 UUID=FF01
    FF02 UUID              20 byte Permit 06 rw  Handle=0015 UUID=FF02
    FA01 UUID               1 byte Permit 06 rw  Handle=0019 UUID=FA01

This bms progam currently only uses 3 of the characteristics as listed in devices.txt above.


btlib has ability to collect BLE device advertisements and then associated characteristics
available for that device.  One could then write code to search that info to find your BMS
device and characteristics thus making the devices file mute. But for simplicity and speed
I used the hardcoded method.



Basic Program outline:

    initialize the  btlib.
        init structures
        connect to system HCI socket.
        read devices.txt file to get bluetooth device basic information.

    connect to JBD BMS device.

    send characteristic read cmd  to get BMS name.
       characteristic 2A00 (name)

    set a callback function so btlib will call us back when
    async messages come in from the BMS.


    loop a couple of times
       send status request 03 message to BMS.
       look for response;  response msg processed in callback

       send status request 04 message to BMS.
       look for response;  response msg processed in callback

    disconnect from all devices

    print the status information.
    exit


If you need or want verbose/debug info from the bms program then uncomment #define DEBUG
line in bms.c and recompile.



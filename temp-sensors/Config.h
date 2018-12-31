#ifndef _CONFIG_H_
#define _CONFIG_H_

//==============================================================================
//
//-------------------- PLEASE REVIEW FOLLOW VARIABLES ------------------
//
// SECURITY WARNING : uncomment ENABLE_CORS=0 in production!
//---------------------
// I used CORS policy to allow me write index.htm and app.js outside atmega controller from pc
// using atmega webapis
//
#define ENABLE_CORS 1

// choose one of follow two interface
#define USE_ENC28J60
//#define USE_W5500

//
// static ip address to save some flash
//
#define MACADDRESS 0x02, 0xca, 0xfe, 0x62, 0x2e, 0x92
#define MYIPADDR 10, 10, 2, 5
#define MYIPMASK 255, 255, 255, 0
#define MYDNS 10, 10, 0, 6
#define MYGW 10, 10, 0, 1
#define LISTENPORT 80
#define MAX_HEADER_SIZE 80
#define ONE_WIRE_BUS 3

// EDIT DebugMacros to set SERIAL_SPEED and enable/disable DPRINT_SERIAL

#define TEMPERATURE_HISTORY_BACKLOG_HOURS 32
#define TEMPERATURE_HISTORY_FREERAM_THRESHOLD 400
#define UPDATE_TEMPERATURE_MS 5000

//
//==============================================================================


#ifdef USE_ENC28J60
#include <UIPEthernet.h>
// edit UIPEthernet/utility/uipethernet-conf.h to customize
// - define UIP_CONF_UDP=0 to reduce flash size
#endif

#ifdef USE_W5500
#include <Ethernet.h>
#endif

#endif
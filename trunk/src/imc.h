/* IMC2 Freedom Client - Developed by Mud Domain.
 *
 * Copyright 2004-2008 by Roger Libiez ( Samson )
 * Contributions by Johnathan Walker ( Xorith ), Copyright 2004
 * Additional contributions by Jesse Defer ( Garil ), Copyright 2004
 * Additional contributions by Rogel, Copyright 2004
 * Comments and suggestions welcome: http://www.mudbytes.net/imc2-support-forum
 * License terms are available in the imc2freedom.license file.
 */

#ifndef __IMC2_H__
#define __IMC2_H__

/* The all important version ID string, which is hardcoded for now out of laziness.
 * This name was chosen to represent the ideals of not only the code, but of the
 * network which spawned it.
 */
#define IMC_VERSION_STRING "IMC2 Freedom CL-2.2 "
#define IMC_VERSION 2

/* Number of entries to keep in the channel histories */
#define MAX_IMCHISTORY 20
#define MAX_IMCTELLHISTORY 20

/* Make sure you set the macros in the imccfg.h file properly or things get ugly from here. */
#include "imccfg.h"

#define IMC_BUFF_SIZE 16384

/* Should not need to edit anything below this point */

#define LGST 4096 /* Large String */
#define SMST 1024 /* Small String */

/* No real functional difference in alot of this, but double linked lists DO seem to handle better,
 * and they look alot neater too. Yes, readability IS important! - Samson
 */
typedef struct imc_channel IMC_CHANNEL;   /* Channels, both local and non-local */
typedef struct imc_packet IMC_PACKET;  /* It's a packet! */
typedef struct imc_packet_data IMC_PDATA; /* Extra data fields for packets */
typedef struct imc_siteinfo SITEINFO;  /* The given mud :) */
typedef struct imc_remoteinfo REMOTEINFO; /* Information on a mud connected to IMC */
typedef struct imc_ban_data IMC_BAN;   /* Mud level bans */
typedef struct imcchar_data IMC_CHARDATA; /* Player flags */
typedef struct imc_ignore IMC_IGNORE;  /* Player level ignores */
typedef struct imcucache_data IMCUCACHE_DATA;   /* User cache data for gender targetting socials */
typedef struct imc_color_table IMC_COLOR; /* The Color config */
typedef struct imc_command_table IMC_CMD_DATA;  /* Command table */
typedef struct imc_help_table IMC_HELP_DATA; /* Help table */
typedef struct imc_cmd_alias IMC_ALIAS;   /* Big, bad, bloated command alias thing */
typedef struct imc_packet_handler IMC_PHANDLER; /* custom packet handlers added dynamically */
typedef struct who_template WHO_TEMPLATE; /* The who templates */

typedef void IMC_FUN( CHAR_DATA * ch, const char *argument );
#define IMC_CMD( name ) void (name)( CHAR_DATA *ch, const char *argument )

typedef void PACKET_FUN( IMC_PACKET * q, const char *packet );
#define PFUN( name ) void (name)( IMC_PACKET *q, const char *packet )

/*
extern REMOTEINFO *first_rinfo;
extern REMOTEINFO *last_rinfo;
*/
extern SITEINFO *this_imcmud;

/* Oh yeah, baby, that raunchy looking Merc structure just got the facelift of the century.
 * Thanks to Thoric and friends for the slick idea.
 */
struct imc_cmd_alias
{
   IMC_ALIAS *next;
   IMC_ALIAS *prev;
   char *name;
};

struct imc_command_table
{
   IMC_CMD_DATA *next;
   IMC_CMD_DATA *prev;
   IMC_ALIAS *first_alias;
   IMC_ALIAS *last_alias;
   IMC_FUN *function;
   char *name;
   int level;
   bool connected;
};

struct imc_help_table
{
   IMC_HELP_DATA *next;
   IMC_HELP_DATA *prev;
   char *name;
   char *text;
   int level;
};

struct imc_color_table
{
   IMC_COLOR *next;
   IMC_COLOR *prev;
   char *name; /* the name of the color */
   char *mudtag;  /* What the mud uses for the raw tag */
   char *imctag;  /* The imc tilde code that represents the mudtag to the network */
};

struct imc_ignore
{
   IMC_IGNORE *next;
   IMC_IGNORE *prev;
   char *name;
};

struct imcucache_data
{
   IMCUCACHE_DATA *next;
   IMCUCACHE_DATA *prev;
   char *name;
   time_t time;
   int gender;
};

struct imcchar_data
{
   IMC_IGNORE *imcfirst_ignore;  /* List of ignored people */
   IMC_IGNORE *imclast_ignore;
   char *rreply;  /* IMC reply-to */
   char *rreply_name;   /* IMC reply-to shown to char */
   char *imc_listen; /* Channels the player is listening to */
   char *imc_denied; /* Channels the player has been denied use of */
   char *imc_tellhistory[MAX_IMCTELLHISTORY];   /* History of received imctells - Samson 1-21-04 */
   char *email;   /* Person's email address - for imcfinger - Samson 3-21-04 */
   char *homepage;   /* Person's homepage - Samson 3-21-04 */
   char *aim;  /* Person's AOL Instant Messenger screenname - Samson 3-21-04 */
   char *yahoo;   /* Person's Y! screenname - Samson 3-21-04 */
   char *msn;  /* Person's MSN Messenger screenname - Samson 3-21-04 */
   char *comment; /* Person's personal comment - Samson 3-21-04 */
   long imcflag;  /* Flags set on the player */
   int icq; /* Person's ICQ UIN Number - Samson 3-21-04 */
   int imcperm;   /* Permission level for the player */
};

struct imc_channel
{
   IMC_CHANNEL *next;
   IMC_CHANNEL *prev;
   char *name; /* name of channel */
   char *owner;   /* owner (singular) of channel */
   char *operators;  /* current operators of channel */
   char *invited;
   char *excluded;
   char *local_name; /* Operational localname */
   char *regformat;
   char *emoteformat;
   char *socformat;
   char *history[MAX_IMCHISTORY];
   long flags;
   short level;
   bool open;
   bool refreshed;
};

struct imc_packet_data
{
   IMC_PDATA *next;
   IMC_PDATA *prev;
   char field[IMC_BUFF_SIZE];
};

struct imc_packet
{
   IMC_PDATA *first_data;
   IMC_PDATA *last_data;
   char from[SMST];
   char to[SMST];
   char type[SMST];
   char route[SMST]; /* This is only used internally and not sent */
};

/* The mud's connection data for the server */
struct imc_siteinfo
{
   char *servername; /* name of server */
   char *rhost;   /* DNS/IP of server */
   char *network; /* Network name of the server, set at keepalive - Samson */
   char *serverpw;   /* server password */
   char *clientpw;   /* client password */
   char *localname;  /* One word localname */
   char *fullname;   /* FULL name of mud */
   char *ihost;   /* host AND port of mud */
   char *email;   /* contact address (email) */
   char *www;  /* homepage */
   char *base; /* The mud's codebase name */
   char *details; /* BRIEF description of mud */
   int iport;  /* The port the mud itself is on */
   int minlevel;  /* Minimum player level */
   int immlevel;  /* Immortal level */
   int adminlevel;   /* Admin level */
   int implevel;  /* Implementor level */
   unsigned short rport;   /* remote port of server */
   bool sha256;   /* Client will support SHA-256 authentication */
   bool sha256pass;  /* Client is using SHA-256 authentication */
   bool autoconnect; /* Do we autoconnect on bootup or not? - Samson */

   /*
    * Conection parameters - These don't save in the config file 
    */
   char inbuf[IMC_BUFF_SIZE]; /* input buffer */
   char incomm[IMC_BUFF_SIZE];
   char *outbuf;  /* output buffer */
   char *versionid;  /* Transient version id for the imclist */
   unsigned long outsize;
   int outtop;
   SOCKET desc;   /* descriptor */
   unsigned short state;   /* connection state */
};

struct imc_remoteinfo
{
   REMOTEINFO *next;
   REMOTEINFO *prev;
   char *name;
   char *version;
   char *network;
   char *path;
   char *url;
   char *host;
   char *port;
   bool expired;
};

/* A mudwide ban */
struct imc_ban_data
{
   IMC_BAN *next;
   IMC_BAN *prev;
   char *name;
};

struct imc_packet_handler
{
   IMC_PHANDLER *next;
   IMC_PHANDLER *prev;
   PACKET_FUN *func;
   char *name;
};

struct who_template
{
   char *head;
   char *plrheader;
   char *immheader;
   char *plrline;
   char *immline;
   char *tail;
   char *master;
};

bool imc_command_hook( CHAR_DATA * ch, const char *command, const char *argument );
void imc_hotboot( void );
void imc_startup( bool force, SOCKET desc, bool connected );
void imc_shutdown( bool reconnect );
void imc_initchar( CHAR_DATA * ch );
bool imc_loadchar( CHAR_DATA * ch, FILE * fp, const char *word );
void imc_savechar( const CHAR_DATA * ch, FILE * fp );
void imc_freechardata( CHAR_DATA * ch );
void imc_loop( void );

#endif

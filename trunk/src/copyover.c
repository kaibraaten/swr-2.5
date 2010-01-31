/***************************************************************************
 *           	Star Wars: Storm of Vengeance Alpha 0.1			   *
 *==========================================================================*
 * Sw-Storm Alpha 0.1 Code Changes by Iczer/K.lopes w/ help from Maelfius   *
 * Additional Code within go to their respective owners.			   *
 *==========================================================================*
 * Star Wars Reality Code Additions and changes from the Smaug Code         *
 * copyright (c) 1997 by Sean Cooper                                        *
 *==========================================================================*
 * Starwars and Starwars Names copyright(c) Lucas Film Ltd.                 *
 *==========================================================================*
 * SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider                           *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,                    *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops                *
 *==========================================================================*
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 *==========================================================================*
 *                       Warmboot/Copyover Module			   *
 ****************************************************************************/
/* Origional Copyover Code by Erwin S. Andreasen http://www.andreasen.org/ */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "mud.h"
#include "os.h"

/*
 * OS-dependent local functions.
 */
SOCKET init_socket( int port );
void new_descriptor( SOCKET new_desc );
bool read_from_descriptor( DESCRIPTOR_DATA * d );
bool write_to_descriptor( SOCKET desc, const char *txt, int length );
void init_descriptor( DESCRIPTOR_DATA * dnew, SOCKET desc );

/*  Warm reboot stuff, gotta make sure to thank Erwin for this :) */
extern int port;		/* Port number to be used       */
extern SOCKET control;		/* Controlling descriptor       */

void do_copyover( CHAR_DATA * ch, char *argument )
{
  DESCRIPTOR_DATA *d, *d_next;
  char buf[100];
  FILE *fp = fopen( COPYOVER_FILE, "w" );

#if defined(AMIGA) || defined(__MORPHOS__)
  long error_code = 0;
#else
  char buf2[100];
#endif

  if( !fp )
  {
    send_to_char( "Copyover file not writeable, aborted.\r\n", ch );
    log_printf( "Could not write to copyover file: %s", COPYOVER_FILE );
    perror( "do_copyover:fopen" );
    return;
  }

  strcpy( buf,
      "\r\nA Blinding Flash of light starts heading towards you, before you can think it engulfs you!\r\n" );

  /* For each playing descriptor, save its state */
  for( d = first_descriptor; d; d = d_next )
  {
    CHAR_DATA *och = d->original ? d->original : d->character;
    d_next = d->next;		/* We delete from the list , so need to save this */

    if( !d->character || d->connected != CON_PLAYING )	/* drop those logging on */
    {
      write_to_descriptor( d->descriptor, "\r\nSorry, we are rebooting."
	  " Come back in a few minutes.\r\n", 0 );
      close_socket( d, FALSE );	/* throw'em out */
    }
    else
    {
#if defined(AMIGA) || defined(__MORPHOS__)
      static long sockID = 0;
      SOCKET cur_desc = INVALID_SOCKET;

      ++sockID;
      cur_desc = ReleaseCopyOfSocket( d->descriptor, sockID );

      if( cur_desc == SOCKET_ERROR )
      {
	fprintf( out_stream, "ReleaseCopyOfSocket() failed.\n" );
	fclose( fp );
	exit( 1 );
      }
#else
      SOCKET cur_desc = d->descriptor;
#endif

      fprintf( fp, "%d %s %s\n", cur_desc, och->name, d->host );
      save_char_obj( och );
      write_to_descriptor( d->descriptor, buf, 0 );
    }
  }

  fprintf( fp, "-1\n" );
  fclose( fp );

#if defined(AMIGA) || defined(__MORPHOS__)
  sprintf( buf, "run >NIL: %s %d copyover %d",
      sysdata.exe_filename, port, control );

  error_code = System( (CONST_STRPTR) buf, NULL );

  if( error_code == -1 )
  {
    bug( "Copyover failure, executable could not be run." );
    fprintf( out_stream, "Failed to run %s\n", sysdata.exe_filename );
    ch_printf( ch, "Copyover FAILED!\r\n" );
  }
  else
  {
    exit( 0 );
  }

#else
  /* exec - descriptors are inherited */
  sprintf( buf, "%d", port );
  sprintf( buf2, "%d", control );

  fclose( out_stream );
  out_stream = NULL;

  execl( sysdata.exe_filename, sysdata.exe_filename,
      buf, "copyover", buf2, ( char * ) NULL );

  /* Failed - sucessful exec will not return */

  perror( "do_copyover: execl" );
  send_to_char( "Copyover FAILED!\r\n", ch );
#endif
}

/* Recover from a copyover - load players */
void copyover_recover( void )
{
  DESCRIPTOR_DATA *d = NULL;
  FILE *fp = NULL;
  char name[100];
  char host[MAX_STRING_LENGTH];
  SOCKET desc = 0;
  bool fOld = FALSE;

  log_string( "Copyover recovery initiated" );

  fp = fopen( COPYOVER_FILE, "r" );

  if( !fp )			/* there are some descriptors open which will hang forever then ? */
  {
    perror( "copyover_recover:fopen" );
    log_string( "Copyover file not found. Exitting.\r\n" );
    exit( 1 );
  }

  remove( COPYOVER_FILE );	/* In case something crashes
				   - doesn't prevent reading */
  for( ;; )
  {
    fscanf( fp, "%d %s %s\n", &desc, name, host );

    if( desc == -1 )
      break;

#if defined(AMIGA) || defined(__MORPHOS__)
    desc = ObtainSocket( desc, PF_INET, SOCK_STREAM, IPPROTO_TCP );

    if( desc == INVALID_SOCKET )
    {
      bug( "ObtainSocket returned error." );
      continue;
    }
#endif

    /* Write something, and check if it goes error-free */
    if( !write_to_descriptor
	( desc,
	  "\r\nThe surge of Light passes leaving you unscathed and your world reshaped anew\r\n",
	  0 ) )
    {
      closesocket( desc );	/* nope */
      continue;
    }

    CREATE( d, DESCRIPTOR_DATA, 1 );
    init_descriptor( d, desc );	/* set up various stuff */

    d->host = STRALLOC( host );

    LINK( d, first_descriptor, last_descriptor, next, prev );
    d->connected = CON_COPYOVER_RECOVER;	/* negative so close_socket
						   will cut them off */

    /* Now, find the pfile */
    fOld = load_char_obj( d, name, FALSE );

    if( !fOld )		/* Player file not found?! */
    {
      write_to_descriptor( desc,
	  "\r\nSomehow, your character was lost in the copyover sorry.\r\n",
	  0 );
      close_socket( d, FALSE );
    }
    else			/* ok! */
    {
      char argument[MAX_INPUT_LENGTH];
      snprintf( argument, MAX_INPUT_LENGTH, "%s", "auto noprog" );
      write_to_descriptor( desc, "\r\nCopyover recovery complete.\r\n",
	  0 );

      /* Just In Case,  Someone said this isn't necassary, but _why_
	 do we want to dump someone in limbo? */
      if( !d->character->in_room )
	d->character->in_room = get_room_index( ROOM_VNUM_SCHOOL );

      /* Insert in the char_list */
      LINK( d->character, first_char, last_char, next, prev );

      char_to_room( d->character, d->character->in_room );
      do_look( d->character, argument );
      load_home( d->character );
      act( AT_ACTION, "$n materializes!", d->character, NULL, NULL,
	  TO_ROOM );
      d->connected = CON_PLAYING;
    }
  }

  fclose( fp );
}

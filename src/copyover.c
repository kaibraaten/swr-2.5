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

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include "mud.h"
/*
 * Socket and TCP/IP stuff.
 */
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <netdb.h>
/*
 * OS-dependent local functions.
 */
int	init_socket		args( ( int port ) );
void	new_descriptor		args( ( int new_desc ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, const char *txt, int length ) );
void    init_descriptor		args( ( DESCRIPTOR_DATA *dnew, int desc) );

/*  Warm reboot stuff, gotta make sure to thank Erwin for this :) */
int port;               /* Port number to be used       */
int control;		/* Controlling descriptor	*/

void do_copyover (CHAR_DATA *ch, char * argument)
{
  DESCRIPTOR_DATA *d, *d_next;
  char buf [100], buf2[100];
  FILE *fp = fopen (COPYOVER_FILE, "w");

  if (!fp)
    {
      send_to_char ("Copyover file not writeable, aborted.\n\r",ch);
      log_printf ("Could not write to copyover file: %s", COPYOVER_FILE);
      perror ("do_copyover:fopen");
      return;
    }

  strcpy(buf, "\n\rA Blinding Flash of light starts heading towards you, before you can think it engulfs you!\n\r" );

  /* For each playing descriptor, save its state */
  for (d = first_descriptor; d ; d = d_next)
    {
      CHAR_DATA * och = CH(d);
      d_next = d->next; /* We delete from the list , so need to save this */

      if (!d->character || d->connected != CON_PLAYING) /* drop those logging on */
	{
	  write_to_descriptor (d->descriptor, "\n\rSorry, we are rebooting."
			       " Come back in a few minutes.\n\r", 0);
	  close_socket (d, FALSE); /* throw'em out */
	}
      else
	{
	  fprintf (fp, "%d %s %s\n", d->descriptor, och->name, d->host);
	  save_char_obj (och);
	  write_to_descriptor (d->descriptor, buf, 0);
	}
    }

  fprintf (fp, "-1\n");
  fclose (fp);

  /* Close reserve and other always-open files and release other resources */
  fclose (fpReserve);
  fclose (fpLOG);

  /* exec - descriptors are inherited */

  sprintf (buf, "%d", port);
  sprintf (buf2, "%d", control);

  execl (EXE_FILE, "swreality", buf, "copyover", buf2, (char*) NULL );

  /* Failed - sucessful exec will not return */

  perror ("do_copyover: execl");
  send_to_char ("Copyover FAILED!\n\r",ch);

  /* Here you might want to reopen fpReserve */
  /* Since I'm a neophyte type guy, I'll assume this is
     a good idea and cut and past from main()  */

  if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
      perror( NULL_FILE );
      exit( 1 );
    }

  if ( ( fpLOG = fopen( NULL_FILE, "r" ) ) == NULL )
    {
      perror( NULL_FILE );
      exit( 1 );
    }
}

/* Recover from a copyover - load players */
void copyover_recover( void )
{
  DESCRIPTOR_DATA *d;
  FILE *fp;
  char name [100];
  char host[MAX_STRING_LENGTH];
  int desc;
  bool fOld;
/*  CHAR_DATA *ch; */ /* Uncomment This Line if putting functions in comm.c -Iczer */
  log_string ("Copyover recovery initiated");

  fp = fopen (COPYOVER_FILE, "r");

  if (!fp) /* there are some descriptors open which will hang forever then ? */
        {
          perror ("copyover_recover:fopen");
          log_string("Copyover file not found. Exitting.\n\r");
           exit (1);
        }

  unlink (COPYOVER_FILE); /* In case something crashes
                              - doesn't prevent reading */
  for (;;)
   {
     fscanf (fp, "%d %s %s\n", &desc, name, host);
     if (desc == -1)
       break;

        /* Write something, and check if it goes error-free */
     if (!write_to_descriptor (desc, "\n\rThe surge of Light passes leaving you unscathed and your world reshaped anew\n\r", 0))
       {
         close (desc); /* nope */
         continue;
        }

      CREATE(d, DESCRIPTOR_DATA, 1);
      init_descriptor (d, desc); /* set up various stuff */

      d->host = STRALLOC( host );

      LINK( d, first_descriptor, last_descriptor, next, prev );
      d->connected = CON_COPYOVER_RECOVER; /* negative so close_socket
                                              will cut them off */

        /* Now, find the pfile */

      fOld = load_char_obj (d, name, FALSE);

      if (!fOld) /* Player file not found?! */
       {
          write_to_descriptor (desc, "\n\rSomehow, your character was lost in the copyover sorry.\n\r", 0);
          close_socket (d, FALSE);
       }
      else /* ok! */
       {
          write_to_descriptor (desc, "\n\rWarmboot recovery complete.\n\r",0);

           /* Just In Case,  Someone said this isn't necassary, but _why_
              do we want to dump someone in limbo? */
           if (!d->character->in_room)
                d->character->in_room = get_room_index (ROOM_VNUM_SCHOOL);

           /* Insert in the char_list */
           LINK( d->character, first_char, last_char, next, prev );

           char_to_room (d->character, d->character->in_room);
           do_look (d->character, const_char_to_nonconst("auto noprog"));
	   load_home(d->character);	
           act (AT_ACTION, "$n materializes!", d->character, NULL, NULL, TO_ROOM);
           d->connected = CON_PLAYING;
       }

   }
   fclose (fp);
}

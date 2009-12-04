/*
 * Copyright (c) 2008-2010 Kai Braaten
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "os.h"

struct Library *SocketBase = NULL;
struct Library *UserGroupBase = NULL;
struct UtilityBase *UtilityBase = NULL;
extern FILE *out_stream;

#ifndef __DATE2__
#define __DATE2__ __DATE__
#endif

#define MUD_NAME "SWR Factor 2.0"
#define VERSTAG "\0$VER: " MUD_NAME " (" __DATE2__ ")"
const char *VersTag = VERSTAG;

static const char *get_next_filename( CONST_STRPTR directory )
{
  static char filename[256];
  int high_num = 1000;
  BPTR sourcelock = NULL;
  struct ExAllControl *excontrol = NULL;
  struct ExAllData buffer, *ead = NULL;
  BOOL exmore = TRUE;

  *filename = '\0';
  memset( &buffer, 0, sizeof( buffer ) );
  sourcelock = Lock( directory, SHARED_LOCK );
  excontrol = (struct ExAllControl*) AllocDosObject( DOS_EXALLCONTROL, NULL );
  excontrol->eac_LastKey = 0;

  do
  {
    exmore = ExAll( sourcelock, &buffer, sizeof( buffer ),
	ED_NAME, excontrol );

    if( !exmore && IoErr() != ERROR_NO_MORE_ENTRIES )
    {
      /* Abnormal abort */
      break;
    }

    if( excontrol->eac_Entries == 0 )
    {
      continue;
    }

    ead = &buffer;

    do
    {
      if( ead->ed_Name[0] != '.' )
      {
	int curr = strtol( (const char*) ead->ed_Name, 0, 10 );
	high_num = curr > high_num ? curr : high_num;
      }

      ead = ead->ed_Next;
    }
    while( ead );

  }
  while( exmore );

  FreeDosObject( DOS_EXALLCONTROL, excontrol );
  UnLock( sourcelock );
  ++high_num;
  sprintf( filename, "%s%d.log", directory, high_num );
  return filename;
}

void os_setup( void )
{
  out_stream = fopen( get_next_filename( (CONST_STRPTR) "PROGDIR:log/" ), "w+" );

  if( !( SocketBase = OpenLibrary( (CONST_STRPTR) "bsdsocket.library", 2 ) ) )
  {
    fprintf( out_stream, "%s (%s:%d) - Failed to open bsdsocket.library v2+\n",
	__FUNCTION__, __FILE__, __LINE__ );
    exit( 1 );
  }

  if( !( UserGroupBase = OpenLibrary( (CONST_STRPTR) "usergroup.library", 0 ) ) )
  {
    fprintf( out_stream, "%s (%s:%d) - Failed to open usergroup.library\n",
	__FUNCTION__, __FILE__, __LINE__ );
    exit( 1 );
  }

  if( !( UtilityBase = (struct UtilityBase*) OpenLibrary( (CONST_STRPTR) "utility.library", 0 ) ) )
  {
    fprintf( out_stream, "%s (%s:%d) - Failed to open utility.library\n",
	__FUNCTION__, __FILE__, __LINE__ );
    exit( 1 );
  }
}

void os_cleanup( void )
{
  if( UtilityBase )
  {
    CloseLibrary( (struct Library*) UtilityBase );
    UtilityBase = NULL;
  }

  if( UserGroupBase )
  {
    CloseLibrary( UserGroupBase );
    UserGroupBase = NULL;
  }

  if( SocketBase )
  {
    CloseLibrary( SocketBase );
    SocketBase = NULL;
  }

  if( out_stream )
  {
    fclose( out_stream );
    out_stream = 0;
  }
}

int set_nonblocking( SOCKET sock )
{
  char optval = 1;
  return IoctlSocket( sock, FIONBIO, &optval );
}

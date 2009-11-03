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
#include "os.h"

struct Library *SocketBase = NULL;
struct Library *UserGroupBase = NULL;
struct UtilityBase *UtilityBase = NULL;
extern FILE *out_stream;

void network_startup( void )
{
  out_stream = fopen( "CON:800/800/640/480/SWR Factor 2.0/AUTO/CLOSE/INACTIVE", "a" );

  if( !( SocketBase = OpenLibrary( "bsdsocket.library", 2 ) ) )
    {
      fprintf( out_stream, "%s (%s:%d) - Failed to open bsdsocket.library v2+\n",
	       __FUNCTION__, __FILE__, __LINE__ );
      exit( 1 );
    }

  if( !( UserGroupBase = OpenLibrary( "usergroup.library", 0 ) ) )
    {
      fprintf( out_stream, "%s (%s:%d) - Failed to open usergroup.library\n",
	       __FUNCTION__, __FILE__, __LINE__ );
      exit( 1 );
    }

  if( !( UtilityBase = (struct UtilityBase*) OpenLibrary( "utility.library", 0 ) ) )
    {
      fprintf( out_stream, "%s (%s:%d) - Failed to open utility.library\n",
	       __FUNCTION__, __FILE__, __LINE__ );
      exit( 1 );
    }
}

void network_teardown( void )
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

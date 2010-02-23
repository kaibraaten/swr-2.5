#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "mud.h"

VOTE_DATA *first_poll = NULL;
VOTE_DATA *last_poll = NULL;

/* local routines */
static void fread_poll( VOTE_DATA * vote, FILE * fp );
static bool load_poll_file( const char *pollfile );

/*
 * Save a poll's data to its data file
 */
void save_poll( const VOTE_DATA *vote )
{
  FILE *fp;
  char player[256];

  if( !vote )
  {
    bug( "save_poll: null poll pointer!", 0 );
    return;
  }

  if( !vote->player || vote->player[0] == '\0' )
    return;

  sprintf( player, "%s%s", VOTE_DIR, vote->player );

  if( ( fp = fopen( player, "w" ) ) == NULL )
  {
    bug( "save_poll: fopen", 0 );
    perror( player );
  }
  else
  {
    fprintf( fp, "#VOTE\n" );
    fprintf( fp, "Player       %s~\n", vote->player );
    fprintf( fp, "Type         %d\n", vote->type );
    fprintf( fp, "Infavour     %d\n", vote->in_favour );
    fprintf( fp, "Against      %d\n", vote->against );
    fprintf( fp, "Timeofvote   %s~\n", vote->time_of_vote );
    fprintf( fp, "Iplist       %s~\n", vote->ip_list );
    fprintf( fp, "Namelist     %s~\n", vote->name_list );
    fprintf( fp, "End\n\n" );
    fprintf( fp, "#END\n" );
  }
  fclose( fp );
}

/*
 * Read in actual poll data.
 */
void fread_poll( VOTE_DATA *vote, FILE * fp )
{
  char buf[MAX_STRING_LENGTH];
  const char *word;
  bool fMatch;

  for( ;; )
  {
    word = feof( fp ) ? "End" : fread_word( fp );
    fMatch = FALSE;

    switch ( UPPER( word[0] ) )
    {
      case '*':
	fMatch = TRUE;
	fread_to_eol( fp );
	break;

      case 'A':
	KEY( "Against", vote->against, fread_number( fp ) );
	break;

      case 'E':
	if( !str_cmp( word, "End" ) )
	{
	  if( !vote->time_of_vote )
	    vote->time_of_vote = STRALLOC( "" );
	  if( !vote->ip_list )
	    vote->ip_list = STRALLOC( "" );
	  if( !vote->name_list )
	    vote->name_list = STRALLOC( "" );
	  return;
	}
	break;

      case 'I':
	KEY( "Infavour", vote->in_favour, fread_number( fp ) );
	KEY( "Iplist", vote->ip_list, fread_string( fp ) );
	break;

      case 'N':
	KEY( "Namelist", vote->name_list, fread_string( fp ) );
	break;

      case 'P':
	KEY( "Player", vote->player, fread_string( fp ) );
	break;

      case 'T':
	KEY( "Timeofvote", vote->time_of_vote, fread_string( fp ) );
	KEY( "Type", vote->type, fread_number( fp ) );
	break;

    }

    if( !fMatch )
    {
      sprintf( buf, "Fread_poll: no match: %s", word );
      bug( buf, 0 );
    }

  }
}

/*
 * Load a poll file
 */

bool load_poll_file( const char *pollfile )
{
  char player[256];
  VOTE_DATA *vote = NULL;
  FILE *fp = NULL;
  bool found = FALSE;

  CREATE( vote, VOTE_DATA, 1 );

  sprintf( player, "%s%s", VOTE_DIR, pollfile );

  if( ( fp = fopen( player, "r" ) ) != NULL )
  {

    found = TRUE;
    for( ;; )
    {
      char letter;
      char *word;

      letter = fread_letter( fp );
      if( letter == '*' )
      {
	fread_to_eol( fp );
	continue;
      }

      if( letter != '#' )
      {
	bug( "Load_poll_file: # not found.", 0 );
	break;
      }

      word = fread_word( fp );
      if( !str_cmp( word, "VOTE" ) )
      {
	fread_poll( vote, fp );
	break;
      }
      else if( !str_cmp( word, "END" ) )
	break;
      else
      {
	char buf[MAX_STRING_LENGTH];

	sprintf( buf, "Load_poll_file: bad section: %s.", word );
	bug( buf, 0 );
	break;
      }
    }
    fclose( fp );
  }

  if( found )
  {
    LINK( vote, first_poll, last_poll, next, prev );
    return found;
  }
  else
    DISPOSE( vote );

  return found;
}


/*
 * Load in all the poll files.
 */
void load_polls( void )
{
  FILE *fpList;
  const char *player;
  char polllist[256];
  char buf[MAX_STRING_LENGTH];

  first_poll = NULL;
  last_poll = NULL;

  log_string( "Loading polls..." );

  sprintf( polllist, "%s%s", VOTE_DIR, VOTE_LIST );

  if( ( fpList = fopen( polllist, "r" ) ) == NULL )
  {
    perror( polllist );
    exit( 1 );
  }

  for( ;; )
  {
    player = feof( fpList ) ? "$" : fread_word( fpList );
    log_string( player );
    if( player[0] == '$' )
      break;

    if( !load_poll_file( player ) )
    {
      sprintf( buf, "Cannot load poll file: %s", player );
      bug( buf, 0 );
    }
  }
  fclose( fpList );
  log_string( " Done polls\r\n" );
}

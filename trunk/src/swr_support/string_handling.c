#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "swr_support.h"

#define HIDDEN_TILDE    '*'

void bug( const char *str, ... );

/*
 * See if a string is one of the names of an object.
 */
bool is_name( const char *str, char *namelist )
{
  char name[MAX_INPUT_LENGTH];

  for ( ; ; )
    {
      namelist = one_argument( namelist, name );
      if ( name[0] == '\0' )
	return FALSE;
      if ( !str_cmp( str, name ) )
	return TRUE;
    }
}

bool is_name_prefix( const char *str, char *namelist )
{
  char name[MAX_INPUT_LENGTH];

  for ( ; ; )
    {
      namelist = one_argument( namelist, name );
      if ( name[0] == '\0' )
	return FALSE;
      if ( !str_prefix( str, name ) )
	return TRUE;
    }
}

/*
 * See if a string is one of the names of an object.            -Thoric
 * Treats a dash as a word delimiter as well as a space
 */
bool is_name2( const char *str, char *namelist )
{
  char name[MAX_INPUT_LENGTH];

  for ( ; ; )
    {
      namelist = one_argument2( namelist, name );
      if ( name[0] == '\0' )
	return FALSE;
      if ( !str_cmp( str, name ) )
	return TRUE;
    }
}

bool is_name2_prefix( const char *str, char *namelist )
{
  char name[MAX_INPUT_LENGTH];

  for ( ; ; )
    {
      namelist = one_argument2( namelist, name );
      if ( name[0] == '\0' )
	return FALSE;
      if ( !str_prefix( str, name ) )
	return TRUE;
    }
}

/*                                                              -Thoric
 * Checks if str is a name in namelist supporting multiple keywords
 */
bool nifty_is_name( char *str, char *namelist )
{
  char name[MAX_INPUT_LENGTH];

  if ( !str || str[0] == '\0' )
    return FALSE;

  for ( ; ; )
    {
      str = one_argument2( str, name );
      if ( name[0] == '\0' )
	return TRUE;
      if ( !is_name2( name, namelist ) )
	return FALSE;
    }
}

bool nifty_is_name_prefix( char *str, char *namelist )
{
  char name[MAX_INPUT_LENGTH];

  if ( !str || str[0] == '\0' )
    return FALSE;

  for ( ; ; )
    {
      str = one_argument2( str, name );
      if ( name[0] == '\0' )
	return TRUE;
      if ( !is_name2_prefix( name, namelist ) )
	return FALSE;
    }
}

/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde( char *str )
{
  for ( ; *str != '\0'; str++ )
    if ( *str == '~' )
      *str = '-';

  return;
}

/*
 * Encodes the tildes in a string.                              -Thoric
 * Used for player-entered strings that go into disk files.
 */
void hide_tilde( char *str )
{
  for ( ; *str != '\0'; str++ )
    if ( *str == '~' )
      *str = HIDDEN_TILDE;

  return;
}

char *show_tilde( char *str )
{
  static char buf[MAX_STRING_LENGTH];
  char *bufptr;

  bufptr = buf;
  for ( ; *str != '\0'; str++, bufptr++ )
    {
      if ( *str == HIDDEN_TILDE )
        *bufptr = '~';
      else
        *bufptr = *str;
    }
  *bufptr = '\0';

  return buf;
}

/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr )
{
  if ( !astr )
    {
      bug( "Str_cmp: null astr." );
      if ( bstr )
	fprintf( stderr, "str_cmp: astr: (null)  bstr: %s\n", bstr );
      return TRUE;
    }

  if ( !bstr )
    {
      bug( "Str_cmp: null bstr." );
      if ( astr )
	fprintf( stderr, "str_cmp: astr: %s  bstr: (null)\n", astr );
      return TRUE;
    }

  for ( ; *astr || *bstr; astr++, bstr++ )
    {
      if ( LOWER(*astr) != LOWER(*bstr) )
	return TRUE;
    }

  return FALSE;
}

/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
  if ( !astr )
    {
      bug( "Strn_cmp: null astr." );
      return TRUE;
    }

  if ( !bstr )
    {
      bug( "Strn_cmp: null bstr." );
      return TRUE;
    }

  for ( ; *astr; astr++, bstr++ )
    {
      if ( LOWER(*astr) != LOWER(*bstr) )
	return TRUE;
    }

  return FALSE;
}

/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix( const char *astr, const char *bstr )
{
  int sstr1;
  int sstr2;
  int ichar;
  char c0;

  if ( ( c0 = LOWER(astr[0]) ) == '\0' )
    return FALSE;

  sstr1 = strlen(astr);
  sstr2 = strlen(bstr);

  for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
    if ( c0 == LOWER(bstr[ichar]) && !str_prefix( astr, bstr + ichar ) )
      return FALSE;

  return TRUE;
}

/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
  int sstr1;
  int sstr2;

  sstr1 = strlen(astr);
  sstr2 = strlen(bstr);
  if ( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
    return FALSE;
  else
    return TRUE;
}

/*
 * Returns an initial-capped string.
 * Rewritten by FearItself@AvP
 */
char *capitalize( const char *str )
{
  static char buf[MAX_STRING_LENGTH];
  char *dest = buf;
  enum { Normal, Color } state = Normal;
  bool bFirst = TRUE;
  char c;

  while( (c = *str++) )
    {
      if( state == Normal )
        {
          if( c == '&' || c == '^' || c == '}' )
            {
              state = Color;
            }
          else if( isalpha( (int) c ) )
            {
              c = bFirst ? toupper( (int) c )  : tolower( (int) c );
              bFirst = FALSE;
            }
        }
      else
        {
          state = Normal;
        }

      *dest++ = c;
    }

  *dest = c;

  return buf;
}

/*
 * Returns a lowercase string.
 */
char *strlower( const char *str )
{
  static char strlow[MAX_STRING_LENGTH];
  int i;

  for ( i = 0; str[i] != '\0'; i++ )
    strlow[i] = LOWER(str[i]);
  strlow[i] = '\0';
  return strlow;
}

/*
 * Returns an uppercase string.
 */
char *strupper( const char *str )
{
  static char strup[MAX_STRING_LENGTH];
  int i;

  for ( i = 0; str[i] != '\0'; i++ )
    strup[i] = UPPER(str[i]);
  strup[i] = '\0';
  return strup;
}

/*
 * Returns TRUE or FALSE if a letter is a vowel                 -Thoric
 */
bool isavowel( char letter )
{
  char c;

  c = tolower( (int)letter );
  if ( c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' )
    return TRUE;
  else
    return FALSE;
}

/*
 * Shove either "a " or "an " onto the beginning of a string    -Thoric
 */
const char *aoran( const char *str )
{
  static char temp[MAX_STRING_LENGTH];

  if ( !str )
    {
      bug( "Aoran(): NULL str" );
      return "";
    }

  if ( isavowel(str[0])
       || ( strlen(str) > 1 && tolower((int)str[0]) == 'y'
	    && !isavowel(str[1])) )
    strcpy( temp, "an " );
  else
    strcpy( temp, "a " );
  strcat( temp, str );
  return temp;
}

void replace_char( char *buf, char replace, char with )
{
  size_t i = 0;

  for( i = 0; i < strlen( buf ); ++i )
    {
      if( buf[i] == replace )
        {
          buf[i] = with;
        }
    }
}

/*
 * Return true if an argument is completely numeric.
 */
bool is_number( const char *arg )
{
  if ( *arg == '\0' )
    return FALSE;

  for ( ; *arg != '\0'; arg++ )
    {
      if ( !isdigit((int) *arg) )
	return FALSE;
    }

  return TRUE;
}

/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( const char *orig_argument, char *arg )
{
  char *pdot;
  int number;
  char argument[MAX_STRING_LENGTH];
  sprintf( argument, "%s", orig_argument );

  for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
      if ( *pdot == '.' )
        {
          *pdot = '\0';
          number = atoi( argument );
          *pdot = '.';
          strcpy( arg, pdot+1 );
          return number;
        }
    }

  strcpy( arg, argument );
  return 1;
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument( char *argument, char *arg_first )
{
  char cEnd = ' ';
  short count = 0;

  while ( isspace((int) *argument) )
    argument++;

  if ( *argument == '\'' || *argument == '"' )
    cEnd = *argument++;

  while ( *argument != '\0' || ++count >= 255 )
    {
      if ( *argument == cEnd )
        {
          argument++;
          break;
        }

      *arg_first = *argument;
      arg_first++;
      argument++;
    }

  *arg_first = '\0';

  while ( isspace((int) *argument) )
    argument++;

  return argument;
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.  Delimiters = { ' ', '-' }
 */
char *one_argument2( char *argument, char *arg_first )
{
  char cEnd = ' ';
  short count = 0;

  while ( isspace((int) *argument) )
    argument++;

  if ( *argument == '\'' || *argument == '"' )
    cEnd = *argument++;

  while ( *argument != '\0' || ++count >= 255 )
    {
      if ( *argument == cEnd || *argument == '-' )
        {
          argument++;
          break;
        }

      *arg_first = *argument;
      arg_first++;
      argument++;
    }

  *arg_first = '\0';

  while ( isspace((int) *argument) )
    argument++;

  return argument;
}

/*
 * Remove carriage returns from a line
 */
char *strip_cr( const char *str )
{
  static char newstr[MAX_STRING_LENGTH];
  int i, j;

  for ( i=j=0; str[i] != '\0'; i++ )
    if ( str[i] != '\r' )
      {
	newstr[j++] = str[i];
      }
  newstr[j] = '\0';
  return newstr;
}

/*
 * Removes the tildes from a line, except if it's the last character.
 */
void smush_tilde( char *str )
{
  int len;
  char last;
  char *strptr;

  strptr = str;

  len  = strlen( str );
  if ( len )
    last = strptr[len-1];
  else
    last = '\0';

  for ( ; *str != '\0'; str++ )
    {
      if ( *str == '~' )
	*str = '-';
    }
  if ( len )
    strptr[len-1] = last;
}

static char *grab_word( char *argument, char *arg_first )
{
  char cEnd =  ' ';
  short count = 0;

  while ( isspace((int)*argument) )
    argument++;

  if ( *argument == '\'' || *argument == '"' )
    cEnd = *argument++;

  while ( *argument != '\0' || ++count >= 255 )
    {
      if ( *argument == cEnd )
        {
          argument++;
          break;
        }

      *arg_first++ = *argument++;
    }

  *arg_first = '\0';

  while ( isspace((int)*argument) )
    argument++;

  return argument;
}

char *wordwrap( char *txt, short wrap )
{
  static char buf[MAX_STRING_LENGTH];
  char *bufp;

  buf[0] = '\0';
  bufp = buf;
  if ( txt != NULL )
    {
      char line[MAX_STRING_LENGTH];
      char temp[MAX_STRING_LENGTH];
      char *ptr, *p;
      int ln, x;

      ++bufp;
      line[0] = '\0';
      ptr = txt;
      while ( *ptr )
        {
          ptr = grab_word( ptr, temp );
          ln = strlen( line );  x = strlen( temp );
          if ( (ln + x + 1) < wrap )
	    {
	      if ( line[ln-1] == '.' )
		strcat( line, "  " );
	      else
		strcat( line, " " );
	      strcat( line, temp );
	      p = strchr( line, '\n' );
	      if ( !p )
		p = strchr( line, '\r' );
	      if ( p )
		{
		  strcat( buf, line );
		  line[0] = '\0';
		}
	    }
          else
	    {
	      strcat( line, "\r\n" );
	      strcat( buf, line );
	      strcpy( line, temp );
	    }
        }
      if ( line[0] != '\0' )
	strcat( buf, line );
    }
  return bufp;
}

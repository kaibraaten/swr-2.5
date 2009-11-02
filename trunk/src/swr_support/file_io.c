#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#include "swr_support.h"

/*
 * Uni-directional dependencies. Fix that.
 */
#ifdef __cplusplus
extern "C" {
#endif
void bug( const char *str, ... );
void shutdown_mud( const char *reason );
#ifdef __cplusplus
}
#endif
/*
 * Added lots of EOF checks, as most of the file crashes are based on them.
 * If an area file encounters EOF, the fread_* functions will shutdown the
 * MUD, as all area files should be read in in full or bad things will
 * happen during the game.  Any files loaded in without fBootDb which
 * encounter EOF will return what they have read so far.   These files
 * should include player files, and in-progress areas that are not loaded
 * upon bootup.
 * -- Altrag
 */

extern bool fBootDb;

/*
 * Read a letter from a file.
 */
char fread_letter( FILE *fp )
{
  char c;

  do
    {
      if ( feof(fp) )
        {
          bug("fread_letter: EOF encountered on read.\r\n");
          if ( fBootDb )
            exit(1);

          return '\0';
        }

      c = getc( fp );
    }
  while ( isspace((int) c) );

  return c;
}

/*
 * Read a float number from a file. Turn the result into a float value.
 */
float fread_float( FILE *fp )
{
  float number;
  bool sign, decimal;
  char c;
  double place = 0;

   do
     {
       if( feof( fp ) )
         {
           bug( "%s: EOF encountered on read.", __FUNCTION__ );
           if( fBootDb )
             {
               shutdown_mud( "Corrupt file somewhere." );
               exit( 1 );
             }
           return 0;
         }
       c = getc( fp );
     }
   while( isspace( (int) c ) );

   number = 0;

   sign = FALSE;
   decimal = FALSE;

   if( c == '+' )
     c = getc( fp );
   else if( c == '-' )
     {
       sign = TRUE;
       c = getc( fp );
     }

   if( !isdigit( (int) c ) )
     {
       bug( "%s: bad format. (%c)", __FUNCTION__, c );

       if( fBootDb )
         exit( 1 );
       return 0;
     }

   while( 1 )
     {
       if( c == '.' || isdigit( (int) c ) )
         {
           if( c == '.' )
             {
               decimal = TRUE;
               c = getc( fp );
             }

           if( feof( fp ) )
             {
               bug( "%s: EOF encountered on read.", __FUNCTION__ );
               if( fBootDb )
                 exit( 1 );
               return number;
             }
           if( !decimal )
             number = number * 10 + c - '0';
           else
             {
               place++;
               number += pow( (double) 10, ( -1 * place ) ) * ( c - '0' );
             }
           c = getc( fp );
         }
       else
         break;
     }

   if( sign )
     number = 0 - number;

   if( c == '|' )
     number += fread_float( fp );
   else if( c != ' ' )
     ungetc( c, fp );

   return number;
}

/*
 * Read a number from a file.
 */
int fread_number( FILE *fp )
{
  int number;
  bool sign;
  char c;

    do
      {
        if ( feof(fp) )
	  {
	    bug("fread_number: EOF encountered on read.\r\n");
	    if ( fBootDb )
	      exit(1);
	    return 0;
	  }
        c = getc( fp );
      }
    while ( isspace( (int) c ) );

    number = 0;

    sign   = FALSE;
    if ( c == '+' )
      {
        c = getc( fp );
      }
    else if ( c == '-' )
      {
        sign = TRUE;
        c = getc( fp );
      }

    if ( !isdigit((int) c) )
      {
        bug( "Fread_number: bad format. (%c)", c );
        if ( fBootDb )
          exit( 1 );
        return 0;
      }

    while ( isdigit( (int) c ) )
      {
        if ( feof(fp) )
	  {
	    bug("fread_number: EOF encountered on read.\r\n");
	    if ( fBootDb )
	      exit(1);
	    return number;
	  }
        number = number * 10 + c - '0';
        c      = getc( fp );
      }

    if ( sign )
      number = 0 - number;

    if ( c == '|' )
      number += fread_number( fp );
    else if ( c != ' ' )
      ungetc( c, fp );

    return number;
}


/*
 * custom str_dup using create                                  -Thoric
 */
char *str_dup( const char *str )
{
  static char *ret = NULL;
  size_t len = 0;

  if ( !str )
    return NULL;

  len = strlen(str)+1;

  CREATE( ret, char, len );
  strcpy( ret, str );
  return ret;
}

/*
 * Read a string from file fp
 */
char *fread_string( FILE *fp )
{
  char buf[MAX_STRING_LENGTH];
  char *plast;
  char c;
  int ln;

  plast = buf;
  buf[0] = '\0';
  ln = 0;

  /*
   * Skip blanks.
   * Read first char.
   */
    do
      {
        if ( feof(fp) )
	  {
            bug("fread_string: EOF encountered on read.\r\n");
            if ( fBootDb )
	      exit(1);
            return STRALLOC("");
	  }
        c = getc( fp );
      }
    while ( isspace((int)c) );

    if ( ( *plast++ = c ) == '~' )
      return STRALLOC( "" );

    for ( ;; )
      {
        if ( ln >= (MAX_STRING_LENGTH - 1) )
	  {
	    bug( "fread_string: string too long" );
	    *plast = '\0';
	    return STRALLOC( buf );
	  }
        switch ( *plast = getc( fp ) )
	  {
	  default:
            plast++; ln++;
            break;

	  case EOF:
            bug( "Fread_string: EOF" );
            if ( fBootDb )
              exit( 1 );
            *plast = '\0';
            return STRALLOC(buf);
            break;

	  case '\n':
            plast++;  ln++;
            *plast++ = '\r';  ln++;
            break;

	  case '\r':
            break;

	  case '~':
            *plast = '\0';
            return STRALLOC( buf );
	  }
      }
}

/*
 * Read a string from file fp using str_dup (ie: no string hashing)
 */
char *fread_string_nohash( FILE *fp )
{
  char buf[MAX_STRING_LENGTH];
  char *plast;
  char c;
  int ln;

  plast = buf;
  buf[0] = '\0';
  ln = 0;

  /*
   * Skip blanks.
   * Read first char.
   */
    do
      {
        if ( feof(fp) )
	  {
            bug("fread_string_no_hash: EOF encountered on read.\r\n");
            if ( fBootDb )
	      exit(1);
            return str_dup("");
	  }
        c = getc( fp );
      }
    while ( isspace((int)c) );

    if ( ( *plast++ = c ) == '~' )
      return str_dup( "" );

    for ( ;; )
      {
        if ( ln >= (MAX_STRING_LENGTH - 1) )
	  {
	    bug( "fread_string_no_hash: string too long" );
	    *plast = '\0';
	    return str_dup( buf );
	  }
        switch ( *plast = getc( fp ) )
	  {
	  default:
            plast++; ln++;
            break;

	  case EOF:
            bug( "Fread_string_no_hash: EOF" );
            if ( fBootDb )
              exit( 1 );
            *plast = '\0';
            return str_dup(buf);
            break;

	  case '\n':
            plast++;  ln++;
            *plast++ = '\r';  ln++;
            break;

	  case '\r':
            break;

	  case '~':
            *plast = '\0';
            return str_dup( buf );
	  }
      }
}

/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE *fp )
{
  char c;

    do
      {
        if ( feof(fp) )
	  {
            bug("fread_to_eol: EOF encountered on read.\r\n");
            if ( fBootDb )
	      exit(1);
            return;
	  }
        c = getc( fp );
      }
    while ( c != '\n' && c != '\r' );

    do
      {
        c = getc( fp );
      }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    return;
}

/*
 * Read to end of line into static buffer                       -Thoric
 */
char *fread_line( FILE *fp )
{
  static char line[MAX_STRING_LENGTH];
  char *pline;
  char c;
  int ln;

  pline = line;
  line[0] = '\0';
  ln = 0;

  /*
   * Skip blanks.
   * Read first char.
   */
    do
      {
        if ( feof(fp) )
	  {
            bug("fread_line: EOF encountered on read.\r\n");
            if ( fBootDb )
	      exit(1);
            strcpy(line, "");
            return line;
	  }
        c = getc( fp );
      }
    while ( isspace((int)c) );

    ungetc( c, fp );
    do
      {
        if ( feof(fp) )
	  {
            bug("fread_line: EOF encountered on read.\r\n");
            if ( fBootDb )
	      exit(1);
            *pline = '\0';
	    return line;
	  }
        c = getc( fp );
        *pline++ = c; ln++;
        if ( ln >= (MAX_STRING_LENGTH - 1) )
	  {
            bug( "fread_line: line too long" );
            break;
	  }
      }
    while ( c != '\n' && c != '\r' );

    do
      {
        c = getc( fp );
      }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    *pline = '\0';
    return line;
}

/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE *fp )
{
  static char word[MAX_INPUT_LENGTH];
  char *pword;
  char cEnd;

    do
      {
        if ( feof(fp) )
	  {
            bug("fread_word: EOF encountered on read.\r\n");
            if ( fBootDb )
	      exit(1);
            word[0] = '\0';
            return word;
	  }
        cEnd = getc( fp );
      }
    while ( isspace( (int) cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
      {
        pword   = word;
      }
    else
      {
        word[0] = cEnd;
        pword   = word+1;
        cEnd    = ' ';
      }

    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
      {
        if ( feof(fp) )
	  {
            bug("fread_word: EOF encountered on read.\r\n");
            if ( fBootDb )
	      exit(1);
            *pword = '\0';
            return word;
	  }
        *pword = getc( fp );
        if ( cEnd == ' ' ? isspace((int) *pword) : *pword == cEnd )
	  {
            if ( cEnd == ' ' )
	      ungetc( *pword, fp );
            *pword = '\0';
            return word;
	  }
      }

    bug( "Fread_word: word too long" );
    exit( 1 );
    return NULL;
}

/*
 * Append a string to a file.
 */
void append_to_file( const char *file, const char *str )
{
  FILE *fp;

  if ( ( fp = fopen( file, "a" ) ) )
    {
      fprintf( fp, "%s\n", str );
      fclose( fp );
    }
}

/*
 * Dynamically expanding text buffer by Erwin Adreasen.
 * This version was lifted from SocketMUD 2.4
 */
#ifndef _SWR2_BUFFER_H_
#define _SWR2_BUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct buffer_type
{
  char   * data;        /* The data                      */
  int      len;         /* The current len of the buffer */
  int      size;        /* The allocated size of data    */
} BUFFER;

BUFFER *buffer_new( int size );
void buffer_strcat( BUFFER *buffer, const char *text );
void buffer_free( BUFFER *buffer );
void buffer_clear( BUFFER *buffer );
int bprintf( BUFFER *buffer, char *fmt, ... );

#ifdef __cplusplus
}
#endif

#endif /* include guard */

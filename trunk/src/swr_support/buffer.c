#include "swr_support.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif
void bug( const char *str, ... );
#ifdef __cplusplus
}
#endif

/*
 * Create a new buffer.
 */
BUFFER *buffer_new(int size)
{
  BUFFER *buffer = malloc(sizeof(BUFFER));
  buffer->size = size;
  buffer->data = malloc(size);
  buffer->len = 0;
  return buffer;
}

/*
 * Add a string to a buffer. Expand if necessary
 */
void buffer_strcat(BUFFER *buffer, const char *text)
{
  int new_size = 0;
  int text_len = 0;
  char *new_data = NULL;

  /* Adding NULL string ? */
  if (!text)
    return;

  text_len = strlen(text);

  /* Adding empty string ? */
  if (text_len == 0)
    return;

  /* Will the combined len of the added text and the
     current text exceed our buffer? */
  if ((text_len + buffer->len + 1) > buffer->size)
    {
      new_size = buffer->size + text_len + 1;

      /* Allocate the new buffer */
      new_data = malloc(new_size);

      /* Copy the current buffer to the new buffer */
      memcpy(new_data, buffer->data, buffer->len);
      free(buffer->data);
      buffer->data = new_data;
      buffer->size = new_size;
    }

  memcpy(buffer->data + buffer->len, text, text_len);
  buffer->len += text_len;
  buffer->data[buffer->len] = '\0';
}

/* free a buffer */
void buffer_free(BUFFER *buffer)
{
  /* Free data */
  free(buffer->data);

  /* Free buffer */
  free(buffer);
}

/* Clear a buffer's contents, but do not deallocate anything */
void buffer_clear(BUFFER *buffer)
{
  buffer->len = 0;
  buffer->data[0] = '\0';
}

/* print stuff, append to buffer. safe. */
int bprintf(BUFFER *buffer, char *fmt, ...)
{
  char buf[MAX_STRING_LENGTH];
  va_list va;
  int res = 0;

  va_start(va, fmt);
  res = vsnprintf(buf, MAX_STRING_LENGTH, fmt, va);
  va_end(va);

  if (res >= MAX_STRING_LENGTH - 1)
    {
      buf[0] = '\0';
      bug("Overflow when printing string %s", fmt);
    }
  else
    buffer_strcat(buffer, buf);

  return res;
}

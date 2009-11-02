#include <stdio.h>
#include "os.h"

extern FILE *out_stream;

void network_startup( void )
{
  out_stream = stderr;
}

void network_teardown( void )
{

}

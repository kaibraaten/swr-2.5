#include <time.h>

struct tm *update_time ( struct tm *old_time )
{
  time_t t = mktime(old_time);
  return localtime(&t);
}

#include <iostream>
#include <iomanip>
#include <sstream>
#include "date.h"
using namespace std;

bool string_to_date(const char *s, Date &d)
{
  int y, m, dd;

  int res = sscanf(s, "%d-%d-%d", &y, &m, &dd);  // not check return value eq 3, lex guarantee

  if (res != 3)
    return false;

  d.year  = y;
  d.month = m;
  d.day   = dd;

  if (d.month < 1 || d.month > 12)
    return false;
  if (d.day < 1 ||
      d.day > monthday[d.month] + (d.month == 2 && (d.year % 400 == 0 || (d.year % 4 == 0 && d.year % 100 != 0))))
    return false;

  return true;
}

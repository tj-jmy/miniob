#pragma once

using namespace std;

const int monthday[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

typedef struct Date
{
  uint16_t year;
  uint8_t  month;
  uint8_t  day;
} Date;

bool string_to_date(const char *s, Date &d);

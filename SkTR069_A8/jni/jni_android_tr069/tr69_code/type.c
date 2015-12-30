// $Id: type.c 12 2011-02-18 04:05:43Z cedric.shih@gmail.com $
/*
 * Copyright (C) 2010 AXIM Communications Inc.
 * Copyright (C) 2010 Cedric Shih <cedric.shih@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "log.h"
#include "util.h"

#include "type.h"


#ifdef WIN32
static char * strptime(const char *buf, const char *fmt, struct tm *tm);
#endif

const char *evcpe_type_to_str(enum evcpe_type type)
{
	switch (type) {
	case EVCPE_TYPE_OBJECT:
		return "object";
	case EVCPE_TYPE_MULTIPLE:
		return "multipleObject";
	case EVCPE_TYPE_STRING:
		return "string";
	case EVCPE_TYPE_INT:
		return "int";
	case EVCPE_TYPE_UNSIGNED_INT:
		return "unsignedInt";
	case EVCPE_TYPE_BOOLEAN:
		return "boolean";
	case EVCPE_TYPE_DATETIME:
		return "dateTime";
	case EVCPE_TYPE_BASE64:
		return "base64";
	case EVCPE_TYPE_UNKNOWN:
	default:
		return "unknown";
	}
}

int evcpe_type_validate(enum evcpe_type type, const char *value, unsigned len,
		struct evcpe_constraint *cons)
{
	int rc;
	long val;
	char *dup;
	struct tm tm;

	evcpe_debug(__func__, "validating value of type: %s",
			evcpe_type_to_str(type));

	switch(type) {
	case EVCPE_TYPE_STRING:
	case EVCPE_TYPE_BOOLEAN:	
		break;
	case EVCPE_TYPE_BASE64:
		// TODO
		break;
	case EVCPE_TYPE_INT:
	case EVCPE_TYPE_UNSIGNED_INT:
		if ((rc = evcpe_atol(value, len, &val))) 
		{
			evcpe_error(__func__, "failed to convert to "
					"integer: %.*s", len, value);
			goto finally;
		}
		
		if (type == EVCPE_TYPE_UNSIGNED_INT) 
		{
			if (val < 0) {
				evcpe_error(__func__, "not a positive integer: %ld", val);
				goto finally;
			}
		} 

		switch(cons->type) {
		case EVCPE_CONSTRAINT_NONE:
			break;
		case EVCPE_CONSTRAINT_MIN:
		case EVCPE_CONSTRAINT_MAX:
		case EVCPE_CONSTRAINT_RANGE:
			if (cons->type != EVCPE_CONSTRAINT_MAX &&
					val < cons->value.range.min) {
				evcpe_error(__func__, "value out of range: %ld < %ld",
						val, cons->value.range.min);
				rc = EINVAL;
				goto finally;
			}
			if (cons->type != EVCPE_CONSTRAINT_MIN &&
					val > cons->value.range.max) {
				evcpe_error(__func__, "value out of range: %ld > %ld",
						val, cons->value.range.max);
				rc = EINVAL;
				goto finally;
			}
			break;
		default:
			evcpe_error(__func__, "unexpected constraint type: %d", cons->type);
			rc = EINVAL;
			goto finally;
		}
		break;
	case EVCPE_TYPE_DATETIME:
		if (!(dup = malloc(len + 1))) {
			evcpe_error(__func__, "failed to malloc: %d bytes", len + 1);
			rc = ENOMEM;
			goto finally;
		}
		memcpy(dup, value, len);
		dup[len] = '\0';
		if (!strptime(dup, "%Y-%m-%dT%H:%M:%S", &tm)
				&& !strptime(dup, "%Y-%m-%dT%H:%M:%S%z", &tm)) {
			evcpe_error(__func__, "failed to parse dateTime: %s", dup);
			free(dup);
			rc = EINVAL;
			goto finally;
		}
		free(dup);
		break;
	case EVCPE_TYPE_UNKNOWN:
	case EVCPE_TYPE_OBJECT:
	case EVCPE_TYPE_MULTIPLE:
	default:
		evcpe_error(__func__, "value is not applicable to "
				"type: %d", type);
		rc = EINVAL;
		goto finally;
	}
	rc = 0;

finally:
	return rc;
}

 
 #ifdef WIN32
/*
 * We do not implement alternate representations. However, we always
 * check whether a given modifier is allowed for a certain conversion.
 */
#define ALT_E          0x01
#define ALT_O          0x02
//#define LEGAL_ALT(x)       { if (alt_format & ~(x)) return (0); }
#define LEGAL_ALT(x)       { ; }
#define TM_YEAR_BASE   (1970)
 
static   int conv_num(const char **, int *, int, int);

static const char *day[7] = {
     "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
     "Friday", "Saturday"
};
static const char *abday[7] = {
     "Sun","Mon","Tue","Wed","Thu","Fri","Sat"
};
static const char *mon[12] = {
     "January", "February", "March", "April", "May", "June", "July",
     "August", "September", "October", "November", "December"
};
static const char *abmon[12] = {
     "Jan", "Feb", "Mar", "Apr", "May", "Jun",
     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
static const char *am_pm[2] = {
     "AM", "PM"
};
 
 
static char * strptime(const char *buf, const char *fmt, struct tm *tm)
{
     char c;
     const char *bp;
     size_t len = 0;
     int alt_format, i, split_year = 0;
 
     bp = buf;
 
     while ((c = *fmt) != '\0') {
         /* Clear `alternate' modifier prior to new conversion. */
         alt_format = 0;
 
         /* Eat up white-space. */
         if (isspace(c)) {
              while (isspace(*bp))
                   bp++;
 
              fmt++;
              continue;
         }
                  
         if ((c = *fmt++) != '%')
              goto literal;
 
 
again:        switch (c = *fmt++) {
         case '%': /* "%%" is converted to "%". */
literal:
              if (c != *bp++)
                   return (0);
              break;
 
         /*
          * "Alternative" modifiers. Just set the appropriate flag
          * and start over again.
          */
         case 'E': /* "%E?" alternative conversion modifier. */
              LEGAL_ALT(0);
              alt_format |= ALT_E;
              goto again;
 
         case 'O': /* "%O?" alternative conversion modifier. */
              LEGAL_ALT(0);
              alt_format |= ALT_O;
              goto again;
             
         /*
          * "Complex" conversion rules, implemented through recursion.
          */
         case 'c': /* Date and time, using the locale's format. */
              LEGAL_ALT(ALT_E);
              if (!(bp = strptime(bp, "%x %X", tm)))
                   return (0);
              break;
 
         case 'D': /* The date as "%m/%d/%y". */
              LEGAL_ALT(0);
              if (!(bp = strptime(bp, "%m/%d/%y", tm)))
                   return (0);
              break;
 
         case 'R': /* The time as "%H:%M". */
              LEGAL_ALT(0);
              if (!(bp = strptime(bp, "%H:%M", tm)))
                   return (0);
              break;
 
         case 'r': /* The time in 12-hour clock representation. */
              LEGAL_ALT(0);
              if (!(bp = strptime(bp, "%I:%M:%S %p", tm)))
                   return (0);
              break;
 
         case 'T': /* The time as "%H:%M:%S". */
              LEGAL_ALT(0);
              if (!(bp = strptime(bp, "%H:%M:%S", tm)))
                   return (0);
              break;
 
         case 'X': /* The time, using the locale's format. */
              LEGAL_ALT(ALT_E);
              if (!(bp = strptime(bp, "%H:%M:%S", tm)))
                   return (0);
              break;
 
         case 'x': /* The date, using the locale's format. */
              LEGAL_ALT(ALT_E);
              if (!(bp = strptime(bp, "%m/%d/%y", tm)))
                   return (0);
              break;
 
         /*
          * "Elementary" conversion rules.
          */
         case 'A': /* The day of week, using the locale's form. */
         case 'a':
              LEGAL_ALT(0);
              for (i = 0; i < 7; i++) {
                   /* Full name. */
                   len = strlen(day[i]);
                   if (strncasecmp((char *)(day[i]), (char *)bp, len) == 0)
                       break;
 
                   /* Abbreviated name. */
                   len = strlen(abday[i]);
                   if (strncasecmp((char *)(abday[i]), (char *)bp, len) == 0)
                       break;
              }
 
              /* Nothing matched. */
              if (i == 7)
                   return (0);
 
              tm->tm_wday = i;
              bp += len;
              break;
 
         case 'B': /* The month, using the locale's form. */
         case 'b':
         case 'h':
              LEGAL_ALT(0);
              for (i = 0; i < 12; i++) {
                   /* Full name. */
                   len = strlen(mon[i]);
                   if (strncasecmp((char *)(mon[i]), (char *)bp, len) == 0)
                       break;
 
                   /* Abbreviated name. */
                   len = strlen(abmon[i]);
                   if (strncasecmp((char *)(abmon[i]),(char *) bp, len) == 0)
                       break;
              }
 
              /* Nothing matched. */
              if (i == 12)
                   return (0);
 
              tm->tm_mon = i;
              bp += len;
              break;
 
         case 'C': /* The century number. */
              LEGAL_ALT(ALT_E);
              if (!(conv_num(&bp, &i, 0, 99)))
                   return (0);
 
              if (split_year) {
                   tm->tm_year = (tm->tm_year % 100) + (i * 100);
              } else {
                   tm->tm_year = i * 100;
                   split_year = 1;
              }
              break;
 
         case 'd': /* The day of month. */
         case 'e':
              LEGAL_ALT(ALT_O);
              if (!(conv_num(&bp, &tm->tm_mday, 1, 31)))
                   return (0);
              break;
 
         case 'k': /* The hour (24-hour clock representation). */
              LEGAL_ALT(0);
              /* FALLTHROUGH */
         case 'H':
              LEGAL_ALT(ALT_O);
              if (!(conv_num(&bp, &tm->tm_hour, 0, 23)))
                   return (0);
              break;
 
         case 'l': /* The hour (12-hour clock representation). */
              LEGAL_ALT(0);
              /* FALLTHROUGH */
         case 'I':
              LEGAL_ALT(ALT_O);
              if (!(conv_num(&bp, &tm->tm_hour, 1, 12)))
                   return (0);
              if (tm->tm_hour == 12)
                   tm->tm_hour = 0;
              break;
 
         case 'j': /* The day of year. */
              LEGAL_ALT(0);
              if (!(conv_num(&bp, &i, 1, 366)))
                   return (0);
              tm->tm_yday = i - 1;
              break;
 
          case 'M': /* The minute. */
              LEGAL_ALT(ALT_O);
              if (!(conv_num(&bp, &tm->tm_min, 0, 59)))
                   return (0);
              break;
 
         case 'm': /* The month. */
              LEGAL_ALT(ALT_O);
              if (!(conv_num(&bp, &i, 1, 12)))
                   return (0);
              tm->tm_mon = i - 1;
              break;
 
       case 'p': /* The locale's equivalent of AM/PM. */
            LEGAL_ALT(0);
            /* AM? */
            if (strcmp(am_pm[0], bp) == 0) {
                 if (tm->tm_hour > 11)
                     return (0);

                 bp += strlen(am_pm[0]);
                 break;
            }
            /* PM? */
            else if (strcmp(am_pm[1], bp) == 0) {
                 if (tm->tm_hour > 11)
                     return (0);

                 tm->tm_hour += 12;
                 bp += strlen(am_pm[1]);
                 break;
            }

            /* Nothing matched. */
            return (0);
 
         case 'S': /* The seconds. */
              LEGAL_ALT(ALT_O);
              if (!(conv_num(&bp, &tm->tm_sec, 0, 61)))
                   return (0);
              break;
 
         case 'U': /* The week of year, beginning on sunday. */
         case 'W': /* The week of year, beginning on monday. */
              LEGAL_ALT(ALT_O);
              /*
               * XXX This is bogus, as we can not assume any valid
               * information present in the tm structure at this
               * point to calculate a real value, so just check the
               * range for now.
               */
               if (!(conv_num(&bp, &i, 0, 53)))
                   return (0);
               break;
 
         case 'w': /* The day of week, beginning on sunday. */
              LEGAL_ALT(ALT_O);
              if (!(conv_num(&bp, &tm->tm_wday, 0, 6)))
                   return (0);
              break;
 
         case 'Y': /* The year. */
              LEGAL_ALT(ALT_E);
              if (!(conv_num(&bp, &i, 0, 9999)))
                   return (0);
 
              tm->tm_year = i - TM_YEAR_BASE;
              break;
 
         case 'y': /* The year within 100 years of the epoch. */
              LEGAL_ALT(ALT_E | ALT_O);
              if (!(conv_num(&bp, &i, 0, 99)))
                   return (0);
 
              if (split_year) {
                   tm->tm_year = ((tm->tm_year / 100) * 100) + i;
                   break;
              }
              split_year = 1;
              if (i <= 68)
                   tm->tm_year = i + 2000 - TM_YEAR_BASE;
              else
                   tm->tm_year = i + 1900 - TM_YEAR_BASE;
              break;
 
         /*
          * Miscellaneous conversions.
          */
         case 'n': /* Any kind of white-space. */
         case 't':
              LEGAL_ALT(0);
              while (isspace(*bp))
                   bp++;
              break;
              //modify by lijc 目前还没有考虑周全%z情况
         case 'Z':
         case 'z':
              break;
 
         default: /* Unknown/unsupported conversion. */
              return (0);
         }
 
 
     }
 
     /* LINTED functional specification */
     return ((char *)bp);
}
 
 
static int
conv_num(const char **buf, int *dest, int llim, int ulim)
{
     int result = 0;
 
     /* The limit also determines the number of valid digits. */
     int rulim = ulim;
 
     if (**buf < '0' || **buf > '9')
         return (0);
 
     do {
         result *= 10;
         result += *(*buf)++ - '0';
         rulim /= 10;
     } while ((result * 10 <= ulim) && rulim && **buf >= '0' && **buf <= '9');
 
     if (result < llim || result > ulim)
         return (0);
 
     *dest = result;
     return (1);
}
#endif//WIN32

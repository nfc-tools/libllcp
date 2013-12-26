/*-
 * Copyright (C) 2011, Romain Tartière
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

/*
 * $Id$
 */

#include "config.h"

#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>

#include "llcp_log.h"

#ifdef DEBUG

int
llcp_log_init(void)
{
  return 0;
}

int
llcp_log_fini(void)
{
  return 0;
}

void
llcp_log_log(const char *category, int priority, const char *format, ...)
{
/*Windows doesn't support ANSI escape sequences*/
#ifndef WIN32
  switch (priority) {
    case LLC_PRIORITY_FATAL:
      printf("\033[37;41;1m");
      break;
    case LLC_PRIORITY_ALERT:
    case LLC_PRIORITY_CRIT:
    case LLC_PRIORITY_ERROR:
      printf("\033[31;1m");
      break;
    case LLC_PRIORITY_WARN:
      printf("\033[33;1m");
      break;
    case LLC_PRIORITY_NOTICE:
      printf("\033[34;1m");
      break;
    default:
      printf("\033[32m");
  }
#endif
  va_list va;
  va_start(va, format);
  printf("%s\t", category);
  vprintf(format, va);
#ifndef WIN32
  printf("[0m");
#endif
  printf("\n");
  fflush(stdout);
}

void llcp_log_hex(const char *category, int priority, const char *buf, int len, const char *prompt, ...)
{
  priority = priority;

  va_list va;
  va_start(va, prompt);
  printf("%s\t", category);
  vprintf(prompt, va);

  for(int i=0; i<len; i++){
    printf("%02x ", (unsigned char)buf[i]);
  }
  printf("\n");
}

#endif

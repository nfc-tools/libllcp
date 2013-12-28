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
#include <stdint.h>

#include <pthread.h>

#include "llcp_log.h"

#ifdef DEBUG
/* use mutex to make log */
pthread_mutex_t log_mutex;

int
llcp_log_init(void)
{
  int ret = pthread_mutex_init(&log_mutex, NULL);
  if (ret != 0) {
    printf("llcp.log err (%d)\n", ret);
    return -1;
  }
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
  pthread_mutex_lock(&log_mutex);
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
#else
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
  WORD saved_attributes;
  WORD textAttributes;
  /* Save current attributes */
  GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
  saved_attributes = consoleInfo.wAttributes;

  switch (priority) {
    case LLC_PRIORITY_FATAL:
      /* foreground white, background red */
      textAttributes = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_RED;
      break;
    case LLC_PRIORITY_ALERT:
    case LLC_PRIORITY_CRIT:
    case LLC_PRIORITY_ERROR:
      textAttributes = FOREGROUND_RED | FOREGROUND_INTENSITY;
      break;
    case LLC_PRIORITY_WARN:
      /* foreground yellow */
      textAttributes = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
      break;
    case LLC_PRIORITY_NOTICE:
      textAttributes = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
      break;
    default:
      textAttributes = FOREGROUND_GREEN;// | FOREGROUND_INTENSITY;
  }
  SetConsoleTextAttribute(hConsole, textAttributes);

#endif

  va_list va;
  va_start(va, format);
  printf("%s\t", category);
  vprintf(format, va);

#ifndef WIN32
  printf("[0m");
#else
  /* Restore original attributes */
  SetConsoleTextAttribute(hConsole, saved_attributes);
#endif

  printf("\n");
  fflush(stdout);
  pthread_mutex_unlock(&log_mutex);
}

void llcp_log_hex(const char *category, int priority, const char *buf, int len, const char *prompt, ...)
{
  pthread_mutex_lock(&log_mutex);
  priority = priority;
#ifdef WIN32
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
  WORD saved_attributes;
  WORD textAttributes = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
  /* Save current attributes */
  GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
  saved_attributes = consoleInfo.wAttributes;
  SetConsoleTextAttribute(hConsole, textAttributes);
#else
  printf("\033[1m");
#endif
  va_list va;
  va_start(va, prompt);
  printf("%s\t", category);
  vprintf(prompt, va);

  for(int i=0; i<len; i++){
    printf("%02x ", (unsigned char)buf[i]);
  }
  printf("\n");
#ifdef WIN32
  /* Restore original attributes */
  SetConsoleTextAttribute(hConsole, saved_attributes);
#else
  printf("[0m");
#endif
  pthread_mutex_unlock(&log_mutex);
}

void llcp_log_print_pdu_header(const char *category, const char *buf)
{
  const char ptypeName [][16]={
    "SYMM",
    "PAX",
    "AGF",
    "UI",
    "CONNECT",
    "DISC",
    "CC",
    "DM",
    "FRMR",
    "SNL",
    "RESERVED",
    "RESERVED",
    "I",
    "RR",
    "RNR",
    "RESERVED",
  };
  pthread_mutex_lock(&log_mutex);
  uint8_t dsap, ptype, ssap;
  dsap = (uint8_t)buf[0]>>2;
  ptype = (((uint8_t)buf[0]&0x03)<<2) | (((uint8_t)buf[1]&0xC0)>>6);
  ssap = (uint8_t)buf[1]&0x3F;
#ifndef WIN32
  printf("\033[36;1m%s\tDSAP: %02X, PTYPE: %s(%02X), SSAP: %02X\033[0m\n", category, dsap, ptypeName[ptype], ptype, ssap);
#else
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
  WORD saved_attributes;
  WORD textAttributes = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
  /* Save current attributes */
  GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
  saved_attributes = consoleInfo.wAttributes;
  SetConsoleTextAttribute(hConsole, textAttributes);
  printf("%s\tDSAP: %02X, PTYPE: %s(%02X), SSAP: %02X\n", category, dsap, ptypeName[ptype], ptype, ssap);
  /* Restore original attributes */
  SetConsoleTextAttribute(hConsole, saved_attributes);
#endif
  pthread_mutex_unlock(&log_mutex);
}

#endif

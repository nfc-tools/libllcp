/*-
 * Copyright (C) 2011, Romain Tartière
 * Copyright (C) 2013, JiapengLi
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
 * This implementation was written based on information provided by the
 * following documents:
 *
 * NFC Forum SNEP and LLCP specfication
 * 
 * Version 1 - 2013-12-04 - Modified By JiapengLi<gapleehit@gmail.com>
 *
 * http://www.nfc-forum.org/specs/
 * 
 */

/*
 * $Id$
 */

#include "config.h"

#include <err.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <llcp.h>
#include <llc_service.h>
#include <llc_link.h>
#include <mac.h>
#include <llc_connection.h>

struct mac_link *mac_link;
nfc_device *device;

static void
stop_mac_link(int sig)
{
  (void) sig;

  if (mac_link && mac_link->device)
    nfc_abort_command(mac_link->device);
}

static void
bye(void)
{
  if (device)
    nfc_close(device);
}

static size_t
shexdump(char *dest, const uint8_t *buf, const size_t size)
{
  size_t res = 0;
  for (size_t s = 0; s < size; s++) {
    sprintf(dest + res, "%02x  ", *(buf + s));
    res += 4;
  }
  return res;
}

FILE *info_stream = NULL;
FILE *ndef_stream = NULL;

static void *
com_android_snep_thread(void *arg)
{
  struct llc_connection *connection = (struct llc_connection *) arg;
  uint8_t buffer[1024], frame[1024];

  int len;
  if ((len = llc_connection_recv(connection, buffer, sizeof(buffer), NULL)) < 0)
    return NULL;

  if (len < 2) // SNEP's header (2 bytes)  and NDEF entry header (5 bytes)
    return NULL;

  size_t n = 0;

  // Header
  fprintf(info_stream, "SNEP version: %d.%d\n", (buffer[0]>>4), (buffer[0]&0x0F));
  if (buffer[n++] != 0x10){
    printf("snep-server is developed to support snep version 1.0, version %d.%d may be not supported.\n", (buffer[0]>>4), (buffer[0]&0x0F));
  }
  switch(buffer[1]){
    case 0x00:      /** Continue */
      break;
    case 0x01:      /** GET */
      break;
    case 0x02:      /** PUT */
      {
        uint32_t ndef_length = be32toh(*((uint32_t *)(buffer + 2)));  // NDEF length
        if ((len - 6) < ndef_length)
          return NULL; // Less received bytes than expected ?

        /** return snep success response package */
        frame[0] = 0x10;    /** SNEP version */
        frame[1] = 0x81;
        frame[2] = 0;
        frame[3] = 0;
        frame[4] = 0;
        frame[5] = 0;
        llc_connection_send(connection, frame, 6);

        char ndef_msg[1024];
        shexdump(ndef_msg, buffer + 6, ndef_length);
        fprintf(info_stream, "NDEF message received (%u bytes): %s\n", ndef_length, ndef_msg);

        if (ndef_stream) {
          if (fwrite(buffer + 6, 1, ndef_length, ndef_stream) != ndef_length) {
            fprintf(stderr, "Could not write to file.\n");
            fclose(ndef_stream);
            ndef_stream = NULL;
          } else {
            fclose(ndef_stream);
            ndef_stream = NULL;
          }
        }
      }
      break;
  }

  // TODO Stop the LLCP when this is reached
  llc_connection_stop(connection);
  return NULL;
}

static void
print_usage(char *progname)
{
  fprintf(stderr, "usage: %s -o FILE\n", progname);
  fprintf(stderr, "\nOptions:\n");
  fprintf(stderr, "  -o     Extract NDEF message if available in FILE\n");
}

int
main(int argc, char *argv[])
{
  int ch;
  char *ndef_output = NULL;
  while ((ch = getopt(argc, argv, "ho:")) != -1) {
    switch (ch) {
      case 'h':
        print_usage(argv[0]);
        exit(EXIT_SUCCESS);
        break;
      case 'o':
        ndef_output = optarg;
        break;
      case '?':
        if (optopt == 'o')
          fprintf(stderr, "Option -%c requires an argument.\n", optopt);
      default:
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  if (ndef_output == NULL) {
    // No output sets by user
    print_usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  if ((strlen(ndef_output) == 1) && (ndef_output[0] == '-')) {
    info_stream = stderr;
    ndef_stream = stdout;
  } else {
    info_stream = stdout;
    ndef_stream = fopen(ndef_output, "wb");
    if (!ndef_stream) {
      fprintf(stderr, "Could not open file %s.\n", ndef_output);
      exit(EXIT_FAILURE);
    }
  }

  nfc_context *context;
  nfc_init(&context);

  if (llcp_init() < 0)
    errx(EXIT_FAILURE, "llcp_init()");

  signal(SIGINT, stop_mac_link);
  atexit(bye);

  if (!(device = nfc_open(context, NULL))) {
    errx(EXIT_FAILURE, "Cannot connect to NFC device");
  }

  struct llc_link *llc_link = llc_link_new();
  if (!llc_link) {
    errx(EXIT_FAILURE, "Cannot allocate LLC link data structures");
  }

  struct llc_service *com_android_snep;
  if (!(com_android_snep = llc_service_new_with_uri(NULL, com_android_snep_thread, "urn:nfc:sn:snep", NULL)))
    errx(EXIT_FAILURE, "Cannot create com.android.snep service");

  llc_service_set_miu(com_android_snep, 512);
  llc_service_set_rw(com_android_snep, 2);

  if (llc_link_service_bind(llc_link, com_android_snep, LLCP_SNEP_SAP) < 0)
    errx(EXIT_FAILURE, "Cannot bind service");

  mac_link = mac_link_new(device, llc_link);
  if (!mac_link)
    errx(EXIT_FAILURE, "Cannot create MAC link");

  if (mac_link_activate_as_target(mac_link) < 0) {
    errx(EXIT_FAILURE, "Cannot activate MAC link");
  }

  void *err;
  mac_link_wait(mac_link, &err);

  mac_link_free(mac_link);
  llc_link_free(llc_link);

  nfc_close(device);
  device = NULL;

  llcp_fini();
  nfc_exit(context);
  exit(EXIT_SUCCESS);
}

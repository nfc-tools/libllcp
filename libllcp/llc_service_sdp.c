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

#include <sys/types.h>
#ifndef WIN32
#  include <sys/socket.h>
#endif

#include <fcntl.h>
#include <pthread.h>
#include <string.h>

#include "llcp.h"
#include "llcp_log.h"
#include "llc_connection.h"
#include "llc_link.h"
#include "llc_service.h"
#include "llc_service_sdp.h"
#include "llcp_parameters.h"

#define LOG_LLC_SDP "libllcp.llc.sdp"
#define LLC_SDP_MSG(priority, message) llcp_log_log (LOG_LLC_SDP, priority, "(%p) %s", pthread_self (), message)
#define LLC_SDP_LOG(priority, format, ...) llcp_log_log (LOG_LLC_SDP, priority, "(%p) " format, pthread_self (), __VA_ARGS__)

/* Service Discovery Protocol */

void
llc_service_sdp_thread_cleanup(void *arg)
{
  struct llc_connection *connection = (struct llc_connection *) arg;

  (void) connection;
}

void *
llc_service_sdp_thread(void *arg)
{
  struct llc_connection *connection = (struct llc_connection *) arg;
  sod_t llc_up;

  int old_cancelstate;

  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_cancelstate);

  llc_up = connection->llc_so_up;

  if (llc_up == INVALID_SOCKET)
    LLC_SDP_MSG(LLC_PRIORITY_ERROR, "Invalid socket");

  pthread_cleanup_push(llc_service_sdp_thread_cleanup, arg);
  pthread_setcancelstate(old_cancelstate, NULL);
  LLC_SDP_MSG(LLC_PRIORITY_INFO, "Service Discovery Protocol started");

  int res;

  uint8_t buffer[1024];
  LLC_SDP_MSG(LLC_PRIORITY_TRACE, "socket receive+");
  pthread_testcancel();
  res = recv(llc_up, (char *) buffer, sizeof(buffer), 0);
  pthread_testcancel();
  if (res < 0) {
    pthread_testcancel();
  }
  LLC_SDP_LOG(LLC_PRIORITY_TRACE, "Received %d bytes", res);

  uint8_t tid;
  char *uri;

  switch (buffer[2]) {
    case LLCP_PARAMETER_SDREQ:
      if (parameter_decode_sdreq(buffer + 2, res - 2, &tid, &uri) < 0) {
        LLC_SDP_MSG(LLC_PRIORITY_ERROR, "Ignoring PDU");
      } else {
        LLC_SDP_LOG(LLC_PRIORITY_TRACE, "Service Discovery Request #0x%02x for '%s'", tid, uri);

        uint8_t sap = llc_link_find_sap_by_uri(connection->link, uri);

        if (!sap) {
          LLC_SDP_LOG(LLC_PRIORITY_ERROR, "No registered service provide '%s'", uri);
        }
        buffer[0] = 0x06;
        buffer[1] = 0x41;
        int n = parameter_encode_sdres(buffer + 2, sizeof(buffer) - 2, tid, sap);

        send(llc_up, (char *) buffer, n + 2, 0);
        LLC_SDP_LOG(LLC_PRIORITY_TRACE, "Sent %d bytes", n + 2);

      }
      break;
    default:
      LLC_SDP_MSG(LLC_PRIORITY_ERROR, "Invalid parameter type");
  }

  pthread_cleanup_pop(1);
  llc_connection_stop(connection);
  return NULL;
}

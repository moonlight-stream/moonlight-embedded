/*
 * This file is part of Moonlight Embedded.
 *
 * Based on Avahi example client-browse-services
 *
 * Moonlight is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Moonlight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Moonlight; if not, see <http://www.gnu.org/licenses/>.
 */

#include "errors.h"

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>

#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

static AvahiSimplePoll *simple_poll = NULL;

static void client_callback(AvahiClient *c, AvahiClientState state, void *userdata) {
  if (state == AVAHI_CLIENT_FAILURE) {
    gs_error = "Server connection failure";
    avahi_simple_poll_quit(simple_poll);
  }
}

static void resolve_callback(AvahiServiceResolver *r, AvahiIfIndex interface, AvahiProtocol protocol, AvahiResolverEvent event, const char *name, const char *type, const char *domain, const char *host_name, const AvahiAddress *address, uint16_t port, AvahiStringList *txt, AvahiLookupResultFlags flags, void *userdata) {
  if (event == AVAHI_RESOLVER_FOUND) {
    if (userdata != NULL) {
      avahi_address_snprint(userdata, AVAHI_ADDRESS_STR_MAX, address);
      avahi_simple_poll_quit(simple_poll);
    } else {
      char strAddress[AVAHI_ADDRESS_STR_MAX];
      avahi_address_snprint(strAddress, sizeof(strAddress), address);
      printf(" %s (%s)\n", host_name, strAddress);
    }
  }

  avahi_service_resolver_free(r);
}

static void browse_callback(AvahiServiceBrowser *b, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char *name, const char *type, const char *domain, AvahiLookupResultFlags flags, void* userdata) {
  AvahiClient *c = avahi_service_browser_get_client(b);

  switch (event) {
  case AVAHI_BROWSER_FAILURE:
    gs_error = "Server browser failure";
    avahi_simple_poll_quit(simple_poll);
    break;
  case AVAHI_BROWSER_NEW:
    if (!(avahi_service_resolver_new(c, interface, protocol, name, type, domain, AVAHI_PROTO_INET, 0, resolve_callback, userdata)))
      gs_error = "Failed to resolve service";

    break;
  case AVAHI_BROWSER_REMOVE:
    break;
  }
}

void gs_discover_server(char* dest) {
  AvahiClient *client = NULL;
  AvahiServiceBrowser *sb = NULL;

  if (!(simple_poll = avahi_simple_poll_new())) {
    gs_error = "Failed to create simple poll object";
    goto cleanup;
  }

  int error;
  client = avahi_client_new(avahi_simple_poll_get(simple_poll), 0, client_callback, NULL, &error);
  if (!client) {
    gs_error = "Failed to create client";
    goto cleanup;
  }

  if (!(sb = avahi_service_browser_new(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_INET, "_nvstream._tcp", NULL, 0, browse_callback, dest))) {
    gs_error = "Failed to create service browser";
    goto cleanup;
  }

  avahi_simple_poll_loop(simple_poll);

  cleanup:
  if (sb)
    avahi_service_browser_free(sb);

  if (client)
    avahi_client_free(client);

  if (simple_poll)
    avahi_simple_poll_free(simple_poll);
}

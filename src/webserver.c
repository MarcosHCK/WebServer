/* Copyright 2023 MarcosHCK
 * This file is part of WebServer.
 *
 * WebServer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * WebServer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with WebServer. If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>
#include <webconnection.h>
#include <webendpoint.h>
#include <webmessage.h>
#include <webserver.h>

#define WEB_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), WEB_TYPE_SERVER, WebServerClass))
#define WEB_IS_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), WEB_TYPE_SERVER))
#define WEB_SERVER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), WEB_TYPE_SERVER, WebServerClass))
typedef struct _WebServerClass WebServerClass;
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

struct _WebServer
{
  GObject parent;

  /* private */
  GQueue clients;
  GQueue listeners;
};

struct _WebServerClass
{
  GObjectClass parent;
};

enum
{
  signal_listen_error,
  signal_request_error,
  signal_request_started,
  signal_number,
};

G_DEFINE_FINAL_TYPE (WebServer, web_server, G_TYPE_OBJECT);
static guint signals [signal_number] = {0};

static void web_server_class_dispose (GObject* pself)
{
  WebServer* self = (gpointer) pself;
  g_queue_clear_full (& self->clients, g_object_unref);
  g_queue_clear_full (& self->listeners, g_object_unref);
G_OBJECT_CLASS (web_server_parent_class)->dispose (pself);
}

static void web_server_class_finalize (GObject* pself)
{
  WebServer* self = (gpointer) pself;
G_OBJECT_CLASS (web_server_parent_class)->finalize (pself);
}

static void web_server_class_init (WebServerClass* klass)
{
  G_OBJECT_CLASS (klass)->dispose = web_server_class_dispose;
  G_OBJECT_CLASS (klass)->finalize = web_server_class_finalize;

  const GType gtype = G_TYPE_FROM_CLASS (klass);
  const GSignalFlags flags1 = G_SIGNAL_RUN_LAST;
  const GSignalFlags flags2 = G_SIGNAL_RUN_FIRST;
  const GSignalCMarshaller marshaller1 = g_cclosure_marshal_VOID__BOXED;
  const GSignalCMarshaller marshaller2 = g_cclosure_marshal_VOID__OBJECT;

  signals [signal_listen_error] = g_signal_new ("listen-error", gtype, flags1, 0, NULL, NULL, marshaller1, G_TYPE_NONE, 1, G_TYPE_ERROR);
  signals [signal_request_error] = g_signal_new ("request-error", gtype, flags1, 0, NULL, NULL, marshaller1, G_TYPE_NONE, 1, G_TYPE_ERROR);
  signals [signal_request_started] = g_signal_new ("request-started", gtype, flags2, 0, NULL, NULL, marshaller2, G_TYPE_NONE, 1, WEB_TYPE_MESSAGE);
}

static void web_server_init (WebServer* self)
{
  g_queue_init (& self->clients);
  g_queue_init (& self->listeners);
}

WebServer* web_server_new ()
{
  return g_object_new (WEB_TYPE_SERVER, NULL);
}

static void on_client_disconnected (WebServer* self, WebConnection* web_connection)
{
  g_queue_remove (& self->clients, web_connection);
}

static void on_client_request_failed (WebServer* self, GError* tmperr, WebConnection* web_connection)
{
  g_signal_emit (self, signals [signal_request_error], 0, tmperr);
}

static void on_client_request_started (WebServer* self, WebMessage* web_message)
{
  g_signal_emit (self, signals [signal_request_started], 0, web_message);
}

static void on_failed_connection (WebServer* self, GError* tmperr)
{
  g_signal_emit (self, signals [signal_listen_error], 0, tmperr);
}

static gboolean on_new_connection (WebServer* self, GSocket* client_socket)
{
  WebConnection* web_connection = web_connection_new (client_socket, FALSE);

  g_queue_push_head (& self->clients, g_object_ref (web_connection));
  g_signal_connect_swapped (web_connection, "disconnected", G_CALLBACK (on_client_disconnected), self);
  g_signal_connect_swapped (web_connection, "request-failed", G_CALLBACK (on_client_request_failed), self);
  g_signal_connect_swapped (web_connection, "request-started", G_CALLBACK (on_client_request_started), self);
  web_connection_accepted (web_connection);
return (g_object_unref (web_connection), TRUE);
}

static GSocket* listen_internal (WebServer* self, GSocketAddress* socket_address, WebListenOptions options, GError** error)
{
  WebEndpoint* web_endpoint;
  GSocket* socket = NULL;
  GError* tmperr = NULL;

  const GSocketFamily socket_family = g_socket_address_get_family (socket_address);
  const GSocketType socket_type = G_SOCKET_TYPE_STREAM;
  const GSocketProtocol socket_proto = G_SOCKET_PROTOCOL_TCP;

  if ((socket = g_socket_new (socket_family, socket_type, socket_proto, &tmperr)), G_UNLIKELY (tmperr != NULL))
    {
      _g_object_unref0 (socket);
      g_propagate_error (error, tmperr);
      return NULL;
    }

  g_socket_set_blocking (socket, FALSE);
  g_socket_set_listen_backlog (socket, 3);

  if ((g_socket_bind (socket, socket_address, TRUE, &tmperr)), G_UNLIKELY (tmperr != NULL))
    {
      _g_object_unref0 (socket);
      g_propagate_error (error, tmperr);
      return NULL;
    }

  if ((g_socket_listen (socket, &tmperr)), G_UNLIKELY (tmperr != NULL))
    {
      _g_object_unref0 (socket);
      g_propagate_error (error, tmperr);
      return NULL;
    }

  if ((web_endpoint = web_endpoint_new (socket, &tmperr), g_object_unref (socket)), G_UNLIKELY (tmperr != NULL))
    {
      _g_object_unref0 (web_endpoint);
      g_propagate_error (error, tmperr);
      return NULL;
    }

  g_signal_connect_swapped (web_endpoint, "new-connection", G_CALLBACK (on_new_connection), self);
  g_queue_push_head (& self->listeners, web_endpoint);
return (web_endpoint_get_socket (web_endpoint));
}

void web_server_listen (WebServer* web_server, GSocketAddress* address, WebListenOptions options, GError** error)
{
  g_return_if_fail (WEB_IS_SERVER (web_server));
  g_return_if_fail (G_IS_SOCKET_ADDRESS (address));
  g_return_if_fail (error == NULL || *error == NULL);

  listen_internal (web_server, address, options, error);
}

void web_server_listen_any (WebServer* web_server, guint16 port, WebListenOptions options, GError** error)
{
  g_return_if_fail (WEB_IS_SERVER (web_server));
  g_return_if_fail (error == NULL || *error == NULL);
  WebServer* self = (web_server);
  gboolean activate = TRUE;
  GInetAddress* inet_address;
  GSocket* socket;
  GSocketAddress* socket_address;
  GSocketFamily socket_family;
  GError* tmperr = NULL;
  guint i, remain;

  enum
    {
      sockets_ipv6,
      sockets_ipv4,
      sockets_number,
    };

  GSocket* sockets [sockets_number] = {0};

  struct _Family
    {
      GSocketFamily family;
      WebListenOptions flag;
    };

  const struct _Family families [] =
    {
      { G_SOCKET_FAMILY_IPV6, WEB_LISTEN_OPTION_IPV6_ONLY, },
      { G_SOCKET_FAMILY_IPV4, WEB_LISTEN_OPTION_IPV4_ONLY, },
    };

  const guint onlyflags = WEB_LISTEN_OPTION_IPV4_ONLY | WEB_LISTEN_OPTION_IPV6_ONLY;
  const guint otherflags = ~onlyflags;

  if ((options & WEB_LISTEN_OPTION_IPV4_ONLY) > 0)
    {
      activate = FALSE;
      g_return_if_fail (((options & (~WEB_LISTEN_OPTION_IPV4_ONLY)) & onlyflags) == 0);
    }
  if ((options & WEB_LISTEN_OPTION_IPV6_ONLY) > 0)
    {
      activate = FALSE;
      g_return_if_fail (((options & (~WEB_LISTEN_OPTION_IPV6_ONLY)) & onlyflags) == 0);
    }

  G_STATIC_ASSERT (G_N_ELEMENTS (families) == sockets_number);

  for (i = 0; i < sockets_number; ++i)
    {
      if (activate || (options & (families [i].flag)) > 0)
        {
          socket_family = families [i].family;

          if (i == sockets_ipv4 && sockets [sockets_ipv6] != NULL)
          if (g_socket_speaks_ipv4 (sockets [sockets_ipv6]))
            continue;

          inet_address = g_inet_address_new_any (socket_family);
          socket_address = g_inet_socket_address_new (inet_address, port);
          remain = options & otherflags;
          sockets [i] = listen_internal (self, socket_address, remain, &tmperr);

          _g_object_unref0 (inet_address);
          _g_object_unref0 (socket_address);

          if (G_UNLIKELY (tmperr != NULL))
            {
              g_propagate_error (error, tmperr);
              break;
            }
        }
    }
}

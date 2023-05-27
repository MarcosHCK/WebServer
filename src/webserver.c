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
#include <marshals.h>
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
typedef struct _WebConnection WebConnection;
typedef union _SignalData SignalData;

struct _WebServer
{
  GObject parent;

  /* private */
  GMainContext* context;
  GQueue listeners;
  GThreadPool* workers;
};

struct _WebServerClass
{
  GObjectClass parent;
};

union _SignalData
{
  GValue values [2];

  struct
  {
    GValue instance;
    GValue argument;
  };
};

enum
{
  signal_got_failure,
  signal_got_request,
  signal_number,
};

G_DEFINE_FINAL_TYPE (WebServer, web_server, G_TYPE_OBJECT);
static guint signals [signal_number] = {0};

static void web_server_class_dispose (GObject* pself)
{
  WebServer* self = (gpointer) pself;
  g_queue_clear_full (& self->listeners, g_object_unref);
G_OBJECT_CLASS (web_server_parent_class)->dispose (pself);
}

static void web_server_class_finalize (GObject* pself)
{
  WebServer* self = (gpointer) pself;
  g_main_context_unref (self->context);
  g_thread_pool_free (self->workers, TRUE, TRUE);
G_OBJECT_CLASS (web_server_parent_class)->finalize (pself);
}

static void web_server_class_init (WebServerClass* klass)
{
  G_OBJECT_CLASS (klass)->dispose = web_server_class_dispose;
  G_OBJECT_CLASS (klass)->finalize = web_server_class_finalize;

  const GType gtype = G_TYPE_FROM_CLASS (klass);
  const GSignalFlags flags1 = G_SIGNAL_RUN_LAST;
  const GSignalFlags flags2 = G_SIGNAL_RUN_LAST;
  const GSignalCMarshaller marshaller1 = web_cclosure_marshal_VOID__BOXED;
  const GSignalCMarshaller marshaller2 = web_cclosure_marshal_VOID__OBJECT;

  signals [signal_got_failure] = g_signal_new ("got-failure", gtype, flags1, 0, NULL, NULL, marshaller1, G_TYPE_NONE, 1, G_TYPE_ERROR);
  signals [signal_got_request] = g_signal_new ("got-request", gtype, flags2, 0, NULL, NULL, marshaller2, G_TYPE_NONE, 1, WEB_TYPE_MESSAGE);
}

static gboolean do_got_failure (gpointer values)
{
  return (g_signal_emitv (values, signals [signal_got_failure], 0, NULL), G_SOURCE_REMOVE);
}

static gboolean do_got_request (gpointer values)
{
  return (g_signal_emitv (values, signals [signal_got_request], 0, NULL), G_SOURCE_REMOVE);
}

static void signal_data_unref (gpointer ptr)
{
  g_value_unset (& G_STRUCT_MEMBER (GValue, ptr, sizeof (GValue) * 0));
  g_value_unset (& G_STRUCT_MEMBER (GValue, ptr, sizeof (GValue) * 1));
  g_slice_free (SignalData, ptr);
}

static void process (WebConnection* web_connection, WebServer* self)
{
  WebMessage* web_message = NULL;
  GError* tmperr = NULL;

  if ((web_message = web_connection_step (web_connection, &tmperr)), G_UNLIKELY (tmperr != NULL))
    {
      if (g_error_matches (tmperr, G_IO_ERROR, G_IO_ERROR_CONNECTION_CLOSED))
        g_error_free (tmperr);
      else
        {
          SignalData* data = g_slice_new0 (SignalData);

          g_value_init_from_instance (& data->instance, self);
          g_value_init (& data->argument, G_TYPE_ERROR);
          g_value_take_boxed (& data->argument, tmperr);

          g_main_context_invoke_full (self->context, G_PRIORITY_HIGH_IDLE, G_SOURCE_FUNC (do_got_failure), data, signal_data_unref);
        }

      g_object_unref (web_connection);
    }
  else
    {
      if (web_message != NULL)
        {
          SignalData* data = g_slice_new0 (SignalData);

          g_value_init_from_instance (& data->instance, self);
          g_value_init_from_instance (& data->argument, web_message);

          g_main_context_invoke_full (self->context, G_PRIORITY_HIGH_IDLE, G_SOURCE_FUNC (do_got_request), data, signal_data_unref);
          g_object_unref (web_message);
        }

      g_thread_pool_push (self->workers, web_connection, NULL);
    }
}

static void web_server_init (WebServer* self)
{
  g_queue_init (& self->listeners);

  const GFunc func1 = (GFunc) process;
  const GDestroyNotify notify = (GDestroyNotify) g_object_unref;
  const guint max_threads = g_get_num_processors ();

  self->context = g_main_context_ref_thread_default ();
  self->workers = g_thread_pool_new_full (func1, self, notify, max_threads, FALSE, NULL);
}

WebServer* web_server_new ()
{
  return g_object_new (WEB_TYPE_SERVER, NULL);
}

static gboolean on_new_connection (WebServer* self, GSocket* client_socket, WebEndpoint* web_endpoint)
{
  gboolean is_https = web_endpoint_get_is_https (web_endpoint);
  WebConnection* web_connection = web_connection_new (client_socket, is_https);
return (g_thread_pool_push (self->workers, web_connection, NULL), TRUE);
}

static gboolean on_failed_connection (WebServer* self, GError* tmperr)
{
  return (g_signal_emit (self, signals [signal_got_failure], 0, tmperr), TRUE);
}

static GSocket* listen_internal (WebServer* self, GSocketAddress* socket_address, WebListenOptions options, GError** error)
{
  WebEndpoint* web_endpoint;
  GSocket* socket = NULL;
  GError* tmperr = NULL;

  const gboolean is_https = ! ((options & WEB_LISTEN_OPTION_HTTPS) == 0);
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

  if ((web_endpoint = web_endpoint_new (socket, is_https, &tmperr), g_object_unref (socket)), G_UNLIKELY (tmperr != NULL))
    {
      _g_object_unref0 (web_endpoint);
      g_propagate_error (error, tmperr);
      return NULL;
    }

  g_signal_connect_swapped (web_endpoint, "new-connection", G_CALLBACK (on_new_connection), self);
  g_signal_connect_swapped (web_endpoint, "failed-connection", G_CALLBACK (on_failed_connection), self);
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

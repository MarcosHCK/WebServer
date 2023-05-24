/* Copyright 2023 MarcosHCK
 * This file is part of WebbServer.
 *
 * WebbServer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * WebbServer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with WebbServer. If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>
#include <gio/gio.h>
#include <webclient.h>
#include <webserver.h>

#define WEB_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), WEB_TYPE_SERVER, WebServerClass))
#define WEB_IS_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), WEB_TYPE_SERVER))
#define WEB_SERVER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), WEB_TYPE_SERVER, WebServerClass))
typedef struct _WebServerClass WebServerClass;
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _web_endpoint_unref0(var) ((var == NULL) ? NULL : (var = (web_endpoint_unref (var), NULL)))
typedef struct _WebEndpoint WebEndpoint;
static WebEndpoint* web_endpoint_ref (WebEndpoint* endpoint);
static void web_endpoint_unref (WebEndpoint* endpoint);
typedef struct _AcceptData AcceptData;

struct _WebServer
{
  GApplication parent;

  /* private */
  guint active : 1;

  union
  {
    GPtrArray array_;

    struct
    {
      struct _WebEndpoint
      {
        guint refs;
        guint port : 16;
        guint secure : 1;

        GFile* home;
        GCancellable* cancellable;

        union
          {
            struct
              {
                GSocket* socket4;
                GSocket* socket6;
              };

            GSocket* sockets [2];
          };
      } **ar;

      guint count;
    };
  } *endpoints;
};

struct _WebServerClass
{
  GApplicationClass parent;
};

struct _AcceptData
{
  WebEndpoint* endpoint;
  WebServer* web_server;
};

enum
{
  prop_0,
  prop_active,
  prop_number,
};

enum
{
  signal_incoming,
  signal_number,
};

G_DEFINE_FINAL_TYPE (WebServer, web_server, G_TYPE_APPLICATION);
G_DEFINE_BOXED_TYPE (WebEndpoint, web_endpoint, web_endpoint_ref, web_endpoint_unref);

static GParamSpec* properties [prop_number] = {0};
static guint signals [signal_number] = {0};

static WebEndpoint* web_endpoint_new (guint16 port, gboolean secure, GError** error)
{
  WebEndpoint* self;

  self = g_slice_new0 (WebEndpoint);
  self->refs = 1;
  self->port = port;
  self->secure = secure;

  GError* tmperr = NULL;
  GInetAddress* inet_address;
  GSocket* socket;
  GSocketAddress* socket_address;
  GSocketFamily family;
  guint i;

  const GSocketFamily families [] =
    {
      G_SOCKET_FAMILY_IPV6,
      G_SOCKET_FAMILY_IPV4,
    };

  const GSocketType socket_type = G_SOCKET_TYPE_STREAM;
  const GSocketProtocol socket_proto = G_SOCKET_PROTOCOL_TCP;

  for (i = 0; i < G_N_ELEMENTS (families); ++i)
    {
      family = families [i];

      if ((socket = g_socket_new (family, socket_type, socket_proto, &tmperr)), G_UNLIKELY (tmperr != NULL))
        {
          _web_endpoint_unref0 (self);
          _g_object_unref0 (socket);
          g_propagate_error (error, tmperr);
          return NULL;
        }

      inet_address = g_inet_address_new_any (family);
      socket_address = g_inet_socket_address_new (inet_address, port);

      g_socket_set_blocking (socket, FALSE);
      g_socket_set_listen_backlog (socket, 3);
      g_object_unref (inet_address);

      if ((g_socket_bind (socket, socket_address, TRUE, &tmperr), g_object_unref (socket_address)), G_UNLIKELY (tmperr != NULL))
        {
          _web_endpoint_unref0 (self);
          _g_object_unref0 (socket);
          g_propagate_error (error, tmperr);
          return NULL;
        }

      if ((g_socket_listen (socket, &tmperr)), G_UNLIKELY (tmperr != NULL))
        {
          _web_endpoint_unref0 (self);
          _g_object_unref0 (socket);
          g_propagate_error (error, tmperr);
          return NULL;
        }

      self->sockets [i] = socket;

      if (i == 0 && g_socket_speaks_ipv4 (socket))
        break;
    }
return self;
}

static WebEndpoint* web_endpoint_ref (WebEndpoint* endpoint)
{
  g_return_val_if_fail (endpoint != NULL, NULL);
return (g_atomic_int_inc (& endpoint->refs), endpoint);
}

static void web_endpoint_unref (WebEndpoint* endpoint)
{
  g_return_if_fail (endpoint != NULL);
  WebEndpoint* self = (endpoint);

  if (g_atomic_int_dec_and_test (& self->refs))
    {
      _g_object_unref0 (self->socket4);
      _g_object_unref0 (self->socket6);
      _g_object_unref0 (self->home);
      _g_object_unref0 (self->cancellable);
      g_slice_free (WebEndpoint, self);
    }
}

static void emit_client (WebServer* self, GIOStream* stream)
{
  WebClient* client = web_client_new (stream);
  GValue param_values [2] = { G_VALUE_INIT, G_VALUE_INIT, };
  GValue return_value [1] = { G_VALUE_INIT, };

  g_value_init (return_value, G_TYPE_BOOLEAN);
  g_value_init_from_instance (param_values + 0, self);
  g_value_init_from_instance (param_values + 1, client);
  g_object_unref (client);

  g_signal_emitv (param_values, signals [signal_incoming], 0, return_value);

  g_value_unset (param_values + 1);
  g_value_unset (param_values + 0);
  g_value_unset (return_value);
}

static gboolean accept_source (GSocket* socket, GIOCondition condition, AcceptData* data)
{
  if (g_source_is_destroyed (g_main_current_source ()))
    return G_SOURCE_REMOVE;
  else
    {
      switch (condition)
        {
          default: g_assert_not_reached ();

          case G_IO_HUP:
          case G_IO_ERR:
            {
              g_critical ("(" G_STRLOC "): socket error, shuting down");
              return G_SOURCE_REMOVE;
            }

          case G_IO_IN:
            {
              GError* tmperr = NULL;
              GCancellable* cancellable = data->endpoint->cancellable;
              GSocket* client_socket = g_socket_accept (socket, cancellable, &tmperr);
              GSocketConnection* connection = g_socket_connection_factory_create_connection (socket);
              GIOStream* stream = (gpointer) connection;

              emit_client (data->web_server, stream);

              g_object_unref (connection);
              g_object_unref (client_socket);
              break;
            }
        }
    }
return G_SOURCE_CONTINUE;
}

static void accept_data_free (AcceptData* data)
{
  _web_endpoint_unref0 (data->endpoint);
  _g_object_unref0 (data->web_server);
  g_slice_free (AcceptData, data);
}

static void web_endpoint_start (WebEndpoint* endpoint, WebServer* web_server)
{
  g_return_if_fail (endpoint != NULL);
  WebEndpoint* self = (endpoint);
  GCancellable* cancellable = g_cancellable_new ();
  guint i;

  g_set_object (& self->cancellable, cancellable);

  for (i = 0; i < G_N_ELEMENTS (self->sockets); ++i)
    {
      if (self->sockets [i] != NULL)
        {
          GSourceFunc func = (GSourceFunc) accept_source;
          GDestroyNotify notify = (GDestroyNotify) accept_data_free;
          GSource* source = g_socket_create_source (self->sockets [i], G_IO_IN, self->cancellable);
          GMainContext* context = g_main_context_get_thread_default ();
          AcceptData* data = g_slice_new (AcceptData);

          data->endpoint = web_endpoint_ref (endpoint);
          data->web_server = g_object_ref (web_server);

          g_source_attach (source, context);
          g_source_set_callback (source, func, data, notify);
          g_source_set_priority (source, G_PRIORITY_DEFAULT_IDLE);
          g_source_set_static_name (source, "[WebServer.AcceptSource]");
          g_source_unref (source);
        }
    }
}

static void web_endpoint_stop (WebEndpoint* endpoint)
{
  g_return_if_fail (endpoint != NULL);
  WebEndpoint* self = (endpoint);

  g_cancellable_cancel (self->cancellable);
}

static void web_server_class_open (GApplication* pself, GFile** files, gint n_files, const gchar* hint)
{
  WebServer* self = (gpointer) pself;
  GFile* current = g_file_new_for_path (".");
  GFile* subst = g_file_new_for_path ("/var/www");
  GError* tmperr = NULL;
  guint64 port_u64;
  guint i;

  for (i = 0; i < n_files; i += 2)
    {
      GFile* port = ((i + 0) < n_files) ? files [(i + 0)] : NULL;
      GFile* home = ((i + 1) < n_files) ? files [(i + 1)] : subst;
      WebEndpoint* endpoint = NULL;
      gchar* port_name = g_file_get_relative_path (current, port);
      guint64 port_number = 8080;

      if ((g_ascii_string_to_unsigned (port_name, 10, 0, G_MAXUINT16, &port_u64, &tmperr)), G_UNLIKELY (tmperr == NULL))
        port_number = (guint16) port_u64;
      else
        {
          g_warning ("Bad port number '%s', defaulting 8080", port_name);
          g_error_free (tmperr);
        }

      if ((endpoint = web_endpoint_new (port_number, FALSE, &tmperr)), G_UNLIKELY (tmperr == NULL))
        endpoint->home = g_object_ref (home);
      else
        {
          g_warning ("%s", tmperr->message);
          g_error_free(tmperr);
        }

      if (self->active == TRUE)
        {
          web_endpoint_start (endpoint, self);
        }

      g_ptr_array_add ((gpointer) self->endpoints, endpoint);
    }

  _g_object_unref0 (current);
  _g_object_unref0 (subst);
}

static void web_server_class_dispose (GObject* pself)
{
  WebServer* self = (gpointer) pself;
  g_ptr_array_remove_range ((gpointer) self->endpoints, 0, self->endpoints->count);
G_OBJECT_CLASS (web_server_parent_class)->dispose (pself);
}

static void web_server_class_finalize (GObject* pself)
{
  WebServer* self = (gpointer) pself;
  g_ptr_array_unref ((gpointer) self->endpoints);
G_OBJECT_CLASS (web_server_parent_class)->finalize (pself);
}

static void web_server_class_get_property (GObject* pself, guint property_id, GValue* value, GParamSpec* pspec)
{
  WebServer* self = (gpointer) pself;

  switch (property_id)
    {
      case prop_active:
        g_value_set_boolean (value, self->active);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (pself, property_id, pspec);
        break;
    }
}


static void web_server_class_set_property (GObject* pself, guint property_id, const GValue* value, GParamSpec* pspec)
{
  WebServer* self = (gpointer) pself;

  switch (property_id)
    {
      case prop_active:
        if (g_value_get_boolean (value))
          web_server_start (self);
        else
          web_server_stop (self);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (pself, property_id, pspec);
        break;
    }
}

static void web_server_class_init (WebServerClass* klass)
{
  G_APPLICATION_CLASS (klass)->open = web_server_class_open;
  G_OBJECT_CLASS (klass)->dispose = web_server_class_dispose;
  G_OBJECT_CLASS (klass)->finalize = web_server_class_finalize;
  G_OBJECT_CLASS (klass)->get_property = web_server_class_get_property;
  G_OBJECT_CLASS (klass)->set_property = web_server_class_set_property;

  const GParamFlags flags1 = G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS;
  const GParamFlags flags2 = G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS;
        GType gtype = G_TYPE_FROM_CLASS (klass);
  const GSignalFlags flags3 = G_SIGNAL_RUN_FIRST;
  const GSignalAccumulator accumulator1 = g_signal_accumulator_true_handled;
  const GSignalCMarshaller marshaller1 = g_cclosure_marshal_VOID__OBJECT;
  const GSignalCVaMarshaller vamarshaller1 = g_cclosure_marshal_VOID__OBJECTv;

  properties [prop_active] = g_param_spec_boolean ("active", "active", "active", 0, flags2);
  g_object_class_install_properties (G_OBJECT_CLASS (klass), prop_number, properties);
  signals [signal_incoming] = g_signal_new ("incoming", gtype, flags3, 0, accumulator1, NULL, marshaller1, G_TYPE_BOOLEAN, 1, WEB_TYPE_CLIENT);
  g_signal_set_va_marshaller (signals [signal_incoming], gtype, vamarshaller1);
}

static void web_server_init (WebServer* self)
{
  self->endpoints = (gpointer) g_ptr_array_new_with_free_func ((GDestroyNotify) web_endpoint_unref);
}

WebServer* web_server_new (const gchar* first_property_name, ...)
{
  GObject* object;
  va_list l;

  va_start (l, first_property_name);
  object = g_object_new_valist (WEB_TYPE_SERVER, first_property_name, l);
return (va_end (l), WEB_SERVER (object));
}

void web_server_start (WebServer* web_server)
{
  g_return_if_fail (WEB_IS_SERVER (web_server));
  WebServer* self = (web_server);
  guint i;

  if ((self->active == FALSE) && (self->active = TRUE))
    {
      for (i = 0; i < self->endpoints->count; ++i)
        {
          web_endpoint_start (self->endpoints->ar [i], self);
        }

      g_application_hold (G_APPLICATION (self));
      g_object_notify_by_pspec (G_OBJECT (self), properties [prop_active]);
    }
}

void web_server_stop (WebServer* web_server)
{
  g_return_if_fail (WEB_IS_SERVER (web_server));
  WebServer* self = (web_server);
  guint i;

  if ((self->active == TRUE) && !(self->active = FALSE))
    {
      for (i = 0; i < self->endpoints->count; ++i)
        {
          web_endpoint_stop (self->endpoints->ar [i]);
        }

      g_application_release (G_APPLICATION (self));
      g_object_notify_by_pspec (G_OBJECT (self), properties [prop_active]);
    }
}

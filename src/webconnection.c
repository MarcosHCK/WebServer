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
#include <gio/gnetworking.h>
#include <webconnection.h>

#define WEB_CONNECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), WEB_TYPE_CONNECTION, WebConnectionClass))
#define WEB_IS_CONNECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), WEB_TYPE_CONNECTION))
#define WEB_CONNECTION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), WEB_TYPE_CONNECTION, WebConnectionClass))
typedef struct _WebConnectionClass WebConnectionClass;
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

struct _WebConnection
{
  GObject parent;

  /* private */
  GIOStream* connection;
  WebHttpVersion http_version;
  guint is_https : 1;
  GSocket* socket;
};

struct _WebConnectionClass
{
  GObjectClass parent;
};

enum
{
  prop_0,
  prop_http_version,
  prop_is_https,
  prop_socket,
  prop_number,
};

enum
{
  signal_connected,
  signal_disconnected,
  signal_request_started,
  signal_number,
};

G_DEFINE_FINAL_TYPE (WebConnection, web_connection, G_TYPE_OBJECT);
static GParamSpec* properties [prop_number] = {0};
static guint signals [signal_number] = {0};

static void web_connection_class_dispose (GObject* pself)
{
  WebConnection* self = (gpointer) pself;
  _g_object_unref0 (self->connection);
  _g_object_unref0 (self->socket);
G_OBJECT_CLASS (web_connection_parent_class)->dispose (pself);
}

static void web_connection_class_finalize (GObject* pself)
{
  WebConnection* self = (gpointer) pself;
G_OBJECT_CLASS (web_connection_parent_class)->finalize (pself);
}

static void web_connection_class_get_property (GObject* pself, guint property_id, GValue* value, GParamSpec* pspec)
{
  WebConnection* self = (gpointer) pself;

  switch (property_id)
    {
      case prop_http_version:
        g_value_set_enum (value, web_connection_get_http_version (self));
        break;
      case prop_is_https:
        g_value_set_boolean (value, web_connection_get_is_https (self));
        break;
      case prop_socket:
        g_value_set_object (value, web_connection_get_socket (self));
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (pself, property_id, pspec);
        break;
    }
}

static void web_connection_class_set_property (GObject* pself, guint property_id, const GValue* value, GParamSpec* pspec)
{
  WebConnection* self = (gpointer) pself;

  switch (property_id)
    {
      case prop_http_version:
        self->http_version = g_value_get_enum (value);
        break;
      case prop_is_https:
        self->is_https = g_value_get_boolean (value);
        break;
      case prop_socket:
        g_set_object (& self->socket, g_value_get_object (value));
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (pself, property_id, pspec);
        break;
    }
}

static void web_connection_class_init (WebConnectionClass* klass)
{
  G_OBJECT_CLASS (klass)->dispose = web_connection_class_dispose;
  G_OBJECT_CLASS (klass)->finalize = web_connection_class_finalize;
  G_OBJECT_CLASS (klass)->get_property = web_connection_class_get_property;
  G_OBJECT_CLASS (klass)->set_property = web_connection_class_set_property;

  const GType gtype = G_TYPE_FROM_CLASS (klass);
  const GParamFlags flags1 = G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS;
  const GSignalFlags flags2 = G_SIGNAL_RUN_FIRST;
  const GSignalFlags flags3 = G_SIGNAL_RUN_LAST;
  const GSignalCMarshaller marshaller1 = g_cclosure_marshal_VOID__OBJECT;

  properties [prop_http_version] = g_param_spec_enum ("http-version", "http-version", "http-version", WEB_TYPE_HTTP_VERSION, WEB_HTTP_VERSION_2_0, flags1);
  properties [prop_is_https] = g_param_spec_boolean ("is-https", "is-https", "is-https", FALSE, flags1);
  properties [prop_socket] = g_param_spec_object ("socket", "socket", "socket", G_TYPE_SOCKET, flags1);
  g_object_class_install_properties (G_OBJECT_CLASS (klass), prop_number, properties);
  signals [signal_connected] = g_signal_new ("connected", gtype, flags2, 0, NULL, NULL, marshaller1, G_TYPE_NONE, 0);
  signals [signal_disconnected] = g_signal_new ("disconnected", gtype, flags2, 0, NULL, NULL, marshaller1, G_TYPE_NONE, 0);
  signals [signal_request_started] = g_signal_new ("request-started", gtype, flags3, 0, NULL, NULL, marshaller1, G_TYPE_NONE, 1, G_TYPE_OBJECT);
}

static void web_connection_init (WebConnection* self)
{
}

WebConnection* web_connection_new (GSocket* socket, WebHttpVersion http_version, gboolean is_https)
{
  return g_object_new (WEB_TYPE_CONNECTION, "socket", socket, "http-version", http_version, "is-https", is_https, NULL);
}

void web_connection_accepted (WebConnection* web_connection)
{
  g_return_if_fail (WEB_IS_CONNECTION (web_connection));
  WebConnection* self = (web_connection);
  GSocketConnection* socket_connection = NULL;

  self->connection = (gpointer) g_socket_connection_factory_create_connection (self->socket);

  g_socket_set_option (self->socket, IPPROTO_TCP, TCP_NODELAY, TRUE, NULL);
  g_signal_emit (self, signals [signal_connected], 0);
}

WebHttpVersion web_connection_get_http_version (WebConnection* web_connection)
{
  g_return_val_if_fail (WEB_IS_CONNECTION (web_connection), 0);
return web_connection->http_version;
}

gboolean web_connection_get_is_https (WebConnection* web_connection)
{
  g_return_val_if_fail (WEB_IS_CONNECTION (web_connection), 0);
return web_connection->is_https;
}

GSocket* web_connection_get_socket (WebConnection* web_connection)
{
  g_return_val_if_fail (WEB_IS_CONNECTION (web_connection), NULL);
return web_connection->socket;
}

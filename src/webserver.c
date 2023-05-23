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
static void web_server_g_async_initable_iface (GAsyncInitableIface* iface);
static void web_server_g_initable_iface (GInitableIface* iface);

struct _WebServer
{
  GApplication parent;

  /* private */
  gchar* home_dir;
  guint port : 16;
  guint active : 1;

  union
  {
    struct
    {
      GSocket* socket4;
      GSocket* socket6;
    };

    GSocket* sockets [2];
  };

  GCancellable* cancellable;
};

struct _WebServerClass
{
  GApplicationClass parent;
};

enum
{
  prop_0,
  prop_active,
  prop_port,
  prop_number,
};

G_DEFINE_FINAL_TYPE_WITH_CODE
  (WebServer, web_server, G_TYPE_APPLICATION,
   G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE, web_server_g_initable_iface)
   G_IMPLEMENT_INTERFACE (G_TYPE_ASYNC_INITABLE, web_server_g_async_initable_iface));

static GParamSpec* properties [prop_number] = {0};

static void web_server_g_async_initable_iface (GAsyncInitableIface* iface)
{
}

static gboolean web_server_g_initable_iface_init (GInitable* pself, GCancellable* cancellable, GError** error)
{
  WebServer* self = (gpointer) pself;
  GError* tmperr = NULL;

  G_STATIC_ASSERT (
    G_STRUCT_OFFSET (WebServer, socket4) +
    G_SIZEOF_MEMBER (WebServer, socket4) ==
    G_STRUCT_OFFSET (WebServer, socket6));

  const GSocketFamily families [] =
    {
      G_SOCKET_FAMILY_IPV6,
      G_SOCKET_FAMILY_IPV4,
    };

  const GSocketType socket_type = G_SOCKET_TYPE_STREAM;
  const GSocketProtocol socket_proto = G_SOCKET_PROTOCOL_TCP;

  for (guint i = 0; i < G_N_ELEMENTS (families); ++i)
    {
      GInetAddress* inet_address = NULL;
      GSocket* socket = NULL;
      GSocketAddress* socket_address = NULL;
      GSocketFamily family = families [i];

      if ((socket = g_socket_new (family, socket_type, socket_proto, &tmperr)), G_UNLIKELY (tmperr != NULL))
        {
          g_object_unref (socket);
          g_propagate_error (error, tmperr);
          return FALSE;
        }

      inet_address = g_inet_address_new_any (family);
      socket_address = g_inet_socket_address_new (inet_address, self->port);
                      _g_object_unref0 (inet_address);

      g_socket_set_blocking (socket, FALSE);
      g_socket_set_listen_backlog (socket, 3);

      if ((g_socket_bind (socket, socket_address, TRUE, &tmperr), _g_object_unref0 (socket_address)), G_UNLIKELY (tmperr != NULL))
        {
          g_object_unref (socket);
          g_propagate_error (error, tmperr);
          return FALSE;
        }

      if ((g_socket_listen (socket, &tmperr)), G_UNLIKELY (tmperr != NULL))
        {
          g_object_unref (socket);
          g_propagate_error (error, tmperr);
          return FALSE;
        }

      self->sockets [i] = socket;

      if (i == 0 && g_socket_speaks_ipv4 (socket))
        break;
    }
return TRUE;
}

static void web_server_g_initable_iface (GInitableIface* iface)
{
  iface->init = web_server_g_initable_iface_init;
}

static void web_server_class_dispose (GObject* pself)
{
  WebServer* self = (gpointer) pself;
  _g_object_unref0 (self->socket4);
  _g_object_unref0 (self->socket6);
  _g_object_unref0 (self->cancellable);
G_OBJECT_CLASS (web_server_parent_class)->dispose (pself);
}

static void web_server_class_finalize (GObject* pself)
{
  WebServer* self = (gpointer) pself;
  _g_free0 (self->home_dir);
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
      case prop_port:
        g_value_set_uint (value, web_server_get_port (self));
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
      case prop_port:
        self->port = (guint16) g_value_get_uint (value);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (pself, property_id, pspec);
        break;
    }
}

static void web_server_class_init (WebServerClass* klass)
{
  G_OBJECT_CLASS (klass)->dispose = web_server_class_dispose;
  G_OBJECT_CLASS (klass)->finalize = web_server_class_finalize;
  G_OBJECT_CLASS (klass)->get_property = web_server_class_get_property;
  G_OBJECT_CLASS (klass)->set_property = web_server_class_set_property;

  const GParamFlags flags1 = G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS;
  const GParamFlags flags2 = G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS;

  properties [prop_active] = g_param_spec_boolean ("active", "active", "active", 0, flags2);
  properties [prop_port] = g_param_spec_uint ("port", "port", "port", 0, G_MAXUINT16, 0, flags1);
  g_object_class_install_properties (G_OBJECT_CLASS (klass), prop_number, properties);
}

static void web_server_init (WebServer* self)
{
}

WebServer* web_server_new (guint16 port, GError** error)
{
 return g_initable_new (WEB_TYPE_SERVER, NULL, error, "port", (guint) port, NULL);
}

const gchar* web_server_get_home_dir (WebServer* web_server)
{
  g_return_val_if_fail (WEB_IS_SERVER (web_server), NULL);
return web_server->home_dir;
}

guint16 web_server_get_port (WebServer* web_server)
{
  g_return_val_if_fail (WEB_IS_SERVER (web_server), 0);
return web_server->port;
}

static gboolean accept_source (GSocket* socket, GIOCondition condition, WebServer* self)
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
              g_application_quit (G_APPLICATION (self));
            }

          case G_IO_IN:
            {
              GError* tmperr = NULL;
              GSocket* client = g_socket_accept (socket, self->cancellable, &tmperr);

              g_socket_close (client, NULL);
              g_object_unref (client);
              break;
            }
        }
    }
return G_SOURCE_CONTINUE;
}

void web_server_start (WebServer* web_server)
{
  g_return_if_fail (WEB_IS_SERVER (web_server));
  WebServer* self = (web_server);

  if ((self->active == FALSE) && (self->active = TRUE))
    {
      _g_object_unref0 (self->cancellable);

      self->cancellable = g_cancellable_new ();

      for (guint i = 0; i < G_N_ELEMENTS (self->sockets); ++i)
      if (self->sockets [i] != NULL)
        {
          GSourceFunc func = (GSourceFunc) accept_source;
          GDestroyNotify notify = (GDestroyNotify) g_object_unref;
          GSource* source = g_socket_create_source (self->sockets [i], G_IO_IN, self->cancellable);
          GMainContext* context = g_main_context_get_thread_default ();

          g_source_attach (source, context);
          g_source_set_callback (source, func, self, notify);
          g_source_set_priority (source, G_PRIORITY_DEFAULT_IDLE);
          g_source_set_static_name (source, "[WebServer.AcceptSource]");
          g_object_ref ((g_source_unref (source), self));
        }

      g_application_hold (G_APPLICATION (self));
      g_object_notify_by_pspec (G_OBJECT (self), properties [prop_active]);
    }
}

void web_server_stop (WebServer* web_server)
{
  g_return_if_fail (WEB_IS_SERVER (web_server));
  WebServer* self = (web_server);

  if ((self->active == TRUE) && !(self->active = FALSE))
    {
      g_cancellable_cancel (self->cancellable);
      g_application_release (G_APPLICATION (self));
      g_object_notify_by_pspec (G_OBJECT (self), properties [prop_active]);
    }
}

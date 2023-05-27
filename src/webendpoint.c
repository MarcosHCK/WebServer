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
#include <glib/gi18n.h>
#include <gio/gnetworking.h>
#include <marshals.h>
#include <webendpoint.h>

#define WEB_ENDPOINT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), WEB_TYPE_ENDPOINT, WebEndpointClass))
#define WEB_IS_ENDPOINT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), WEB_TYPE_ENDPOINT))
#define WEB_ENDPOINT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), WEB_TYPE_ENDPOINT, WebEndpointClass))
typedef struct _WebEndpointClass WebEndpointClass;
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

struct _WebEndpoint
{
  GObject parent;

  /* private */
  GSocket* socket;
  GSource* source;
};

struct _WebEndpointClass
{
  GObjectClass parent;
};

enum
{
  prop_0,
  prop_socket,
  prop_number,
};

enum
{
  signal_failed_connection,
  signal_new_connection,
  signal_number,
};

G_DEFINE_FINAL_TYPE (WebEndpoint, web_endpoint, G_TYPE_OBJECT);
static GParamSpec* properties [prop_number] = {0};
static guint signals [signal_number] = {0};

static gboolean accept_source (GSocket* socket, GIOCondition condition, WebEndpoint* self)
{
  if (g_source_is_destroyed (g_main_current_source ()))
    return G_SOURCE_REMOVE;
  else
    {
      if (condition & ~(G_IO_IN | G_IO_ERR | G_IO_HUP))
        g_assert_not_reached ();

      if (condition & (G_IO_ERR | G_IO_HUP))
        {
          GError* tmperr = NULL;

          if ((g_socket_condition_wait (socket, G_IO_ERR | G_IO_HUP, NULL, &tmperr)), G_UNLIKELY (tmperr == NULL))
            g_error (_("Socket error reported but no error returned"));
          else
            {
              g_signal_emit (self, signals [signal_failed_connection], 0, tmperr);
              g_error_free (tmperr);
            }
        }

      if (condition & G_IO_IN)
        {
          GSocket* client_socket = NULL;
          gboolean handled = FALSE;
          GError* tmperr = NULL;

          if ((client_socket = g_socket_accept (socket, NULL, &tmperr)), G_UNLIKELY (tmperr != NULL))
            {
              _g_object_unref0 (client_socket);
              g_signal_emit (self, signals [signal_failed_connection], 0, tmperr);
              g_error_free (tmperr);
            }

          if ((g_signal_emit (self, signals [signal_new_connection], 0, client_socket, &handled)), !handled)
            g_socket_close (client_socket, NULL);
        }
    }
return G_SOURCE_CONTINUE;
}

static void web_endpoint_class_constructed (GObject* pself)
{
  WebEndpoint* self = (gpointer) pself;
G_OBJECT_CLASS (web_endpoint_parent_class)->constructed (pself);
  GSocket* socket = self->socket;
  GSource* source = g_socket_create_source (socket, G_IO_IN, NULL);
  GSourceFunc func = (GSourceFunc) accept_source;
  GMainContext* context = g_main_context_get_thread_default ();

  g_source_set_callback (source, func, self, NULL);
  g_source_set_priority (source, G_PRIORITY_DEFAULT);
#if GLIB_CHECK_VERSION(2, 70, 0)
  g_source_set_static_name (source, "[WebEndpoint.AcceptSource]");
#else // GLIB_CHECK_VERSION(2, 70, 0)
  g_source_set_name (source, "[WebEndpoint.AcceptSource]");
#endif // GLIB_CHECK_VERSION(2, 70, 0)
  g_source_attach ((self->source = source), context);
}

static void web_endpoint_class_dispose (GObject* pself)
{
  WebEndpoint* self = (gpointer) pself;
  _g_object_unref0 (self->socket);
G_OBJECT_CLASS (web_endpoint_parent_class)->dispose (pself);
}

static void web_endpoint_class_finalize (GObject* pself)
{
  WebEndpoint* self = (gpointer) pself;
  g_source_destroy (self->source);
  g_source_unref (self->source);
G_OBJECT_CLASS (web_endpoint_parent_class)->finalize (pself);
}

static void web_endpoint_class_get_property (GObject* pself, guint property_id, GValue* value, GParamSpec* pspec)
{
  WebEndpoint* self = (gpointer) pself;

  switch (property_id)
    {
      case prop_socket:
        g_value_set_object (value, web_endpoint_get_socket (self));
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (pself, property_id, pspec);
        break;
    }
}

static void web_endpoint_class_set_property (GObject* pself, guint property_id, const GValue* value, GParamSpec* pspec)
{
  WebEndpoint* self = (gpointer) pself;

  switch (property_id)
    {
      case prop_socket:
        g_set_object (& self->socket, g_value_get_object (value));
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (pself, property_id, pspec);
        break;
    }
}

static void web_endpoint_class_init (WebEndpointClass* klass)
{
  G_OBJECT_CLASS (klass)->constructed = web_endpoint_class_constructed;
  G_OBJECT_CLASS (klass)->dispose = web_endpoint_class_dispose;
  G_OBJECT_CLASS (klass)->finalize = web_endpoint_class_finalize;
  G_OBJECT_CLASS (klass)->get_property = web_endpoint_class_get_property;
  G_OBJECT_CLASS (klass)->set_property = web_endpoint_class_set_property;

  const GType gtype = G_TYPE_FROM_CLASS (klass);
  const GParamFlags flags1 = G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS;
  const GSignalFlags flags2 = G_SIGNAL_RUN_FIRST;
  const GSignalAccumulator accum1 = g_signal_accumulator_true_handled;
  const GSignalCMarshaller marshaller1 = web_cclosure_marshal_VOID__BOXED;
  const GSignalCMarshaller marshaller2 = web_cclosure_marshal_BOOLEAN__OBJECT;

  properties [prop_socket] = g_param_spec_object ("socket", "socket", "socket", G_TYPE_SOCKET, flags1);
  g_object_class_install_properties (G_OBJECT_CLASS (klass), prop_number, properties);
  signals [signal_failed_connection] = g_signal_new ("failed-connection", gtype, flags2, 0, NULL, NULL, marshaller1, G_TYPE_NONE, 1, G_TYPE_ERROR);
  signals [signal_new_connection] = g_signal_new ("new-connection", gtype, flags2, 0, accum1, NULL, marshaller2, G_TYPE_BOOLEAN, 1, G_TYPE_SOCKET);
}

static void web_endpoint_init (WebEndpoint* self)
{
}

WebEndpoint* web_endpoint_new (GSocket* socket, GError** error)
{
  g_return_val_if_fail (G_IS_SOCKET (socket), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  GError* tmperr = NULL;
  gboolean listening;

  if ((g_socket_get_option (socket, SOL_SOCKET, SO_ACCEPTCONN, &listening, &tmperr)), G_UNLIKELY (tmperr != NULL))
    {
      g_prefix_error (error, _("Could not import existing socket: "));
      return NULL;
    }

  if (!listening && !g_socket_is_connected (socket))
    {
      g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED, _("Can't import unconnected socket"));
      return NULL;
    }
return g_object_new (WEB_TYPE_ENDPOINT, "socket", socket, NULL);
}

GSocket* web_endpoint_get_socket (WebEndpoint* web_endpoint)
{
  g_return_val_if_fail (WEB_IS_ENDPOINT (web_endpoint), NULL);
return web_endpoint->socket;
}

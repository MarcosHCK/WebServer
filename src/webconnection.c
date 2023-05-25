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
#include <webconnection.h>

#define WEB_CONNECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), WEB_TYPE_CONNECTION, WebConnectionClass))
#define WEB_IS_CONNECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), WEB_TYPE_CONNECTION))
#define WEB_CONNECTION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), WEB_TYPE_CONNECTION, WebConnectionClass))
typedef struct _WebConnectionClass WebConnectionClass;
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
typedef struct _InputIO InputIO;

struct _WebConnection
{
  GObject parent;

  /* private */
  GIOStream* connection;
  WebHttpVersion http_version;
  GInputStream* input_stream;
  GIOStream* iostream;
  guint is_https : 1;
  GOutputStream* output_stream;
  GSocket* socket;
  GSource* source;

  struct _InputIO
  {
    gsize allocated;
    gpointer buffer;
    GQueue frames;
    gsize length;
    GQueue lines;
  } io;
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
  signal_request_failed,
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
  _g_object_unref0 (self->input_stream);
  _g_object_unref0 (self->iostream);
  _g_object_unref0 (self->output_stream);
  _g_object_unref0 (self->socket);
G_OBJECT_CLASS (web_connection_parent_class)->dispose (pself);
}

static void web_connection_class_finalize (GObject* pself)
{
  WebConnection* self = (gpointer) pself;
  g_free (self->io.buffer);
  g_queue_clear_full (& self->io.frames, (GDestroyNotify) g_strfreev);
  g_queue_clear_full (& self->io.lines, (GDestroyNotify) g_free);

  if (G_LIKELY (self->source != NULL))
    {
      g_source_destroy (self->source);
      g_source_unref (self->source);
    }
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
  const GSignalCMarshaller marshaller1 = g_cclosure_marshal_VOID__VOID;
  const GSignalCMarshaller marshaller2 = g_cclosure_marshal_VOID__BOXED;
  const GSignalCMarshaller marshaller3 = g_cclosure_marshal_VOID__OBJECT;

  properties [prop_http_version] = g_param_spec_enum ("http-version", "http-version", "http-version", WEB_TYPE_HTTP_VERSION, WEB_HTTP_VERSION_2_0, flags1);
  properties [prop_is_https] = g_param_spec_boolean ("is-https", "is-https", "is-https", FALSE, flags1);
  properties [prop_socket] = g_param_spec_object ("socket", "socket", "socket", G_TYPE_SOCKET, flags1);
  g_object_class_install_properties (G_OBJECT_CLASS (klass), prop_number, properties);
  signals [signal_connected] = g_signal_new ("connected", gtype, flags2, 0, NULL, NULL, marshaller1, G_TYPE_NONE, 0);
  signals [signal_disconnected] = g_signal_new ("disconnected", gtype, flags2, 0, NULL, NULL, marshaller1, G_TYPE_NONE, 0);
  signals [signal_request_failed] = g_signal_new ("request-failed", gtype, flags3, 0, NULL, NULL, marshaller2, G_TYPE_NONE, 1, G_TYPE_ERROR);
  signals [signal_request_started] = g_signal_new ("request-started", gtype, flags3, 0, NULL, NULL, marshaller3, G_TYPE_NONE, 1, G_TYPE_OBJECT);
}

static void web_connection_init (WebConnection* self)
{
  self->io.allocated = 0;
  self->io.buffer = NULL;
  self->io.length = 0;

  g_queue_init (& self->io.frames);
  g_queue_init (& self->io.lines);
}

WebConnection* web_connection_new (GSocket* socket, WebHttpVersion http_version, gboolean is_https)
{
  return g_object_new (WEB_TYPE_CONNECTION, "socket", socket, "http-version", http_version, "is-https", is_https, NULL);
}

static GIOStatus input_io_read (InputIO* io, GPollableInputStream* stream, GError** error)
{
  GError* tmperr = NULL;
  gpointer block;
  gssize read;

  const goffset blocksz = 256;

  while (TRUE)
    {
      if ((io->length + blocksz) > io->allocated)
        {
          io->allocated = io->length + blocksz;
          io->buffer = g_realloc (io->buffer, io->allocated);
        }

      block = G_STRUCT_MEMBER_P (io->buffer, io->length);
      read = g_pollable_input_stream_read_nonblocking (stream, block, blocksz, NULL, &tmperr);

      if (G_UNLIKELY (tmperr != NULL))
        {
          if (g_error_matches (tmperr, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK))
            {
              g_error_free (tmperr);
              g_assert (read == -1);
              break;
            }
          else
            {
              g_propagate_error (error, tmperr);
              return G_IO_STATUS_ERROR;
            }
        }
      else if (read == 0)
        return G_IO_STATUS_EOF;
      else
        {
          gchar *line, **frame;
          guint framesz, linesz, ignore;
          gsize i, j, last = 0;

          io->length += read;

          for (i = io->length - read; i < io->length; ++i)
            {
              if (G_STRUCT_MEMBER (gchar, io->buffer, i) == '\n')
                {
                  ignore = (i > 0 && G_STRUCT_MEMBER (gchar, io->buffer, i - 1) == '\r') ? 1 : 0;
                  linesz = (i - last) - ignore;
                  line = (linesz == 0) ? NULL : g_strndup (& G_STRUCT_MEMBER (gchar, io->buffer, last), linesz);
                  last = i + 1;

                  if (linesz > 0)
                    g_queue_push_tail (& io->lines, line);
                  else
                    {
                      framesz = g_queue_get_length (& io->lines);
                      frame = g_new0 (gchar*, framesz + 1);

                      for (j = 0; j < framesz; ++j)
                        {
                          frame [j] = g_queue_pop_head (& io->lines);
                        }

                      g_queue_push_tail (& io->frames, frame);
                    }
                }
            }

          if (last > 0)
            {
              memmove (io->buffer, io->buffer + last, io->length - last);
              io->length -= last;
            }
        }
    }
return G_IO_STATUS_NORMAL;
}

static gboolean input_source (GSocket* socket, GIOCondition condition, WebConnection* self)
{
  if (g_source_is_destroyed (g_main_current_source ()))
    return G_SOURCE_REMOVE;
  else
    {
      GError* tmperr = NULL;

      if (condition & ~(G_IO_IN | G_IO_ERR | G_IO_HUP))
        g_assert_not_reached ();

      if (condition & (G_IO_ERR | G_IO_HUP))
        {
          if ((g_socket_condition_wait (socket, G_IO_ERR | G_IO_HUP, NULL, &tmperr)), G_UNLIKELY (tmperr == NULL))
            g_error (_("Socket error reported but no error returned"));
          else
            {
              if (g_error_matches (tmperr, G_IO_ERROR, G_IO_ERROR_CONNECTION_CLOSED))
                {
                  g_signal_emit (self, signals [signal_disconnected], 0);
                  return (g_error_free (tmperr), G_SOURCE_REMOVE);
                }
              else
                {
                  g_signal_emit (self, signals [signal_request_started], 0, NULL);
                  g_signal_emit (self, signals [signal_request_failed], 0, tmperr);
                  return (g_error_free (tmperr), G_SOURCE_REMOVE);
                }
            }
        }

      if (condition & G_IO_IN)
        {
          GIOStatus status;

          if ((status = input_io_read (& self->io, G_POLLABLE_INPUT_STREAM (self->input_stream), &tmperr)), G_UNLIKELY (tmperr != NULL))
            {
              g_signal_emit (self, signals [signal_request_started], 0, NULL);
              g_signal_emit (self, signals [signal_request_failed], 0, tmperr);
              return (g_error_free (tmperr), G_SOURCE_REMOVE);
            }
          else
            {
              switch (status)
                {
                  case G_IO_STATUS_EOF:
                    g_signal_emit (self, signals [signal_disconnected], 0);
                    return G_SOURCE_REMOVE;
                  case G_IO_STATUS_NORMAL:
                    {
                      gchar** frame;

                      while ((frame = g_queue_pop_head (& self->io.frames)) != NULL)
                        {
                          g_print ("FRAME\n");
                          g_strfreev (frame);
                        }
                      break;
                    }
                  default: g_assert_not_reached ();
                }
            }
        }
    }
return G_SOURCE_CONTINUE;
}

void web_connection_accepted (WebConnection* web_connection)
{
  g_return_if_fail (WEB_IS_CONNECTION (web_connection));
  WebConnection* self = (web_connection);
  GSocketConnection* socket_connection = NULL;
  GSource* source = NULL;

  self->connection = (gpointer) g_socket_connection_factory_create_connection (self->socket);
  self->input_stream = g_object_ref (g_io_stream_get_input_stream (self->connection));
  self->output_stream = g_object_ref (g_io_stream_get_output_stream (self->connection));
  self->iostream = g_simple_io_stream_new (self->input_stream, self->output_stream);
  self->source = (source = g_socket_create_source (self->socket, G_IO_IN, NULL));

  g_socket_set_option (self->socket, IPPROTO_TCP, TCP_NODELAY, TRUE, NULL);
  g_source_set_callback (source, (GSourceFunc) input_source, self, NULL);
  g_source_set_priority (source, G_PRIORITY_DEFAULT);
#if GLIB_CHECK_VERSION(2, 70, 0)
  g_source_set_static_name (source, "[WebConnection.InputSource]");
#else // GLIB_CHECK_VERSION(2, 70, 0)
  g_source_set_name (source, "[WebConnection.InputSource]");
#endif // GLIB_CHECK_VERSION(2, 70, 0)
  g_source_attach (source, g_main_context_get_thread_default ());
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

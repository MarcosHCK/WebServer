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
#include <glib/gi18n.h>
#include <gio/gnetworking.h>
#include <webconnection.h>
#include <webmessagefields.h>
#include <webmessagemethods.h>
#include <webrequest.h>
#include <webparser.h>

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
  guint is_https : 1;
  GInputStream* input_stream;
  GIOStream* iostream;
  GOutputStream* output_stream;
  GSocket* socket;
  GSocketConnection* socket_connection;

  struct _InputIO
  {
    gsize allocated;
    gpointer buffer;
    gsize length;
    gsize unscanned;

    WebParser parser;
  } in;
};

struct _WebConnectionClass
{
  GObjectClass parent;
};

enum
{
  prop_0,
  prop_is_https,
  prop_socket,
  prop_number,
};

G_DEFINE_FINAL_TYPE (WebConnection, web_connection, G_TYPE_OBJECT);
static GParamSpec* properties [prop_number] = {0};

static void web_connection_class_constructed (GObject* pself)
{
  WebConnection* self = (gpointer) pself;
G_OBJECT_CLASS (web_connection_parent_class)->constructed (pself);
  if (!self->is_https)
    {
      self->socket_connection = g_socket_connection_factory_create_connection (self->socket);
      self->input_stream = g_object_ref (g_io_stream_get_input_stream (G_IO_STREAM (self->socket_connection)));
      self->output_stream = g_object_ref (g_io_stream_get_output_stream (G_IO_STREAM (self->socket_connection)));
      self->iostream = g_object_ref (G_IO_STREAM (self->socket_connection));
    }
  else g_assert_not_reached ();
}

static void web_connection_class_dispose (GObject* pself)
{
  WebConnection* self = (gpointer) pself;
  _g_object_unref0 (self->input_stream);
  _g_object_unref0 (self->iostream);
  _g_object_unref0 (self->output_stream);
  _g_object_unref0 (self->socket);
  _g_object_unref0 (self->socket_connection);
G_OBJECT_CLASS (web_connection_parent_class)->dispose (pself);
}

static void web_connection_class_finalize (GObject* pself)
{
  WebConnection* self = (gpointer) pself;
  g_free (self->in.buffer);
  web_parser_clear (& self->in.parser);
G_OBJECT_CLASS (web_connection_parent_class)->finalize (pself);
}

static void web_connection_class_set_property (GObject* pself, guint property_id, const GValue* value, GParamSpec* pspec)
{
  WebConnection* self = (gpointer) pself;

  switch (property_id)
    {
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
  G_OBJECT_CLASS (klass)->constructed = web_connection_class_constructed;
  G_OBJECT_CLASS (klass)->dispose = web_connection_class_dispose;
  G_OBJECT_CLASS (klass)->finalize = web_connection_class_finalize;
  G_OBJECT_CLASS (klass)->set_property = web_connection_class_set_property;

  const GType gtype = G_TYPE_FROM_CLASS (klass);
  const GSignalFlags flags1 = G_SIGNAL_RUN_FIRST;
  const GSignalCMarshaller marshal1 = web_cclosure_marshal_VOID__BOXED;
  const GSignalCMarshaller marshal2 = web_cclosure_marshal_VOID__OBJECT;

  properties [prop_is_https] = g_param_spec_boolean ("is-https", "is-https", "is-https", 0, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  properties [prop_socket] = g_param_spec_object ("socket", "socket", "socket", G_TYPE_SOCKET, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties (G_OBJECT_CLASS (klass), prop_number, properties);
}

static void web_connection_init (WebConnection* self)
{
  self->in.allocated = 0;
  self->in.buffer = NULL;
  self->in.length = 0;
  self->in.unscanned = 0;

  web_parser_init (& self->in.parser);
}

WebConnection* web_connection_new (GSocket* socket, gboolean is_https)
{
  return g_object_new (WEB_TYPE_CONNECTION, "socket", socket, "is-https", is_https, NULL);
}

static GIOStatus process_in (struct _InputIO* io, GPollableInputStream* stream, GError** error)
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
      else if (read == 0 && io->length == 0)
        return G_IO_STATUS_EOF;
      else
        {
          gchar *line, **frame;
          guint framesz, linesz, ignore;
          gsize unscanned = io->unscanned;
          gsize i, j, last = 0;

          io->length += read;
          io->unscanned = 0;

          for (i = (io->length - unscanned - read); i < io->length; ++i)
            {
              if (G_STRUCT_MEMBER (gchar, io->buffer, i) == '\n')
                {
                  ignore = (i > 0 && G_STRUCT_MEMBER (gchar, io->buffer, i - 1) == '\r') ? 1 : 0;
                  linesz = (i - last) - ignore;
                  line = & G_STRUCT_MEMBER (gchar, io->buffer, last);
                  last = i + 1;

                  if ((web_parser_feed (& io->parser, line, linesz, &tmperr)), G_UNLIKELY (tmperr != NULL))
                    {
                      g_propagate_error (error, tmperr);
                      return G_IO_STATUS_ERROR;
                    }
                  else if (io->parser.complete == TRUE)
                    {
                      io->unscanned = io->length - i - 1;
                      io->length -= last;

                      memmove (io->buffer, io->buffer + last, io->length);
                      return G_IO_STATUS_NORMAL;
                    }
                }
            }

          if (last > 0)
            {
              io->length -= last;
              memmove (io->buffer, io->buffer + last, io->length);
            }
        }
    }
return (io->parser.complete == FALSE) ? G_IO_STATUS_AGAIN : G_IO_STATUS_NORMAL;
}

WebMessage* web_connection_step (WebConnection* self, GError** error)
{
  WebMessage* web_message = NULL;
  GError* tmperr = NULL;
  GIOStatus status = 0;

  if ((status = process_in (& self->in, G_POLLABLE_INPUT_STREAM (self->input_stream), &tmperr)), G_UNLIKELY (tmperr != NULL))
    g_propagate_error (error, tmperr);
  else
    {
      switch (status)
        {
          case G_IO_STATUS_AGAIN:
            break;

          case G_IO_STATUS_EOF:
          case G_IO_STATUS_ERROR:
            g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_CONNECTION_CLOSED, "Connection closed");
            break;

          case G_IO_STATUS_NORMAL:
            {
              struct _InputIO* io = NULL;
              WebRequest* web_request = NULL;
              WebParserField* field;

              io = G_STRUCT_MEMBER_P (self, G_STRUCT_OFFSET (WebConnection, in));
              web_message = web_request_new();
              web_request = WEB_REQUEST (web_message);

              web_request_set_http_version (web_request, io->parser.http_version);
              web_request_set_method (web_request, io->parser.method);
              web_request_set_uri (web_request, io->parser.uri);

              while ((field = g_queue_pop_head (& io->parser.fields)) != NULL)
                {
                  const gchar* old = NULL;
                  const gchar* name = field->name;
                  const gchar* value = field->value;

                  if ((old = web_message_get_field (web_message, name)) == NULL)
                    {
                      gchar* name_ptr = g_steal_pointer (&field->name);
                      gchar* value_ptr = g_steal_pointer (&field->value);
                      web_message_insert_field_take (web_message, name_ptr, value_ptr);
                    }
                  else
                    {
                      web_parser_field_add_value (field, old, strlen (old));
                      web_message_delete_field (web_message, name);
                      g_queue_push_head (& io->parser.fields, field);
                      continue;
                    }

                  web_parser_field_free (field);
                }

              web_parser_clear (& io->parser);
              web_parser_init (& io->parser);
              break;
            }
        }
    }
return (web_message);
}

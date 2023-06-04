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
#include <gio/gnetworking.h>
#include <glib/gi18n.h>
#include <webconnection.h>
#include <webmessage.h>
#include <webmessagefields.h>
#include <webmessagemethods.h>
#include <webparser.h>

#define WEB_CONNECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), WEB_TYPE_CONNECTION, WebConnectionClass))
#define WEB_IS_CONNECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), WEB_TYPE_CONNECTION))
#define WEB_CONNECTION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), WEB_TYPE_CONNECTION, WebConnectionClass))
typedef struct _WebConnectionClass WebConnectionClass;
G_GNUC_INTERNAL guint _web_message_get_freeze_count (WebMessage* web_message);
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
const guint keepalive_timeout_secs = 6;
typedef struct _Frame Frame;
typedef struct _Range Range;

struct _WebConnection
{
  GObject parent;

  /*<private>*/
  WebHttpVersion http_version;
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
    guint closed : 1;
    gsize length;
    WebParser parser;
    gsize unscanned;
    gint64 uptime;
  } in;

  struct _OutputIO
  {
    gsize allocated;
    gpointer buffer;
    guint closed : 1;
    guint is_closure : 1;
    gsize length;
    GMutex lock;
    GQueue queue;
    GQueue ranges;
    guint seqidn;
    guint seqidp;
    GPollableInputStream* splice;
    gsize wrote;
  } out;
};

struct _WebConnectionClass
{
  GObjectClass parent;
};

struct _Frame
{
  union
  {
    guint seqid;
    gpointer seqid_ptr;
  };

  WebMessage* web_message;
};

struct _Range
{
  goffset offset;
  goffset length;
};

enum
{
  prop_0,
  prop_is_https,
  prop_socket,
  prop_number,
};

G_DEFINE_FINAL_TYPE (WebConnection, web_connection, G_TYPE_OBJECT);
G_DEFINE_QUARK (web-connection-error-quark, web_connection_error);
G_DEFINE_QUARK (web-connection-sequence-id-quark, seqid);
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

static int frame_cmp (gconstpointer a_, gconstpointer b_, gpointer u)
{
  guint a = G_STRUCT_MEMBER (guint, a_, G_STRUCT_OFFSET (struct _Frame, seqid));
  guint b = G_STRUCT_MEMBER (guint, b_, G_STRUCT_OFFSET (struct _Frame, seqid));
return (a < b) ? -1 : ((a == b) ? 0 : 1);
}

static void frame_free (gpointer ptr)
{
  struct _Frame* frame = ptr;

  _g_object_unref0 (frame->web_message);
  g_slice_free (struct _Frame, frame);
}

static int range_cmp (gconstpointer a_, gconstpointer b_, gpointer u)
{
  gsize a = G_STRUCT_MEMBER (gsize, a_, G_STRUCT_OFFSET (Range, offset));
  gsize b = G_STRUCT_MEMBER (gsize, b_, G_STRUCT_OFFSET (Range, offset));

  if (a < b)
    return -1;
  else if (a > b)
    return 1;
  else
    {
      gsize x = G_STRUCT_MEMBER (gsize, a_, G_STRUCT_OFFSET (Range, length));
      gsize z = G_STRUCT_MEMBER (gsize, b_, G_STRUCT_OFFSET (Range, length));
      return (x < z) ? -1 : ((x == z) ? 0 : 1);
    }
}

static void range_free (gpointer ptr)
{
  g_slice_free (Range, ptr);
}

static void web_connection_class_dispose (GObject* pself)
{
  WebConnection* self = (gpointer) pself;
  _g_object_unref0 (self->input_stream);
  _g_object_unref0 (self->iostream);
  g_mutex_clear (& self->out.lock);
  _g_object_unref0 (self->out.splice);
  g_queue_clear_full (& self->out.queue, frame_free);
  g_queue_clear_full (& self->out.ranges, range_free);
  _g_object_unref0 (self->output_stream);
  _g_object_unref0 (self->socket);
  _g_object_unref0 (self->socket_connection);
G_OBJECT_CLASS (web_connection_parent_class)->dispose (pself);
}

static void web_connection_class_finalize (GObject* pself)
{
  WebConnection* self = (gpointer) pself;

  _g_free0 (self->in.buffer);
  web_parser_clear (& self->in.parser);
  _g_free0 (self->out.buffer);
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
  self->http_version = WEB_HTTP_VERSION_NONE;

  self->in.allocated = 0;
  self->in.buffer = NULL;
  self->in.length = 0;
  self->in.unscanned = 0;
  self->in.uptime = g_get_monotonic_time ();
  self->out.allocated = 0;
  self->out.buffer = NULL;
  self->out.is_closure = 0;
  self->out.length = 0;
  self->out.seqidn = 0;
  self->out.seqidp = 0;
  self->out.wrote = 0;

  web_parser_init (& self->in.parser);
  g_mutex_init (& self->out.lock);
  g_queue_init (& self->out.queue);
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
          if (!g_error_matches (tmperr, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK))
            return (g_propagate_error (error, tmperr), G_IO_STATUS_ERROR);
          else
            {
              g_error_free (tmperr);
              g_assert (read == -1);
              break;
            }
        }
      else if (read == 0 && io->unscanned == 0)
        return G_IO_STATUS_EOF;
      else
        {
          gchar *line, **frame;
          guint framesz, linesz, ignore;
          gsize unscanned = io->unscanned;
          gsize i, j, last = 0;

          io->length += read;
          io->unscanned = 0;
          io->uptime = g_get_monotonic_time ();

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

static void printout (struct _OutputIO* io, const gchar* fmt, ...)
{
  va_list try, list;
  gsize needed, wrote;
  gpointer block;

  va_start (try, fmt);
  G_VA_COPY (list, try);

  needed = g_printf_string_upper_bound (fmt, try);

  if ((io->length + needed) > io->allocated)
    {
      io->allocated = (io->length + needed) << 1;
      io->buffer = g_realloc (io->buffer, io->allocated);
    }

  block = G_STRUCT_MEMBER_P (io->buffer, io->length);
  wrote = g_vsnprintf (block, io->allocated, fmt, list);
  io->length += wrote;
return (va_end (try), va_end (list), (void) wrote);
}

static void serialize (struct _OutputIO* io, WebMessage* web_message)
{
  WebMessageBody* body = NULL;
  const gchar* content_encoding = NULL;
  GInputStream* content_stream = NULL;
  const gchar* content_type = NULL;
  gchar* date = NULL;
  GDateTime* datetime = NULL;
  WebHttpVersion http_version = 0;
  gboolean is_closure = FALSE;
  gsize length = 0;
  GIOStatus status = 0;
  WebStatusCode status_code = 0;

  body = web_message_get_response (web_message);
  datetime = g_date_time_new_now_utc ();
  http_version = web_message_get_http_version (web_message);
  is_closure = web_message_get_is_closure (web_message);
  status_code = web_message_get_status (web_message);

  io->is_closure = is_closure;

  printout (io, "HTTP/%s", web_http_version_to_string (http_version));
  printout (io, " %i %s", status_code, web_status_code_get_inline (status_code));
  printout (io, "\r\n");
  printout (io, "Connection: %s\r\n", is_closure ? "Close" : "Keep-Alive");

  if ((length = web_message_body_get_content_length (body)) > 0)
    {
      content_encoding = web_message_body_get_content_encoding (body);
      content_stream = web_message_body_get_stream (body);
      content_type = web_message_body_get_content_type (body);

      io->splice = g_object_ref (G_POLLABLE_INPUT_STREAM (content_stream));

      if (content_encoding != NULL)
      printout (io, "Content-Encoding: %s\r\n", content_encoding);
      printout (io, "Content-Length: %" G_GINT64_MODIFIER "i\r\n", length);
      printout (io, "Content-Type: %s\r\n", content_type);
    }

  printout (io, "Date: %s\r\n", date = g_date_time_format (datetime, "%a, %d %b %Y %T GMT"));
  printout (io, "Keep-Alive: timeout=%u\r\n", keepalive_timeout_secs);
  printout (io, "Server: " PACKAGE_NAME "/" PACKAGE_VERSION "\r\n");
  printout (io, "\r\n");
  g_free (date);
}

static GIOStatus process_out (struct _OutputIO* io, GPollableOutputStream* stream, GError** error)
{
  const goffset blocksz = 256;

  if (io->wrote == io->length)
    {
      io->length = 0;
      io->wrote = 0;

      if (io->splice != NULL)
        {
          while (TRUE)
            {
              GError* tmperr = NULL;
              gpointer block = NULL;
              gssize read = 0;

              if ((io->length + blocksz) > io->allocated)
                {
                  io->allocated = io->length + blocksz;
                  io->buffer = g_realloc (io->buffer, io->allocated);
                }

              block = G_STRUCT_MEMBER_P (io->buffer, io->length);
              read = g_pollable_input_stream_read_nonblocking (io->splice, block, blocksz, NULL, &tmperr);

              if (G_UNLIKELY (tmperr == NULL))
                {
                  if (read > 0)
                    io->length += read;
                  else
                    {
                      _g_object_unref0 (io->splice);
                      break;
                    }
                }
              else
                {
                  if (!g_error_matches (tmperr, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK))
                    return (g_propagate_error (error, tmperr), G_IO_STATUS_ERROR);
                  else
                    {
                      g_error_free (tmperr);
                      g_assert (read == -1);
                      break;
                    }
                }
            }
        }
      else if (io->is_closure == TRUE)
        {
          return G_IO_STATUS_EOF;
        }
      else if (g_mutex_trylock (& io->lock))
        {
          struct _Frame* frame = NULL;

          if ((frame = g_queue_peek_head (& io->queue)) == NULL)
            g_mutex_unlock (& io->lock);
          else if ((frame->seqid == 0) || ((frame->seqid - 1) == io->seqidp) == FALSE)
            g_mutex_unlock (& io->lock);
          else
            {
              io->seqidp = frame->seqid;
              g_queue_pop_head (& io->queue);
              g_mutex_unlock (& io->lock);
              serialize (io, frame->web_message);
              frame_free (frame);
            }
        }
    }
  else while (TRUE)
    {
      GError* tmperr = NULL;
      gpointer block = NULL;
      gsize wrote = 0;

      block = G_STRUCT_MEMBER_P (io->buffer, io->wrote);
      wrote = g_pollable_output_stream_write_nonblocking (stream, block, io->length - io->wrote, NULL, &tmperr);

      if (G_UNLIKELY (tmperr == NULL))
        io->wrote += wrote;
      else
        {
          if (!g_error_matches (tmperr, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK))
            return (g_propagate_error (error, tmperr), G_IO_STATUS_ERROR);
          else
            {
              g_error_free (tmperr);
              g_assert (wrote == -1);
              break;
            }
        }

      break;
    }
return (io->length > io->wrote) ? G_IO_STATUS_AGAIN : G_IO_STATUS_NORMAL;
}

static void on_message_thawed (WebMessage* web_message, guint freeze_count, WebConnection* web_connection)
{
  if (freeze_count == 0)
    {
      g_signal_handlers_disconnect_by_data (web_message, web_connection);
      web_connection_send (web_connection, web_message);
    }
}

void web_connection_send (WebConnection* web_connection, WebMessage* web_message)
{
  g_return_if_fail (WEB_IS_CONNECTION (web_connection));
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  WebConnection* self = (web_connection);
  struct _Frame* frame = NULL;

  if (_web_message_get_freeze_count (web_message) > 0)
    g_signal_connect_object (web_message, "thawed", G_CALLBACK (on_message_thawed), web_connection, 0);
  else
    {
      frame = g_slice_new (struct _Frame);
      frame->web_message = g_object_ref (web_message);
      frame->seqid_ptr = g_object_get_qdata (G_OBJECT (web_message), seqid_quark ());

      g_mutex_lock (& self->out.lock);
      g_queue_insert_sorted (& self->out.queue, frame, frame_cmp, NULL);
      g_mutex_unlock (& self->out.lock);
    }
}

WebMessage* web_connection_step (WebConnection* web_connection, GError** error)
{
  g_return_val_if_fail (WEB_IS_CONNECTION (web_connection), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  WebConnection* self = (web_connection);
  WebMessage* web_message = NULL;
  WebMessageHeaders* web_message_headers = NULL;
  GError* tmperr = NULL;
  GIOStatus status = 0;

  if ((status = process_out (& self->out, G_POLLABLE_OUTPUT_STREAM (self->output_stream), &tmperr)), G_UNLIKELY (tmperr != NULL))
    g_propagate_error (error, tmperr);
  else
    {
      switch (status)
      {
        case G_IO_STATUS_AGAIN:
          break;

        case G_IO_STATUS_EOF:
          g_output_stream_flush (self->output_stream, NULL, NULL);
          g_output_stream_close (self->output_stream, NULL, NULL);
          G_GNUC_FALLTHROUGH;
        case G_IO_STATUS_ERROR:
          g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_CONNECTION_CLOSED, _("Connection closed"));
          break;

        case G_IO_STATUS_NORMAL:
          {
            if ((status = process_in (& self->in, G_POLLABLE_INPUT_STREAM (self->input_stream), &tmperr)), G_UNLIKELY (tmperr != NULL))
              g_propagate_error (error, tmperr);
            else
              {
                switch (status)
                  {
                    case G_IO_STATUS_AGAIN:
                      {
                        if ((g_get_monotonic_time () - self->in.uptime) < (keepalive_timeout_secs * G_USEC_PER_SEC))
                          break;
                        G_GNUC_FALLTHROUGH;
                      }

                    case G_IO_STATUS_EOF:
                      g_input_stream_close (self->input_stream, NULL, NULL);
                      G_GNUC_FALLTHROUGH;
                    case G_IO_STATUS_ERROR:
                      g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_CONNECTION_CLOSED, _("Connection closed"));
                      break;

                    case G_IO_STATUS_NORMAL:
                      {
                        struct _InputIO* io = & self->in;
                        WebParserField* field = NULL;

                        if (self->http_version == WEB_HTTP_VERSION_NONE)
                          self->http_version = io->parser.http_version;
                        else
                          {
                            if (io->parser.http_version != self->http_version)
                              {
                                web_parser_clear (& io->parser);
                                web_parser_init (& io->parser);

                                g_set_error_literal (error, WEB_CONNECTION_ERROR, WEB_CONNECTION_ERROR_MISMATCH_VERSION, _("Request versions mismatches"));
                                break;
                              }
                          }

                        web_message = web_message_new ();
                        web_message_headers = web_message_get_headers (web_message);

                        web_message_set_http_version (web_message, io->parser.http_version);
                        web_message_set_is_closure (web_message, FALSE);
                        web_message_set_method (web_message, io->parser.method);
                        web_message_set_uri (web_message, io->parser.uri);

                        if (self->http_version >= WEB_HTTP_VERSION_2_0)
                          g_object_set_qdata (G_OBJECT (web_message), seqid_quark (), GUINT_TO_POINTER (0));
                        else
                          {
                            if (!g_uint_checked_add (& self->out.seqidn, 1, self->out.seqidn))
                              {
                                web_parser_clear (& io->parser);
                                web_parser_init (& io->parser);

                                g_set_error_literal (error, WEB_CONNECTION_ERROR, WEB_CONNECTION_ERROR_REQUEST_OVERFLOW, _("Too much requests for connection"));
                                break;
                              }

                            g_object_set_qdata (G_OBJECT (web_message), seqid_quark (), GUINT_TO_POINTER (self->out.seqidn));
                          }

                        while ((field = g_queue_pop_head (& io->parser.fields)) != NULL)
                          {
                            gchar* name = g_steal_pointer (& field->name);
                            gchar* value = g_steal_pointer (& field->value);

                            web_message_headers_append_take (web_message_headers, name, value, &tmperr);
                            web_parser_field_free (field);

                            if (G_UNLIKELY (tmperr != NULL))
                              break;
                          }

                        if (G_UNLIKELY (tmperr != NULL))
                          {
                            _g_object_unref0 (web_message);
                            g_propagate_error (error, tmperr);
                          }
                        else
                          {
                            if (self->http_version < WEB_HTTP_VERSION_1_1)
                              web_message_set_is_closure (web_message, TRUE);
                            else
                              {
                                if (!web_message_headers_get_keep_alive (web_message_headers))
                                  web_message_set_is_closure (web_message, TRUE);
                              }
                          }

                        web_parser_clear (& io->parser);
                        web_parser_init (& io->parser);
                        break;
                      }
                  }
              }
            break;
          }
      }
    }
return (web_message);
}

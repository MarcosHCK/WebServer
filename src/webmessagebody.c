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
#include <webmessage.h>

#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

struct _WebMessageBody
{
  gchar* content_encoding;
  gsize  content_length;
  gchar* content_type;

  union /*<variant>*/
  {
    GInputStream* stream;
    GMemoryInputStream* backend;
  };
};

static gpointer nullfunc (gpointer ptr)
{
  g_assert_not_reached ();
}

G_DEFINE_BOXED_TYPE (WebMessageBody, web_message_body, nullfunc, web_message_body_free);

WebMessageBody* web_message_body_new ()
{
return g_slice_new0 (WebMessageBody);
}

void web_message_body_add_bytes (WebMessageBody* web_message_body, GBytes* bytes)
{
  g_return_if_fail (web_message_body != NULL);
  g_return_if_fail (bytes != NULL);
  WebMessageBody* self = (web_message_body);

  if (G_IS_MEMORY_INPUT_STREAM (self->stream))
    g_memory_input_stream_add_bytes (self->backend, bytes);
  else
    {
      g_set_object (&self->stream, g_memory_input_stream_new_from_bytes (bytes));
      g_object_unref (&self->stream);
    }
}

void web_message_body_add_data (WebMessageBody* web_message_body, gpointer data, gsize length, GDestroyNotify notify)
{
  g_return_if_fail (web_message_body != NULL);
  g_return_if_fail (length == 0 || data != NULL);
  WebMessageBody* self = (web_message_body);

  if (G_IS_MEMORY_INPUT_STREAM (self->stream))
    g_memory_input_stream_add_data (self->backend, data, length, notify);
  else
    {
      g_set_object (&self->stream, g_memory_input_stream_new_from_data (data, length, notify));
      g_object_unref (&self->stream);
    }
}

void web_message_body_set_content_encoding (WebMessageBody* web_message_body, const gchar* content_encoding)
{
  g_return_if_fail (web_message_body != NULL);
  _g_free0 (web_message_body->content_encoding);
  web_message_body->content_encoding = g_strdup (content_encoding);
}

void web_message_body_set_content_length (WebMessageBody* web_message_body, gsize content_length)
{
  g_return_if_fail (web_message_body != NULL);
  web_message_body->content_length = content_length;
}

void web_message_body_set_content_type (WebMessageBody* web_message_body, const gchar* content_type)
{
  g_return_if_fail (web_message_body != NULL);
  _g_free0 (web_message_body->content_type);
  web_message_body->content_type = g_strdup (content_type);
}

void web_message_body_set_stream (WebMessageBody* web_message_body, GInputStream* stream)
{
  g_return_if_fail (web_message_body != NULL);
  g_return_if_fail (G_IS_INPUT_STREAM (stream));
  g_return_if_fail (G_IS_POLLABLE_INPUT_STREAM (stream));
  g_set_object (& web_message_body->stream, stream);
}

void web_message_body_free (WebMessageBody* web_message_body)
{
  g_return_if_fail (web_message_body != NULL);
  WebMessageBody* self = (web_message_body);

  _g_free0 (self->content_encoding);
  _g_free0 (self->content_type);
  _g_object_unref0 (self->stream);
  g_slice_free (WebMessageBody, self);
}

const gchar* web_message_body_get_content_encoding (WebMessageBody* web_message_body)
{
  g_return_val_if_fail (web_message_body != NULL, NULL);
return web_message_body->content_encoding;
}

gsize web_message_body_get_content_length (WebMessageBody* web_message_body)
{
  g_return_val_if_fail (web_message_body != NULL, 0);
return web_message_body->content_length;
}

const gchar* web_message_body_get_content_type (WebMessageBody* web_message_body)
{
  g_return_val_if_fail (web_message_body != NULL, NULL);
return web_message_body->content_type;
}

GInputStream* web_message_body_get_stream (WebMessageBody* web_message_body)
{
  g_return_val_if_fail (web_message_body != NULL, NULL);
return G_INPUT_STREAM (web_message_body->stream);
}

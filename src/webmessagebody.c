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
  guint ref_count;

  union /*<variant>*/
  {
    GInputStream* stream;
    GMemoryInputStream* backend;
  };
};

WebMessageBody* web_message_body_new ()
{
  WebMessageBody* self;

  self = g_slice_new (WebMessageBody);
  self->ref_count = 1;
  self->backend = NULL;
return self;
}

WebMessageBody* web_message_body_ref (WebMessageBody* web_message_body)
{
  g_return_val_if_fail (web_message_body != NULL, NULL);
  WebMessageBody* self = (web_message_body);
return (g_atomic_int_inc (&self->ref_count), self);
}

void web_message_body_unref (WebMessageBody* web_message_body)
{
  g_return_if_fail (web_message_body != NULL);
  WebMessageBody* self = (web_message_body);

  if (g_atomic_int_dec_and_test (&self->ref_count))
    {
      _g_object_unref0 (self->stream);
      g_slice_free (WebMessageBody, self);
    }
}

G_DEFINE_BOXED_TYPE (WebMessageBody, web_message_body, web_message_body_ref, web_message_body_unref);\

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
      g_object_unref (self->stream);
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
      g_object_unref (self->stream);
    }
}

void web_message_body_set_stream (WebMessageBody* web_message_body, GInputStream* stream)
{
  g_return_if_fail (web_message_body != NULL);
  g_return_if_fail (G_IS_INPUT_STREAM (stream));
  g_return_if_fail (G_IS_POLLABLE_INPUT_STREAM (stream));
  g_set_object (& web_message_body->stream, stream);
}

GInputStream* web_message_body_get_stream (WebMessageBody* web_message_body)
{
  g_return_val_if_fail (web_message_body != NULL, NULL);
return web_message_body->stream;
}

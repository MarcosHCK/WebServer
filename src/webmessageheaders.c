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
#include <webmessagefields.h>
#include <webmessageheaderparse.h>
#include <webmessagemethods.h>

static gpointer nullfunc (gpointer ptr)
{
  g_assert_not_reached ();
}

G_DEFINE_BOXED_TYPE (WebMessageHeaders, web_message_headers, nullfunc, web_message_headers_free);

void _web_message_range_free (WebMessageRange* range)
{
  g_slice_free (WebMessageRange, range);
}

WebMessageHeaders* web_message_headers_new ()
{
  WebMessageHeaders* self;
  const GHashFunc func1 = (GHashFunc) g_str_hash;
  const GEqualFunc func2 = (GEqualFunc) g_str_equal;
  const GDestroyNotify func3 = (GDestroyNotify) g_queue_free;

  self = g_slice_new (WebMessageHeaders);
  self->fields = g_hash_table_new_full (func1, func2, g_free, func3);
return (g_queue_init (& self->taken), self);
}

void web_message_headers_free (WebMessageHeaders* web_message_headers)
{
  g_return_if_fail (web_message_headers != NULL);
  WebMessageHeaders* self = (web_message_headers);

  g_hash_table_remove_all (self->fields);
  g_hash_table_unref (self->fields);
  g_queue_clear_full (& self->ranges, (GDestroyNotify) _web_message_range_free);
  g_queue_clear_full (& self->taken, (GDestroyNotify) g_free);
}

void web_message_headers_append (WebMessageHeaders* web_message_headers, const gchar* key, const gchar* value, GError** error)
{
  g_return_if_fail (web_message_headers != NULL);
  g_return_if_fail (key != NULL && value != NULL);
  WebMessageHeaders* self = (web_message_headers);

  web_message_headers_append_take (self, g_strdup (key), g_strdup (value), error);
}

void web_message_headers_append_take (WebMessageHeaders* web_message_headers, gchar* key, gchar* value, GError** error)
{
  g_return_if_fail (web_message_headers != NULL);
  g_return_if_fail (key != NULL && value != NULL);
  WebMessageHeaders* self = (web_message_headers);

  _web_message_headers_parse_header (self, key, value, error);
}

void web_message_headers_clear (WebMessageHeaders* web_message_headers)
{
  g_return_if_fail (web_message_headers != NULL);
  WebMessageHeaders* self = (web_message_headers);

  g_hash_table_remove_all (self->fields);
}

gboolean web_message_headers_contains (WebMessageHeaders* web_message_headers, const gchar* key)
{
  g_return_val_if_fail (web_message_headers != NULL, FALSE);
  g_return_val_if_fail (key != NULL, FALSE);
  WebMessageHeaders* self = (web_message_headers);
return g_hash_table_contains (self->fields, key);
}

goffset web_message_headers_get_content_length (WebMessageHeaders* web_message_headers)
{
  g_return_val_if_fail (web_message_headers != NULL, 0);
  WebMessageHeaders* self = (web_message_headers);
return -1;
}

const gchar* web_message_headers_get_content_type (WebMessageHeaders* web_message_headers)
{
  g_return_val_if_fail (web_message_headers != NULL, NULL);
  WebMessageHeaders* self = (web_message_headers);
return NULL;
}

WebMessageEncoding web_message_headers_get_encoding (WebMessageHeaders* web_message_headers)
{
  g_return_val_if_fail (web_message_headers != NULL, 0);
  WebMessageHeaders* self = (web_message_headers);
return WEB_MESSAGE_ENCODING_CONTENT_LENGTH;
}

WebMessageExpectation web_message_headers_get_expectations (WebMessageHeaders* web_message_headers)
{
  g_return_val_if_fail (web_message_headers != NULL, 0);
  WebMessageHeaders* self = (web_message_headers);
return WEB_MESSAGE_EXPECTATION_UNKNOWN;
}

gboolean web_message_headers_get_keep_alive (WebMessageHeaders* web_message_headers)
{
  g_return_val_if_fail (web_message_headers != NULL, 0);
  WebMessageHeaders* self = (web_message_headers);

  if (!web_message_headers_contains (self, WEB_MESSAGE_FIELD_CONNECTION))
    return FALSE;
  else
    {
      const gchar* a = "keep-alive";
      const gchar* b = web_message_headers_get_one (self, WEB_MESSAGE_FIELD_CONNECTION);
      return !g_ascii_strcasecmp (a, b);
    }
}

GList* web_message_headers_get_list (WebMessageHeaders* web_message_headers, const gchar* key)
{
  g_return_val_if_fail (web_message_headers != NULL, NULL);
  g_return_val_if_fail (key != NULL, NULL);
  WebMessageHeaders* self = (web_message_headers);
  GQueue* queue = g_hash_table_lookup (self->fields, key);
return (queue == NULL) ? NULL : (g_queue_peek_head_link (queue));
}

const gchar* web_message_headers_get_one (WebMessageHeaders* web_message_headers, const gchar* key)
{
  g_return_val_if_fail (web_message_headers != NULL, NULL);
  g_return_val_if_fail (key != NULL, NULL);
  WebMessageHeaders* self = (web_message_headers);
  GQueue* queue = g_hash_table_lookup (self->fields, key);
return (queue == NULL) ? NULL : (g_queue_peek_head (queue));
}

GList* web_message_headers_get_ranges (WebMessageHeaders* web_message_headers)
{
  g_return_val_if_fail (web_message_headers != NULL, 0);
  WebMessageHeaders* self = (web_message_headers);
return g_queue_peek_head_link (& self->ranges);
}

void web_message_headers_iter_init (WebMessageHeadersIter* iter, WebMessageHeaders* web_message_headers)
{
  g_return_if_fail (iter != NULL);
  g_return_if_fail (web_message_headers != NULL);
  WebMessageHeaders* self = (web_message_headers);

  g_hash_table_iter_init (& iter->iter, self->fields);
}

gboolean web_message_headers_iter_next (WebMessageHeadersIter* iter, gchar const** key, GList** values)
{
  g_return_val_if_fail (iter != NULL, FALSE);
  gboolean have = FALSE;
  GQueue* list = NULL;

  if ((have = g_hash_table_iter_next (& iter->iter, (gpointer*) key, (gpointer*) &list)) == TRUE)
    {
      if (values != NULL)
        {
          *values = list->head;
        }
    }
return (have);
}

void web_message_headers_remove (WebMessageHeaders* web_message_headers, const gchar* key)
{
  g_return_if_fail (web_message_headers != NULL);
  g_return_if_fail (key != NULL);
  WebMessageHeaders* self = (web_message_headers);

  g_hash_table_remove (self->fields, key);
}

void web_message_headers_replace (WebMessageHeaders* web_message_headers, const gchar* key, const gchar* value, GError** error)
{
  g_return_if_fail (web_message_headers != NULL);
  g_return_if_fail (key != NULL && value != NULL);
  WebMessageHeaders* self = (web_message_headers);

  web_message_headers_replace_take (self, g_strdup (key), g_strdup (value), error);
}

void web_message_headers_replace_take (WebMessageHeaders* web_message_headers, gchar* key, gchar* value, GError** error)
{
  g_return_if_fail (web_message_headers != NULL);
  g_return_if_fail (key != NULL && value != NULL);
  WebMessageHeaders* self = (web_message_headers);

  web_message_headers_remove (self, key);
  web_message_headers_append_take (self, key, value, error);
}

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

WebMessageHeaders* web_message_headers_new ()
{
  WebMessageHeaders* self;
  const GHashFunc func1 = (GHashFunc) g_str_hash;
  const GEqualFunc func2 = (GEqualFunc) g_str_equal;
  const GDestroyNotify func3 = (GDestroyNotify) g_queue_free;

  self = g_slice_new (WebMessageHeaders);
  self->ref_count = 1;
  self->fields = g_hash_table_new_full (func1, func2, g_free, func3);
return (g_queue_init (& self->taken), self);
}

WebMessageHeaders* web_message_headers_ref (WebMessageHeaders* web_message_headers)
{
  g_return_val_if_fail (web_message_headers != NULL, NULL);
  WebMessageHeaders* self = (web_message_headers);
return (g_atomic_int_inc (&self->ref_count), self);
}

void web_message_headers_unref (WebMessageHeaders* web_message_headers)
{
  g_return_if_fail (web_message_headers != NULL);
  WebMessageHeaders* self = (web_message_headers);

  if (g_atomic_int_dec_and_test (&self->ref_count))
    {
      g_hash_table_remove_all (self->fields);
      g_hash_table_unref (self->fields);
      g_queue_clear_full (& self->ranges, (GDestroyNotify) _web_message_range_free);
      g_queue_clear_full (& self->taken, (GDestroyNotify) g_free);
      g_slice_free (WebMessageHeaders, self);
    }
}

G_DEFINE_BOXED_TYPE (WebMessageHeaders, web_message_headers, web_message_headers_ref, web_message_headers_unref);

void _web_message_range_free (WebMessageRange* range)
{
  g_slice_free (WebMessageRange, range);
}

void web_message_headers_append (WebMessageHeaders* web_message_headers, const gchar* key, const gchar* value)
{
  g_return_if_fail (web_message_headers != NULL);
  g_return_if_fail (key != NULL && value != NULL);
  WebMessageHeaders* self = (web_message_headers);

  web_message_headers_append_take (self, g_strdup (key), g_strdup (value));
}

void web_message_headers_append_take (WebMessageHeaders* web_message_headers, gchar* key, gchar* value)
{
  g_return_if_fail (web_message_headers != NULL);
  g_return_if_fail (key != NULL && value != NULL);
  WebMessageHeaders* self = (web_message_headers);

  _web_message_headers_parse_header (self, key, value);
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

void web_message_headers_replace (WebMessageHeaders* web_message_headers, const gchar* key, const gchar* value)
{
  g_return_if_fail (web_message_headers != NULL);
  g_return_if_fail (key != NULL && value != NULL);
  WebMessageHeaders* self = (web_message_headers);

  web_message_headers_replace_take (self, g_strdup (key), g_strdup (value));
}

void web_message_headers_replace_take (WebMessageHeaders* web_message_headers, gchar* key, gchar* value)
{
  g_return_if_fail (web_message_headers != NULL);
  g_return_if_fail (key != NULL && value != NULL);
  WebMessageHeaders* self = (web_message_headers);

  web_message_headers_remove (self, key);
  web_message_headers_append_take (self, key, value);
}

void web_message_headers_set_content_disposition (WebMessageHeaders* web_message_headers, const gchar* disposition, ...)
{
  g_return_if_fail (web_message_headers != NULL);
  g_return_if_fail (disposition != NULL);
  WebMessageHeaders* self = (web_message_headers);
  va_list l;

  va_start (l, disposition);
  web_message_headers_set_content_disposition_va (self, disposition, l);
  va_end (l);
}

void web_message_headers_set_content_disposition_va (WebMessageHeaders* web_message_headers, const gchar* disposition, va_list l)
{
  g_return_if_fail (web_message_headers != NULL);
  g_return_if_fail (disposition != NULL);
  WebMessageHeaders* self = (web_message_headers);
  GQueue* list = NULL;
  const gchar *key_ = WEB_MESSAGE_FIELD_CONTENT_DISPOSITION;
  const gchar *key, *value;
  gchar* value_ = NULL;

  web_message_headers_replace (self, key_, disposition);
  list = g_hash_table_lookup (self->fields, key_);
  g_assert (list != NULL);

  while (TRUE)
    {
      if ((key = va_arg (l, gchar*)) == NULL)
        break;
      else
        {
          value = va_arg (l, gchar*);
          value_ = g_strdup_printf ("%s=%s", key, value);

          g_queue_push_tail (list, value_);
          g_queue_push_tail (& self->taken, value_);
        }
    }
}

void web_message_headers_set_content_length (WebMessageHeaders* web_message_headers, gsize length)
{
  g_return_if_fail (web_message_headers != NULL);
  WebMessageHeaders* self = (web_message_headers);

  web_message_headers_replace_take (self, g_strdup (WEB_MESSAGE_FIELD_CONTENT_LENGTH), g_strdup_printf ("%" G_GINT64_MODIFIER "u", length));
}

void web_message_headers_set_content_range (WebMessageHeaders* web_message_headers, goffset begin_offset, goffset end_offset, goffset length)
{
  g_return_if_fail (web_message_headers != NULL);
  WebMessageHeaders* self = (web_message_headers);

  web_message_headers_replace_take (self, g_strdup (WEB_MESSAGE_FIELD_CONTENT_RANGE),
    g_strdup_printf ("%" G_GINT64_MODIFIER "u-%" G_GINT64_MODIFIER "u/%" G_GINT64_MODIFIER "u",
      begin_offset, end_offset, length));
}

void web_message_headers_set_content_type (WebMessageHeaders* web_message_headers, const gchar* type)
{
  g_return_if_fail (web_message_headers != NULL);
  WebMessageHeaders* self = (web_message_headers);

  web_message_headers_replace_take (self, g_strdup (WEB_MESSAGE_FIELD_CONTENT_TYPE), g_strdup (type));
}

void web_message_headers_set_location (WebMessageHeaders* web_message_headers, const gchar* uri)
{
  g_return_if_fail (web_message_headers != NULL);
  WebMessageHeaders* self = (web_message_headers);

  web_message_headers_replace_take (self, g_strdup (WEB_MESSAGE_FIELD_LOCATION), g_strdup (uri));
}

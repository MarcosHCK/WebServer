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

struct _WebMessageHeaders
{
  GHashTable* fields;
  GQueue taken;
};

static gpointer nullfunc (gpointer ptr)
{
  g_assert_not_reached ();
}

G_DEFINE_BOXED_TYPE (WebMessageHeaders, web_message_headers, nullfunc, web_message_headers_free);

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
  g_queue_clear_full (& self->taken, g_free);
}

void web_message_headers_append (WebMessageHeaders* web_message_headers, const gchar* key, const gchar* value)
{
  g_return_if_fail (web_message_headers != NULL);
  g_return_if_fail (key != NULL && value != NULL);
  WebMessageHeaders* self = (web_message_headers);

  web_message_headers_append_take (self, g_strdup (key), g_strdup (value));
}

static void append_values (GQueue* list, gchar* value)
{
  gsize length = strlen (value);
  gsize i;

  g_queue_push_tail (list, value);

  for (i = 0; i < length; ++i)
  if (value [i] == ',')
    {
      value [i] = 0;

      if (i < length)
      {
        g_queue_push_tail (list, & value [i + 1]);
      }
    }
}

void web_message_headers_append_take (WebMessageHeaders* web_message_headers, gchar* key, gchar* value)
{
  g_return_if_fail (web_message_headers != NULL);
  g_return_if_fail (key != NULL && value != NULL);
  WebMessageHeaders* self = (web_message_headers);
  GQueue* list = NULL;

  if ((list = g_hash_table_lookup (self->fields, key)) != NULL)
    {
      append_values (list, value);
      g_queue_push_tail (& self->taken, value);
    }
  else
    {
      list = g_queue_new ();

      append_values (list, value);
      g_queue_push_tail (& self->taken, value);
      g_hash_table_insert (self->fields, key, list);
    }
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

const gchar* web_message_headers_get_list (WebMessageHeaders* web_message_headers, const gchar* key)
{
  g_return_val_if_fail (web_message_headers != NULL, NULL);
  g_return_val_if_fail (key != NULL, NULL);
  WebMessageHeaders* self = (web_message_headers);
return NULL;
}

const gchar* web_message_headers_get_one (WebMessageHeaders* web_message_headers, const gchar* key)
{
  g_return_val_if_fail (web_message_headers != NULL, NULL);
  g_return_val_if_fail (key != NULL, NULL);
  WebMessageHeaders* self = (web_message_headers);
return NULL;
}

WebMessageRange* web_message_headers_get_ranges (WebMessageHeaders* web_message_headers, guint* n_ranges)
{
  g_return_val_if_fail (web_message_headers != NULL, 0);
  WebMessageHeaders* self = (web_message_headers);
return NULL;
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

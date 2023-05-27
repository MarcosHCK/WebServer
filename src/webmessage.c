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

#define _g_bytes_unref0(var) ((var == NULL) ? NULL : (var = (g_bytes_unref (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

struct _WebMessagePrivate
{
  GInputStream* body;
  GHashTable* fields;
};

enum
{
  prop_0,
  prop_body,
  prop_number,
};

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (WebMessage, web_message, G_TYPE_OBJECT);
static GParamSpec* properties [prop_number] = {0};

static void web_message_class_dispose (GObject* pself)
{
  WebMessage* self = (gpointer) pself;
  WebMessagePrivate* priv = self->priv;
  _g_object_unref0 (priv->body);
  g_hash_table_remove_all (priv->fields);
G_OBJECT_CLASS (web_message_parent_class)->dispose (pself);
}

static void web_message_class_finalize (GObject* pself)
{
  WebMessage* self = (gpointer) pself;
  WebMessagePrivate* priv = self->priv;
  g_hash_table_unref (priv->fields);
G_OBJECT_CLASS (web_message_parent_class)->finalize (pself);
}

static void web_message_class_get_property (GObject* pself, guint property_id, GValue* value, GParamSpec* pspec)
{
  WebMessage* self = (gpointer) pself;
  WebMessagePrivate* priv = self->priv;

  switch (property_id)
    {
      case prop_body:
        g_value_set_object (value, priv->body);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (pself, property_id, pspec);
        break;
    }
}

static void web_message_class_set_property (GObject* pself, guint property_id, const GValue* value, GParamSpec* pspec)
{
  WebMessage* self = (gpointer) pself;
  WebMessagePrivate* priv = self->priv;

  switch (property_id)
    {
      case prop_body:
        web_message_set_body_stream (self, g_value_get_object (value));
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (pself, property_id, pspec);
        break;
    }
}

static void web_message_class_init (WebMessageClass* klass)
{
  G_OBJECT_CLASS (klass)->dispose = web_message_class_dispose;
  G_OBJECT_CLASS (klass)->finalize = web_message_class_finalize;
  G_OBJECT_CLASS (klass)->get_property = web_message_class_get_property;
  G_OBJECT_CLASS (klass)->set_property = web_message_class_set_property;

  properties [prop_body] = g_param_spec_boxed ("body", "body", "body", G_TYPE_BYTES, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties (G_OBJECT_CLASS (klass), prop_number, properties);
}

static void web_message_init (WebMessage* self)
{
  const GHashFunc func1 = g_str_hash;
  const GEqualFunc func2 = g_str_equal;

  self->priv = web_message_get_instance_private (self);
  self->priv->fields = g_hash_table_new_full (func1, func2, g_free, g_free);
}

const gchar* web_message_get_field (WebMessage* web_message, const gchar* key)
{
  g_return_val_if_fail (WEB_IS_MESSAGE (web_message), NULL);
  g_return_val_if_fail (key != NULL, NULL);
  WebMessagePrivate* priv = web_message->priv;
return g_hash_table_lookup (priv->fields, key);
}

void web_message_get_field_iter (WebMessage* web_message, GHashTableIter* iter)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  g_return_if_fail (iter != NULL);
  WebMessagePrivate* priv = web_message->priv;
  g_hash_table_iter_init (iter, priv->fields);
}

void web_message_delete_field (WebMessage* web_message, const gchar* key)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  g_return_if_fail (key != NULL);
  WebMessagePrivate* priv = web_message->priv;
  g_hash_table_remove (priv->fields, key);
}

void web_message_insert_field (WebMessage* web_message, const gchar* key, const gchar* value)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  g_return_if_fail (key != NULL);
  WebMessagePrivate* priv = web_message->priv;
  g_hash_table_insert (priv->fields, g_strdup (key), g_strdup (value));
}

void web_message_insert_field_take (WebMessage* web_message, gchar* key, gchar* value)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  g_return_if_fail (key != NULL);
  WebMessagePrivate* priv = web_message->priv;
  g_hash_table_insert (priv->fields, key, value);
}

void web_message_set_body (WebMessage* web_message, const gchar* contents, gsize length)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  g_return_if_fail (length == 0 || contents != NULL);
  WebMessagePrivate* priv = web_message->priv;

  _g_object_unref0 (priv->body);
  priv->body = g_memory_input_stream_new_from_data (g_strndup (contents, length), length, g_free);
}

void web_message_set_body_bytes (WebMessage* web_message, GBytes* bytes)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  g_return_if_fail (bytes != NULL);
  WebMessagePrivate* priv = web_message->priv;

  _g_object_unref0 (priv->body);
  priv->body = g_memory_input_stream_new_from_bytes (bytes);
}

void web_message_set_body_stream (WebMessage* web_message, GInputStream* stream)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  g_return_if_fail (G_IS_INPUT_STREAM (stream));
  WebMessagePrivate* priv = web_message->priv;

  _g_object_unref0 (priv->body);
  priv->body = g_object_ref (stream);
}

void web_message_set_body_take (WebMessage* web_message, gchar* contents, gsize length)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  g_return_if_fail (length == 0 || contents != NULL);
  WebMessagePrivate* priv = web_message->priv;

  _g_object_unref0 (priv->body);
  priv->body = g_memory_input_stream_new_from_data (contents, length, g_free);
}

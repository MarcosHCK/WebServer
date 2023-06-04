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
#include <webmessage.h>

G_GNUC_INTERNAL guint _web_message_get_freeze_count (WebMessage* web_message);
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_uri_unref0(var) ((var == NULL) ? NULL : (var = (g_uri_unref (var), NULL)))

struct _WebMessagePrivate
{
  guint freeze_count : 7;
  WebHttpVersion http_version;
  guint is_closure : 1;
  const gchar* method;
  WebMessageBody* request_body;
  WebMessageHeaders* request_headers;
  WebMessageBody* response_body;
  WebMessageHeaders* response_headers;
  WebStatusCode status_code;
  GUri* uri;
};

enum
{
  prop_0,
  prop_http_version,
  prop_method,
  prop_request_body,
  prop_request_headers,
  prop_response_body,
  prop_response_headers,
  prop_status,
  prop_uri,
  prop_number,
};

enum
{
  signal_frozen,
  signal_thawed,
  signal_number,
};

G_DEFINE_TYPE_WITH_PRIVATE (WebMessage, web_message, G_TYPE_OBJECT);
static GParamSpec* properties [prop_number] = {0};
static guint signals [signal_number] = {0};

static void web_message_class_dispose (GObject* pself)
{
  WebMessage* self = (gpointer) pself;
  WebMessagePrivate* priv = self->priv;
G_OBJECT_CLASS (web_message_parent_class)->dispose (pself);
}

static void web_message_class_finalize (GObject* pself)
{
  WebMessage* self = (gpointer) pself;
  WebMessagePrivate* priv = self->priv;
  web_message_headers_unref (priv->request_headers);
  web_message_headers_unref (priv->response_headers);
  web_message_body_unref (priv->request_body);
  web_message_body_unref (priv->response_body);
  _g_uri_unref0 (priv->uri);
G_OBJECT_CLASS (web_message_parent_class)->finalize (pself);
}

static void web_message_class_get_property (GObject* pself, guint property_id, GValue* value, GParamSpec* pspec)
{
  WebMessage* self = (gpointer) pself;
  WebMessagePrivate* priv = self->priv;

  switch (property_id)
    {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (pself, property_id, pspec);
        break;

      case prop_http_version:
        g_value_set_enum (value, web_message_get_http_version (self));
        break;
      case prop_method:
        g_value_set_pointer (value, (gpointer) web_message_get_method (self));
        break;
      case prop_request_body:
        g_value_set_boxed (value, priv->request_body);
        break;
      case prop_request_headers:
        g_value_set_boxed (value, priv->request_headers);
        break;
      case prop_response_body:
        g_value_set_boxed (value, priv->response_body);
        break;
      case prop_response_headers:
        g_value_set_boxed (value, priv->response_headers);
        break;
      case prop_status:
        g_value_set_enum (value, priv->status_code);
        break;
      case prop_uri:
        g_value_set_boxed (value, web_message_get_uri (self));
        break;
    }
}

static void web_message_class_set_property (GObject* pself, guint property_id, const GValue* value, GParamSpec* pspec)
{
  WebMessage* self = (gpointer) pself;
  WebMessagePrivate* priv = self->priv;

  switch (property_id)
    {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (pself, property_id, pspec);
        break;

      case prop_http_version:
        web_message_set_http_version (self, g_value_get_enum (value));
        break;
      case prop_method:
        web_message_set_method (self, g_value_get_pointer (value));
        break;
      case prop_status:
        web_message_set_status (self, g_value_get_enum (value));
        break;
      case prop_uri:
        web_message_set_uri (self, g_value_get_boxed (value));
        break;
    }
}

static void web_message_class_init (WebMessageClass* klass)
{
  G_OBJECT_CLASS (klass)->dispose = web_message_class_dispose;
  G_OBJECT_CLASS (klass)->finalize = web_message_class_finalize;
  G_OBJECT_CLASS (klass)->get_property = web_message_class_get_property;
  G_OBJECT_CLASS (klass)->set_property = web_message_class_set_property;

  const GParamFlags flags1 = G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS;
  const GParamFlags flags2 = G_PARAM_READABLE | G_PARAM_STATIC_STRINGS;
  const GType g_type = G_TYPE_FROM_CLASS (klass);
  const GSignalFlags flags3 = G_SIGNAL_RUN_FIRST;
  const GSignalCMarshaller marshal1 = web_cclosure_marshal_VOID__INT;

  properties [prop_http_version] = g_param_spec_enum ("http-version", "http-version", "http-version", WEB_TYPE_HTTP_VERSION, WEB_HTTP_VERSION_NONE, flags2);
  properties [prop_method] = g_param_spec_string ("method", "method", "method", NULL, flags2);
  properties [prop_request_body] = g_param_spec_boxed ("request-body", "request-body", "request-body", WEB_TYPE_MESSAGE_BODY, flags2);
  properties [prop_request_headers] = g_param_spec_boxed ("request-headers", "request-headers", "request-headers", WEB_TYPE_MESSAGE_HEADERS, flags2);
  properties [prop_response_body] = g_param_spec_boxed ("response-body", "response-body", "response-body", WEB_TYPE_MESSAGE_BODY, flags2);
  properties [prop_response_headers] = g_param_spec_boxed ("response-headers", "response-headers", "response-headers", WEB_TYPE_MESSAGE_HEADERS, flags2);
  properties [prop_status] = g_param_spec_enum ("status", "status", "status", WEB_TYPE_STATUS_CODE, WEB_STATUS_CODE_NONE, flags2);
  properties [prop_uri] = g_param_spec_boxed ("uri", "uri", "uri", G_TYPE_URI, flags2);
  g_object_class_install_properties (G_OBJECT_CLASS (klass), prop_number, properties);
  signals [signal_frozen] = g_signal_new ("frozen", g_type, flags3, 0, NULL, NULL, marshal1, G_TYPE_NONE, 1, G_TYPE_INT);
  signals [signal_thawed] = g_signal_new ("thawed", g_type, flags3, 0, NULL, NULL, marshal1, G_TYPE_NONE, 1, G_TYPE_INT);
}

static void web_message_init (WebMessage* self)
{
  self->priv = web_message_get_instance_private (self);
  self->priv->freeze_count = 0;
  self->priv->http_version = WEB_HTTP_VERSION_NONE;
  self->priv->method = NULL;
  self->priv->request_body = web_message_body_new ();
  self->priv->request_headers = web_message_headers_new ();
  self->priv->response_body = web_message_body_new ();
  self->priv->response_headers = web_message_headers_new ();
  self->priv->status_code = WEB_STATUS_CODE_NONE;
  self->priv->uri = NULL;
}

guint _web_message_get_freeze_count (WebMessage* web_message)
{
  g_return_val_if_fail (WEB_IS_MESSAGE (web_message), 0);
  return web_message->priv->freeze_count;
}

WebMessage* web_message_new ()
{
  return g_object_new (WEB_TYPE_MESSAGE, NULL);
}

void web_message_freeze (WebMessage* web_message)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  WebMessagePrivate* priv = web_message->priv;
  g_return_if_fail (priv->freeze_count < (127 - 1));

  g_signal_emit (web_message, signals [signal_frozen], 0, ++priv->freeze_count);
}

WebHttpVersion web_message_get_http_version (WebMessage* web_message)
{
  g_return_val_if_fail (WEB_IS_MESSAGE (web_message), 0);
  WebMessagePrivate* priv = web_message->priv;
return priv->http_version;
}

gboolean web_message_get_is_closure (WebMessage* web_message)
{
  g_return_val_if_fail (WEB_IS_MESSAGE (web_message), FALSE);
  WebMessagePrivate* priv = web_message->priv;
return priv->is_closure;
}

const gchar* web_message_get_method (WebMessage* web_message)
{
  g_return_val_if_fail (WEB_IS_MESSAGE (web_message), NULL);
  WebMessagePrivate* priv = web_message->priv;
return priv->method;
}

WebStatusCode web_message_get_status (WebMessage* web_message)
{
  g_return_val_if_fail (WEB_IS_MESSAGE (web_message), 0);
  WebMessagePrivate* priv = web_message->priv;
return priv->status_code;
}

GUri* web_message_get_uri (WebMessage* web_message)
{
  g_return_val_if_fail (WEB_IS_MESSAGE (web_message), NULL);
  WebMessagePrivate* priv = web_message->priv;
return priv->uri;
}

void web_message_set_http_version (WebMessage* web_message, WebHttpVersion http_version)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  WebMessagePrivate* priv = web_message->priv;
  priv->http_version = http_version;
}

void web_message_set_is_closure (WebMessage* web_message, gboolean is_closure)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  WebMessagePrivate* priv = web_message->priv;
  priv->is_closure = is_closure;
}

void web_message_set_method (WebMessage* web_message, const gchar* method)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  WebMessagePrivate* priv = web_message->priv;
  priv->method = g_intern_string (method);
}

void web_message_set_redirect (WebMessage* web_message, WebStatusCode status_code, const gchar* redirect_uri)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  WebMessagePrivate* priv = web_message->priv;

  web_message_set_status_full (web_message, status_code, web_status_code_get_inline (status_code));
  web_message_headers_set_location (priv->response_headers, redirect_uri);
}

void web_message_set_request (WebMessage* web_message, const gchar* content_type, const gchar* request, gsize length)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));

  web_message_set_request_take (web_message, content_type, g_strndup (request, length), length);
}

void web_message_set_request_bytes (WebMessage* web_message, const gchar* content_type, GBytes* bytes)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  WebMessagePrivate* priv = web_message->priv;

  web_message_body_add_bytes (priv->request_body, bytes);
  web_message_headers_set_content_type (priv->request_headers, content_type);
}

void web_message_set_request_take (WebMessage* web_message, const gchar* content_type, gchar* request, gsize length)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  WebMessagePrivate* priv = web_message->priv;

  web_message_body_add_data (priv->request_body, request, length, g_free);
  web_message_headers_set_content_type (priv->request_headers, content_type);
}

void web_message_set_response (WebMessage* web_message, const gchar* content_type, const gchar* response, gsize length)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  web_message_set_response_take (web_message, content_type, g_strndup (response, length), length);
}

void web_message_set_response_bytes (WebMessage* web_message, const gchar* content_type, GBytes* bytes)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  WebMessagePrivate* priv = web_message->priv;

  web_message_body_add_bytes (priv->response_body, bytes);
  web_message_headers_set_content_length (priv->response_headers, g_bytes_get_size (bytes));
  web_message_headers_set_content_type (priv->response_headers, content_type);
}

void web_message_set_response_take (WebMessage* web_message, const gchar* content_type, gchar* response, gsize length)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  WebMessagePrivate* priv = web_message->priv;

  web_message_body_add_data (priv->response_body, response, length, g_free);
  web_message_headers_set_content_length (priv->response_headers, length);
  web_message_headers_set_content_type (priv->response_headers, content_type);
}

void web_message_set_status (WebMessage* web_message, WebStatusCode status_code)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  WebMessagePrivate* priv = web_message->priv;
  priv->status_code = status_code;
}

void web_message_set_status_full (WebMessage* web_message, WebStatusCode status_code, const gchar* reason)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  WebMessagePrivate* priv = web_message->priv;

  priv->status_code = status_code;
  web_message_set_response (web_message, "text/plain", reason, strlen (reason));
}

void web_message_set_uri (WebMessage* web_message, GUri* uri)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  WebMessagePrivate* priv = web_message->priv;

  _g_uri_unref0 (priv->uri);
  priv->uri = g_uri_ref (uri);
}

void web_message_thaw (WebMessage* web_message)
{
  g_return_if_fail (WEB_IS_MESSAGE (web_message));
  WebMessagePrivate* priv = web_message->priv;
  g_return_if_fail (priv->freeze_count > 0);

  g_signal_emit (web_message, signals [signal_thawed], 0, --priv->freeze_count);
}

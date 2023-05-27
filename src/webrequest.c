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
#include <webrequest.h>

#define _g_uri_unref0(var) ((var == NULL) ? NULL : (var = (g_uri_unref (var), NULL)))

struct _WebRequestPrivate
{
  WebHttpVersion http_version;
  gchar* method;
  GUri* uri;
};

enum
{
  prop_0,
  prop_http_version,
  prop_method,
  prop_uri,
  prop_number,
};

G_DEFINE_TYPE_WITH_PRIVATE (WebRequest, web_request, WEB_TYPE_MESSAGE);
static GParamSpec* properties [prop_number] = {0};

static void web_request_class_dispose (GObject* pself)
{
  WebRequest* self = (gpointer) pself;
  WebRequestPrivate* priv = self->priv;
G_OBJECT_CLASS (web_request_parent_class)->dispose (pself);
}

static void web_request_class_finalize (GObject* pself)
{
  WebRequest* self = (gpointer) pself;
  WebRequestPrivate* priv = self->priv;
  _g_uri_unref0 (priv->uri);
G_OBJECT_CLASS (web_request_parent_class)->finalize (pself);
}

static void web_request_class_get_property (GObject* pself, guint property_id, GValue* value, GParamSpec* pspec)
{
  WebRequest* self = (gpointer) pself;
  WebRequestPrivate* priv = self->priv;

  switch (property_id)
    {
      case prop_http_version:
        g_value_set_enum (value, priv->http_version);
        break;
      case prop_method:
        g_value_set_pointer (value, (gpointer) web_request_get_method (self));
        break;
      case prop_uri:
        g_value_set_boxed (value, web_request_get_uri (self));
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (pself, property_id, pspec);
        break;
    }
}

static void web_request_class_set_property (GObject* pself, guint property_id, const GValue* value, GParamSpec* pspec)
{
  WebRequest* self = (gpointer) pself;
  WebRequestPrivate* priv = self->priv;

  switch (property_id)
    {
      case prop_http_version:
        web_request_set_http_version (self, g_value_get_enum (value));
        break;
      case prop_method:
        web_request_set_method (self, g_value_get_pointer (value));
        break;
      case prop_uri:
        web_request_set_uri (self, g_value_get_boxed (value));
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (pself, property_id, pspec);
        break;
    }
}

static void web_request_class_init (WebRequestClass* klass)
{
  G_OBJECT_CLASS (klass)->dispose = web_request_class_dispose;
  G_OBJECT_CLASS (klass)->finalize = web_request_class_finalize;
  G_OBJECT_CLASS (klass)->get_property = web_request_class_get_property;
  G_OBJECT_CLASS (klass)->set_property = web_request_class_set_property;

  properties [prop_http_version] = g_param_spec_enum ("http-version", "http-version", "http-version", WEB_TYPE_HTTP_VERSION, WEB_HTTP_VERSION_NONE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties [prop_method] = g_param_spec_pointer ("method", "method", "method", G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  properties [prop_uri] = g_param_spec_boxed ("uri", "uri", "uri", G_TYPE_URI, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties (G_OBJECT_CLASS (klass), prop_number, properties);
}

static void web_request_init (WebRequest* self)
{
  self->priv = web_request_get_instance_private (self);
  self->priv->http_version = WEB_HTTP_VERSION_NONE;
  self->priv->method = NULL;
  self->priv->uri = NULL;
}

WebMessage* web_request_new ()
{
  return g_object_new (WEB_TYPE_REQUEST, NULL);
}

WebHttpVersion web_request_get_http_version (WebRequest* web_request)
{
  g_return_val_if_fail (WEB_IS_REQUEST (web_request), 0);
  WebRequestPrivate* priv = web_request->priv;
return priv->http_version;
}

const gchar* web_request_get_method (WebRequest* web_request)
{
  g_return_val_if_fail (WEB_IS_REQUEST (web_request), NULL);
  WebRequestPrivate* priv = web_request->priv;
return priv->method;
}

GUri* web_request_get_uri (WebRequest* web_request)
{
  g_return_val_if_fail (WEB_IS_REQUEST (web_request), NULL);
  WebRequestPrivate* priv = web_request->priv;
return priv->uri;
}

void web_request_set_http_version (WebRequest* web_request, WebHttpVersion http_version)
{
  g_return_if_fail (WEB_IS_REQUEST (web_request));
  WebRequestPrivate* priv = web_request->priv;

  priv->http_version = http_version;
}

void web_request_set_method (WebRequest* web_request, const gchar* method)
{
  g_return_if_fail (WEB_IS_REQUEST (web_request));
  g_return_if_fail (method != NULL);
  WebRequestPrivate* priv = web_request->priv;

  priv->method = (gchar*) method;
}

void web_request_set_uri (WebRequest* web_request, GUri* uri)
{
  g_return_if_fail (WEB_IS_REQUEST (web_request));
  g_return_if_fail (uri != NULL);
  WebRequestPrivate* priv = web_request->priv;

  _g_uri_unref0 (priv->uri);
  priv->uri = g_uri_ref (uri);
}

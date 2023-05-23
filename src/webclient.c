/* Copyright 2023 MarcosHCK
 * This file is part of WebbServer.
 *
 * WebbServer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * WebbServer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with WebbServer. If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>
#include <webclient.h>

#define WEB_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), WEB_TYPE_CLIENT, WebClientClass))
#define WEB_IS_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), WEB_TYPE_CLIENT))
#define WEB_CLIENT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), WEB_TYPE_CLIENT, WebClientClass))
typedef struct _WebClientClass WebClientClass;
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

struct _WebClient
{
  GObject parent;

  /* private */
  GIOStream* stream;
};

struct _WebClientClass
{
  GObjectClass parent;
};

enum
{
  prop_0,
  prop_stream,
  prop_number,
};

G_DEFINE_FINAL_TYPE (WebClient, web_client, G_TYPE_OBJECT);
static GParamSpec* properties [prop_number] = {0};

static void web_client_class_dispose (GObject* pself)
{
  WebClient* self = (gpointer) pself;
  _g_object_unref0 (self->stream);
G_OBJECT_CLASS (web_client_parent_class)->dispose (pself);
}

static void web_client_class_finalize (GObject* pself)
{
  WebClient* self = (gpointer) pself;
G_OBJECT_CLASS (web_client_parent_class)->finalize (pself);
}

static void web_client_class_get_property (GObject* pself, guint property_id, GValue* value, GParamSpec* pspec)
{
  WebClient* self = (gpointer) pself;

  switch (property_id)
    {
      case prop_stream:
        g_value_set_object (value, web_client_get_stream (self));
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (pself, property_id, pspec);
        break;
    }
}

static void web_client_class_set_property (GObject* pself, guint property_id, const GValue* value, GParamSpec* pspec)
{
  WebClient* self = (gpointer) pself;

  switch (property_id)
    {
      case prop_stream:
        g_set_object (& self->stream, g_value_get_object (value));
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (pself, property_id, pspec);
        break;
    }
}

static void web_client_class_init (WebClientClass* klass)
{
  G_OBJECT_CLASS (klass)->dispose = web_client_class_dispose;
  G_OBJECT_CLASS (klass)->finalize = web_client_class_finalize;
  G_OBJECT_CLASS (klass)->get_property = web_client_class_get_property;
  G_OBJECT_CLASS (klass)->set_property = web_client_class_set_property;

  const GParamFlags flags1 = G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS;

  properties [prop_stream] = g_param_spec_object ("stream", "stream", "stream", G_TYPE_IO_STREAM, flags1);
  g_object_class_install_properties (G_OBJECT_CLASS (klass), prop_number, properties);
}

static void web_client_init (WebClient* self)
{
}

WebClient* web_client_new (GIOStream* stream)
{
  return g_object_new (WEB_TYPE_CLIENT, "stream", stream, NULL);
}

GIOStream* web_client_get_stream (WebClient* web_client)
{
  g_return_val_if_fail (WEB_IS_CLIENT (web_client), NULL);
return web_client->stream;
}

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
#ifndef __WEB_MESSAGE__
#define __WEB_MESSAGE__ 1
#include <glib-object.h>
#include <webhttpversion.h>

#define WEB_TYPE_MESSAGE (web_message_get_type ())
#define WEB_MESSAGE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), WEB_TYPE_MESSAGE, WebMessage))
#define WEB_IS_MESSAGE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), WEB_TYPE_MESSAGE))
typedef struct _WebMessage WebMessage;
typedef struct _WebMessagePrivate WebMessagePrivate;
#define WEB_MESSAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), WEB_TYPE_MESSAGE, WebMessageClass))
#define WEB_IS_MESSAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), WEB_TYPE_MESSAGE))
#define WEB_MESSAGE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), WEB_TYPE_MESSAGE, WebMessageClass))
typedef struct _WebMessageClass WebMessageClass;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _WebMessage
  {
    GObject parent;
    WebMessagePrivate* priv;
  };

  struct _WebMessageClass
  {
    GObjectClass parent;
  };

  G_GNUC_INTERNAL GType web_message_get_type (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* web_message_get_field (WebMessage* web_message, const gchar* key);
  G_GNUC_INTERNAL void web_message_get_field_iter (WebMessage* web_message, GHashTableIter* iter);
  G_GNUC_INTERNAL WebHttpVersion web_message_get_http_version (WebMessage* web_message);
  G_GNUC_INTERNAL const gchar* web_message_get_method (WebMessage* web_message);
  G_GNUC_INTERNAL GUri* web_message_get_uri (WebMessage* web_message);
  G_GNUC_INTERNAL void web_message_insert_field (WebMessage* web_message, const gchar* key, const gchar* value);
  G_GNUC_INTERNAL void web_message_insert_field_take (WebMessage* web_message, gchar* key, gchar* value);
  G_GNUC_INTERNAL void web_message_delete_field (WebMessage* web_message, const gchar* key);
  G_GNUC_INTERNAL void web_message_set_body (WebMessage* web_message, GBytes* bytes);
  G_GNUC_INTERNAL void web_message_set_http_version (WebMessage* web_message, WebHttpVersion http_version);
  G_GNUC_INTERNAL void web_message_set_method (WebMessage* web_message, const gchar* method);
  G_GNUC_INTERNAL void web_message_set_uri (WebMessage* web_message, GUri* uri);

#if __cplusplus
}
#endif // __cplusplus

#endif // __WEB_MESSAGE__

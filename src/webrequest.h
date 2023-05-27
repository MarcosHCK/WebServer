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
#ifndef __WEB_REQUEST__
#define __WEB_REQUEST__ 1
#include <webmessage.h>

#define WEB_TYPE_REQUEST (web_request_get_type ())
#define WEB_REQUEST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), WEB_TYPE_REQUEST, WebRequest))
#define WEB_IS_REQUEST(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), WEB_TYPE_REQUEST))
typedef struct _WebRequest WebRequest;
typedef struct _WebRequestPrivate WebRequestPrivate;
#define WEB_REQUEST_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), WEB_TYPE_REQUEST, WebRequestClass))
#define WEB_IS_REQUEST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), WEB_TYPE_REQUEST))
#define WEB_REQUEST_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), WEB_TYPE_REQUEST, WebRequestClass))
typedef struct _WebRequestClass WebRequestClass;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _WebRequest
  {
    WebMessage parent;
    WebRequestPrivate* priv;
  };

  struct _WebRequestClass
  {
    WebMessageClass parent;
  };

  G_GNUC_INTERNAL GType web_request_get_type (void) G_GNUC_CONST;
  G_GNUC_INTERNAL WebMessage* web_request_new ();
  G_GNUC_INTERNAL WebHttpVersion web_request_get_http_version (WebRequest* web_request);
  G_GNUC_INTERNAL const gchar* web_request_get_method (WebRequest* web_message);
  G_GNUC_INTERNAL GUri* web_request_get_uri (WebRequest* web_message);
  G_GNUC_INTERNAL void web_request_set_http_version (WebRequest* web_request, WebHttpVersion http_version);
  G_GNUC_INTERNAL void web_request_set_method (WebRequest* web_message, const gchar* method);
  G_GNUC_INTERNAL void web_request_set_uri (WebRequest* web_message, GUri* uri);

#if __cplusplus
}
#endif // __cplusplus

#endif // __WEB_REQUEST__

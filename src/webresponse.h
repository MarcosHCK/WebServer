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
#ifndef __WEB_RESPONSE__
#define __WEB_RESPONSE__ 1
#include <webmessage.h>
#include <webstatuscode.h>

#define WEB_TYPE_RESPONSE (web_response_get_type ())
#define WEB_RESPONSE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), WEB_TYPE_RESPONSE, WebResponse))
#define WEB_IS_RESPONSE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), WEB_TYPE_RESPONSE))
typedef struct _WebResponse WebResponse;
typedef struct _WebResponsePrivate WebResponsePrivate;
#define WEB_RESPONSE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), WEB_TYPE_RESPONSE, WebResponseClass))
#define WEB_IS_RESPONSE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), WEB_TYPE_RESPONSE))
#define WEB_RESPONSE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), WEB_TYPE_RESPONSE, WebResponseClass))
typedef struct _WebResponseClass WebResponseClass;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _WebResponse
  {
    WebMessage parent;
    WebResponsePrivate* priv;
  };

  struct _WebResponseClass
  {
    WebMessageClass parent;
  };

  G_GNUC_INTERNAL GType web_response_get_type (void) G_GNUC_CONST;
  G_GNUC_INTERNAL WebMessage* web_response_new ();

#if __cplusplus
}
#endif // __cplusplus

#endif // __WEB_RESPONSE__

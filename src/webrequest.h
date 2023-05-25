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
#define WEB_REQUEST_PARSE_ERROR (web_request_parse_error_quark ())

#if __cplusplus
extern "C" {
#endif // __cplusplus

  typedef enum
  {
    WEB_REQUEST_PARSE_ERROR_FAILED,
    WEB_REQUEST_PARSE_ERROR_UNKNOWN_METHOD,
    WEB_REQUEST_PARSE_ERROR_UNSUPPORTED_VERSION,
    WEB_REQUEST_PARSE_ERROR_MALFORMED_FIELD,
    WEB_REQUEST_PARSE_ERROR_MALFORMED_REQUEST,
  } WebRequestParseError;

  G_GNUC_INTERNAL GType web_request_get_type (void) G_GNUC_CONST;
  G_GNUC_INTERNAL GQuark web_request_parse_error_quark (void) G_GNUC_CONST;
  G_GNUC_INTERNAL WebRequest* web_request_new ();
  G_GNUC_INTERNAL gboolean web_request_is_complete (WebRequest* web_request);
  G_GNUC_INTERNAL void web_request_parse_line (WebRequest* web_request, const gchar* line, gsize length, GError** error);
  G_GNUC_INTERNAL void web_request_parse_body (WebRequest* web_request, const gchar* body, gsize length, GError** error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __WEB_REQUEST__

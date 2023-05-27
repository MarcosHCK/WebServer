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
#ifndef __WEB_STATUS_CODE__
#define __WEB_STATUS_CODE__ 1
#include <glib-object.h>

#define WEB_TYPE_STATUS_CODE (web_status_code_get_type ())

#if __cplusplus
extern "C" {
#endif // __cplusplus

  typedef enum
  {
    WEB_STATUS_CODE_NONE,
    WEB_STATUS_CODE_CONTINUE = 100,
    WEB_STATUS_CODE_SWITCHING_PROTOCOL = 101,
    WEB_STATUS_CODE_OK = 200,
    WEB_STATUS_CODE_CREATED = 201,
    WEB_STATUS_CODE_ACCEPTED = 202,
    WEB_STATUS_CODE_NON_AUTHORITATIVE_INFORMATION = 203,
    WEB_STATUS_CODE_NO_CONTENT = 204,
    WEB_STATUS_CODE_RESET_CONTENT = 205,
    WEB_STATUS_CODE_PARTIAL_CONTENT = 206,
    WEB_STATUS_CODE_MULTIPLE_CHOICES = 300,
    WEB_STATUS_CODE_MOVED_PERMANENTLY = 301,
    WEB_STATUS_CODE_FOUND = 302,
    WEB_STATUS_CODE_SEE_OTHER = 303,
    WEB_STATUS_CODE_NOT_MODIFIED = 304,
    WEB_STATUS_CODE_TEMPORARY_REDIRECT = 307,
    WEB_STATUS_CODE_PERMANENT_REDIRECT = 308,
    WEB_STATUS_CODE_BAD_REQUEST = 400,
    WEB_STATUS_CODE_UNAUTHORIZED = 401,
    WEB_STATUS_CODE_FORBIDDEN = 403,
    WEB_STATUS_CODE_NOT_FOUND = 404,
    WEB_STATUS_CODE_METHOD_NOT_ALLOWED = 405,
    WEB_STATUS_CODE_NOT_ACCEPTABLE = 406,
    WEB_STATUS_CODE_PROXY_AUTHENTICATION_REQUIRED = 407,
    WEB_STATUS_CODE_REQUEST_TIMEOUT = 408,
    WEB_STATUS_CODE_CONFLICT = 409,
    WEB_STATUS_CODE_GONE = 410,
    WEB_STATUS_CODE_LENGTH_REQUIRED = 411,
    WEB_STATUS_CODE_PRECONDITION_FAILED = 412,
    WEB_STATUS_CODE_CONTENT_TOO_LARGE = 413,
    WEB_STATUS_CODE_URI_TOO_LONG = 414,
    WEB_STATUS_CODE_UNSUPPORTED_MEDIA_TYPE = 415,
    WEB_STATUS_CODE_RANGE_NO_SATISFIABLE = 416,
    WEB_STATUS_CODE_EXPECTATION_FAILED = 417,
    WEB_STATUS_CODE_MISDIRECTED_REQUEST = 421,
    WEB_STATUS_CODE_UNPROCESSABLE_CONTENT = 422,
    WEB_STATUS_CODE_UPGRADE_REQUIRED = 426,
    WEB_STATUS_CODE_INTERNAL_SERVER_ERROR = 500,
    WEB_STATUS_CODE_NOT_IMPLEMENTED = 501,
    WEB_STATUS_CODE_BAD_GATEWAY = 502,
    WEB_STATUS_CODE_SERVICE_UNAVAILABLE = 503,
    WEB_STATUS_CODE_GATEWAY_TIMEOUT = 504,
    WEB_STATUS_CODE_HTTP_VERSION_NOT_SUPPORTED = 505,
  } WebStatusCode;

  G_GNUC_INTERNAL GType web_status_code_get_type (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* web_status_code_get_inline (WebStatusCode status_code);

#if __cplusplus
}
#endif // __cplusplus

#endif // __WEB_STATUS_CODE__

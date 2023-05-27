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
#include <webstatuscode.h>

G_DEFINE_ENUM_TYPE
  (WebStatusCode, web_status_code,
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_NONE, "none"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_CONTINUE, "Continue"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_SWITCHING_PROTOCOL, "Switching Protocol"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_OK, "OK"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_CREATED, "Created"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_ACCEPTED, "Accepted"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_NON_AUTHORITATIVE_INFORMATION, "Non Authoritative Information"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_NO_CONTENT, "No Content"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_RESET_CONTENT, "Reset Content"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_PARTIAL_CONTENT, "Partial Content"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_MULTIPLE_CHOICES, "Multiple Choices"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_MOVED_PERMANENTLY, "Moved Permanently"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_FOUND, "Found"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_SEE_OTHER, "See Other"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_NOT_MODIFIED, "Not Modified"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_TEMPORARY_REDIRECT, "Temporary Redirect"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_PERMANENT_REDIRECT, "Permanent Redirect"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_BAD_REQUEST, "Bad Request"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_UNAUTHORIZED, "Unauthorized"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_FORBIDDEN, "Forbidden"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_NOT_FOUND, "Not Found"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_METHOD_NOT_ALLOWED, "Method Not Allowed"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_NOT_ACCEPTABLE, "Not Acceptable"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_PROXY_AUTHENTICATION_REQUIRED, "Proxy Authentication Required"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_REQUEST_TIMEOUT, "Request Timeout"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_CONFLICT, "Conflict"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_GONE, "Gone"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_LENGTH_REQUIRED, "Length Required"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_PRECONDITION_FAILED, "Precondition Failed"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_CONTENT_TOO_LARGE, "Content Too Large"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_URI_TOO_LONG, "Uri Too Long"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_RANGE_NO_SATISFIABLE, "Range No Satisfiable"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_EXPECTATION_FAILED, "Expectation Failed"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_MISDIRECTED_REQUEST, "Misdirected Request"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_UNPROCESSABLE_CONTENT, "Unprocessable Content"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_UPGRADE_REQUIRED, "Upgrade Required"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_INTERNAL_SERVER_ERROR, "Internal Server Error"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_NOT_IMPLEMENTED, "Not Implemented"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_BAD_GATEWAY, "Bad Gateway"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_SERVICE_UNAVAILABLE, "Service Unavailable"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_GATEWAY_TIMEOUT, "Gateway Timeout"),
    G_DEFINE_ENUM_VALUE (WEB_STATUS_CODE_HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported")
      );

const gchar* web_status_code_get_inline (WebStatusCode status_code)
{
  GEnumClass* klass = g_type_class_ref (WEB_TYPE_STATUS_CODE);
  GEnumValue* value = g_enum_get_value (klass, (gint) status_code);
  const gchar* nick = g_intern_string (value->value_nick);
return (g_type_class_unref (klass), nick);
}

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
#ifndef __WEB_MESSAGE_HEADER_PARSE__
#define __WEB_MESSAGE_HEADER_PARSE__ 1
#include <webmessage.h>

typedef struct _WebMessageHeaders WebMessageHeaders;
#define WEB_MESSAGE_HEADER_PARSE_ERROR (web_message_header_parse_error_quark ())

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _WebMessageHeaders
  {
    GHashTable* fields;
    GQueue ranges;
    GQueue taken;
  };

  typedef enum
  {
    WEB_MESSAGE_HEADER_PARSE_ERROR_FAILED,
    WEB_MESSAGE_HEADER_PARSE_ERROR_INVALID_FIELD,
    WEB_MESSAGE_HEADER_PARSE_ERROR_INVALID_RANGE,
  } WebMessageHeaderParseError;

  G_GNUC_INTERNAL GQuark web_message_header_parse_error_quark (void) G_GNUC_CONST;
  G_GNUC_INTERNAL void _web_message_headers_parse_header (WebMessageHeaders* headers, gchar* key, gchar* value, GError** error);
  G_GNUC_INTERNAL void _web_message_range_free (WebMessageRange* range);

#if __cplusplus
}
#endif // __cplusplus

#endif // __WEB_MESSAGE_HEADER_PARSE__

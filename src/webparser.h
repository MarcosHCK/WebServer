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
#ifndef __WEB_PARSER__
#define __WEB_PARSER__ 1
#include <glib.h>

typedef struct _WebParser WebParser;
typedef struct _WebParserField WebParserField;
#define WEB_PARSER_ERROR (web_parser_error_quark ())

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _WebParser
  {
    guint complete : 1;
    guint got_simple_request : 1;
    guint got_request_line : 1;
    const gchar* method;
    GQueue fields;
    GUri* uri;
    WebHttpVersion http_version;
  };

  struct _WebParserField
  {
    gchar* name;
    gchar* value;
    gsize valuesz;
  };

  typedef enum
  {
    WEB_PARSER_ERROR_FAILED,
    WEB_PARSER_ERROR_UNKNOWN_METHOD,
    WEB_PARSER_ERROR_UNSUPPORTED_VERSION,
    WEB_PARSER_ERROR_MALFORMED_FIELD,
    WEB_PARSER_ERROR_MALFORMED_REQUEST,
  } WebParserError;

  G_GNUC_INTERNAL GQuark web_parser_error_quark (void) G_GNUC_CONST;
  G_GNUC_INTERNAL void web_parser_clear (WebParser* parser);
  G_GNUC_INTERNAL void web_parser_feed (WebParser* parser, const gchar* line, gsize length, GError** error);
  G_GNUC_INTERNAL void web_parser_field_set_name (WebParserField* self, const gchar* name, gsize length);
  G_GNUC_INTERNAL void web_parser_field_add_value (WebParserField* self, const gchar* value, gsize length);
  G_GNUC_INTERNAL void web_parser_field_free (WebParserField* self);
  G_GNUC_INTERNAL void web_parser_init (WebParser* parser);

#if __cplusplus
}
#endif // __cplusplus

#endif // __WEB_PARSER__

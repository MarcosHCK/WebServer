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
#include <webhttpversion.h>
#include <webmessagefields.h>
#include <webmessagemethods.h>
#include <webparser.h>

typedef struct _Patterns Patterns;
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))
#define _g_match_info_free0(var) ((var == NULL) ? NULL : (var = (g_match_info_free (var), NULL)))
#define _g_uri_unref0(var) ((var == NULL) ? NULL : (var = (g_uri_unref (var), NULL)))
#define _web_parser_patterns_unref0(var) ((var == NULL) ? NULL : (var = (web_parser_patterns_unref (var), NULL)))
#define web_parser_field_new() (g_slice_new0 (WebParserField))

#define PATTERNS_INIT Patterns* patterns = patterns_peek ()

static void parse_header_line (WebParser* self, const gchar* line, gsize length, GError** error);
static guint parse_http_version_bit (const gchar* line, gsize length, gsize offset, GError** error);
static const gchar* parse_method (const gchar* line, gsize length, gsize offset, GError** error);
static void parse_request_line (WebParser* self, const gchar* line, gsize length, GError** error);
static void parse_request_line_full (WebParser* self, GMatchInfo* info, const gchar* line, gsize length, GError** error);
static void parse_request_line_simple (WebParser* self, GMatchInfo* info, const gchar* line, gsize length, GError** error);

struct _Patterns
{
  guint ref_count;
  GUri* base_uri;
  GRegex* folded_field;
  GRegex* full_request_line;
  GRegex* simple_field;
  GRegex* simple_request_line;
};

G_DEFINE_QUARK (web-parser-error-quark, web_parser_error);

static GUri* _web_uri_parse (const gchar* uri_string, GUriFlags uri_flags)
{
#if DEVELOPER == 1
  GError* tmperr = NULL;
  GUri* uri = NULL;

  if ((uri = g_uri_parse (uri_string, uri_flags, &tmperr)), G_UNLIKELY (tmperr == NULL))
      return uri;
    else
      {
        const gchar* domain = g_quark_to_string (tmperr->domain);
        const gchar* message = tmperr->message;
        const guint code = tmperr->code;

        g_error ("(" G_STRLOC "): %s: %d: %s", domain, code, message);
        g_assert_not_reached ();
      }
#else // DEVELOPER
  return g_uri_parse (uri_string, uri_flags, NULL);
#endif // DEVELOPER
}

static GRegex* _web_regex_new (const gchar* pattern, GRegexCompileFlags compile_flags, GRegexMatchFlags match_flags)
{
#if DEVELOPER == 1
  GError* tmperr = NULL;
  GRegex* regex = NULL;

  if ((regex = g_regex_new (pattern, compile_flags, match_flags, &tmperr)), G_UNLIKELY (tmperr == NULL))
      return regex;
    else
      {
        const gchar* domain = g_quark_to_string (tmperr->domain);
        const gchar* message = tmperr->message;
        const guint code = tmperr->code;

        g_error ("(" G_STRLOC "): %s: %d: %s", domain, code, message);
        g_assert_not_reached ();
      }
#else // DEVELOPER
  return g_regex_new (pattern, compile_flags, match_flags, NULL);
#endif // DEVELOPER
}

static Patterns* patterns_peek ()
{
  static gsize __value__ = 0;

  if (g_once_init_enter (&__value__))
    {
      Patterns* self;

      /* components */
      #define CTL "\\x{0}-\\x{1f}"
      #define HEX "[:xdigit:]"
      #define CR "\\x{0d}"
      #define LF "\\x{0a}"
      #define CRLF CR LF
      #define SP "\\x{20}"
      #define HT "\\x{09}"
      #define SPECIALS "\\(\\)<>@,;:\\\\\"/\\[\\]\\?={}\\x{0a}\\x{0d}"

      /* entities */
      #define TEXT "[^" CTL "\\x{0a}\\x{0d}]"
      #define TOKEN "[^" CTL SPECIALS "]"

      self = g_slice_new (Patterns);
      self->base_uri = _web_uri_parse ("http://localhost/", G_URI_FLAGS_NON_DNS);
      self->folded_field = _web_regex_new ("^[" SP HT "](.+)$", G_REGEX_OPTIMIZE | G_REGEX_RAW, 0);
      self->full_request_line = _web_regex_new ("^(" TOKEN "+)\\s(.+?)\\sHTTP/([0-9]+)\\.([0-9]+)$", G_REGEX_OPTIMIZE | G_REGEX_RAW, 0);
      self->simple_field = _web_regex_new ("^(" TOKEN "+):\\s(.+)$", G_REGEX_OPTIMIZE | G_REGEX_RAW, 0);
      self->simple_request_line = _web_regex_new ("^(" TOKEN "+)\\s(.+)$", G_REGEX_OPTIMIZE | G_REGEX_RAW, 0);

      g_once_init_leave (& __value__, GPOINTER_TO_SIZE (self));
      G_STATIC_ASSERT (sizeof (__value__) == GLIB_SIZEOF_VOID_P);
    }
return GSIZE_TO_POINTER (__value__);
}

static void parse_header_line (WebParser* self, const gchar* line, gsize length, GError** error)
{
  gboolean matches;
  GMatchInfo* info;
  GError* tmperr = NULL;
  WebParserField* field;

  PATTERNS_INIT;

  gint name_start, name_end;
  gint value_start, value_end;

  if ((matches = g_regex_match_full (patterns->simple_field, line, length, 0, 0, &info, &tmperr)), G_UNLIKELY (tmperr != NULL))
    {
      g_propagate_error (error, tmperr);
      _g_match_info_free0 (info);
    }
  else if (matches == TRUE)
    {
      g_match_info_fetch_pos (info, 1, &name_start, &name_end);
      g_match_info_fetch_pos (info, 2, &value_start, &value_end);
      _g_match_info_free0 (info);

      field = web_parser_field_new ();

      web_parser_field_set_name (field, & G_STRUCT_MEMBER (gchar, line, name_start), name_end - name_start);
      web_parser_field_add_value (field, & G_STRUCT_MEMBER (gchar, line, value_start), value_end - value_start);
      g_queue_push_tail (& self->fields, g_steal_pointer (&field));
    }
  else
    {
      _g_match_info_free0 (info);

      if ((matches = g_regex_match_full (patterns->folded_field, line, length, 0, 0, &info, &tmperr)), G_UNLIKELY (tmperr != NULL))
        {
          g_propagate_error (error, tmperr);
          _g_match_info_free0 (info);
        }
      else if (matches == FALSE)
        {
          g_set_error (error, WEB_PARSER_ERROR, WEB_PARSER_ERROR_MALFORMED_FIELD, "Malformed field");
          _g_match_info_free0 (info);
        }
      else
        {
          if (g_queue_get_length (& self->fields) == 0)
            {
              g_set_error (error, WEB_PARSER_ERROR, WEB_PARSER_ERROR_MALFORMED_FIELD, "Misplaced folded field");
              _g_match_info_free0 (info);
            }
          else
            {
              g_match_info_fetch_pos (info, 1, &value_start, &value_end);
              _g_match_info_free0 (info);

              field = g_queue_peek_tail (& self->fields);
              const gchar* chunk = & G_STRUCT_MEMBER (gchar, line, value_start);
              const gsize chunksz = value_end - value_start;

              web_parser_field_add_value (g_steal_pointer (&field), chunk, chunksz);
            }
        }
    }
}

static guint parse_http_version_bit (const gchar* line, gsize length, gsize offset, GError** error)
{
  gsize chars = length - offset;
  guint64 n = 0;

  gchar statbuf [32];
  gchar* dynbuf = NULL;
  gchar* buffer = NULL;

  if (chars < G_N_ELEMENTS (statbuf))
    {
      memcpy (statbuf, & G_STRUCT_MEMBER (gchar, line, offset), chars);
      buffer = (statbuf [chars] = 0, statbuf);
    }
  else
    {
      dynbuf = g_strndup (& G_STRUCT_MEMBER (gchar, line, offset), chars);
      buffer = (dynbuf);
    }

  g_ascii_string_to_unsigned (buffer, 10, 0, G_MAXINT, &n, error);
return (_g_free0 (dynbuf), (guint) n);
}

static const gchar* parse_method (const gchar* line, gsize length, gsize offset, GError** error)
{
  if (!g_ascii_strncasecmp (WEB_MESSAGE_METHOD_GET, & G_STRUCT_MEMBER (gchar, line, offset), length - offset))
    return WEB_MESSAGE_METHOD_GET;
  else if (!g_ascii_strncasecmp (WEB_MESSAGE_METHOD_HEAD, & G_STRUCT_MEMBER (gchar, line, offset), length - offset))
    return WEB_MESSAGE_METHOD_HEAD;
  else if (!g_ascii_strncasecmp (WEB_MESSAGE_METHOD_POST, & G_STRUCT_MEMBER (gchar, line, offset), length - offset))
    return WEB_MESSAGE_METHOD_POST;
  else
    {
      const gchar* method = & G_STRUCT_MEMBER (gchar, line, offset);
      const gsize size = length - offset;

      g_set_error (error, WEB_PARSER_ERROR, WEB_PARSER_ERROR_UNKNOWN_METHOD, "Unknown method '%.*s'", (int) size, method);
    }
return (NULL);
}

static void parse_request_line (WebParser* self, const gchar* line, gsize length, GError** error)
{
  gboolean matches;
  GMatchInfo* info;
  GError* tmperr = NULL;

  PATTERNS_INIT;

  if ((matches = g_regex_match_full (patterns->full_request_line, line, length, 0, 0, &info, &tmperr)), G_UNLIKELY (tmperr != NULL))
    {
      g_propagate_error (error, tmperr);
      _g_match_info_free0 (info);
    }
  else if (matches == TRUE)
    {
      parse_request_line_full (self, info, line, length, error);
      _g_match_info_free0 (info);
    }
  else
    {
      _g_match_info_free0 (info);

      if ((matches = g_regex_match_full (patterns->simple_request_line, line, length, 0, 0, &info, &tmperr)), G_UNLIKELY (tmperr != NULL))
        {
          g_propagate_error (error, tmperr);
          _g_match_info_free0 (info);
        }
      else if (matches == TRUE)
        {
          self->got_simple_request = TRUE;
          self->http_version = WEB_HTTP_VERSION_0_9;

          parse_request_line_simple (self, info, line, length, error);
          _g_match_info_free0 (info);
        }
      else
        {
          g_set_error_literal (error, WEB_PARSER_ERROR, WEB_PARSER_ERROR_MALFORMED_REQUEST, "Invalid request line");
          _g_match_info_free0 (info);
        }
    }
}

static void parse_request_line_full (WebParser* self, GMatchInfo* info, const gchar* line, gsize length, GError** error)
{
  GError* tmperr = NULL;
  WebHttpVersion version = 0;
  gint major_start, major_end;
  gint minor_start, minor_end;
  guint major, minor;

  g_match_info_fetch_pos (info, 3, &major_start, &major_end);
  g_match_info_fetch_pos (info, 4, &minor_start, &minor_end);

  if ((parse_request_line_simple (self, info, line, length, &tmperr)), G_UNLIKELY (tmperr != NULL))
    g_propagate_error (error, tmperr);
  else if ((major = parse_http_version_bit (line, major_end, major_start, &tmperr)), G_UNLIKELY (tmperr != NULL))
    g_propagate_error (error, tmperr);
  else if ((minor = parse_http_version_bit (line, minor_end, minor_start, &tmperr)), G_UNLIKELY (tmperr != NULL))
    g_propagate_error (error, tmperr);
  else if ((version = web_http_version_from_bits (major, minor)) == WEB_HTTP_VERSION_NONE)
    g_set_error_literal (error, WEB_PARSER_ERROR, WEB_PARSER_ERROR_UNSUPPORTED_VERSION, "Unsupported HTTP version");
  else
    self->http_version = version;
}

static void parse_request_line_simple (WebParser* self, GMatchInfo* info, const gchar* line, gsize length, GError** error)
{
  gchar* path = NULL;
  GError* tmperr = NULL;
  GUri* uri = NULL;

  PATTERNS_INIT;

  const gchar* method;
  gint method_start, method_end;
  gint path_start, path_end;

  g_match_info_fetch_pos (info, 1, &method_start, &method_end);
  g_match_info_fetch_pos (info, 2, &path_start, &path_end);

  if ((method = parse_method (line, method_end, method_start, &tmperr)), G_UNLIKELY (tmperr != NULL))
    g_propagate_error (error, tmperr);
  else
    {
      self->method = method;
      path = g_utf8_make_valid (& G_STRUCT_MEMBER (gchar, line, path_start), path_end - path_start);
      uri = g_uri_parse_relative (patterns->base_uri, path, G_URI_FLAGS_NON_DNS, &tmperr);

      if ((g_free (path)), G_UNLIKELY (tmperr != NULL))
        g_propagate_error (error, tmperr);
      else
        {
          _g_uri_unref0 (self->uri);
          self->uri = uri;
        }
    }
}

void web_parser_clear (WebParser* self)
{
  g_queue_clear_full (& self->fields, (GDestroyNotify) web_parser_field_free);
  _g_uri_unref0 (self->uri);
}

void web_parser_feed (WebParser* self, const gchar* line, gsize length, GError** error)
{
  g_return_if_fail (self->complete == FALSE);

  if (self->got_request_line == TRUE)
    {
      if (length == 0)
        self->complete = TRUE;
      else
        parse_header_line (self, line, length, error);
    }
  else
    {
      GError* tmperr = NULL;

      if ((parse_request_line (self, line, length, &tmperr)), G_UNLIKELY (tmperr != NULL))
        g_propagate_error (error, tmperr);
      else
        {
          self->got_request_line = TRUE;

          if (self->got_simple_request == TRUE)
            self->complete = TRUE;
        }
    }
}

void web_parser_field_set_name (WebParserField* self, const gchar* name, gsize length)
{
  _g_free0 (self->name);
  self->name = g_ascii_strdown (name, length);
}

void web_parser_field_add_value (WebParserField* self, const gchar* value, gsize length)
{
  gsize oldsz;

  if (self->value == NULL)
    {
      self->valuesz = length;
      self->value = g_strndup (value, length);
    }
  else
    {
      oldsz = self->valuesz;
      self->valuesz += (length + 2);
      self->value = g_realloc (self->value, (self->valuesz + 1));

      memcpy (& G_STRUCT_MEMBER (gchar, self->value, oldsz + 0), ", ", 2);
      memcpy (& G_STRUCT_MEMBER (gchar, self->value, oldsz + 2), value, length);
      G_STRUCT_MEMBER (gchar, self->value, self->valuesz) = 0;
    }
}

void web_parser_field_free (WebParserField* self)
{
  _g_free0 (self->name);
  _g_free0 (self->value);
  g_slice_free (WebParserField, self);
}

void web_parser_init (WebParser* self)
{
  self->complete = FALSE;
  self->got_request_line = FALSE;
  self->got_simple_request = FALSE;
  self->http_version = WEB_HTTP_VERSION_NONE;
  self->method = NULL;
  self->uri = NULL;

  g_queue_init (& self->fields);
}

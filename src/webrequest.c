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
#include <webmessage.h>
#include <webmessagefields.h>
#include <webmessagemethods.h>
#include <webrequest.h>

#define WEB_REQUEST_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), WEB_TYPE_REQUEST, WebRequestClass))
#define WEB_IS_REQUEST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), WEB_TYPE_REQUEST))
#define WEB_REQUEST_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), WEB_TYPE_REQUEST, WebRequestClass))
typedef struct _WebRequestClass WebRequestClass;
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))
#define _g_match_info_free0(var) ((var == NULL) ? NULL : (var = (g_match_info_free (var), NULL)))
#define _g_uri_unref0(var) ((var == NULL) ? NULL : (var = (g_uri_unref (var), NULL)))
static void parse_header_line (WebRequest* self, const gchar* line, gsize length, GError** error);
static guint parse_http_version_bit (const gchar* line, gsize length, gsize offset, GError** error);
static const gchar* parse_method (const gchar* line, gsize length, gsize offset, GError** error);
static void parse_request_line (WebRequest* self, const gchar* line, gsize length, GError** error);
static void parse_request_line_full (WebRequest* self, GMatchInfo* info, const gchar* line, gsize length, GError** error);
static void parse_request_line_simple (WebRequest* self, GMatchInfo* info, const gchar* line, gsize length, GError** error);

struct _WebRequest
{
  WebMessage parent;

  /* private */
  gchar* last_field;
  guint complete : 1;
  guint got_simple_request : 1;
  guint got_request_line : 1;
};

struct _WebRequestClass
{
  WebMessageClass parent;

  /* private */
  GUri* base_uri;
  GRegex* folded_field;
  GRegex* full_request_line;
  GRegex* http_version;
  GRegex* simple_field;
  GRegex* simple_request_line;
};

G_DEFINE_FINAL_TYPE (WebRequest, web_request, WEB_TYPE_MESSAGE);
G_DEFINE_QUARK (web-request-parse-error-quark, web_request_parse_error);

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

static void web_request_class_init (WebRequestClass* klass)
{
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

  klass->base_uri = _web_uri_parse ("http://localhost/", G_URI_FLAGS_NON_DNS);
  klass->folded_field = _web_regex_new ("^[" SP HT "](.+)$", G_REGEX_OPTIMIZE | G_REGEX_RAW, 0);
  klass->full_request_line = _web_regex_new ("^(" TOKEN "+)\\s(.+?)\\sHTTP/([0-9]+)\\.([0-9]+)$", G_REGEX_OPTIMIZE | G_REGEX_RAW, 0);
  klass->simple_field = _web_regex_new ("^(" TOKEN "+):\\s(.+)$", G_REGEX_OPTIMIZE | G_REGEX_RAW, 0);
  klass->simple_request_line = _web_regex_new ("^(" TOKEN "+)\\s(.+)$", G_REGEX_OPTIMIZE | G_REGEX_RAW, 0);
}

static void web_request_init (WebRequest* self)
{
  self->complete = FALSE;
  self->got_request_line = FALSE;
  self->got_simple_request = FALSE;
}

WebRequest* web_request_new ()
{
  return g_object_new (WEB_TYPE_REQUEST, NULL);
}

gboolean web_request_is_complete (WebRequest* web_request)
{
  g_return_val_if_fail (WEB_IS_REQUEST (web_request), FALSE);
return web_request->complete;
}

static void parse_header_line (WebRequest* self, const gchar* line, gsize length, GError** error)
{
  gboolean matches;
  GMatchInfo* info;
  GError* tmperr = NULL;

  WebRequestClass* klass = WEB_REQUEST_GET_CLASS (self);

  gint name_start, name_end;
  gint value_start, value_end;

  if ((matches = g_regex_match_full (klass->simple_field, line, length, 0, 0, &info, &tmperr)), G_UNLIKELY (tmperr != NULL))
    {
      g_propagate_error (error, tmperr);
      _g_match_info_free0 (info);
    }
  else if (matches == TRUE)
    {
      g_match_info_fetch_pos (info, 1, &name_start, &name_end);
      g_match_info_fetch_pos (info, 2, &value_start, &value_end);
      _g_match_info_free0 (info);

      gchar* name = g_strndup (& G_STRUCT_MEMBER (gchar, line, name_start), name_end - name_start);
      gchar* value = g_strndup (& G_STRUCT_MEMBER (gchar, line, value_start), value_end - value_start);

      web_message_insert_field_take (WEB_MESSAGE (self), name, value);
      self->last_field = name;
    }
  else
    {
      _g_match_info_free0 (info);

      if ((matches = g_regex_match_full (klass->folded_field, line, length, 0, 0, &info, &tmperr)), G_UNLIKELY (tmperr != NULL))
        {
          g_propagate_error (error, tmperr);
          _g_match_info_free0 (info);
        }
      else if (matches == FALSE)
        {
          g_set_error (error, WEB_REQUEST_PARSE_ERROR, WEB_REQUEST_PARSE_ERROR_MALFORMED_FIELD, "Malformed field");
          _g_match_info_free0 (info);
        }
      else
        {
          if (self->last_field == NULL)
            {
              g_set_error (error, WEB_REQUEST_PARSE_ERROR, WEB_REQUEST_PARSE_ERROR_MALFORMED_FIELD, "Misplaced folded field");
              _g_match_info_free0 (info);
            }
          else
            {
              g_match_info_fetch_pos (info, 1, &value_start, &value_end);
              _g_match_info_free0 (info);

              const gchar* old = web_message_get_field (WEB_MESSAGE (self), self->last_field);
              const gchar* new = & G_STRUCT_MEMBER (gchar, line, value_start);
              const gsize oldsz = strlen (old);
              GString* buffer = g_string_sized_new (value_end - value_start + 2 + oldsz);

              g_string_append_len (buffer, old, oldsz);
              g_string_append_len (buffer, ", ", 2);
              g_string_append_len (buffer, new, value_end - value_start);

              gchar* name = g_strdup (self->last_field);
              gchar* value = g_string_free (buffer, FALSE);

              web_message_insert_field_take (WEB_MESSAGE (self), name, value);
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

      g_set_error (error, WEB_REQUEST_PARSE_ERROR, WEB_REQUEST_PARSE_ERROR_UNKNOWN_METHOD, "Unknown method '%.*s'", (int) size, method);
    }
return (NULL);
}

static void parse_request_line (WebRequest* self, const gchar* line, gsize length, GError** error)
{
  gboolean matches;
  GMatchInfo* info;
  GError* tmperr = NULL;

  WebRequestClass* klass = WEB_REQUEST_GET_CLASS (self);

  if ((matches = g_regex_match_full (klass->full_request_line, line, length, 0, 0, &info, &tmperr)), G_UNLIKELY (tmperr != NULL))
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

      if ((matches = g_regex_match_full (klass->simple_request_line, line, length, 0, 0, &info, &tmperr)), G_UNLIKELY (tmperr != NULL))
        {
          g_propagate_error (error, tmperr);
          _g_match_info_free0 (info);
        }
      else if (matches == TRUE)
        {
          parse_request_line_simple (self, info, line, length, error);
          _g_match_info_free0 (info);
        }
      else
        {
          g_set_error_literal (error, WEB_REQUEST_PARSE_ERROR, WEB_REQUEST_PARSE_ERROR_MALFORMED_REQUEST, "Invalid request line");
          _g_match_info_free0 (info);
        }
    }
}

static void parse_request_line_full (WebRequest* self, GMatchInfo* info, const gchar* line, gsize length, GError** error)
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
    g_set_error_literal (error, WEB_REQUEST_PARSE_ERROR, WEB_REQUEST_PARSE_ERROR_UNSUPPORTED_VERSION, "Unsupported HTTP version");
  else
    web_message_set_http_version (WEB_MESSAGE (self), version);
}

static void parse_request_line_simple (WebRequest* self, GMatchInfo* info, const gchar* line, gsize length, GError** error)
{
  gchar* path = NULL;
  GError* tmperr = NULL;
  GUri* uri = NULL;

  const gchar* method;
  WebRequestClass* klass;
  gint method_start, method_end;
  gint path_start, path_end;

  g_match_info_fetch_pos (info, 1, &method_start, &method_end);
  g_match_info_fetch_pos (info, 2, &path_start, &path_end);

  if ((method = parse_method (line, method_end, method_start, &tmperr)), G_UNLIKELY (tmperr != NULL))
    g_propagate_error (error, tmperr);
  else
    {
      klass = WEB_REQUEST_GET_CLASS (self);
      path = g_utf8_make_valid (& G_STRUCT_MEMBER (gchar, line, path_start), path_end - path_start);
      uri = g_uri_parse_relative (klass->base_uri, path, G_URI_FLAGS_NON_DNS, &tmperr);

      web_message_set_method (WEB_MESSAGE (self), method);

      if ((g_free (path)), G_UNLIKELY (tmperr != NULL))
        g_propagate_error (error, tmperr);
      else
        {
          web_message_set_uri (WEB_MESSAGE (self), uri);
          _g_uri_unref0 (uri);
        }
    }
}

void web_request_parse_line (WebRequest* web_request, const gchar* line, gsize length, GError** error)
{
  g_return_if_fail (WEB_IS_REQUEST (web_request));
  g_return_if_fail (error == NULL || *error == NULL);
  WebRequest* self = (web_request);

  if (length == 0)
    self->complete = TRUE;
  else
    {
      if (self->got_request_line)
        parse_header_line (self, line, length, error);
      else
        {
          GError* tmperr = NULL;

          if ((parse_request_line (self, line, length, &tmperr)), G_UNLIKELY (tmperr != NULL))
            g_propagate_error (error, tmperr);
          else
            {
              self->got_request_line = TRUE;

              if (self->got_simple_request)
                {
                  self->complete = TRUE;
                }
            }
        }
    }
}

void web_request_parse_body (WebRequest* web_request, const gchar* body, gsize length, GError** error)
{
  g_return_if_fail (WEB_IS_REQUEST (web_request));
  g_return_if_fail (error == NULL || *error == NULL);
  g_assert_not_reached ();
}

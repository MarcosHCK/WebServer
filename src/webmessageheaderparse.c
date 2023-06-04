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
#include <glib/gi18n.h>
#include <webmessagefields.h>
#include <webmessageheaderparse.h>

#define _g_match_info_free0(var) ((var == NULL) ? NULL : (var = (g_match_info_free (var), NULL)))

#define _DEFINE_PATTERN(pattern_name, pattern) \
  static const GRegex* _getpattern_##pattern_name (void) G_GNUC_CONST; \
  static const GRegex* _getpattern_##pattern_name (void) \
    { \
      static gsize __value__ = 0; \
      if (g_once_init_enter (&__value__)) \
        { \
          GError* tmperr = NULL; \
          const gchar* string = ((pattern)); \
          const GRegex* regex = g_regex_new (string, G_REGEX_OPTIMIZE | G_REGEX_RAW, 0, &tmperr); \
          g_assert_no_error (tmperr); \
          g_once_init_leave (&__value__, GPOINTER_TO_SIZE (regex)); \
        } \
      G_STATIC_ASSERT (sizeof (__value__) == GLIB_SIZEOF_VOID_P); \
    return GSIZE_TO_POINTER (__value__); \
    }

_DEFINE_PATTERN (split, "([^,]+?)")
_DEFINE_PATTERN (range_split, "([a-z]+)=([0-9,\\-]+)")
_DEFINE_PATTERN (range_split2, "([0-9]*)\\-([0-9]*)")
#undef _DEFINE_PATTERN

#define _PARSE_ERROR WEB_MESSAGE_HEADER_PARSE_ERROR
#define _PARSE_ERROR_FAILED WEB_MESSAGE_HEADER_PARSE_ERROR_FAILED
#define _PARSE_ERROR_INVALID_FIELD WEB_MESSAGE_HEADER_PARSE_ERROR_INVALID_FIELD
#define _PARSE_ERROR_INVALID_RANGE WEB_MESSAGE_HEADER_PARSE_ERROR_INVALID_RANGE
G_DEFINE_QUARK (web-message-header-parse-error-quark, web_message_header_parse_error);

static goffset parsebound (const gchar* value, GError** error)
{
  guint64 val;

  if (strlen (value) == 0)
    return G_GOFFSET_CONSTANT (-1);
  else
    return (g_ascii_string_to_unsigned (value, 10, 0, G_MAXOFFSET, &val, error), val);
}

static void pushranges (WebMessageHeaders* self, const gchar* value, guint length, GError** error)
{
  GError* tmperr = NULL;
  GMatchInfo* info = NULL;
  WebMessageRange* range = NULL;
  gint start_start, start_stop;
  gint stop_start, stop_stop;
  goffset start, stop;
  gchar* str = NULL;

  if ((g_regex_match_full (_getpattern_range_split2 (), value, length, 0, 0, &info, &tmperr)), G_UNLIKELY (tmperr != NULL))
    g_propagate_error (error, tmperr);
  else
    {
      while (g_match_info_matches (info))
        {
          if ((start = parsebound (str = g_match_info_fetch (info, 1), &tmperr), g_free (str)), G_UNLIKELY (tmperr != NULL))
            {
              g_propagate_error (error, tmperr);
              break;
            }

          if ((stop = parsebound (str = g_match_info_fetch (info, 2), &tmperr), g_free (str)), G_UNLIKELY (tmperr != NULL))
            {
              g_propagate_error (error, tmperr);
              break;
            }

          if (start == G_GOFFSET_CONSTANT (-1) && stop == G_GOFFSET_CONSTANT (-1))
            {
              g_set_error (error, _PARSE_ERROR, _PARSE_ERROR_INVALID_RANGE, _("Invalid range %s"), str = g_match_info_fetch (info, 0));
              g_free (str);
              break;
            }

          range = g_slice_new (WebMessageRange);
          range->begin_offset = start;
          range->end_offset = stop;

          g_queue_push_tail (& self->ranges, range);

          if ((g_match_info_next (info, &tmperr)), G_UNLIKELY (tmperr != NULL))
            {
              g_propagate_error (error, tmperr);
              break;
            }
        }
    }

  _g_match_info_free0 (info);
}

static void parseranges (WebMessageHeaders* self, const gchar* value, guint length, GError** error)
{
  GError* tmperr = NULL;
  GMatchInfo* info = NULL;
  gint type_start, type_stop;
  gint list_start, list_stop;

  if ((g_regex_match_full (_getpattern_range_split (), value, length, 0, 0, &info, &tmperr)), G_UNLIKELY (tmperr != NULL))
    g_propagate_error (error, tmperr);
  else
    {
      while (g_match_info_matches (info))
        {
          g_match_info_fetch_pos (info, 1, &type_start, &type_stop);
          g_match_info_fetch_pos (info, 2, &list_start, &list_stop);

          if (strncmp ("bytes", value + type_start, type_stop - type_start))
            {
              const gchar* unit = value + type_start;
              const guint length = type_stop - type_start;

              g_set_error (error, _PARSE_ERROR, _PARSE_ERROR_INVALID_RANGE, _("Invalid range unit '%.*s'"), length, unit);
              break;
            }

          if ((pushranges (self, value + list_start, list_stop - list_start, &tmperr)), G_UNLIKELY (tmperr != NULL))
            {
              g_propagate_error (error, tmperr);
              break;
            }

          if ((g_match_info_next (info, &tmperr)), G_UNLIKELY (tmperr != NULL))
            {
              g_propagate_error (error, tmperr);
              break;
            }
        }
    }

  _g_match_info_free0 (info);
}

static guint append_values (GQueue* list, gchar* value)
{
  GQueue tmp = G_QUEUE_INIT;
  GList* link = NULL;
  guint added = 0;
  gsize length = strlen (value);
  gsize i, l;

  g_queue_push_tail (& tmp, value);

  for (i = 0, l = 0; i < length; ++i)
  if (value [i] == ',')
    {
      value [i] = 0;

      if (i < length && (i - l > 0))
        {
          g_queue_push_tail (& tmp, (l = i, & value [i + 1]));
        }
    }

  while ((link = g_queue_pop_head_link (& tmp)) != NULL)
    {
      if (G_STRUCT_MEMBER (gchar, link->data, 0) == 0)
        g_list_free (link);
      else
        {
          ++added;
          link->data = g_strstrip (link->data);

          g_queue_push_tail_link (list, link);
        }
    }
return (g_queue_clear (& tmp), added);
}

void _web_message_headers_parse_header (WebMessageHeaders* self, gchar* key, gchar* value)
{
  if (!g_strcmp0 (key, WEB_MESSAGE_FIELD_RANGE))
    {
      GError* tmperr = NULL;

      if ((parseranges (self, value, strlen (value), &tmperr)), G_UNLIKELY (tmperr != NULL))
        {
          g_queue_clear_full (& self->ranges, (GDestroyNotify) _web_message_range_free);
          g_error_free (tmperr);
        }
    }
  else
    {
      GQueue* list;

      if ((list = g_hash_table_lookup (self->fields, key)) != NULL)
        {
          if (append_values (list, value) == 0)
            g_free (value);
          else
            {
              g_queue_push_tail (& self->taken, value);
            }
          g_free (key);
        }
      else
        {
          list = g_queue_new ();

          if (append_values (list, value) == 0)
            {
              g_queue_free (list);
              g_free (value);
            }
          else
            {
              g_queue_push_tail (& self->taken, value);
              g_hash_table_insert (self->fields, key, list);
            }
        }
    }
}

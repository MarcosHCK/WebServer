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
#include <appprivate.h>

G_GNUC_INTERNAL GResource* appresource_get_resource (void) G_GNUC_CONST;
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_string_unref0(var) ((var == NULL) ? NULL : (var = (g_string_free (var, TRUE), NULL)))

typedef struct _ReplaceData ReplaceData;
static const gchar guard_left [] = "<!--{";
static const gchar guard_right [] = "}-->";

struct _ReplaceData
{
  GFile* file;
  GFileInfo* info;
  GFile* root;
  gpointer plus;
};

#define _DEFINE_PAGE_SECTION(name) \
  static const gchar* page##name (void) G_GNUC_CONST; \
  static const gchar* page##name (void) \
  { \
    static gsize __value__ = 0; \
    static const gchar* path = "/org/hck/webserver/" G_STRINGIFY (name) ".html"; \
 ; \
    if (g_once_init_enter (&__value__)) \
      { \
        GBytes* bytes = NULL; \
        GResource* resource = NULL; \
        GError* tmperr = NULL; \
 ; \
        resource = appresource_get_resource (); \
 ; \
        if ((bytes = g_resource_lookup_data (resource, path, 0, &tmperr)), G_UNLIKELY (tmperr != NULL)) \
          { \
            const gchar* domain = g_quark_to_string (tmperr->domain); \
            const gchar* message = tmperr->message; \
            const guint code = tmperr->code; \
 ; \
            g_error ("(" G_STRLOC "): %s: %d: %s", domain, code, message); \
          } \
        g_once_init_leave (&__value__, GPOINTER_TO_SIZE (bytes)); \
      } \
    G_STATIC_ASSERT (sizeof (__value__) == GLIB_SIZEOF_VOID_P); \
  return g_bytes_get_data (GSIZE_TO_POINTER (__value__), NULL); \
  }

  static const GRegex* getpattern (void) G_GNUC_CONST;
  static const GRegex* getpattern (void)
  {
    static gsize __value__ = 0;
    if (g_once_init_enter (&__value__))
      {
        GError* tmperr = NULL;
        gchar* guard_left_ = g_regex_escape_string (guard_left, G_N_ELEMENTS (guard_left) - 1);
        gchar* guard_right_ = g_regex_escape_string (guard_right, G_N_ELEMENTS (guard_right) - 1);
        gchar* pattern = g_strjoin (NULL, guard_left_, "([a-z\\-:]+)", guard_right_, NULL);
        GRegex* regex = g_regex_new (pattern, G_REGEX_OPTIMIZE, 0, &tmperr);

        g_assert_no_error (tmperr);
        g_free (guard_left_);
        g_free (guard_right_);
        g_free (pattern);

        g_once_init_leave (&__value__, GPOINTER_TO_SIZE (regex));
      }
    G_STATIC_ASSERT (sizeof (__value__) == GLIB_SIZEOF_VOID_P);
  return GSIZE_TO_POINTER (__value__);
  }

_DEFINE_PAGE_SECTION (body)
_DEFINE_PAGE_SECTION (item)
#undef _DEFINE_PAGE_SECTION

static void _forbidden (WebMessage* message, GFile* target)
{
  web_message_set_status_full (message, WEB_STATUS_CODE_FORBIDDEN, "forbidden");
}

static gboolean replace_info (const GMatchInfo* match_info, GString* buffer, GFileInfo* info)
{
  gchar name [128], value [128];
  gint attr_start, attr_stop;
  GFileAttributeType type;

  g_match_info_fetch_pos (match_info, 1, &attr_start, &attr_stop);

  const gchar* full = g_match_info_get_string (match_info);
  const gchar* attr = G_STRUCT_MEMBER_P (full, attr_start);
  const gint length = attr_stop - attr_start;

  if (length >= G_N_ELEMENTS (name))
    g_error ("(" G_STRLOC "): Too long attribute '%.*s'", length, attr);
  else
    {
      memcpy (name, attr, length);

      name [length] = 0;
      type = g_file_info_get_attribute_type (info, name);
    }

  if (type != G_FILE_ATTRIBUTE_TYPE_INVALID)
    {
      switch (type)
        {
          default:
            g_error ("(" G_STRLOC "): Invalid attribute '%s', type %i", name, type);
            break;

          case G_FILE_ATTRIBUTE_TYPE_STRING:
            g_string_append (buffer, g_file_info_get_attribute_string (info, name));
            break;
          case G_FILE_ATTRIBUTE_TYPE_BYTE_STRING:
            g_string_append (buffer, g_file_info_get_attribute_byte_string (info, name));
            break;

          case G_FILE_ATTRIBUTE_TYPE_BOOLEAN:
            {
              if (g_file_info_get_attribute_boolean (info, name))
                {
                  g_string_append_c (buffer, 't');
                  g_string_append_c (buffer, 'r');
                  g_string_append_c (buffer, 'u');
                  g_string_append_c (buffer, 'e');
                }
              else
                {
                  g_string_append_c (buffer, 'f');
                  g_string_append_c (buffer, 'a');
                  g_string_append_c (buffer, 'l');
                  g_string_append_c (buffer, 's');
                  g_string_append_c (buffer, 'e');
                }
              break;
            }

          case G_FILE_ATTRIBUTE_TYPE_UINT32:
            {
              guint32 value_ = g_file_info_get_attribute_uint32 (info, name);
              gint wrote = g_snprintf (value, G_N_ELEMENTS (value), "%" G_GINT32_MODIFIER "u", value_);

              g_string_append_len (buffer, value, wrote);
              break;
            }

          case G_FILE_ATTRIBUTE_TYPE_INT32:
            {
              gint32 value_ = g_file_info_get_attribute_int32 (info, name);
              gint wrote = g_snprintf (value, G_N_ELEMENTS (value), "%" G_GINT32_MODIFIER "i", value_);

              g_string_append_len (buffer, value, wrote);
              break;
            }

          case G_FILE_ATTRIBUTE_TYPE_UINT64:
            {
              guint64 value_ = g_file_info_get_attribute_uint64 (info, name);
              gint wrote = g_snprintf (value, G_N_ELEMENTS (value), "%" G_GINT64_MODIFIER "u", value_);

              g_string_append_len (buffer, value, wrote);
              break;
            }

          case G_FILE_ATTRIBUTE_TYPE_INT64:
            {
              gint64 value_ = g_file_info_get_attribute_int64 (info, name);
              gint wrote = g_snprintf (value, G_N_ELEMENTS (value), "%" G_GINT64_MODIFIER "i", value_);

              g_string_append_len (buffer, value, wrote);
              break;
            }
        }
    }
return FALSE;
}

static gboolean replace_body (const GMatchInfo* info, GString* buffer, ReplaceData* data)
{
  gint repl_start, repl_stop;
  g_match_info_fetch_pos (info, 1, &repl_start, &repl_stop);

  const gchar* full = g_match_info_get_string (info);
  const gchar* repl = G_STRUCT_MEMBER_P (full, repl_start);
  const gsize length = repl_stop - repl_start;

  if (!strncmp ("body::items", repl, length))
    {
      GSList* list;

      for (list = data->plus; list; list = list->next)
        g_string_append (buffer, list->data);
    }
  else if (!strncmp ("body::path", repl, length))
    {
      gchar* rel;

      if ((rel = g_file_get_relative_path (data->root, data->file)) != NULL)
        {
          g_string_append (buffer, rel);
          g_free (rel);
        }
    }
  else
return replace_info (info, buffer, G_STRUCT_MEMBER (gpointer, data, G_STRUCT_OFFSET (ReplaceData, info)));
return FALSE;
}

static gboolean replace_item (const GMatchInfo* info, GString* buffer, ReplaceData* data)
{
  gint repl_start, repl_stop;
  g_match_info_fetch_pos (info, 1, &repl_start, &repl_stop);

  const gchar* full = g_match_info_get_string (info);
  const gchar* repl = G_STRUCT_MEMBER_P (full, repl_start);
  const gsize length = repl_stop - repl_start;

  if (!strncmp ("item::access", repl, length))
    {
      GDateTime* date = g_file_info_get_access_date_time (data->info);
      gchar* value = g_date_time_format_iso8601 (date);

      g_string_append (buffer, value);
      g_date_time_unref (date);
      g_free (value);
    }
  else if (!strncmp ("item::href", repl, length))
    {
      gchar* rel;

      g_string_append (buffer, (rel = g_file_get_relative_path (data->plus, data->file)));
      g_free (rel);

      if (g_file_info_get_file_type (data->info) == G_FILE_TYPE_DIRECTORY)
        {
          g_string_append_c (buffer, G_DIR_SEPARATOR);
        }
    }
  else if (!strncmp ("item::modified", repl, length))
    {
      GDateTime* date = g_file_info_get_modification_date_time (data->info);
      gchar* value = g_date_time_format_iso8601 (date);

      g_string_append (buffer, value);
      g_date_time_unref (date);
      g_free (value);
    }
  else if (!strncmp ("item::size", repl, length))
    {
      if (g_file_info_get_file_type (data->info) == G_FILE_TYPE_REGULAR)
        {
          gsize size = g_file_info_get_size (data->info);
          gchar* value = g_format_size_full (size, G_FORMAT_SIZE_LONG_FORMAT);

          g_string_append (buffer, value);
          g_free (value);
        }
    }
  else if (!strncmp ("item::type", repl, length))
    {
      GFileType type;

      if ((type = g_file_info_get_attribute_uint32 (data->info, G_FILE_ATTRIBUTE_STANDARD_TYPE)) != G_FILE_TYPE_UNKNOWN)
        {
          GEnumClass* klass = g_type_class_ref (G_TYPE_FILE_TYPE);
          GEnumValue* value = g_enum_get_value (klass, type);

          g_string_append (buffer, value->value_nick);
          g_type_class_unref (klass);
        }
    }
  else if (!strncmp ("item::visibility", repl, length))
    {
      gboolean is_hidden;

      if ((is_hidden = g_file_info_get_attribute_boolean (data->info, G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN)))
        g_string_append (buffer, "hidden-object");
    }
  else
return replace_info (info, buffer, G_STRUCT_MEMBER (gpointer, data, G_STRUCT_OFFSET (ReplaceData, info)));
return FALSE;
}

void _app_server_process (WebMessage* message, GFile* root, GError** error)
{
  static const gchar* attr = G_FILE_ATTRIBUTE_ETAG_VALUE ","
                             G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE ","
                             G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME ","
                             G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN ","
                             G_FILE_ATTRIBUTE_STANDARD_IS_SYMLINK ","
                             G_FILE_ATTRIBUTE_STANDARD_NAME ","
                             G_FILE_ATTRIBUTE_STANDARD_SIZE ","
                             G_FILE_ATTRIBUTE_STANDARD_SYMLINK_TARGET ","
                             G_FILE_ATTRIBUTE_STANDARD_TYPE ","
                             G_FILE_ATTRIBUTE_TIME_ACCESS ","
                             G_FILE_ATTRIBUTE_TIME_MODIFIED;

  GError* tmperr = NULL;
  const gchar* apath = g_uri_get_path (web_message_get_uri (message));
  const gchar* rpath = g_path_skip_root (apath) == NULL ? "." : g_path_skip_root (apath);
  GCancellable* cancellable = g_cancellable_new ();
  GFile* target = g_file_resolve_relative_path (root, rpath);
  GFileInfo* info = NULL;

  if ((info = g_file_query_info (target, attr, 0, cancellable, &tmperr)), G_UNLIKELY (tmperr != NULL))
    {
      _g_object_unref0 (info);

      if (!g_error_matches (tmperr, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED))
        g_propagate_error (error, tmperr);
      else
        {
          _forbidden (message, target);
          g_error_free (tmperr);
        }
    }
  else
    {
      switch (g_file_info_get_file_type (info))
        {
          case G_FILE_TYPE_REGULAR:
            {
              GFileInputStream* stream = NULL;
              WebMessageBody* body = NULL;

              body = web_message_get_response (message);
              stream = g_file_read (target, NULL, &tmperr);

              if (G_UNLIKELY (tmperr != NULL))
                g_propagate_error (error, (_g_object_unref0 (stream), tmperr));
              else
                {
                  web_message_set_status (message, WEB_STATUS_CODE_OK);
                  web_message_body_set_content_length (body, g_file_info_get_size (info));
                  web_message_body_set_content_type (body, g_file_info_get_content_type (info));
                  web_message_body_set_stream (body, G_INPUT_STREAM (stream));
                  _g_object_unref0 (stream);
                }
              break;
            }

          case G_FILE_TYPE_DIRECTORY:
            {
              GFileEnumerator* enumerator;
              GFileInfo* info2;
  
              if ((enumerator = g_file_enumerate_children (target, attr, 0, cancellable, &tmperr)), G_UNLIKELY (tmperr != NULL))
                g_propagate_error (error, (_g_object_unref0 (enumerator), tmperr));
              else
                {
                  GSList* items = NULL;
                  GFile* item = NULL;

                  while (TRUE)
                    {
                      if ((g_file_enumerator_iterate (enumerator, &info2, &item, cancellable, &tmperr)), G_UNLIKELY (tmperr != NULL))
                        {
                          g_propagate_error (error, tmperr);
                          break;
                        }

                      if (info2 == NULL)
                        {
                          GRegexEvalCallback func = (GRegexEvalCallback) replace_body;
                          ReplaceData data = { .file = target, .root = root, .info = info, .plus = (items = g_slist_reverse (items)), };
                          gchar* result = NULL;

                          if ((result = g_regex_replace_eval (getpattern (), pagebody (), -1, 0, 0, func, &data, &tmperr)), G_UNLIKELY (tmperr == NULL))
                            {
                              web_message_set_status (message, WEB_STATUS_CODE_OK);
                              web_message_set_response_take (message, "text/html", result, strlen (result));
                              break;
                            }
                          else
                            {
                              _g_free0 (result);
                              g_propagate_error (error, tmperr);
                              break;
                            }
                          break;
                        }
                      else
                        {
                          GRegexEvalCallback func = (GRegexEvalCallback) replace_item;
                          ReplaceData data = { .file = item, .root = root, .info = info2, .plus = target, };
                          gchar* result = NULL;

                          if ((result = g_regex_replace_eval (getpattern (), pageitem (), -1, 0, 0, func, &data, &tmperr)), G_UNLIKELY (tmperr == NULL))
                            items = g_slist_prepend (items, result);
                          else
                            {
                              _g_free0 (result);
                              g_propagate_error (error, tmperr);
                              break;
                            }
                        }
                    }

                  g_slist_free_full (items, g_free);
                  _g_object_unref0 (enumerator);
                }
              break;
            }

          default:
            {
              _forbidden (message, target);
              break;
            }
        }

      _g_object_unref0 (info);
    }

  _g_object_unref0 (target);
  _g_object_unref0 (cancellable);
}

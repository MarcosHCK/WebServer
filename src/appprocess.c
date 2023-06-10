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
#define g_string_append_static(gstr,static_) g_string_append_len ((gstr), (static_), G_N_ELEMENTS ((static_)) - 1)
#define _g_bytes_unref0(var) ((var == NULL) ? NULL : (var = (g_bytes_unref (var), NULL)))
#define _g_date_time_unref0(var) ((var == NULL) ? NULL : (var = (g_date_time_unref (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_string_unref0(var) ((var == NULL) ? NULL : (var = (g_string_free (var, TRUE), NULL)))
#define RESROOT "/org/hck/webserver"

static void _status_forbidden (WebMessage* message)
{
  web_message_set_status_full (message, WEB_STATUS_CODE_FORBIDDEN, "forbidden");
}

static void _status_not_found (WebMessage* message)
{
  web_message_set_status_full (message, WEB_STATUS_CODE_NOT_FOUND, "not found");
}

void _index (AppServer* self, WebMessage* message, GFile* root, GFile* target, GError** error)
{
  static const gchar* attr = G_FILE_ATTRIBUTE_ETAG_VALUE ","
                             G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE ","
                             G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME ","
                             G_FILE_ATTRIBUTE_STANDARD_ICON ","
                             G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN ","
                             G_FILE_ATTRIBUTE_STANDARD_IS_SYMLINK ","
                             G_FILE_ATTRIBUTE_STANDARD_NAME ","
                             G_FILE_ATTRIBUTE_STANDARD_SIZE ","
                             G_FILE_ATTRIBUTE_STANDARD_SYMLINK_TARGET ","
                             G_FILE_ATTRIBUTE_STANDARD_TYPE ","
                             G_FILE_ATTRIBUTE_TIME_ACCESS ","
                             G_FILE_ATTRIBUTE_TIME_MODIFIED;

  GCancellable* cancellable = g_cancellable_new ();
  GError* tmperr = NULL;
  GFileInfo* info = NULL;

  if ((info = g_file_query_info (target, attr, 0, cancellable, &tmperr)), G_UNLIKELY (tmperr != NULL))
    {
      _g_object_unref0 (info);

      if (!g_error_matches (tmperr, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED))
        g_propagate_error (error, tmperr);
      else
        {
          _status_forbidden (message);
          g_error_free (tmperr);
        }
    }
  else
    {
      switch (g_file_info_get_file_type (info))
        {
          case G_FILE_TYPE_REGULAR:
            {
              gchar* filename = NULL;
              GFileInputStream* stream = NULL;
              GInputStream* stream2 = NULL;
              WebMessageBody* body = NULL;
              WebMessageHeaders* headers = NULL;

              stream = g_file_read (target, NULL, &tmperr);

              if (G_UNLIKELY (tmperr != NULL))
                g_propagate_error (error, (_g_object_unref0 (stream), tmperr));
              else
                {
                  stream2 = G_INPUT_STREAM (stream);
                  stream2 = _app_stream_new (stream2);
                  filename = g_file_get_basename (target);
                  _g_object_unref0 (stream);

                  g_object_get (message, "response-body", &body, NULL);
                  g_object_get (message, "response-headers", &headers, NULL);

                  web_message_set_status (message, WEB_STATUS_CODE_OK);
                  web_message_body_set_stream (body, stream2);
                  web_message_body_unref (body);
                  web_message_headers_set_content_length (headers, g_file_info_get_size (info));
                  web_message_headers_set_content_type (headers, g_file_info_get_content_type (info));
                  web_message_headers_unref (headers);
                  _g_object_unref0 (stream2);
                  _g_free0 (filename);
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
                  GString* buffer = NULL;
                  GEnumValue* enumv = NULL;
                  gsize filesize = 0;
                  GFileType filetype = 0;
                  gchar* href = NULL;
                  GFile* item = NULL;
                  GEnumClass* klass = NULL;
                  GDateTime* lastaccess = NULL;
                  gchar* lastaccess_f = NULL;
                  GDateTime* lastmodify = NULL;
                  gchar* lastmodify_f = NULL;
                  gchar* rel = NULL;
                  gsize size = 0;

                  static const gchar blob1 [] =
                    {
                      "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"
                      "<html xmlns=\"http://www.w3.org/1999/xhtml\">"
                      "<head>"
                      "  <meta charset=\"utf-8\" />"
                      "  <script type=\"application/javascript\" src=\"/resources/index.js\"></script>"
                      "  <link rel=\"stylesheet\" type=\"text/css\" charset=\"utf-8\" media=\"all\" href=\"/resources/index.css\">"
                      "  <link rel=\"icon\" type=\"image/png\" href=\"/icons/computer\">"
                    };

                  static const gchar blob2 [] =
                    {
                      "<p id=\"UI_showHidden\" style=\"display: block;\">"
                      "  <label>"
                      "    <input type=\"checkbox\" checked=\"\" onchange=\"updateHidden()\" />"
                      "      Show hidden objects"
                      "  </label>"
                      "</p>"
                      "<table id=\"UI_fileTable\" class=\"\" order=\"\">"
                      "<thead>"
                      "  <tr>"
                      "    <th> <a href=\"\">Name</a> </th>"
                      "    <th> <a href=\"\">Size</a> </th>"
                      "    <th colspan=\"2\"> <a href=\"\">Last Modified</a> </th>"
                      "  </tr>"
                      "</thead>"
                      "<tbody>"
                      "  <tr>"
                      "    <td colspan=\"5\"> <hr /> </td>"
                      "  </tr>"
                      "</tbody>"
                    };

                  buffer = g_string_sized_new (1024);
                  klass = g_type_class_ref (G_TYPE_FILE_TYPE);
                  rel = g_file_get_relative_path (root, target);

                  g_string_append_static (buffer, blob1);
                  g_string_append_printf (buffer, "<title>Index if /%s</title>", rel == NULL ? "" : rel);
                  g_string_append_static (buffer, "</head>" "<body dir=\"ltr\">");
                  g_string_append_printf (buffer, "<h1>Index if /%s</h1>", rel == NULL ? "" : rel);
                  g_string_append_static (buffer, blob2);
                  _g_free0 (rel);

                  if (!g_file_equal (root, target))
                    {
                      filetype = G_FILE_TYPE_DIRECTORY;

                      g_string_append_static (buffer, "<tbody> <tr> <td colspan=\"5\">");
                      g_string_append_printf (buffer, "<a class=\"%s\" href=\"../\">", g_enum_get_value (klass, filetype)->value_nick);
                      g_string_append_printf (buffer, "<img src=\"/icons/go-up\"/ alt=\"[%s]\">", g_enum_get_value (klass, filetype)->value_nick);
                      g_string_append_static (buffer, "..");
                      g_string_append_static (buffer, "</a></td>");
                      g_string_append_static (buffer, "</tr> </tbody>");
                    }

                  g_string_append_static (buffer, "<tbody id=\"UI_fileList\">");

                  while (TRUE)
                    {
                      if ((g_file_enumerator_iterate (enumerator, &info2, &item, cancellable, &tmperr)), G_UNLIKELY (tmperr != NULL))
                        {
                          g_propagate_error (error, tmperr);
                          break;
                        }

                      if (info2 == NULL)
                        {
                          g_string_append_static (buffer, "</tbody> </table>");
                          g_string_append_static (buffer, "</body> </html>");
                          size = buffer->len;
                          web_message_set_status (message, WEB_STATUS_CODE_OK);
                          web_message_set_response_take (message, "text/html", g_string_free (g_steal_pointer (&buffer), FALSE), size);
                          break;
                        }
                      else
                        {
                          filesize = g_file_info_get_size (info2);
                          filetype = g_file_info_get_file_type (info2);
                          href = g_uri_escape_string (rel = g_file_get_relative_path (target, item), NULL, TRUE);
                          lastaccess = g_file_info_get_access_date_time (info2);
                          lastmodify = g_file_info_get_modification_date_time (info2);

                          _g_free0 (rel);
                          g_string_append_printf (buffer, "<tr%s>\r\n", g_file_info_get_is_hidden (info2) == FALSE ? "" : " class=\"hidden-object\"");
                          g_string_append_printf (buffer, "<td sortable-data=\"%s\">", g_file_info_get_display_name (info2));
                          g_string_append_printf (buffer, "<a class=\"%s\" href=\"%s%s\">", g_enum_get_value (klass, filetype)->value_nick, href, filetype != G_FILE_TYPE_DIRECTORY ? "" : "/");
                          _g_free0 (href);
                          g_string_append_printf (buffer, "<img src=\"/icons/%s\"/ alt=\"[%s]\">", g_file_info_get_content_type (info2), g_enum_get_value (klass, filetype)->value_nick);
                          g_string_append (buffer, rel = g_markup_escape_text (g_file_info_get_display_name (info2), -1));
                          _g_free0 (rel);
                          g_string_append_static (buffer, "</a></td>");
                          g_string_append_printf (buffer, "<td sortable-data=\"%" G_GINT64_MODIFIER "u\">%s</td>", filesize, filetype == G_FILE_TYPE_DIRECTORY ? "" : g_format_size_full (filesize, G_FORMAT_SIZE_LONG_FORMAT));
                          g_string_append_printf (buffer, "<td sortable-data=\"%" G_GINT64_MODIFIER "u\">%s</td>", g_date_time_to_unix (lastaccess), lastaccess_f = g_date_time_format (lastaccess, "%T %F"));
                          _g_free0 (lastaccess_f);
                          g_string_append_printf (buffer, "<td sortable-data=\"%" G_GINT64_MODIFIER "u\">%s</td>", g_date_time_to_unix (lastmodify), lastmodify_f = g_date_time_format (lastmodify, "%T %F"));
                          _g_free0 (lastmodify_f);
                          g_string_append_static (buffer, "</tr>");
                          _g_date_time_unref0 (lastaccess);
                          _g_date_time_unref0 (lastmodify);
                        }
                    }

                  g_type_class_unref (klass);
                  _g_string_unref0 (buffer);
                  _g_object_unref0 (enumerator);
                }
              break;
            }

          default: _status_forbidden (message);
              break;
        }

      _g_object_unref0 (info);
    }

  _g_object_unref0 (cancellable);
}

static gboolean _icon_source (gpointer data)
{
  AppServer* self = ((gpointer*) data) [0];
  WebMessage* message = ((gpointer*) data) [1];
  gchar* type = ((gpointer*) data) [2];
  guint* geometry = ((gpointer*) data) [3];
  GError** error = ((gpointer*) data) [4];
  gint* done = ((gpointer*) data) [5];
  gchar** buffer = ((gpointer*) data) [6];
  gsize* length = ((gpointer*) data) [7];

  GError* tmperr = NULL;
  GIcon* icon = g_content_type_get_icon (type);
  GtkIconTheme* theme = gtk_icon_theme_get_default ();
  GtkIconInfo* info = gtk_icon_theme_lookup_by_gicon (theme, icon, *geometry, 0);
  g_object_unref (icon);
  GdkPixbuf* pixbuf = gtk_icon_info_load_icon (info, &tmperr);
  g_object_unref (info);

  if (G_UNLIKELY (tmperr != NULL))
    {
      _g_object_unref0 (pixbuf);
      g_propagate_error (error, tmperr);
    }
  else
    {
      gdk_pixbuf_save_to_buffer (pixbuf, buffer, length, "png", &tmperr, NULL);
      g_object_unref (pixbuf);

      if (G_UNLIKELY (tmperr != NULL))
        {
          _g_free0 (buffer);
          g_propagate_error (error, tmperr);
        }
    }
return (g_atomic_int_set (done, TRUE), G_SOURCE_REMOVE);
}

void _app_process (AppServer* self, WebMessage* message, GFile* root, GError** error)
{
  GError* tmperr = NULL;
  GHashTable* params = NULL;
  GUri* uri = web_message_get_uri (message);
  const gchar* path = g_uri_get_path (uri);
  const gchar* query = g_uri_get_query (uri);

  path = (path == NULL) ? "/" : path;
  query = (query == NULL) ? "" : query;

  if ((params = g_uri_parse_params (query, -1, "&", G_URI_PARAMS_WWW_FORM, &tmperr)), G_UNLIKELY (tmperr != NULL))
    g_propagate_error (error, tmperr);
  else
    {
  #define prefixed(a,static_) (strncmp ((a), (static_), sizeof ((static_)) - 1) == 0)
      if (!g_strcmp0 (path, "/"))
        {
          web_message_set_redirect (message, WEB_STATUS_CODE_SEE_OTHER, "index/");
        }
      else if (prefixed (path, "/index/"))
        {
          GFile* target = NULL;
          const gchar* rpath = NULL;
          const gchar* order = NULL;

          rpath = path + (sizeof ("/index/") - 1);
          order = g_hash_table_contains (params, "order") == FALSE ? "n" : g_hash_table_lookup (params, "order");
          target = rpath [0] == 0 ? g_object_ref (root) : g_file_resolve_relative_path (root, rpath);

          _index (self, message, root, target, &tmperr);
          g_object_unref (target);

          if (G_UNLIKELY (tmperr != NULL))
            {
              if (tmperr->domain != G_IO_ERROR)
                g_propagate_error (error, tmperr);
              else
                {
                  switch (tmperr->code)
                    {
                      default:
                        g_propagate_error (error, tmperr);
                        break;

                      handled:
                        g_error_free (tmperr);
                        break;

                      case G_IO_ERROR_NOT_FOUND:
                        _status_not_found (message);
                        goto handled;

                      case G_IO_ERROR_PERMISSION_DENIED:
                        _status_forbidden (message);
                        goto handled;
                    }
                }
            }
        }
      else if (prefixed (path, "/icons/"))
        {
          const gchar* type = NULL;
          const gchar* size = NULL;
          guint64 size_ = 0;
          guint geometry = 0;

          if (*(type = path + (sizeof ("/icons/") - 1)) == 0)
            _status_forbidden (message);
          else
            {
              size = g_hash_table_contains (params, "size") == FALSE ? "16" : g_hash_table_lookup (params, "size");
              geometry = (g_ascii_string_to_unsigned (size, 10, 0, G_MAXINT, &size_, &tmperr), (guint) size_);

              if (G_UNLIKELY (tmperr != NULL))
                g_propagate_error (error, tmperr);
              else
                {
                  GError* tmperr = NULL;
                  gchar* buffer = NULL;
                  gsize length = 0;
                  gint done = 0;

                  gpointer data [] =
                    {
                      (gpointer) self,
                      (gpointer) message,
                      (gpointer) type,
                      (gpointer) & geometry,
                      (gpointer) & tmperr,
                      (gpointer) & done,
                      (gpointer) & buffer,
                      (gpointer) & length,
                    };

                  g_main_context_invoke (NULL, _icon_source, data);

                  while (g_atomic_int_get (&done) == FALSE)
                    g_thread_yield ();
                  if (G_UNLIKELY (tmperr != NULL))
                    {
                      _g_free0 (buffer);
                      g_propagate_error (error, tmperr);
                    }
                  else
                    {
                      web_message_set_status (message, WEB_STATUS_CODE_OK);
                      web_message_set_response_take (message, "image/png", buffer, length);
                    }
                }
            }
        }
      else if (prefixed (path, "/resources/"))
        {
          GBytes* bytes = NULL;
          GFile* target = NULL;
          gchar* apath = NULL;
          gchar* name = NULL;
          gchar* type = NULL;
          gsize length = 0;
          const guchar* rdata = NULL;
          const gchar* rpath = NULL;

          if (*(rpath = path + (sizeof ("/resources/") - 1)) == 0)
            _status_forbidden (message);
          else
            {
              apath = g_strconcat (RESROOT, "/", rpath, NULL);
              bytes = g_resources_lookup_data (apath, G_RESOURCE_LOOKUP_FLAGS_NONE, &tmperr);
                      _g_free0 (apath);

              if (G_UNLIKELY (tmperr != NULL))
                g_propagate_error (error, tmperr);
              else
                {
                  GInputStream* stream = NULL;
                  WebMessageBody* body = NULL;
                  WebMessageHeaders* headers = NULL;

                  rdata = g_bytes_get_data (bytes, &length);
                  name = g_path_get_basename (rpath);
                  type = g_content_type_guess (name, rdata, length, NULL);
                  stream = g_memory_input_stream_new_from_bytes (bytes);

                  g_object_get (message, "response-body", &body, NULL);
                  g_object_get (message, "response-headers", &headers, NULL);
                  g_bytes_unref (bytes);
                  g_free (name);

                  web_message_set_status (message, WEB_STATUS_CODE_OK);
                  web_message_body_set_stream (body, stream);
                  web_message_body_unref (body);
                  g_object_unref (stream);

                  web_message_headers_set_content_length (headers, length);
                  web_message_headers_set_content_type (headers, type);
                  web_message_headers_unref (headers);
                  g_free (type);
                }
            }
        }
      else
        {
          #undef prefixed
          _status_forbidden (message);
        }

      g_hash_table_remove_all (params);
      g_hash_table_unref (params);
    }
}

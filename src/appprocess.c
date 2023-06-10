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
#include <glib/gi18n.h>

G_GNUC_INTERNAL GResource* appresource_get_resource (void) G_GNUC_CONST;
#define g_string_append_static(gstr,static_) g_string_append_len ((gstr), (static_), G_N_ELEMENTS ((static_)) - 1)
#define _g_bytes_unref0(var) ((var == NULL) ? NULL : (var = (g_bytes_unref (var), NULL)))
#define _g_date_time_unref0(var) ((var == NULL) ? NULL : (var = (g_date_time_unref (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_string_unref0(var) ((var == NULL) ? NULL : (var = (g_string_free (var, TRUE), NULL)))
#define RESROOT "/org/hck/webserver"
static gboolean _hierarchy (GFile* target, GFile* root) G_GNUC_PURE;
static gboolean _hierarchy_inner (GFile* target, GFile* root) G_GNUC_PURE;
static gboolean _icon_source (gpointer data);
static void _root (AppServer* self, WebMessage* message, GFile* root, GError** error);
static void _status (WebMessage* message, WebStatusCode status_code, const gchar* description);

void _app_process (AppServer* self, WebMessage* message, GFile* root)
{
  GError* tmperr = NULL;

  static const gchar s403_description [] = "Your request was understood but you have not permission to access the target resource.";
  static const gchar s404_description [] = "We did not found the target resource.";
  static const gchar s500_description [] = "We encountered an unexpected condition that prevented us from fulfilling the request.";

  if ((_root (self, message, root, &tmperr)), G_UNLIKELY (tmperr != NULL))
    {
      if (tmperr->domain != G_IO_ERROR)
        _status (message, WEB_STATUS_CODE_INTERNAL_SERVER_ERROR, s500_description);
      else
        {
          switch (tmperr->code)
            {
              default:
                _status (message, WEB_STATUS_CODE_INTERNAL_SERVER_ERROR, s500_description);
                break;
              case G_IO_ERROR_NOT_FOUND:
                _status (message, WEB_STATUS_CODE_NOT_FOUND, s404_description);
                break;
              case G_IO_ERROR_INVALID_ARGUMENT:
                _status (message, WEB_STATUS_CODE_INTERNAL_SERVER_ERROR, s500_description);
                break;
              case G_IO_ERROR_PERMISSION_DENIED:
                _status (message, WEB_STATUS_CODE_FORBIDDEN, s403_description);
                break;
            }
        }

      const guint code = tmperr->code;
      const gchar* domain = g_quark_to_string (tmperr->domain);
      const gchar* message = tmperr->message;

      g_warning ("(" G_STRLOC "): %s: %u: %s", domain, code, message);
      g_error_free (tmperr);
    }
}

static gboolean _hierarchy (GFile* target, GFile* root)
{
  return _hierarchy_inner (g_object_ref (target), root);
}

static gboolean _hierarchy_inner (GFile* target, GFile* root)
{
  if (g_file_equal (target, root))
    return TRUE;
  else
    {
      GFile* parent = NULL;
      gboolean good = FALSE;

      if ((parent = g_file_get_parent (target)) == NULL)
        return FALSE;
      else
        {
          g_object_unref (target);
          return _hierarchy (parent, root);
        }
    }
}

static gboolean _icon_source (gpointer data)
{
  typedef char linecnt [__LINE__ + 1];
  AppServer* self = ((gpointer*) data) [__LINE__ - sizeof (linecnt)];
  WebMessage* message = ((gpointer*) data) [__LINE__ - sizeof (linecnt)];
  GIcon* icon = ((gpointer*) data) [__LINE__ - sizeof (linecnt)];
  gboolean* link = ((gpointer*) data) [__LINE__ - sizeof (linecnt)];
  guint* geometry = ((gpointer*) data) [__LINE__ - sizeof (linecnt)];
  GError** error = ((gpointer*) data) [__LINE__ - sizeof (linecnt)];
  gint* done = ((gpointer*) data) [__LINE__ - sizeof (linecnt)];
  gchar** buffer = ((gpointer*) data) [__LINE__ - sizeof (linecnt)];
  gsize* length = ((gpointer*) data) [__LINE__ - sizeof (linecnt)];
  GtkIconTheme* theme = NULL;
  GtkIconInfo* info = NULL;
  GdkPixbuf* pixbuf = NULL;
  GError* tmperr = NULL;

  theme = gtk_icon_theme_get_default ();

  if ((info = gtk_icon_theme_lookup_by_gicon (theme, icon, (*geometry), 0)) == NULL)
    {
      _g_object_unref0 (info);
      g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND, _("Not found"));
    }
  else
    {
      if ((pixbuf = gtk_icon_info_load_icon (info, &tmperr), g_object_unref (info)), G_UNLIKELY (tmperr != NULL))
        {
          _g_object_unref0 (pixbuf);
          g_propagate_error (error, tmperr);
        }
      else
        {
          if ((gdk_pixbuf_save_to_buffer (pixbuf, buffer, length, "png", &tmperr, NULL), g_object_unref (pixbuf)), G_UNLIKELY (tmperr != NULL))
            {
              _g_free0 ((*buffer));
              g_propagate_error (error, tmperr);
            }
        }
    }
return (g_atomic_int_set (done, TRUE), G_SOURCE_REMOVE);
}

static void _root (AppServer* self, WebMessage* message, GFile* root, GError** error)
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
      else if (prefixed (path, "/icon/"))
        {
          const gchar* type = NULL;
          const gchar* size = NULL;
          gboolean link = FALSE;
          guint64 size_ = 0;
          guint geometry = 0;

          if (*(type = path + (sizeof ("/icon/") - 1)) == 0)
            g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED, _("Permission denied"));
          else
            {
              size = g_hash_table_contains (params, "size") == FALSE ? "16" : g_hash_table_lookup (params, "size");
              link = g_hash_table_contains (params, "link") == FALSE ? link : g_str_equal (g_hash_table_lookup (params, "link"), "true");

              if ((g_ascii_string_to_unsigned (size, 10, 0, G_MAXINT, &size_, &tmperr)), G_UNLIKELY (tmperr != NULL))
                {
                  g_error_free (tmperr);
                  g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT, _("Invalid query argument 'size'"));
                }
              else
                {
                  GError* tmperr = NULL;
                  gchar* buffer = NULL;
                  gint done = 0;
                  GIcon* icon = NULL;
                  gsize length = 0;
                  gchar* serialized = NULL;

                  serialized = (gchar*) g_base64_decode (type, &length);
                  icon = (GIcon*) g_icon_new_for_string (serialized, &tmperr);

                  if ((g_free (serialized)), G_UNLIKELY (tmperr != NULL))
                    {
                      _g_object_unref0 (icon);
                      g_propagate_error (error, tmperr);
                    }
                  else
                    {
                      gpointer data [] =
                      {
                        (gpointer) self,
                        (gpointer) message,
                        (gpointer) icon,
                        (gpointer) & link,
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

                      _g_object_unref0 (icon);
                    }
                }
            }
        }
      else if (prefixed (path, "/index/"))
        {
          GFile* target = NULL;
          const gchar* rpath = NULL;
          const gchar* order = NULL;

          rpath = path + (sizeof ("/index/") - 1);
          order = g_hash_table_contains (params, "order") == FALSE ? "n" : g_hash_table_lookup (params, "order");
          target = rpath [0] == 0 ? g_object_ref (root) : g_file_resolve_relative_path (root, rpath);

          if (_hierarchy (target, root) == FALSE)
            g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED, _("Permission denied"));
          else
            {
              static const gchar* attrs = G_FILE_ATTRIBUTE_ETAG_VALUE ","
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

              if ((info = g_file_query_info (target, attrs, 0, cancellable, &tmperr)), G_UNLIKELY (tmperr != NULL))
                {
                  _g_object_unref0 (info);
                  g_propagate_error (error, tmperr);
                }
              else
                {
                  switch (g_file_info_get_file_type (info))
                    {
                      default:
                        g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED, _("Permission denied"));
                        break;

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
              
                          if ((enumerator = g_file_enumerate_children (target, attrs, 0, cancellable, &tmperr)), G_UNLIKELY (tmperr != NULL))
                            g_propagate_error (error, (_g_object_unref0 (enumerator), tmperr));
                          else
                            {
                              GString* buffer = NULL;
                              GEnumValue* enumv = NULL;
                              gsize filesize = 0;
                              GFileType filetype = 0;
                              gchar* href = NULL;
                              GIcon* icon = NULL;
                              gchar* icondata = NULL;
                              gchar* iconname = NULL;
                              gboolean islink = FALSE;
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
                                  "<p id=\"UI_showHidden\" style=\"display: none;\">"
                                  "  <label>"
                                  "    <input type=\"checkbox\" checked=\"\" onchange=\"updateHidden()\" />"
                                  "      Show hidden objects"
                                  "  </label>"
                                  "</p>"
                                  "<table id=\"UI_fileTable\">"
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

                                  static const gchar upicon [] = "Z28tdXA=" /* echo -ne "go-up" | base64 */;

                                  g_string_append_static (buffer, "<tr> <td colspan=\"5\">");
                                  g_string_append_printf (buffer, "<a class=\"%s\" href=\"../\">", g_enum_get_value (klass, filetype)->value_nick);
                                  g_string_append_printf (buffer, "<img src=\"/icon/%s\"/ alt=\"[%s]\">", upicon, g_enum_get_value (klass, filetype)->value_nick);
                                  g_string_append_static (buffer, "..");
                                  g_string_append_static (buffer, "</a></td>");
                                  g_string_append_static (buffer, "</tr>");
                                }

                              g_string_append_static (buffer, "</tbody>");
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
                                      icon = g_file_info_get_icon (info2);
                                      iconname = g_icon_to_string (icon);
                                      icondata = g_base64_encode ((const guchar*) iconname, strlen (iconname) + 1);
                                      islink = g_file_info_get_is_symlink (info2);
                                      lastaccess = g_file_info_get_access_date_time (info2);
                                      lastmodify = g_file_info_get_modification_date_time (info2);

                                      _g_free0 (rel);
                                      g_string_append_printf (buffer, "<tr%s>\r\n", g_file_info_get_is_hidden (info2) == FALSE ? "" : " class=\"hidden-object\"");
                                      g_string_append_printf (buffer, "<td sortable-data=\"%s\">", g_file_info_get_display_name (info2));
                                      g_string_append_printf (buffer, "<a class=\"%s\" href=\"%s%s\">", g_enum_get_value (klass, filetype)->value_nick, href, filetype != G_FILE_TYPE_DIRECTORY ? "" : "/");
                                      _g_free0 (href);
                                      g_string_append_printf (buffer, "<img src=\"/icon/%s?size=16%s\"/ alt=\"[%s]\">", icondata, islink == FALSE ? "" : "&link=true", g_enum_get_value (klass, filetype)->value_nick);
                                      g_string_append (buffer, rel = g_markup_escape_text (g_file_info_get_display_name (info2), -1));
                                      _g_free0 (rel);
                                      g_string_append_static (buffer, "</a></td>");
                                      g_string_append_printf (buffer, "<td sortable-data=\"%" G_GINT64_MODIFIER "u\">%s</td>", filesize, filetype == G_FILE_TYPE_DIRECTORY ? "" : g_format_size_full (filesize, G_FORMAT_SIZE_LONG_FORMAT));
                                      g_string_append_printf (buffer, "<td sortable-data=\"%" G_GINT64_MODIFIER "u\">%s</td>", g_date_time_to_unix (lastaccess), lastaccess_f = g_date_time_format (lastaccess, "%T %F"));
                                      _g_free0 (lastaccess_f);
                                      g_string_append_printf (buffer, "<td sortable-data=\"%" G_GINT64_MODIFIER "u\">%s</td>", g_date_time_to_unix (lastmodify), lastmodify_f = g_date_time_format (lastmodify, "%T %F"));
                                      _g_free0 (lastmodify_f);
                                      g_string_append_static (buffer, "</tr>");
                                      _g_free0 (icondata);
                                      _g_free0 (iconname);
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
                    }

                  _g_object_unref0 (info);
                }

              _g_object_unref0 (cancellable);
            }

          _g_object_unref0 (target);
        }
      else if (prefixed (path, "/resources/"))
        {
          gchar* apath = NULL;
          GBytes* bytes = NULL;
          gsize length = 0;
          gchar* name = NULL;
          const guchar* rdata = NULL;
          const gchar* rpath = NULL;
          GFile* target = NULL;
          gchar* type = NULL;

          if (*(rpath = path + (sizeof ("/resources/") - 1)) == 0)
            g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED, _("Permission denied"));
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
          g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED, _("Permission denied"));
        }

      g_hash_table_remove_all (params);
      g_hash_table_unref (params);
    }
}

static void _status (WebMessage* message, WebStatusCode status_code, const gchar* description)
{
  gchar* inline_ = NULL;
  gchar* response = NULL;

  static const gchar fmt [] =
    {
      "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
      "<html><head>\r\n"
      "<title>%i %s</title>\r\n"
      "</head><body>\r\n"
      "<h1>%s</h1>\r\n"
      "<p>%s<br />\r\n"
      "</p>\r\n"
      "<hr>\r\n"
      "<address>" PACKAGE_NAME "/" PACKAGE_VERSION "</address>\r\n"
      "</body></html>\r\n"
    };

  inline_ = (gchar*) web_status_code_get_inline (status_code);
  response = (gchar*) g_strdup_printf (fmt, (int) status_code, inline_, inline_, description);

  web_message_set_status (message, status_code);
  web_message_set_response_take (message, "text/html", response, strlen (response));
}

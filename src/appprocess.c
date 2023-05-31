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
#include <appprocess.h>

G_GNUC_INTERNAL GResource* resources_get_resource (void);
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

#define _DEFINE_PAGE_SECTION(name) \
  static const gchar* page##name () \
  { \
    static gsize __value__ = 0; \
    static const gchar* path = "/org/hck/webserver/page" G_STRINGIFY (name) ".fmt"; \
 ; \
    if (g_once_init_enter (&__value__)) \
      { \
        GBytes* bytes = NULL; \
        GResource* resource = NULL; \
        GError* tmperr = NULL; \
 ; \
        resource = resources_get_resource (); \
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

_DEFINE_PAGE_SECTION (footer)
_DEFINE_PAGE_SECTION (header)
_DEFINE_PAGE_SECTION (item)
#undef _DEFINE_PAGE_SECTION

void _app_server_process (AppRequest* request, AppServer* server)
{
  static const gchar* attr = G_FILE_ATTRIBUTE_ETAG_VALUE ","
                             G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE ","
                             G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME ","
                             G_FILE_ATTRIBUTE_STANDARD_IS_SYMLINK ","
                             G_FILE_ATTRIBUTE_STANDARD_SIZE ","
                             G_FILE_ATTRIBUTE_STANDARD_SYMLINK_TARGET ","
                             G_FILE_ATTRIBUTE_STANDARD_TYPE;

  GError* tmperr = NULL;
  const gchar* apath = g_uri_get_path (web_message_get_uri (request->web_message));
  const gchar* rpath = g_path_skip_root (apath) == NULL ? "." : g_path_skip_root (apath);
  GCancellable* cancellable = g_cancellable_new ();
  GFile* target = g_file_resolve_relative_path (request->root, rpath);
  GFileInfo* info = NULL;

  if ((info = g_file_query_info (target, attr, 0, cancellable, &tmperr)), G_UNLIKELY (tmperr != NULL))
    {
      _g_object_unref0 (info);
      web_message_set_status (request->web_message, WEB_STATUS_CODE_INTERNAL_SERVER_ERROR);
      web_message_set_is_closure (request->web_message, TRUE);
      web_message_thaw (request->web_message);
    }
  else
    {
      switch (g_file_info_get_file_type (info))
        {
          case G_FILE_TYPE_REGULAR:
            {
              GFileInputStream* stream = NULL;
              WebMessageBody* body = NULL;

              body = web_message_get_response (request->web_message);
              stream = g_file_read (target, NULL, &tmperr);

              if (G_UNLIKELY (tmperr != NULL))
                {
                  web_message_set_status (request->web_message, WEB_STATUS_CODE_INTERNAL_SERVER_ERROR);
                  web_message_set_is_closure (request->web_message, TRUE);
                  _g_object_unref0 (stream);
                }
              else
                {
                  web_message_set_status (request->web_message, WEB_STATUS_CODE_OK);
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
                {
                  web_message_set_status (request->web_message, WEB_STATUS_CODE_INTERNAL_SERVER_ERROR);
                  web_message_set_is_closure (request->web_message, TRUE);
                  _g_object_unref0 (enumerator);
                }
              else
                {
                  GString* buffer = NULL;
                  gchar* content = NULL;
                  gsize length = 0;

                  buffer = g_string_sized_new (256);

                  g_string_append (buffer, pageheader ());

                  while (TRUE)
                    {
                      if ((g_file_enumerator_iterate (enumerator, &info2, NULL, cancellable, &tmperr)), G_UNLIKELY (tmperr != NULL))
                        {
                          g_string_free (buffer, TRUE);
                          web_message_set_status (request->web_message, WEB_STATUS_CODE_INTERNAL_SERVER_ERROR);
                          web_message_set_is_closure (request->web_message, TRUE);
                          break;
                        }

                      if (info2 == NULL)
                        {
                          g_string_append (buffer, pagefooter ());

                          length = buffer->len;
                          content = g_string_free (buffer, FALSE);

                          web_message_set_status (request->web_message, WEB_STATUS_CODE_OK);
                          web_message_set_response_take (request->web_message, "text/html", content, length);
                          break;
                        }
                      else
                        {
                          
                        }
                    }

                  _g_object_unref0 (enumerator);
                }
              break;
            }

          default:
            {
              web_message_set_status (request->web_message, WEB_STATUS_CODE_FORBIDDEN);
              break;
            }
        }

      web_message_thaw (request->web_message);
      _g_object_unref0 (info);
    }

  _g_object_unref0 (cancellable);
}

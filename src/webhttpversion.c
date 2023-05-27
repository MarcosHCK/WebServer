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

G_DEFINE_ENUM_TYPE
  (
    WebHttpVersion, web_http_version,
    G_DEFINE_ENUM_VALUE (WEB_HTTP_VERSION_NONE, "none"),
    G_DEFINE_ENUM_VALUE (WEB_HTTP_VERSION_0_9, "0.9"),
    G_DEFINE_ENUM_VALUE (WEB_HTTP_VERSION_1_0, "1.0"),
    G_DEFINE_ENUM_VALUE (WEB_HTTP_VERSION_1_1, "1.1"),
    G_DEFINE_ENUM_VALUE (WEB_HTTP_VERSION_2_0, "2.0")
  );

WebHttpVersion web_http_version_from_bits (guint major, guint minor)
{
  switch (major)
    {
      case 0:
        {
          switch (minor)
            {
              case 9: return WEB_HTTP_VERSION_0_9;
              default: return WEB_HTTP_VERSION_NONE;
            }
        }

      case 1:
        {
          switch (minor)
            {
              case 0: return WEB_HTTP_VERSION_1_0;
              case 1: return WEB_HTTP_VERSION_1_1;
              default: return WEB_HTTP_VERSION_NONE;
            }
        }

      case 2:
        {
          switch (minor)
            {
              case 0: return WEB_HTTP_VERSION_2_0;
              default: return WEB_HTTP_VERSION_NONE;
            }
        }

      default: return WEB_HTTP_VERSION_NONE;
    }
}

const gchar* web_http_version_to_string (WebHttpVersion http_version)
{
  GEnumClass* klass = g_type_class_ref (WEB_TYPE_HTTP_VERSION);
  GEnumValue* value = g_enum_get_value (klass, (gint) http_version);
  const gchar* nick = g_intern_string (value->value_nick);
return (g_type_class_unref (klass), nick);
}

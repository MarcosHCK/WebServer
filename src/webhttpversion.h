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
#ifndef __WEB_HTTP_VERSION__
#define __WEB_HTTP_VERSION__ 1
#include <glib-object.h>

#define WEB_TYPE_HTTP_VERSION (web_http_version_get_type ())

#if __cplusplus
extern "C" {
#endif // __cplusplus

  typedef enum
  {
    WEB_HTTP_VERSION_1_0,
    WEB_HTTP_VERSION_1_1,
    WEB_HTTP_VERSION_2_0,
  } WebHttpVersion;

  G_GNUC_INTERNAL GType web_http_version_get_type (void) G_GNUC_CONST;

#if __cplusplus
}
#endif // __cplusplus

#endif // __WEB_HTTP_VERSION__

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
#ifndef __WEB_LISTEN_OPTIONS__
#define __WEB_LISTEN_OPTIONS__ 1
#include <glib-object.h>

#define WEB_TYPE_LISTEN_OPTIONS (web_listen_options_get_type ())

#if __cplusplus
extern "C" {
#endif // __cplusplus

  typedef enum
  {
    WEB_LISTEN_OPTION_NONE = 0,
    WEB_LISTEN_OPTION_IPV4_ONLY = (1 << 0),
    WEB_LISTEN_OPTION_IPV6_ONLY = (1 << 1),
    WEB_LISTEN_OPTION_HTTPS = (1 << 2),
  } WebListenOptions;

  G_GNUC_INTERNAL GType web_listen_options_get_type (void) G_GNUC_CONST;

#if __cplusplus
}
#endif // __cplusplus

#endif // __WEB_LISTEN_OPTIONS__

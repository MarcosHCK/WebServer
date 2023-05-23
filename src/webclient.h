/* Copyright 2023 MarcosHCK
 * This file is part of WebbServer.
 *
 * WebbServer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * WebbServer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with WebbServer. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __WEB_CLIENT__
#define __WEB_CLIENT__ 1
#include <gio/gio.h>

#define WEB_TYPE_CLIENT (web_client_get_type ())
#define WEB_CLIENT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), WEB_TYPE_CLIENT, WebClient))
#define WEB_IS_CLIENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), WEB_TYPE_CLIENT))
typedef struct _WebClient WebClient;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  G_GNUC_INTERNAL GType web_client_get_type (void) G_GNUC_CONST;
  G_GNUC_INTERNAL WebClient* web_client_new (GIOStream* stream);
  G_GNUC_INTERNAL GIOStream* web_client_get_stream (WebClient* web_client);

#if __cplusplus
}
#endif // __cplusplus

#endif // __WEB_CLIENT__

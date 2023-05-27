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
#ifndef __WEB_ENDPOINT__
#define __WEB_ENDPOINT__ 1
#include <gio/gio.h>

#define WEB_TYPE_ENDPOINT (web_endpoint_get_type ())
#define WEB_ENDPOINT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), WEB_TYPE_ENDPOINT, WebEndpoint))
#define WEB_IS_ENDPOINT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), WEB_TYPE_ENDPOINT))
typedef struct _WebEndpoint WebEndpoint;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  G_GNUC_INTERNAL GType web_endpoint_get_type (void) G_GNUC_CONST;
  G_GNUC_INTERNAL WebEndpoint* web_endpoint_new (GSocket* socket, gboolean is_https, GError** error);
  G_GNUC_INTERNAL gboolean web_endpoint_get_is_https (WebEndpoint* web_endpoint);
  G_GNUC_INTERNAL GSocket* web_endpoint_get_socket (WebEndpoint* web_endpoint);

#if __cplusplus
}
#endif // __cplusplus

#endif // __WEB_ENDPOINT__

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
#ifndef __WEB_SERVER__
#define __WEB_SERVER__ 1
#include <gio/gio.h>
#include <weblistenoptions.h>

#define WEB_TYPE_SERVER (web_server_get_type ())
#define WEB_SERVER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), WEB_TYPE_SERVER, WebServer))
#define WEB_IS_SERVER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), WEB_TYPE_SERVER))
typedef struct _WebServer WebServer;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  G_GNUC_INTERNAL GType web_server_get_type (void) G_GNUC_CONST;
  G_GNUC_INTERNAL WebServer* web_server_new ();
  G_GNUC_INTERNAL void web_server_listen (WebServer* web_server, GSocketAddress* address, WebListenOptions options, GError** error);
  G_GNUC_INTERNAL void web_server_listen_any (WebServer* web_server, guint16 port, WebListenOptions options, GError** error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __WEB_SERVER__

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
#ifndef __WEB_CONNECTION__
#define __WEB_CONNECTION__ 1
#include <gio/gio.h>
#include <webmessage.h>

#define WEB_TYPE_CONNECTION (web_connection_get_type ())
#define WEB_CONNECTION(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), WEB_TYPE_CONNECTION, WebConnection))
#define WEB_IS_CONNECTION(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), WEB_TYPE_CONNECTION))
typedef struct _WebConnection WebConnection;
#define WEB_CONNECTION_ERROR (web_connection_error_quark ())

#if __cplusplus
extern "C" {
#endif // __cplusplus

  typedef enum
  {
    WEB_CONNECTION_ERROR_FAILED,
    WEB_CONNECTION_ERROR_MISMATCH_VERSION,
    WEB_CONNECTION_ERROR_OLD_CLIENT,
    WEB_CONNECTION_ERROR_REQUEST_OVERFLOW,
  } WebConnectionError;

  G_GNUC_INTERNAL GType web_connection_get_type (void) G_GNUC_CONST;
  G_GNUC_INTERNAL GQuark web_connection_error_quark (void) G_GNUC_CONST;
  G_GNUC_INTERNAL WebConnection* web_connection_new (GSocket* socket, gboolean is_https);
  G_GNUC_INTERNAL void web_connection_send (WebConnection* web_connection, WebMessage* web_message);
  G_GNUC_INTERNAL WebMessage* web_connection_step (WebConnection* web_connection, GError** error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __WEB_CONNECTION__

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
#ifndef __APP_PROCESS__
#define __APP_PROCESS__ 1
#include <gtk/gtk.h>
#include <webmessage.h>

typedef struct _AppServer AppServer;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _AppServer
  {
    GApplication parent;

    /* private */
    GHashTable* servers;
    GThreadPool* thread_pool;
  };

  G_GNUC_INTERNAL GInputStream* _app_stream_new (GInputStream* base_stream);
  G_GNUC_INTERNAL void _app_process (AppServer* self, WebMessage* message, GFile* root);

#if __cplusplus
}
#endif // __cplusplus

#endif // __APP_PROCESS__

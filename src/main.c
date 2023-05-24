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
#include <config.h>
#include <gio/gio.h>
#include <webclient.h>
#include <webserver.h>
#include <workqueue.h>

#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

static void on_activate (WebServer* web_server)
{
  GFile* defaults [] =
    {
      g_file_new_for_commandline_arg ("8080"),
      g_file_new_for_commandline_arg ("/var/www"),
    };

  g_application_open (G_APPLICATION (web_server), defaults, G_N_ELEMENTS (defaults), NULL);
  _g_object_unref0 (defaults [1]); _g_object_unref0 (defaults [0]);
}

static gboolean on_incoming (WebServer* web_server, WebClient* web_client, WorkQueue* work_queue)
{
  return (work_queue_push (work_queue, web_client), TRUE);
}

int main (int argc, gchar* argv [])
{
  guint exit_code = 0;
  WebServer* web_server = web_server_new ("application-id", "org.hck.webserver", "flags", G_APPLICATION_HANDLES_OPEN, NULL);
  WorkQueue* work_queue = work_queue_new ();

  g_signal_connect_object (web_server, "activate", G_CALLBACK (on_activate), work_queue, 0);
  g_signal_connect_object (web_server, "incoming", G_CALLBACK (on_incoming), work_queue, 0);
  web_server_start (web_server);

  exit_code = g_application_run (G_APPLICATION (web_server), argc, argv);

  web_server_stop (web_server);
  _g_object_unref0 (web_server);
  _g_object_unref0 (work_queue);
return exit_code;
}

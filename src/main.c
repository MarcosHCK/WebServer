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
#include <gio/gio.h>
#include <webmessage.h>
#include <webserver.h>

#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

static void on_activate (WebServer* web_server, GApplication* application)
{
  GFile* defaults [] =
    {
      g_file_new_for_commandline_arg ("8080"),
      g_file_new_for_commandline_arg ("/var/www"),
    };

  const guint n_defaults = G_N_ELEMENTS (defaults);

  g_application_open (application, defaults, n_defaults, NULL);

  g_object_unref (defaults [1]);
  g_object_unref (defaults [0]);
}

static void on_open (WebServer* web_server, GFile** files, gint n_files)
{
  GFile* current = g_file_new_for_path (".");
  GFile* subst = g_file_new_for_path ("/var/www");
  GError* tmperr = NULL;
  guint64 port_u64;
  guint i;

  for (i = 0; i < n_files; i += 2)
    {
      GFile* port = ((i + 0) < n_files) ? files [(i + 0)] : NULL;
      GFile* home = ((i + 1) < n_files) ? files [(i + 1)] : subst;
      gchar* port_name = g_file_get_relative_path (current, port);
      guint64 port_number = 8080;

      if ((g_ascii_string_to_unsigned (port_name, 10, 0, G_MAXUINT16, &port_u64, &tmperr)), G_UNLIKELY (tmperr == NULL))
        port_number = (guint16) port_u64;
      else
        {
          g_warning ("Bad port number '%s', defaulting 8080", port_name);
          g_error_free (tmperr);
        }

      if ((web_server_listen_any (web_server, port_number, 0, &tmperr)), G_UNLIKELY (tmperr != NULL))
        {
          g_warning ("%s", tmperr->message);
          g_error_free (tmperr);
        }
    }

  _g_object_unref0 (current);
  _g_object_unref0 (subst);
}

int main (int argc, gchar* argv [])
{
  guint exit_code = 0;
  GApplication* application;
  WebServer* web_server;

  const GClosureNotify notify = (GClosureNotify) g_object_unref;
  const GConnectFlags flags = (GConnectFlags) G_CONNECT_SWAPPED;

  application = g_application_new ("org.hck.webserver", G_APPLICATION_HANDLES_OPEN);
  web_server = web_server_new (WEB_HTTP_VERSION_2_0);

  g_application_hold (application);
  g_signal_connect_data (application, "activate", G_CALLBACK (on_activate), g_object_ref (web_server), notify, flags);
  g_signal_connect_data (application, "open", G_CALLBACK (on_open), g_object_ref (web_server), notify, flags);
  g_object_unref (web_server);

  exit_code = g_application_run (application, argc, argv);
return (g_object_unref (application), exit_code);
}

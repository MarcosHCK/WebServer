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
  g_message ("Ready");
}

static gboolean on_incoming (WebServer* web_server, WebClient* client)
{
return TRUE;
}

int main (int argc, gchar* argv [])
{
  const gchar* lang_domain = "en_US";
  const gchar* description_string = "";
  const gchar *parameter_string = "<port> <home_dir>";
  const gchar* summary_string = "";

  GOptionContext* context = NULL;
  GError* tmperr = NULL;
  gint exit_code = 0;

  context = g_option_context_new (parameter_string);
#ifdef G_OS_WIN32
  argv = g_win32_get_command_line ();
#endif // G_OS_WIN32
  g_option_context_set_description (context, description_string);
  g_option_context_set_help_enabled (context, TRUE);
  g_option_context_set_ignore_unknown_options (context, FALSE);
  g_option_context_set_strict_posix (context, FALSE);
  g_option_context_set_summary (context, summary_string);
  g_option_context_set_translation_domain (context, lang_domain);
#ifdef G_OS_WIN32
#define _g_strfreev1 g_strfreev
  g_option_context_parse_strv (context, &argv, &tmperr);
  argc = g_strv_length (argv);
#else // G_OS_WIN32
#define _g_strfreev1(argv) ((void) argv)
  g_option_context_parse (context, &argc, &argv, &tmperr);
#endif // G_OS_WIN32

  if (g_option_context_free (context), G_UNLIKELY (tmperr != NULL))
    {
      const gchar* domain = g_quark_to_string (tmperr->domain);
      const gchar* message = tmperr->message;
      const guint code = tmperr->code;

      g_critical ("(" G_STRLOC "): %s: %d: %s", domain, code, message);
      g_error_free ((exit_code = -1, tmperr));
    }
  else
    {
      guint port = 80;
      gchar* home_dir = "/var/www";
      gpointer web_server;

      WorkQueue* work_queue;

      while (TRUE)
        {
          if (argc > 1)
            {
              GError* tmperr = NULL;
              guint64 port_;

              if ((g_ascii_string_to_unsigned (argv [1], 10, 0, G_MAXUINT16, &port_, &tmperr)), G_UNLIKELY (tmperr == NULL))
                port = (guint) port_;
              else
                {
                  const gchar* domain = g_quark_to_string (tmperr->domain);
                  const gchar* message = tmperr->message;
                  const guint code = tmperr->code;

                  g_critical ("(" G_STRLOC "): %s: %d: %s", domain, code, message);
                  g_error_free ((exit_code = -1, tmperr));
                  break;
                }

              if (argc > 2)
                {
                  if (!g_file_test (home_dir = argv [2], G_FILE_TEST_IS_DIR))
                    {
                      g_critical ("(" G_STRLOC"): Home directory is not such thing");
                      break;
                    }

                  if (argc > 3)
                    {
                      g_critical ("(" G_STRLOC "): Too many options");
                      break;
                    }
                }
            }

          web_server = web_server_new (port, &tmperr);
          work_queue = work_queue_new ();

          if (G_UNLIKELY (tmperr == NULL))
            {
              g_signal_connect_object (web_server, "activate", G_CALLBACK (on_activate), work_queue, 0);
              g_signal_connect_object (web_server, "incoming", G_CALLBACK (on_incoming), work_queue, 0);

              web_server_start (web_server);
              exit_code = g_application_run (web_server, 0, NULL);
              web_server_stop (web_server);
            }
          else
            {
              const gchar* domain = g_quark_to_string (tmperr->domain);
              const gchar* message = tmperr->message;
              const guint code = tmperr->code;

              g_critical ("(" G_STRLOC "): %s: %d: %s", domain, code, message);
              g_error_free ((exit_code = -1, tmperr));
            }

          _g_object_unref0 (web_server);
          _g_object_unref0 (work_queue);
          break;
        }
    }
return (_g_strfreev1 (argv), exit_code);
}

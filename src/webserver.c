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
#include <webserver.h>

#define WEB_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), WEB_TYPE_SERVER, WebServerClass))
#define WEB_IS_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), WEB_TYPE_SERVER))
#define WEB_SERVER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), WEB_TYPE_SERVER, WebServerClass))
typedef struct _WebServerClass WebServerClass;
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
static void web_server_g_initable_iface (GInitableIface* iface);

struct _WebServer
{
  GApplication parent;

  /* private */
  gchar* home_dir;
  guint16 port;
};

struct _WebServerClass
{
  GApplicationClass parent;
};

enum
{
  prop_0,
  prop_home_dir,
  prop_port,
  prop_number,
};

G_DEFINE_FINAL_TYPE_WITH_CODE
  (WebServer, web_server, G_TYPE_APPLICATION,
   G_IMPLEMENT_INTERFACE
    (G_TYPE_INITABLE, web_server_g_initable_iface));

static GParamSpec* properties [prop_number] = {0};

static gboolean web_server_g_initable_iface_init (GInitable* initable, GCancellable* cancellable, GError** error)
{
  return TRUE;
}

static void web_server_g_initable_iface (GInitableIface* iface)
{
  iface->init = web_server_g_initable_iface_init;
}

static void web_server_class_activate (GApplication* pself)
{
  WebServer* self = (gpointer) pself;
  g_print ("done\n");
}

static void web_server_class_get_property (GObject* pself, guint property_id, GValue* value, GParamSpec* pspec)
{
  WebServer* self = (gpointer) pself;

  switch (property_id)
    {
      case prop_home_dir:
        g_value_set_string (value, web_server_get_home_dir (self));
        break;
      case prop_port:
        g_value_set_uint (value, (guint) web_server_get_port (self));
        break;
    }
}


static void web_server_class_set_property (GObject* pself, guint property_id, const GValue* value, GParamSpec* pspec)
{
  WebServer* self = (gpointer) pself;

  switch (property_id)
    {
      case prop_home_dir:
        _g_free0 (self->home_dir);
        self->home_dir = g_value_dup_string (value);
        break;
      case prop_port:
        self->port = (guint16) g_value_get_uint (value);
        break;
    }
}

static void web_server_class_finalize (GObject* pself)
{
  WebServer* self = (gpointer) pself;
  _g_free0 (self->home_dir);
G_OBJECT_CLASS (web_server_parent_class)->finalize (pself);
}

static void web_server_class_dispose (GObject* pself)
{
  WebServer* self = (gpointer) pself;
G_OBJECT_CLASS (web_server_parent_class)->dispose (pself);
}

static void web_server_class_init (WebServerClass* klass)
{
  G_APPLICATION_CLASS (klass)->activate = web_server_class_activate;

  GParamFlags flags1 = G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS;

  G_OBJECT_CLASS (klass)->set_property = web_server_class_set_property;
  G_OBJECT_CLASS (klass)->get_property = web_server_class_get_property;
  G_OBJECT_CLASS (klass)->finalize = web_server_class_finalize;
  G_OBJECT_CLASS (klass)->dispose = web_server_class_dispose;

  properties [prop_home_dir] = g_param_spec_string ("home-dir", "home-dir", "home-dir", NULL, flags1);
  properties [prop_port] = g_param_spec_uint ("port", "port", "port", 0, G_MAXUINT16, 0, flags1);
  g_object_class_install_properties (G_OBJECT_CLASS (klass), prop_number, properties);
}

static void web_server_init (WebServer* self)
{
}

static WebServer* web_server_new (gchar* home_dir, guint16 port, GError** error)
{
 return g_initable_new (WEB_TYPE_SERVER, NULL, error, "home-dir", home_dir, "port", (guint) port, NULL);
}

const gchar* web_server_get_home_dir (WebServer* web_server)
{
  g_return_val_if_fail (WEB_IS_SERVER (web_server), NULL);
return web_server->home_dir;
}

guint16 web_server_get_port (WebServer* web_server)
{
  g_return_val_if_fail (WEB_IS_SERVER (web_server), 0);
return web_server->port;
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
      gpointer application;

      while (TRUE)
        {
          if (argc > 1)
            {
              if (argc > 2)
                {
                  if (argc > 3)
                    {
                      g_critical ("(" G_STRLOC "): Too many options");
                      break;
                    }

                  if (!g_file_test (home_dir = argv [2], G_FILE_TEST_IS_DIR))
                    {
                      g_critical ("(" G_STRLOC"): Home directory is not such thing");
                      break;
                    }
                }

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
            }

          if ((application = web_server_new (home_dir, port, &tmperr)), G_UNLIKELY (tmperr != NULL))
            {
              const gchar* domain = g_quark_to_string (tmperr->domain);
              const gchar* message = tmperr->message;
              const guint code = tmperr->code;

              g_critical ("(" G_STRLOC "): %s: %d: %s", domain, code, message);
              g_error_free ((exit_code = -1, tmperr));
              g_object_unref (application);
            }

          g_object_unref ((exit_code = g_application_run (application, 0, NULL), application));
          break;
        }
    }
return (_g_strfreev1 (argv), exit_code);
}

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
#include <appprivate.h>
#include <webmessage.h>
#include <webmessagemethods.h>
#include <webserver.h>

#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

struct _AppRequest
{
  GFile* root;
  guint upload : 1;
  WebMessage* web_message;
};

G_DECLARE_FINAL_TYPE (AppServer, app_server, APP, SERVER, GApplication);
G_DEFINE_FINAL_TYPE (AppServer, app_server, G_TYPE_APPLICATION);

static void app_server_class_activate (GApplication* pself)
{
  AppServer* self = (gpointer) pself;

  GFile* defaults [] =
    {
      g_file_new_for_commandline_arg ("8080"),
      g_file_new_for_commandline_arg ("/var/www"),
    };

  g_application_open (pself, defaults, G_N_ELEMENTS (defaults), NULL);

  g_object_unref (defaults [1]);
  g_object_unref (defaults [0]);
}

static void app_server_class_dispose (GObject* pself)
{
  AppServer* self = (gpointer) pself;
  g_hash_table_remove_all (self->servers);
G_OBJECT_CLASS (app_server_parent_class)->dispose (pself);
}

static void app_server_class_finalize (GObject* pself)
{
  AppServer* self = (gpointer) pself;
  g_thread_pool_free (self->thread_pool, TRUE, TRUE);
  g_hash_table_unref (self->servers);
G_OBJECT_CLASS (app_server_parent_class)->finalize (pself);
}

static void on_got_failure (WebServer* web_server, GError* tmperr, AppServer* self)
{
  const guint code = tmperr->code;
  const gchar* domain = g_quark_to_string (tmperr->domain);
  const gchar* message = tmperr->message;

  g_warning ("(" G_STRLOC "): %s: %d: %s", domain, code, message);
}

static gboolean on_got_request (WebServer* web_server, WebMessage* web_message, AppServer* self)
{
  struct _AppRequest* request = NULL;
  const gchar* method = NULL;
  gboolean normal = TRUE;
  gboolean upload = TRUE;

  method = web_message_get_method (web_message);

  if ((normal = g_str_equal (method, WEB_MESSAGE_METHOD_GET))
   || (normal = g_str_equal (method, WEB_MESSAGE_METHOD_POST))
   || (upload = g_str_equal (method, WEB_MESSAGE_METHOD_HEAD)))
    {
      request = g_slice_new (struct _AppRequest);
      request->root = g_hash_table_lookup (self->servers, web_server);
      request->upload = upload;
      request->web_message = g_object_ref (web_message);

      g_assert (request->root != NULL);

      web_message_freeze (web_message);
      g_thread_pool_push (self->thread_pool, request, NULL);
    }
return request != NULL;
}

static void app_server_class_open (GApplication* pself, GFile** files, gint n_files, const gchar* hint)
{
  AppServer* self = (gpointer) pself;
  WebServer* web_server = NULL;
  GFile* current = g_file_new_for_path (".");
  GFile* subst = g_file_new_for_path ("/var/www");
  GError* tmperr = NULL;
  guint64 port_u64;
  gint i;

  for (i = 0; i < n_files; i += 2)
    {
      GFile* port = ((i + 0) < n_files) ? files [(i + 0)] : NULL;
      GFile* home = ((i + 1) < n_files) ? files [(i + 1)] : subst;
      gchar* port_name = g_file_get_relative_path (current, port);
      guint64 port_number = 8080;

      web_server = web_server_new ();

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

      g_signal_connect (web_server, "got-failure", G_CALLBACK (on_got_failure), self);
      g_signal_connect (web_server, "got-request", G_CALLBACK (on_got_request), self);
      g_hash_table_insert (self->servers, web_server, g_object_ref (home));
    }

  _g_object_unref0 (current);
  _g_object_unref0 (subst);
}

static void app_server_class_init (AppServerClass* klass)
{
  G_APPLICATION_CLASS (klass)->activate = app_server_class_activate;
  G_OBJECT_CLASS (klass)->dispose = app_server_class_dispose;
  G_OBJECT_CLASS (klass)->finalize = app_server_class_finalize;
  G_APPLICATION_CLASS (klass)->open = app_server_class_open;
}

static void request_proc (struct _AppRequest* request, AppServer* self)
{
  GError* tmperr = NULL;
  GFile* root = request->root;
  WebMessage* message = request->web_message;

  if ((_app_process (self, message, root, &tmperr)), G_UNLIKELY (tmperr == NULL))
    web_message_thaw (message);
  else
    {
      web_message_set_status (message, WEB_STATUS_CODE_INTERNAL_SERVER_ERROR);
      web_message_set_is_closure (message, TRUE);
      web_message_thaw (message);

      const guint code = tmperr->code;
      const gchar* domain = g_quark_to_string (tmperr->domain);
      const gchar* message = tmperr->message;

      g_warning ("(" G_STRLOC "): %s: %d: %s", domain, code, message);
      g_error_free (tmperr);
    }
}

static void request_free (struct _AppRequest* request)
{
  g_object_unref (request->root);
  g_object_unref (request->web_message);
  g_slice_free (struct _AppRequest, request);
}

static void app_server_init (AppServer* self)
{
  GHashFunc func1 = (GHashFunc) g_direct_hash;
  GEqualFunc func2 = (GEqualFunc) g_direct_equal;
  GFunc func3 = (GFunc) request_proc;
  GDestroyNotify notify1 = (GDestroyNotify) g_object_unref;
  GDestroyNotify notify2 = (GDestroyNotify) request_free;
  guint max_threads = g_get_num_processors ();

  self->servers = g_hash_table_new_full (func1, func2, notify1, notify1);
  self->thread_pool = g_thread_pool_new_full (func3, self, notify2, max_threads, 0, NULL);
}

int main (int argc, gchar* argv [])
{
  GApplication* application;
  guint exit_code;

  const GClosureNotify notify = (GClosureNotify) g_object_unref;
  const GConnectFlags flags = (GConnectFlags) G_CONNECT_SWAPPED;

  gtk_init (&argc, &argv);

  application = g_object_new (app_server_get_type (),
                              "application-id", "org.hck.webserver",
                                       "flags", G_APPLICATION_HANDLES_OPEN,
                                          NULL);

  exit_code = (g_application_hold (application), 0);
  exit_code = (g_application_run (application, argc, argv));
return (g_object_unref (application), exit_code);
}

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
#include <glib/gi18n.h>
#include <gio/gio.h>

typedef struct _Range Range;
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
static void app_stream_g_pollable_input_stream_iface (GPollableInputStreamInterface* iface);
static void app_ranged_stream_g_pollable_input_stream_iface (GPollableInputStreamInterface* iface);
static const guint blocksz = 2048;

struct _AppStream
{
  GBufferedInputStream parent;

  /*<private>*/
  GInputStream* base_stream;
  gint fill_eof;
  GError* fill_error;
  gint fill_pending;
};

G_DECLARE_FINAL_TYPE (AppStream, app_stream, APP, STREAM, GBufferedInputStream);
G_DEFINE_TYPE_WITH_CODE (AppStream, app_stream, G_TYPE_BUFFERED_INPUT_STREAM,
  G_IMPLEMENT_INTERFACE (G_TYPE_POLLABLE_INPUT_STREAM, app_stream_g_pollable_input_stream_iface));

static void ready (GObject* pself, GAsyncResult* result, gpointer user_data)
{
  AppStream* self = (gpointer) pself;
  GBufferedInputStream* buffered = (gpointer) pself;
  gssize read;

  if ((read = g_buffered_input_stream_fill_finish (buffered, result, & self->fill_error)) == 0)
    g_atomic_int_set (& self->fill_eof, TRUE);
    g_atomic_int_set (& self->fill_pending, FALSE);
}

static gssize app_stream_g_pollable_input_stream_iface_read_nonblocking (GPollableInputStream* pself, void* buffer, gsize count, GError** error)
{
  AppStream* self = (gpointer) pself;
  GBufferedInputStream* buffered = (gpointer) pself;
  GInputStream* stream = (gpointer) pself;
  gsize available = 0;
  gssize read = -1;

  if (g_atomic_int_get (& self->fill_pending) == TRUE)
    g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK, _("Would block"));
  else
    {
      if (G_UNLIKELY (self->fill_error != NULL))
        g_propagate_error (error, g_steal_pointer (& self->fill_error));
      else if (self->fill_eof == TRUE)
        read = 0;
      else if ((available = g_buffered_input_stream_get_available (buffered)) > 0)
        read =  g_input_stream_read (stream, buffer, MIN (count, available), NULL, error);
      else
        {
          g_atomic_int_set (& self->fill_pending, TRUE);
          g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK, _("Would block"));
          g_buffered_input_stream_fill_async (buffered, blocksz, G_PRIORITY_DEFAULT, NULL, ready, NULL);
        }
    }
return (read);
}

static void app_stream_g_pollable_input_stream_iface (GPollableInputStreamInterface* iface)
{
  iface->read_nonblocking = app_stream_g_pollable_input_stream_iface_read_nonblocking;
}

static void app_stream_class_finalize (GObject* pself)
{
  AppStream* self = (gpointer) pself;
  _g_error_free0 (self->fill_error);
G_OBJECT_CLASS (app_stream_parent_class)->finalize (pself);
}

static void app_stream_class_init (AppStreamClass* klass)
{
  G_OBJECT_CLASS (klass)->finalize = app_stream_class_finalize;
}

static void app_stream_init (AppStream* self)
{
  self->base_stream = NULL;
  self->fill_eof = 0;
  self->fill_error = NULL;
  self->fill_pending = 0;
}

GInputStream* _app_stream_new (GInputStream* base_stream)
{
  return g_object_new (app_stream_get_type (), "base-stream", base_stream, "buffer-size", blocksz, NULL);
}

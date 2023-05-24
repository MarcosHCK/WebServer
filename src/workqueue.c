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
#include <workqueue.h>

#define WORK_QUEUE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), WORK_TYPE_QUEUE, WorkQueueClass))
#define WORK_IS_QUEUE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), WORK_TYPE_QUEUE))
#define WORK_QUEUE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), WORK_TYPE_QUEUE, WorkQueueClass))
typedef struct _WorkQueueClass WorkQueueClass;
#define _g_thread_pool_free0(var,...) ((var == NULL) ? NULL : (var = (g_thread_pool_free (var, __VA_ARGS__), NULL)))

struct _WorkQueue
{
  GObject parent;

  /* private */
  GThreadPool* thread_pool;
};

struct _WorkQueueClass
{
  GObjectClass parent;
};

G_DEFINE_FINAL_TYPE (WorkQueue, work_queue, G_TYPE_OBJECT);

static void work_queue_class_dispose (GObject* pself)
{
  WorkQueue* self = (gpointer) pself;
  _g_thread_pool_free0 (self->thread_pool, TRUE, TRUE);
G_OBJECT_CLASS (work_queue_parent_class)->dispose (pself);
}

static void work_queue_class_finalize (GObject* pself)
{
  WorkQueue* self = (gpointer) pself;
G_OBJECT_CLASS (work_queue_parent_class)->finalize (pself);
}

static void work_queue_class_init (WorkQueueClass* klass)
{
  G_OBJECT_CLASS (klass)->dispose = work_queue_class_dispose;
  G_OBJECT_CLASS (klass)->finalize = work_queue_class_finalize;
}

static void process (WebClient* client, WorkQueue* self)
{
}

static void work_queue_init (WorkQueue* self)
{
  const GFunc func = (GFunc) process;
  const GDestroyNotify notify = (GDestroyNotify) g_object_unref;
  const guint max_threads = g_get_num_processors ();

  GError* tmperr = NULL;

  self->thread_pool = g_thread_pool_new_full (func, self, notify, max_threads, FALSE, &tmperr);

  if (G_UNLIKELY (tmperr != NULL))
    {
      const gchar* domain = g_quark_to_string (tmperr->domain);
      const gchar* message = tmperr->message;
      const guint code = tmperr->code;

      g_error ("(" G_STRLOC "): %s: %d: %s", domain, code, message);
      g_assert_not_reached ();
    }
}

WorkQueue* work_queue_new ()
{
  return g_object_new (WORK_TYPE_QUEUE, NULL);
}

void work_queue_push (WorkQueue* work_queue, WebClient* web_client)
{
  g_return_if_fail (WORK_IS_QUEUE (work_queue));
  g_return_if_fail (WEB_IS_CLIENT (web_client));
  g_thread_pool_push (work_queue->thread_pool, g_object_ref (web_client), NULL);
}

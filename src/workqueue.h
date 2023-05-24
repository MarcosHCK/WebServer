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
#ifndef __WORK_QUEUE__
#define __WORK_QUEUE__ 1
#include <webclient.h>

#define WORK_TYPE_QUEUE (work_queue_get_type ())
#define WORK_QUEUE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), WORK_TYPE_QUEUE, WorkQueue))
#define WORK_IS_QUEUE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), WORK_TYPE_QUEUE))
typedef struct _WorkQueue WorkQueue;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  G_GNUC_INTERNAL GType work_queue_get_type (void) G_GNUC_CONST;
  G_GNUC_INTERNAL WorkQueue* work_queue_new ();
  G_GNUC_INTERNAL void work_queue_push (WorkQueue* work_queue, WebClient* web_client);

#if __cplusplus
}
#endif // __cplusplus

#endif // __WEB_WORKQUEUE__

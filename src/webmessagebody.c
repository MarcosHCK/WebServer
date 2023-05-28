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
#include <webmessage.h>

struct _WebMessageBody
{
  int dummy;
};

static gpointer nullfunc (gpointer ptr)
{
  g_assert_not_reached ();
}

G_DEFINE_BOXED_TYPE (WebMessageBody, web_message_body, nullfunc, web_message_body_free);

WebMessageBody* web_message_body_new ()
{
  WebMessageBody* self;

  self = g_slice_new (WebMessageBody);
return (self);
}

void web_message_body_free (WebMessageBody* web_message_body)
{
  g_return_if_fail (web_message_body != NULL);
  WebMessageBody* self = (web_message_body);

  g_slice_free (WebMessageBody, self);
}

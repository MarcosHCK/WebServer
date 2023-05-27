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
#include <webresponse.h>

struct _WebResponsePrivate
{
  WebStatusCode status_code;
};

G_DEFINE_TYPE_WITH_PRIVATE (WebResponse, web_response, WEB_TYPE_MESSAGE);

static void web_response_class_init (WebResponseClass* klass)
{
}

static void web_response_init (WebResponse* self)
{
}

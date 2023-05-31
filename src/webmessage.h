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
#ifndef __WEB_MESSAGE__
#define __WEB_MESSAGE__ 1
#include <gio/gio.h>
#include <webhttpversion.h>
#include <webstatuscode.h>

#define WEB_TYPE_MESSAGE (web_message_get_type ())
#define WEB_MESSAGE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), WEB_TYPE_MESSAGE, WebMessage))
#define WEB_IS_MESSAGE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), WEB_TYPE_MESSAGE))
typedef struct _WebMessage WebMessage;
typedef struct _WebMessageBody WebMessageBody;
typedef struct _WebMessageHeaders WebMessageHeaders;
typedef struct _WebMessageHeadersIter WebMessageHeadersIter;
typedef struct _WebMessagePrivate WebMessagePrivate;
typedef struct _WebMessageRange WebMessageRange;
#define WEB_MESSAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), WEB_TYPE_MESSAGE, WebMessageClass))
#define WEB_IS_MESSAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), WEB_TYPE_MESSAGE))
#define WEB_MESSAGE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), WEB_TYPE_MESSAGE, WebMessageClass))
typedef struct _WebMessageClass WebMessageClass;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _WebMessage
  {
    GObject parent;
    WebMessagePrivate* priv;
  };

  struct _WebMessageClass
  {
    GObjectClass parent;
  };

  struct _WebMessageRange
  {
    goffset start;
    goffset length;
  };

  struct _WebMessageHeadersIter
  {
    GHashTableIter iter;
  };

  typedef enum
  {
    WEB_MESSAGE_ENCODING_UNKNOWN,
    WEB_MESSAGE_ENCODING_NONE,
    WEB_MESSAGE_ENCODING_CONTENT_LENGTH,
    WEB_MESSAGE_ENCODING_EOF,
    WEB_MESSAGE_ENCODING_CHUNKED,
    WEB_MESSAGE_ENCODING_BYTERANGES,
  } WebMessageEncoding;

  typedef enum
  {
    WEB_MESSAGE_EXPECTATION_UNKNOWN,
    WEB_MESSAGE_EXPECTATION_CONTINUE,
  } WebMessageExpectation;

  G_GNUC_INTERNAL GType web_message_get_type (void) G_GNUC_CONST;
  G_GNUC_INTERNAL GType web_message_body_get_type (void) G_GNUC_CONST;
  G_GNUC_INTERNAL GType web_message_headers_get_type (void) G_GNUC_CONST;
  G_GNUC_INTERNAL void web_message_body_free (WebMessageBody* web_message_body);
  G_GNUC_INTERNAL void web_message_headers_free (WebMessageHeaders* web_message_headers);
  G_GNUC_INTERNAL WebMessage* web_message_new ();
  G_GNUC_INTERNAL WebMessageBody* web_message_body_new ();
  G_GNUC_INTERNAL WebMessageHeaders* web_message_headers_new ();
  G_GNUC_INTERNAL void web_message_body_add_bytes (WebMessageBody* web_message_body, GBytes* bytes);
  G_GNUC_INTERNAL void web_message_body_add_data (WebMessageBody* web_message_body, gpointer data, gsize length, GDestroyNotify notify);
  G_GNUC_INTERNAL void web_message_body_set_content_encoding (WebMessageBody* web_message_body, const gchar* content_encoding);
  G_GNUC_INTERNAL void web_message_body_set_content_length (WebMessageBody* web_message_body, gsize content_length);
  G_GNUC_INTERNAL void web_message_body_set_content_type (WebMessageBody* web_message_body, const gchar* content_type);
  G_GNUC_INTERNAL void web_message_body_set_stream (WebMessageBody* web_message_body, GInputStream* stream);
  G_GNUC_INTERNAL const gchar* web_message_body_get_content_encoding (WebMessageBody* web_message_body);
  G_GNUC_INTERNAL gsize web_message_body_get_content_length (WebMessageBody* web_message_body);
  G_GNUC_INTERNAL const gchar* web_message_body_get_content_type (WebMessageBody* web_message_body);
  G_GNUC_INTERNAL GInputStream* web_message_body_get_stream (WebMessageBody* web_message_body);
  G_GNUC_INTERNAL void web_message_freeze (WebMessage* web_message);
  G_GNUC_INTERNAL WebMessageHeaders* web_message_get_headers (WebMessage* web_message);
  G_GNUC_INTERNAL WebHttpVersion web_message_get_http_version (WebMessage* web_message);
  G_GNUC_INTERNAL gboolean web_message_get_is_closure (WebMessage* web_message);
  G_GNUC_INTERNAL const gchar* web_message_get_method (WebMessage* web_message);
  G_GNUC_INTERNAL WebMessageBody* web_message_get_request (WebMessage* web_message);
  G_GNUC_INTERNAL WebMessageBody* web_message_get_response (WebMessage* web_message);
  G_GNUC_INTERNAL WebStatusCode web_message_get_status (WebMessage* web_message);
  G_GNUC_INTERNAL GUri* web_message_get_uri (WebMessage* web_message);
  G_GNUC_INTERNAL void web_message_headers_append (WebMessageHeaders* web_message_headers, const gchar* key, const gchar* value);
  G_GNUC_INTERNAL void web_message_headers_append_take (WebMessageHeaders* web_message_headers, gchar* key, gchar* value);
  G_GNUC_INTERNAL void web_message_headers_clear (WebMessageHeaders* web_message_headers);
  G_GNUC_INTERNAL gboolean web_message_headers_contains (WebMessageHeaders* web_message_headers, const gchar* key);
  G_GNUC_INTERNAL goffset web_message_headers_get_content_length (WebMessageHeaders* web_message_headers);
  G_GNUC_INTERNAL const gchar* web_message_headers_get_content_type (WebMessageHeaders* web_message_headers);
  G_GNUC_INTERNAL WebMessageEncoding web_message_headers_get_encoding (WebMessageHeaders* web_message_headers);
  G_GNUC_INTERNAL WebMessageExpectation web_message_headers_get_expectations (WebMessageHeaders* web_message_headers);
  G_GNUC_INTERNAL gboolean web_message_headers_get_keep_alive (WebMessageHeaders* web_message_headers);
  G_GNUC_INTERNAL GList* web_message_headers_get_list (WebMessageHeaders* web_message_headers, const gchar* key);
  G_GNUC_INTERNAL const gchar* web_message_headers_get_one (WebMessageHeaders* web_message_headers, const gchar* key);
  G_GNUC_INTERNAL WebMessageRange* web_message_headers_get_ranges (WebMessageHeaders* web_message_headers, guint* n_ranges);
  G_GNUC_INTERNAL void web_message_headers_iter_init (WebMessageHeadersIter* iter, WebMessageHeaders* web_message_headers);
  G_GNUC_INTERNAL gboolean web_message_headers_iter_next (WebMessageHeadersIter* iter, gchar const** key, GList** values);
  G_GNUC_INTERNAL void web_message_headers_remove (WebMessageHeaders* web_message_headers, const gchar* key);
  G_GNUC_INTERNAL void web_message_headers_replace (WebMessageHeaders* web_message_headers, const gchar* key, const gchar* value);
  G_GNUC_INTERNAL void web_message_headers_replace_take (WebMessageHeaders* web_message_headers, gchar* key, gchar* value);
  G_GNUC_INTERNAL void web_message_set_http_version (WebMessage* web_message, WebHttpVersion http_version);
  G_GNUC_INTERNAL void web_message_set_is_closure (WebMessage* web_message, gboolean is_closure);
  G_GNUC_INTERNAL void web_message_set_method (WebMessage* web_message, const gchar* method);
  G_GNUC_INTERNAL void web_message_set_request (WebMessage* web_message, const gchar* content_type, const gchar* request, gsize length);
  G_GNUC_INTERNAL void web_message_set_request_bytes (WebMessage* web_message, const gchar* content_type, GBytes* bytes);
  G_GNUC_INTERNAL void web_message_set_request_take (WebMessage* web_message, const gchar* content_type, gchar* request, gsize length);
  G_GNUC_INTERNAL void web_message_set_response (WebMessage* web_message, const gchar* content_type, const gchar* response, gsize length);
  G_GNUC_INTERNAL void web_message_set_response_bytes (WebMessage* web_message, const gchar* content_type, GBytes* bytes);
  G_GNUC_INTERNAL void web_message_set_response_take (WebMessage* web_message, const gchar* content_type, gchar* response, gsize length);
  G_GNUC_INTERNAL void web_message_set_status (WebMessage* web_message, WebStatusCode status_code);
  G_GNUC_INTERNAL void web_message_set_status_full (WebMessage* web_message, WebStatusCode status_code, const gchar* reason);
  G_GNUC_INTERNAL void web_message_set_uri (WebMessage* web_message, GUri* uri);
  G_GNUC_INTERNAL void web_message_thaw (WebMessage* web_message);

#if __cplusplus
}
#endif // __cplusplus

#endif // __WEB_MESSAGE__

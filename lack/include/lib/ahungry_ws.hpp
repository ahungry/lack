#ifndef AHUNGRY_WS_H
#define AHUNGRY_WS_H

#include "nativeui/nativeui.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Network related
#include <pthread.h>
#include <libwebsockets.h>
#include <json-c/json.h>
#include <json-c/json_tokener.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "json_handler.hpp"

static struct lws *wsi_basic;
int force_exit = 0;
static unsigned int opts;
const char *my_message = "{\"type\":\"ping\"}";

// Slack token
char *slack_token = NULL;

// GUI related
nu::Lifetime *gui_lifetime = NULL;
nu::TextEdit *gui_chat_text_edit = NULL;

// Handle tracking JSON structures.
int json_brace_count = 0;
int json_mode = 1;

// Big 'ol buffer to store received data in.
char *rx_buf = NULL;
// char rx_buf[10000];

int
display_rx_buf ()
{
  if (NULL == rx_buf)
    {
      return 1;
    }

  // @todo Parse out the JSON parts of message
  // Ideally in it's own class or codebase.
  // ref: https://json-c.github.io/json-c/json-c-0.12.1/doc/html/json__tokener_8h.html
  printf ("Rx: %s\n\n", rx_buf);

  // Find out what type of JSON we have received.
  json_object *j = json_to_object ((char *) rx_buf);
  int type = j_get_type (j);    /* See what type of object */
  char *buf = NULL;

  switch (type)
    {
      // @todo Refactor to dispatch text to a specific channel.
    case SLACK_TYPE_MESSAGE:
      json_object *j_text = NULL;
      json_object_object_get_ex (j, "text", &j_text);
      const char *text = json_object_get_string (j_text);
      int len_text = strlen (text);

      printf ("Received text: %s of length: %d\n\n", text, len_text);

      int len = strlen (gui_chat_text_edit->GetText ().c_str ());
      buf = (char *) malloc (10 * sizeof (buf) * (len + len_text + 2));
      memcpy (buf, text, len_text);
      buf[len_text] = '\n';
      memcpy (buf + len_text + 1, gui_chat_text_edit->GetText ().c_str (), len);
      buf[len_text + len] = '\0';

      printf ("Rx: %s\n\n", rx_buf);
      gui_chat_text_edit->SetText (buf);

      break;
    }

  free (rx_buf); // Bad idea to have this handle the free
  free (buf);
  rx_buf = NULL;
  buf = NULL;
  json_object_put (j);

  return 0;
}

/* A simple callback block - basically will send ping messages when connecting and
   when it ends up receiving, it will print the output to the user. */
static int callback_protocol_fn (struct lws *wsi, enum lws_callback_reasons reason,
                                 void *user, void *in, size_t len)
{
  char buf[50 + LWS_PRE];
  int deny_deflate = 1;

  switch (reason)
    {
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
      printf ("Connection established.\n");
      lws_callback_on_writable (wsi);

      break;

    case LWS_CALLBACK_CLOSED:
      printf ("Connection closed.\n");
      wsi_basic = NULL; // Deactivate the interaction, so it will re-connect.

      break;

    case LWS_CALLBACK_CLIENT_RECEIVE:
      ((char *) in)[len] = '\0';

      if (json_mode) {
        // Expand our memory
        int rx_buflen = NULL == rx_buf ? 0 : strlen (rx_buf);
        rx_buf = (char *) realloc (rx_buf, sizeof (rx_buf) * (rx_buflen  + len));

        if (NULL == rx_buf)
          {
            fprintf (stderr, "Failed to realloc() memory!");
          }

        // We could copy in batches, but we need to count braces.
        for (uint i = 0; i < len; i++)
          {
            // If we did the PT dispatch, this could be null in our loop.
            if (NULL == rx_buf)
              {
                rx_buf = (char *) realloc (rx_buf, sizeof (rx_buf) + len); // May be a couple bytes too big.
              }

            if ('{' == ((char*) in)[i]) json_brace_count++;
            if ('}' == ((char*) in)[i]) json_brace_count--;

            // Copy in one char at a time.
            rx_buf[rx_buflen + i] = ((char *) in)[i];

            // If even braces, flush buffer and output.
            if (!json_brace_count) {
              // Output to term and GUI
              gui_lifetime->PostTask (display_rx_buf);
            }
          }

        // memcpy (rx_buf + rx_buflen, (char *) in, len + 1);

        // If even braces, flush buffer and output.
        if (!json_brace_count) {
          // Output to term and GUI
          gui_lifetime->PostTask (display_rx_buf);
        }
      } else {
        printf ("%s\n", (char *) in);
      }
      // If we wanted to echo something back...
      // lws_callback_on_writable (wsi);

      break;

    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
      printf ("Connection error, closing client.\n");

      // In the test-client.c sample, this auto-terminates the wsi
      // Maybe that isn't what you want to do though?
      if (wsi == wsi_basic)
        {
          wsi_basic = NULL;
        }

      break;

      // If this returns a non-zero, it denies the extension basically.
    case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
      if ((strcmp((const char *)in, "deflate-stream") == 0) && deny_deflate) {
        printf ("denied deflate-stream extension\n");

        return 1;
      }

      if ((strcmp((const char *)in, "x-webkit-deflate-frame") == 0))
        return 1;

      if ((strcmp((const char *)in, "deflate-frame") == 0))
        return 1;

      return 1;

      break;

      // This is the area where you send your custom messages (you can
      // request it trigger on demand).
    case LWS_CALLBACK_CLIENT_WRITEABLE:
      printf ("Wrote %s of strlen %d\n", my_message, (int) strlen ((char *) my_message));
      strcpy (buf, my_message);
      lws_write (wsi, (unsigned char *) buf, 15, (lws_write_protocol)(opts | LWS_WRITE_TEXT));

      break;

    default:
      break;
    }

  return 0;
}

/* If a server sends a custom protocol, you can dispatch bassed on this.
   If the server doesn't use protocol, the first element in list will be used. */
static const struct lws_protocols protocols[] = {
  {
    "", // A protocol name or identifier (any unmatched falls in here)
    callback_protocol_fn,
    0,
    20
  },
  { NULL, NULL, 0, 0 }
};

/* Extensions indicate things that can be supported (compression etc.)
   by the underlying client. */
static const struct lws_extension exts[] = {
  {
    "permessage-deflate",
    lws_extension_callback_pm_deflate,
    "permessage-deflate; client_max_window_bits" // client_no_context_takeover
  },
  {
    "deflate-frame",
    lws_extension_callback_pm_deflate,
    "deflate_frame"
  },
  { NULL, NULL, NULL }
};

void *
slack_rtm_connect_service_loop (void *ptr)
{
  // Option setting related
  nu::Lifetime *lifetime = (nu::Lifetime *) ptr;
  int n = 0;
  int use_ssl = 0;
  char *path;

  // lws related
  struct lws_context_creation_info info;
  struct lws_client_connect_info i;
  struct lws_context *context;
  // const char *prot, *p;

  // There are LLL_COUNT (11) bits in log levels
  // Unfortunately, unless lib compiled in DEBUG mode, you can't view any of them!
  int log_level = 0;

  for (int i = 0; i < LLL_COUNT; i++)
    {
      log_level |= 1 << i;
    }

  lws_set_log_level (log_level, NULL);

  // Init memory
  memset (&info, 0, sizeof info);
  memset (&i, 0, sizeof i);

  info.port = -1;
  info.protocols = protocols;
  info.gid = -1;
  info.uid = -1;
  info.extensions = exts;
  //info.pt_serv_buf_size = 1024; // Adjust the max bytes sent in a request

  // If you plan to hit wss endpoints, better hope this works and you
  // compiled with openssl support ;)
  info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;

  context = lws_create_context (&info);

  if (context == NULL)
    {
      fprintf (stderr, "Creating libwebsocket context failed\n");

      return 0;
    }

  i.context = context;

  use_ssl = LCCSCF_USE_SSL |
    LCCSCF_ALLOW_SELFSIGNED |
    LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK |
    LCCSCF_ALLOW_EXPIRED;

  i.ssl_connection = use_ssl; // Non-negative to use it
  i.address = "dummy.slack-msgs.com";
  i.port = 443;
  i.host = i.address;
  i.origin = "";

  // Create the path for the user.
  path = (char *) malloc (sizeof (path) + strlen (slack_token) + 20);
  sprintf (path, "/?flannel=1&token=%s", slack_token);
  i.path = path;

  //i.ietf_version_or_minus_one = -1; // Lib says this is deprecated.
  i.protocol = protocols[0].name;
  i.pwsi = &wsi_basic;

  // SSL_set_verify (lws_get_ssl(wsi_basic), SSL_VERIFY_NONE, OpenSSL_client_verify_callback);

  lws_client_connect_via_info (&i);

  while (!force_exit)
    {
      // If you lose connection, try, try again.
      if (NULL == wsi_basic)
        {
          lws_client_connect_via_info (&i);
        }

      // Handle the callback loop I think.
      lws_service (context, 500);

      sleep (0);
    }

  lws_context_destroy (context);

  return 0;
}

int
slack_rtm_connect (char *token, nu::Lifetime *lifetime, nu::TextEdit *chat_text_edit)
{
  printf ("Using token: %s\n", token);

  // Bind to top level variables.
  slack_token = (char *) malloc (strlen (token) * sizeof (char) + 1);
  memcpy (slack_token, token, strlen (token));
  slack_token[strlen (token)] = '\0';

  gui_lifetime = lifetime;
  gui_chat_text_edit = chat_text_edit;

  pthread_t t;
  pthread_create (&t, NULL, slack_rtm_connect_service_loop, (void *) lifetime);
  // pthread_join (t, NULL);

  return 0;
}

#endif

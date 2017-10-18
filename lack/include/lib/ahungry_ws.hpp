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
#include "ahungry_http_request.hpp"
#include "channel_container.hpp"

static struct lws *wsi_basic;
int force_exit = 0;
static unsigned int opts;
const char *my_message = "{\"type\":\"ping\"}";

// {"type":"flannel", "subtype":"user_query_request"}
int request_users_p = 1;
char *my_user_query_request = get_user_query_request_json ((char *) "");
channel_t *g_active_channel = NULL;

// Slack token
char *slack_token = NULL;

// GUI related
nu::Lifetime *gui_lifetime = NULL;
nu::TextEdit *gui_chat_text_edit = NULL;
nu::Scroll *gui_channel_scroll = NULL;

// Handle tracking JSON structures.
int json_brace_count = 0;
int json_mode = 1;

// Big 'ol buffer to store received data in.
char *rx_buf = NULL;
char *ephermal_rx_buf = NULL;
int ephermal_rx_buf_lock = 0;
int lock_in_stream = 0;

// char rx_buf[10000];

int
display_rx_buf ()
{
  // @todo This has a possibility of one json parse blocking another, for
  // very high throughput, could cause message loss.

  // If we're already working on an ephermal buffer, or it never existed.
  if (ephermal_rx_buf_lock || NULL == ephermal_rx_buf)
    {
      return 1;
    }

  ephermal_rx_buf_lock = 1; // mutex for callback

  // @todo Parse out the JSON parts of message
  // Ideally in it's own class or codebase.
  // ref: https://json-c.github.io/json-c/json-c-0.12.1/doc/html/json__tokener_8h.html
  // printf ("DRB: Rx: __%s__ of len: %d\n\n", ephermal_rx_buf, strlen (ephermal_rx_buf));

  // Find out what type of JSON we have received.
  json_object *j = json_to_object ((char *) ephermal_rx_buf);
  int type = j_get_type (j);    /* See what type of object */

  // Used for user query request pagination.
  char *next_marker = json_get_string (j, "next_marker");
  char *buf = NULL;

  switch (type)
    {
    case SLACK_SUBTYPE_user_query_response:
      // If we can find a next_marker value, we need to get more names out.
      if (NULL != next_marker)
        {
          request_users_p = 1;
          my_user_query_request = get_user_query_request_json (next_marker);
        }
      else
        {
          request_users_p = 0;
        }

      slack_user_push ((char *) ephermal_rx_buf); // @todo re-use parsed object.
      break;

    case SLACK_TYPE_message:
      char *text = json_get_string (j, "text");
      int len_text = strlen (text);

      printf ("Received text: %s of length: %d\n\n", text, len_text);

      // Store the message received in a specific channel buffer.
      char *channel = json_get_string (j, "channel");

      printf ("To channel: %s of length: %d\n\n", channel, strlen (channel));

      // @todo Translate user to a display name
      // {"type":"flannel", "subtype":"user_query_request"}
      char *user = json_get_string (j, "user");

      // At this point, user is the id.
      slack_user_t *slack_user = slack_user_get ((char *) user);

      // Oops, no ID found - better re-request from the server sometime soon.
      if (NULL == slack_user)
        {
          buf = (char *) realloc (buf, (6 + strlen (text) + strlen (user)) * sizeof (char));
          sprintf (buf, "<%s>: %s\n", user, text);
        }
      else
        {
          buf = (char *) realloc (buf, (5 + strlen (text) + strlen (slack_user->name)) * sizeof (char));
          sprintf (buf, "<%s>: %s", slack_user->name, text);
        }

      channel_push ((char *) channel, (char *) buf);
      printf ("Received and glued: %s\n", channel_glue ((char *) channel, (char *) "\n"));

      // We don't really need the append logic now, buffer stores it all.
      /*
      int len = strlen (gui_chat_text_edit->GetText ().c_str ());
      buf = (char *) malloc (10 * sizeof (buf) * (len + len_text + 2));
      memcpy (buf, text, len_text);
      buf[len_text] = '\n';
      memcpy (buf + len_text + 1, gui_chat_text_edit->GetText ().c_str (), len);
      buf[len_text + len] = '\0';

      printf ("Rx: %s\n\n", rx_buf);
      gui_chat_text_edit->SetText (buf);
      */
      gui_chat_text_edit->SetText (channel_glue_reverse ((char *) channel, (char *) "\n"));

      break;
    }

  free (ephermal_rx_buf);
  free (buf);
  ephermal_rx_buf = NULL;
  buf = NULL;
  json_object_put (j);

  // Unlock the processing thread
  ephermal_rx_buf_lock = 0;
  lock_in_stream = 0;

  return 0;
}

/* A simple callback block - basically will send ping messages when connecting and
   when it ends up receiving, it will print the output to the user. */
static int callback_protocol_fn (struct lws *wsi, enum lws_callback_reasons reason,
                                 void *user, void *in, size_t len)
{
  char buf[150 + LWS_PRE];
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
      //((char *) in)[len] = '\0';

      if (json_mode && ! lock_in_stream) {
        // Expand our memory
        int rx_buflen = NULL == rx_buf ? 0 : strlen (rx_buf);
        rx_buf = (char *) realloc (rx_buf, (1 + rx_buflen  + len) * sizeof (char));

        if (NULL == rx_buf)
          {
            fprintf (stderr, "Failed to realloc() memory for rx_buf in CB!\n");
            fprintf (stderr, "Received len: %d\n", len);
            fprintf (stderr, "Received text of: %s\n", in);
            exit (1);
          }

        // We could copy in batches, but we need to count braces.
        int c = 0;

        for (uint i = 0; i < len; i++)
          {
            if ('{' == ((char*) in)[i]) json_brace_count++;
            if ('}' == ((char*) in)[i]) json_brace_count--;

            // If we're not in braces, and we didn't just receive one,
            // then ignore the input, its junk.
            // @todo Update this to be better - I mean, we could have
            // a non-object wrapped response (well, maybe not via slack...)
            if (!json_brace_count && '}' != ((char *) in)[i])
              {
                continue;
              }

            // Copy in one char at a time.
            rx_buf[rx_buflen + c] = ((char *) in)[i];
            rx_buf[rx_buflen + c + 1] = '\0';

            // If even braces, flush buffer and output.
            if (!json_brace_count) {
              lock_in_stream = 1;
              ephermal_rx_buf = (char *) calloc (sizeof (char), (2 + rx_buflen + c) * sizeof (char));
              memcpy (ephermal_rx_buf, rx_buf, rx_buflen + c + 1);
              ephermal_rx_buf[rx_buflen + c + 1] = '\0';

              // Give back our buffer and drop out of any future events.
              free (rx_buf);
              rx_buf = NULL;

              // Output to term and GUI
              gui_lifetime->PostTask (display_rx_buf);

              return 0;
            }

            c++; // Increment by chars we don't just 'continue' over.
          }

        // memcpy (rx_buf + rx_buflen, (char *) in, len + 1);

        // If even braces, flush buffer and output.
        // if (!json_brace_count) {
        //   // Output to term and GUI
        //   gui_lifetime->PostTask (display_rx_buf);
        // }
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
      // Send the ping pong to keep connection alive.
      // @todo Track a time delay for how often we want to do this.
      // printf ("Wrote %s of strlen %d\n", my_message, (int) strlen ((char *) my_message));
      // strcpy (buf, my_message);
      // lws_write (wsi, (unsigned char *) buf, 15, (lws_write_protocol)(opts | LWS_WRITE_TEXT));

      // Request the users, if we don't have one after the dummy user.
      if (request_users_p)
        {
          printf ("Wrote %s of strlen %d\n", my_user_query_request, (int) strlen ((char *) my_user_query_request));
          strcpy (buf, my_user_query_request);
          lws_write (wsi, (unsigned char *) buf, strlen (my_user_query_request), (lws_write_protocol)(opts | LWS_WRITE_TEXT));
        }

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

void *
populate_channel_scroll ()
{
  channel_fetch ();
  scoped_refptr<nu::Container> channel_container (new nu::Container ());
  channel_container->SetStyle ("justify-content", "center");

  nu::Button *channel_buttons[g_channel_container.len];

  char *buf = NULL;
  int chan_size = 0;

  nu::App *app = nu::App::GetCurrent ();
  nu::Font *font = app->GetDefaultFont ();

  // How to easily walk through each element of an array of ptrs (total size divided by per-member size).
  for (int i = 0; i < g_channel_container.len; i++)
    {
      channel_t *channel = g_channel_container.ptr[i];

      chan_size = strlen (channel->desc);
      buf = (char *) realloc (buf, 1 + chan_size);

      if (NULL == buf)
        {
          fprintf (stderr, "Failed to malloc() for channel buf\n");
          exit (EXIT_FAILURE);
        }

      memcpy (buf, channel->desc, chan_size);
      buf[chan_size] = '\0';

      scoped_refptr<nu::Button>
        channel_button (new nu::Button (buf, nu::Button::Type::Normal));
      channel_button->SetBackgroundColor (nu::Color (30, 145, 90));

      // Active channel, do special things.
      if (g_active_channel != NULL && !strcmp (channel->name, g_active_channel->name))
        {
           scoped_refptr<nu::Font> my_font (new nu::Font (font->GetName (), font->GetSize () * 1.5, nu::Font::Weight::ExtraBold, font->GetStyle ()));
           channel_button->SetFont (my_font.get ());
        }
      else
        {
           scoped_refptr<nu::Font> my_font (new nu::Font (font->GetName (), font->GetSize () * 1.0, nu::Font::Weight::Thin, font->GetStyle ()));
           channel_button->SetFont (my_font.get ());
        }

      // If we click channel, pop up that channel's text.
      channel_button->on_click.Connect
        ([channel_button, channel] (nu::Button *)
         {
           g_active_channel = channel;
           gui_chat_text_edit->SetText (channel_glue_reverse (channel->name, (char *) "\n"));

           // bump up font of active channel
           nu::App *app = nu::App::GetCurrent ();
           nu::Font *font = app->GetDefaultFont ();
           scoped_refptr<nu::Font> my_font (new nu::Font (font->GetName (), font->GetSize () * 1.5, nu::Font::Weight::ExtraBold, font->GetStyle ()));
           channel_button->SetFont (my_font.get ());
         });

      channel_container->AddChildView (channel_button.get ());
      channel_buttons[i] = channel_button.get ();

      printf ("Chan name: %s initialized (%s) with strlen of: %d!\n",
              channel->name, buf, strlen (channel->name));
    }

  free (buf);
  buf = NULL;

  gui_channel_scroll->SetContentView (channel_container.get ());

  return 0;
}

void *
gui_widget_update_loop (void *)
{
  // Option setting related
  for (;;)
    {
      sleep (5); // @todo This is a quick fix for needing to load all
                 // user data before channels.
      gui_lifetime->PostTask (populate_channel_scroll);
      sleep (600);
    }

  return 0;
}

int
slack_rtm_connect (char *token, nu::Lifetime *lifetime,
                   nu::TextEdit *chat_text_edit, nu::Scroll *channel_scroll)
{
  printf ("Using token: %s\n", token);
  SlackToken::token = token;

  // Bind to top level variables.
  slack_token = (char *) malloc (strlen (token) * sizeof (char) + 1);
  memcpy (slack_token, token, strlen (token));
  slack_token[strlen (token)] = '\0';

  gui_lifetime = lifetime;
  gui_chat_text_edit = chat_text_edit;
  gui_channel_scroll = channel_scroll;

  pthread_t t;
  pthread_create (&t, NULL, slack_rtm_connect_service_loop, (void *) lifetime);
  // pthread_join (t, NULL);

  // Create the polling interval for channel layout/display.
  pthread_t t_gui;
  pthread_create (&t_gui, NULL, gui_widget_update_loop, (void *) lifetime);

  return 0;
}

#endif

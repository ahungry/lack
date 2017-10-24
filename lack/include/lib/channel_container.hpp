/* Handles the channels in slack, such as tracking text sent to each
   of them and so on. */
#ifndef AHUNGRY_CHANNEL_CONTAINER_H
#define AHUNGRY_CHANNEL_CONTAINER_H

#include <iostream>
#include <string>

#include "nativeui/nativeui.h"

#include <getopt.h>
#include <json-c/json.h>
#include <json-c/json_tokener.h>
#include <libwebsockets.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "json_handler.hpp"
#include "ahungry_http_request.hpp"
#include "slack_sdk.hpp"

using namespace std;

typedef struct channel
{
  int idx;
  char *name; // Really, this is the id
  char *desc; // and this is the name. @todo fix it!
  int buflen;
  char **buf;
} channel_t;

typedef struct channel_container
{
  size_t len;
  channel_t **ptr;
} channel_container_t;

channel_container_t g_channel_container = { 0 };

int channel_history_fetch (channel *channel);
vector<string> channels_to_view;

void
set_channels_to_view (const char *csv_channels)
{
  int v = 0;
  string channel_name = "";

  for (int i = 0; i < strlen (csv_channels); i++)
    {
      if (',' == csv_channels[i])
        {
          // New channel if we find a comma.
          channels_to_view.push_back (channel_name);
          channel_name = "";
          v++;
          continue;
        }

      channel_name += csv_channels[i];
    }

  channels_to_view.push_back (channel_name);

  for (uint i = 0; i < channels_to_view.size (); i++)
    {
      cout << "channels in are: " << channels_to_view[i] << '\n';
    }
}

channel_t *
channel_get (char *name)
{
  printf ("Query for channel: %s\n", name);

  int i = 0;

  // Seek for a matching channel based on the name (id)
  for (; i < g_channel_container.len; i++)
    {
      if (!strcmp (name, g_channel_container.ptr[i]->name))
        {
          return g_channel_container.ptr[i];
        }
    }

  channel_t *channel = (channel_t *) calloc (sizeof (channel_t), 1 * sizeof (channel_t));

  if (NULL == channel)
    {
      fprintf (stderr, "calloc() fail\n");
      exit (EXIT_FAILURE);
    }

  channel->name = (char *) calloc (sizeof (char), (1 + strlen (name)) * sizeof (char));

  // Here, we can fetch the desc given the name (id).
  SlackSdk *sdk = new SlackSdk ();
  char *info = sdk->GetChannelsInfo (name);
  json_object *j = json_to_object (info);
  json_object *j_chan;
  char *chan_desc = NULL;

  cout << "-------------------------------" << info << " WAS THE JSON" << '\n';

  if (json_object_object_get_ex (j, "channel", &j_chan))
    {
      // If it is a channel, use the name, otherwise if im use user
      json_object *j_is_im = NULL;
      json_object_object_get_ex (j_chan, "is_im", &j_is_im);
      json_bool is_im = json_object_get_boolean (j_is_im);

      // json_object *j_is_im = json_object_object_get_ex (
      // Fill out from json response.
      if (is_im)
        {
          char *slack_user_id = json_get_string (j_chan, "user");
          slack_user_t *slack_user = slack_user_get ((char *) slack_user_id);
          chan_desc = slack_user->name;
        }
      else
        {
          chan_desc = json_get_string (j_chan, "name");
        }

      cout << "------------\n\n\n------------CHAN DESC " << chan_desc << '\n';
    }
  else
    {
      chan_desc = (char *) calloc (sizeof (char), (1 + strlen (name)) * sizeof (char));
      memcpy (chan_desc, name, strlen (name) + 1);
      cout << "------------\n\n\n------------MISSING CHAN DESC " << chan_desc << '\n';
    }

  channel->desc = (char *) calloc (sizeof (char), (1 + strlen (chan_desc)) * sizeof (char));

  if (NULL == channel->name || NULL == channel->desc)
    {
      fprintf (stderr, "calloc() fail\n");
      exit (EXIT_FAILURE);
    }

  memcpy (channel->name, name, 1 + strlen (name));
  memcpy (channel->desc, chan_desc, 1 + strlen (chan_desc));

  // Here, we need to add to buffer the channel history.
  channel->buf = NULL;
  channel_history_fetch (channel);

  if (NULL == g_channel_container.ptr)
    {
      g_channel_container.ptr = (channel_t **) calloc (sizeof (channel_t *), 1 * sizeof (channel_t *));

      if (NULL == g_channel_container.ptr)
        {
          fprintf (stderr, "calloc() fail\n");
          exit (EXIT_FAILURE);
        }
    }
  else
    {
      channel_t **tmp = (channel_t **) realloc (g_channel_container.ptr,
                                                (1 + i) * sizeof (channel_t *));

      if (NULL == tmp)
        {
          fprintf (stderr, "realloc() fail\n");
          exit (EXIT_FAILURE);
        }

      g_channel_container.ptr = tmp;
    }

  g_channel_container.ptr[i] = channel;
  g_channel_container.len++;

  return g_channel_container.ptr[i];
}

/* Like the other one, but without the explicit get */
int
channel_push_to_channel (char *name, char *buf, channel_t *chan)
{
  if (NULL == chan->buf)
    {
      chan->buf = (char **) malloc (1 * sizeof (char *));

      if (NULL == chan->buf)
        {
          fprintf (stderr, "malloc() fail\n");
          exit (EXIT_FAILURE);
        }
    }
  else
    {
      char **tmp = (char **) realloc (chan->buf, (1 + chan->buflen) * sizeof (char *));

      if (NULL == tmp)
        {
          fprintf (stderr, "realloc() fail\n");
          exit (EXIT_FAILURE);
        }

      chan->buf = tmp;
    }

  chan->buf[chan->buflen] = (char *) malloc ((1 + strlen (buf)) * sizeof (char));

  if (NULL == chan->buf[chan->buflen])
    {
      fprintf (stderr, "malloc() fail\n");
      exit (EXIT_FAILURE);
    }

  printf ("Copy buffer into chan->buf: %s %d\n", buf, strlen (buf));
  memcpy (chan->buf[chan->buflen++], buf, 1 + strlen (buf));

  return 0;
}

/* Push data into a channel, creating it if it doesn't exist. */
int
channel_push (char *name, char *buf)
{
  channel_t *chan = channel_get (name);

  return channel_push_to_channel (name, buf, chan);
}

char *
channel_glue (char *name, char *glue)
{
  channel_t *chan = channel_get (name);
  char *out = (char *) malloc (1 * sizeof (char));
  char *join = (char *) malloc (1 * sizeof (char));
  int i = 0;
  int chars = 0;
  int total = 0;
  int gluelen = strlen (glue);
  int joinlen = 0;

  join[0] = '\0';

  for (; i < chan->buflen; i++)
    {
      // Track the size we are copying in.
      chars = strlen (chan->buf[i]);

      out = (char *) realloc (out, (total + joinlen + chars) * sizeof (char));

      if (NULL == out)
        {
          fprintf (stderr, "realloc() fail\n");
          exit (EXIT_FAILURE);
        }

      memcpy (out + total, join, joinlen);
      memcpy (out + total + joinlen, chan->buf[i], chars);

      // And increment the total.
      total += (chars + joinlen);

      // Join starts as nothing, then switches to the glue set.
      if (i == 0)
        {
          join = (char *) realloc (join, (1 + gluelen) * sizeof (char));
          memcpy (join, glue, gluelen + 1);
          joinlen = gluelen;
        }
    }

  // At very end, add a trailing nul
  out = (char *) realloc (out, (total + 1) * sizeof (char));

  if (NULL == out)
    {
      fprintf (stderr, "realloc() fail\n");
      exit (EXIT_FAILURE);
    }

  out[total] = '\0';

  return out;
}

/* Glues together the buffer data, in reverse order (so we can go down based) */
char *
channel_glue_reverse (char *name, char *glue)
{
  channel_t *chan = channel_get (name);
  char *out = (char *) malloc (1 * sizeof (char));
  char *join = (char *) malloc (1 * sizeof (char));
  int i = chan->buflen - 1;
  int chars = 0;
  int total = 0;
  int gluelen = strlen (glue);
  int joinlen = 0;

  join[0] = '\0';

  for (; i >= 0; i--)
    {
      // Track the size we are copying in.
      chars = strlen (chan->buf[i]);

      out = (char *) realloc (out, (total + joinlen + chars) * sizeof (char));

      if (NULL == out)
        {
          fprintf (stderr, "realloc() fail\n");
          exit (EXIT_FAILURE);
        }

      memcpy (out + total, join, joinlen);
      memcpy (out + total + joinlen, chan->buf[i], chars);

      // And increment the total.
      total += (chars + joinlen);

      // Join starts as nothing, then switches to the glue set.
      if (i == chan->buflen - 1)
        {
          join = (char *) realloc (join, (1 + gluelen) * sizeof (char));
          memcpy (join, glue, gluelen + 1);
          joinlen = gluelen;
        }
    }

  // At very end, add a trailing nul
  out = (char *) realloc (out, (total + 1) * sizeof (char));

  if (NULL == out)
    {
      fprintf (stderr, "realloc() fail\n");
      exit (EXIT_FAILURE);
    }

  out[total] = '\0';

  return out;
}

/* Retrieve remote channel information from server. */
int
channel_fetch ()
{
  SlackSdk *sdk = new SlackSdk ();
  char *list = sdk->GetChannelsList ();

  printf ("Received channel list: %s", list);

  json_object *j = json_to_object (list);
  json_object *j_channels = NULL;

  json_object_object_get_ex (j, "channels", &j_channels);
  int j_chanlen = json_object_array_length (j_channels);

  // For each channel we got, push into the channel container
  // @todo if it's a channel we are configured to care about.
  for (int i = 0; i < j_chanlen; i++)
    {
      json_object *j_chan = json_object_array_get_idx (j_channels, i);
      char *jc_id = json_get_string (j_chan, "id");
      char *jc_name = json_get_string (j_chan, "name");

      // Before we fetch the channel, ensure it's one the user cares
      // about viewing.
      int view_p = 0;

      for (uint ctv = 0; ctv < channels_to_view.size (); ctv++)
        {
          if (!strcmp (channels_to_view[ctv].c_str (), jc_name)
              || !strcmp (channels_to_view[ctv].c_str (), "all"))
            {
              view_p = 1;
              break;
            }
        }

      if (!view_p)
        {
          cout << "Skipping adding channel " << jc_name << '\n';
          continue;
        }

      channel_t *o_chan = channel_get (jc_id);
      o_chan->desc = (char *) malloc ((strlen (jc_name) + 1) * sizeof (char));
      memcpy (o_chan->desc, jc_name, strlen (jc_name) + 1);
    }
}

/* Retrieve the history for the channel currently queried. */
int
channel_history_fetch (channel *channel)
{
  // Fire request for history of channel if we have to make a new one.
  SlackSdk *sdk = new SlackSdk ();
  string history = sdk->GetChannelsHistory (channel->name);

  if (history.size () < 1)
    {
      return 1;
    }

  cout << "Received channel history: " << history << '\n';
  json_object *j = json_to_object ((char *) history.c_str ());

  // If response was not ok, abort.
  json_object *j_ok = NULL;
  json_object_object_get_ex (j, "ok", &j_ok);
  json_bool ok = json_object_get_boolean (j_ok);

  if (!ok)
    {
      return 1;
    }

  json_object *j_messages = NULL;
  json_object_object_get_ex (j, "messages", &j_messages);
  int j_messagelen = json_object_array_length (j_messages);

  printf ("Have %d messages\n", j_messagelen);

  // for each message we got, push into the channel buffer
  for (int i = j_messagelen - 1; i >= 0; i--)
    {
      json_object *j_msg = json_object_array_get_idx (j_messages, i);
      char *user = json_get_string (j_msg, "user");
      char *text = json_get_string (j_msg, "text");
      slack_user_t *slack_user = slack_user_get ((char *) user);
      char *buf = (char *) calloc (sizeof (char), (6 + strlen (text) + strlen (user)) * sizeof (char));

      if (NULL == buf)
        {
          fprintf (stderr, "Failed to calloc() for buf.\n");
          exit (EXIT_FAILURE);
        }

      if (NULL == slack_user)
        {
          sprintf (buf, "<%s>: %s", user, text);
        }
      else
        {
          // May need more space for this one.
          buf = (char *) realloc (buf, (6 + strlen (text) + strlen (slack_user->name)) * sizeof (char));
          sprintf (buf, "<%s>: %s", slack_user->name, text);
        }

      printf ("Buf was: %s\n", buf);
      channel_push_to_channel (channel->name, buf, channel);
    }
}

#endif

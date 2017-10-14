/* Handles the channels in slack, such as tracking text sent to each
   of them and so on. */
#ifndef AHUNGRY_CHANNEL_CONTAINER_H
#define AHUNGRY_CHANNEL_CONTAINER_H

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

static const char *slack_uri_channel_list = "https://slack.com/api/channels.list?token=%s";

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

channel_t *
channel_get (char *name)
{
  printf ("Query for channel: %s\n", name);

  int i = 0;

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
  channel->desc = (char *) calloc (sizeof (char), (1 + strlen (name)) * sizeof (char));

  if (NULL == channel->name)
    {
      fprintf (stderr, "calloc() fail\n");
      exit (EXIT_FAILURE);
    }

  memcpy (channel->name, name, 1 + strlen (name));
  memcpy (channel->desc, name, 1 + strlen (name));
  channel->buf = NULL;

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

int
channel_push (char *name, char *buf)
{
  channel_t *chan = channel_get (name);

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
channel_fetch (char *slack_token)
{
  // May as well fetch channels here...
  // Lets clutter it all! haha
  // https://slack.com/api/channels.list?token=xoxs...
  char *uri = (char *) malloc ((strlen (slack_uri_channel_list) + strlen (slack_token)) * sizeof (char));
  sprintf (uri, slack_uri_channel_list, slack_token);
  Http *http = new Http (uri);
  http->Get ();

  // @todo Refactor into json_handler
  printf ("Received channel list: %s", http->Content ());
  json_object *j = json_to_object (http->Content ());
  json_object *j_channels = NULL;
  json_object_object_get_ex (j, "channels", &j_channels);
  int j_chanlen = json_object_array_length (j_channels);

  // for each channel we got, push into the channel container
  for (int i = 0; i < j_chanlen; i++)
    {
      json_object *j_chan = json_object_array_get_idx (j_channels, i);
      char *jc_id = json_get_string (j_chan, "id");
      char *jc_name = json_get_string (j_chan, "name");
      channel_t *o_chan = channel_get (jc_id);
      o_chan->desc = (char *) malloc ((strlen (jc_name) + 1) * sizeof (char));
      memcpy (o_chan->desc, jc_name, strlen (jc_name) + 1);
    }
}

#endif

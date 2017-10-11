#ifndef AHUNGRY_CHANNEL_CONTAINER_H
#define AHUNGRY_CHANNEL_CONTAINER_H

#include "nativeui/nativeui.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <libwebsockets.h>
#include <json-c/json.h>
#include <json-c/json_tokener.h>
#include <stdlib.h>

typedef struct channel
{
  int idx;
  char *name;
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
  int i = 0;

  for (; i < g_channel_container.len; i++)
    {
      if (!strcmp (name, g_channel_container.ptr[i]->name))
        {
          return g_channel_container.ptr[i];
        }
    }

  channel_t *channel = (channel_t *) malloc (1 * sizeof (channel_t));

  if (NULL == channel)
    {
      fprintf (stderr, "malloc() fail\n");
    }

  channel->name = (char *) malloc ((1 + strlen (name)) * sizeof (char));

  if (NULL == channel->name)
    {
      fprintf (stderr, "malloc() fail\n");
    }

  memcpy (channel->name, name, 1 + strlen (name));

  channel_t **tmp = (channel_t **) realloc (g_channel_container.ptr,
                                            (1 + i) * sizeof (channel_t *));

  if (NULL == tmp)
    {
      fprintf (stderr, "realloc() fail\n");
    }

  g_channel_container.ptr = tmp;

  g_channel_container.ptr[i] = channel;
  g_channel_container.len++;

  return g_channel_container.ptr[i];

}

int
channel_push (char *name, char *buf)
{
  channel_t *chan = channel_get (name);
  char **tmp = (char **) realloc (chan->buf, (1 + chan->buflen) * sizeof (char *));

  if (NULL == tmp)
    {
      fprintf (stderr, "realloc() fail\n");
    }

  chan->buf = tmp;

  chan->buf[chan->buflen] = (char *) malloc ((1 + strlen (buf)) * sizeof (char));

  if (NULL == chan->buf[chan->buflen])
    {
      fprintf (stderr, "malloc() fail\n");
    }

  memcpy (chan->buf[chan->buflen++], buf, 1 + strlen (buf));

  return 0;
}

char *
channel_glued (char *name, char *glue)
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

#endif

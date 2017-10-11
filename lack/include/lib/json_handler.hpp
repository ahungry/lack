#ifndef AHUNGRY_JSON_HANDLER_H
#define AHUNGRY_JSON_HANDLER_H

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

enum
  {
    SLACK_TYPE_UNKNOWN,
    SLACK_TYPE_MESSAGE,
    SLACK_TYPE_HELLO,
    SLACK_TYPE_PONG
  };

/*
 * Parse out the type from a message.
 *
 * {
 *   "type": "message",
 *   "channel": "A0AA00AA0",
 *   "user": "A0AAAA00A",
 *   "text": "test",
 *   "ts": "1507256050.000090",
 *   "source_team": "A0AAAA00A",
 *   "team": "A0AAAA00A"
 * }
 */
int
j_get_type (json_object *j)
{
  json_object *j_type = NULL;

  if (! json_object_object_get_ex (j, "type", &j_type))
    {
      fprintf (stderr, "No 'type' property exists, fail!\n");

      return SLACK_TYPE_UNKNOWN;
    }

  const char *type = json_object_get_string (j_type);

  printf ("Found type: %s\n", type);
  printf ("ENUM: %d %d %d", SLACK_TYPE_PONG, SLACK_TYPE_HELLO, SLACK_TYPE_MESSAGE);

  // presence change, reconnect_url
  if (!strcmp (type, "message")) return SLACK_TYPE_MESSAGE;
  if (!strcmp (type, "hello")) return SLACK_TYPE_HELLO;
  if (!strcmp (type, "pong")) return SLACK_TYPE_PONG;

  return SLACK_TYPE_UNKNOWN;
}

json_object *
json_to_object (char *json)
{
  json_tokener *tok = json_tokener_new ();
  json_object *j = NULL;
  int len = 0;
  enum json_tokener_error jerr;

  do
    {
      len = strlen (json);
      j = json_tokener_parse_ex (tok, json, len);
    }
  while ((jerr = json_tokener_get_error (tok)) == json_tokener_continue);

  if (jerr != json_tokener_success)
    {
      fprintf (stderr, "Messed up on len: %d string: %s\n", len, json);
      fprintf (stderr, "Error: %s\n", json_tokener_error_desc (jerr));
      exit (1);
      // handle error here
    }

  if (tok->char_offset < len)
    {
      // Handle extra chars after parsed object however we like
    }

  return j;
  // json_object *j = NULL;
  // j = json_tokener_parse (json);

  // return j;
}

#endif

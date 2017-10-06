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
      return SLACK_TYPE_UNKNOWN;
    }

  const char *type = json_object_get_string (j_type);

  /*
   * printf ("Found type: %s\n", type);
   * printf ("ENUM: %d %d %d", SLACK_TYPE_PONG, SLACK_TYPE_HELLO, SLACK_TYPE_MESSAGE);
   */

  if (!strcmp (type, "message")) return SLACK_TYPE_MESSAGE;
  if (!strcmp (type, "hello")) return SLACK_TYPE_HELLO;
  if (!strcmp (type, "pong")) return SLACK_TYPE_PONG;

  return SLACK_TYPE_UNKNOWN;
}

json_object *
json_to_object (char *json)
{
  json_object *j = NULL;
  j = json_tokener_parse (json);

  return j;
}

#endif

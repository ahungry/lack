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

// https://stackoverflow.com/questions/9907160/how-to-convert-enum-names-to-string-in-c
// https://gcc.gnu.org/onlinedocs/gcc-4.6.2/cpp/Stringification.html
// We can basically create a hash map of our types using define
#define FOREACH_SLACK_TYPE(SLACK_TYPE) \
  SLACK_TYPE(SLACK_TYPE_START) \
  SLACK_TYPE(unknown) \
  SLACK_TYPE(message) \
  SLACK_TYPE(hello) \
  SLACK_TYPE(pong) \
  SLACK_TYPE(presence_change) \
  SLACK_TYPE(reconnect_url) \
  SLACK_TYPE(flannel) \
  SLACK_TYPE(SLACK_TYPE_END)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum SLACK_TYPE_ENUM {
  FOREACH_SLACK_TYPE(GENERATE_ENUM)
};

static const char *SLACK_TYPE_STRING[] = {
  FOREACH_SLACK_TYPE(GENERATE_STRING)
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

      return unknown;
    }

  const char *type = json_object_get_string (j_type);

  printf ("Found type in j_get_type: %s\n", type);

  // Iterate through enum types, returning on match.
  for (int i = SLACK_TYPE_START; i != SLACK_TYPE_END; i++)
    {
      if (!strcmp (type, SLACK_TYPE_STRING[i]))
        {
          printf ("Message type is: %d, we found: %d\n", message, i);

          return i;
        }
    }

  return unknown;
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

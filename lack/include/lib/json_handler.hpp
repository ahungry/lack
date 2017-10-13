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
// Note in the type map, flannel is a type that includes a subtype (others may as well).
#define SLACK_TYPE_ENUM_PREFIX "SLACK_TYPE_"
#define FOREACH_SLACK_TYPE(TYPE) \
  TYPE(SLACK_TYPE_START) \
  TYPE(SLACK_TYPE_unknown) \
  TYPE(SLACK_TYPE_message) \
  TYPE(SLACK_TYPE_hello) \
  TYPE(SLACK_TYPE_pong) \
  TYPE(SLACK_TYPE_presence_change) \
  TYPE(SLACK_TYPE_reconnect_url) \
  TYPE(SLACK_TYPE_flannel) \
  TYPE(SLACK_TYPE_END)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum SLACK_TYPE_ENUM {
  FOREACH_SLACK_TYPE(GENERATE_ENUM)
};

static const char *SLACK_TYPE_STRING[] = {
  FOREACH_SLACK_TYPE(GENERATE_STRING)
};

// Define the subtypes (typically under flannel type).
#define SLACK_SUBTYPE_ENUM_PREFIX "SLACK_SUBTYPE_"
#define FOREACH_SLACK_SUBTYPE(SUBTYPE) \
  SUBTYPE(SLACK_SUBTYPE_unknown) \
  SUBTYPE(SLACK_SUBTYPE_user_query_response) \
  SUBTYPE(SLACK_SUBTYPE_END)

// Have subtype start where the other one ended (to avoid collisions).
enum SLACK_SUBTYPE_ENUM {
  SLACK_SUBTYPE_START = SLACK_TYPE_END + 1,
  FOREACH_SLACK_SUBTYPE(GENERATE_ENUM)
};

// This is sort of dumb...filling the string array with the type data to pad it up.
static const char *SLACK_SUBTYPE_STRING[] = {
  FOREACH_SLACK_TYPE(GENERATE_STRING)
  "SLACK_SUBTYPE_START",
  FOREACH_SLACK_SUBTYPE(GENERATE_STRING)
};

/* If the marker is blank, pagination starts at 0, otherwise pass in a
   next_marker value from another response */
char *
get_user_query_request_json (char *marker)
{
  const char *user_query_request = "{\"type\":\"flannel\", \"subtype\":\"user_query_request\", \"marker\":\"%s\"}";
  char *buf = (char *) calloc (sizeof (char), (strlen (marker) + strlen (user_query_request)) * sizeof (char));
  sprintf (buf, user_query_request, marker);

  return buf;
}

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

      return SLACK_TYPE_unknown;
    }

  const char *type = json_object_get_string (j_type);

  printf ("Found type in j_get_type: %s\n", type);

  // First, see if its a type with known sub-types.
  if (!strcmp (type, SLACK_TYPE_STRING[SLACK_TYPE_flannel] + strlen (SLACK_TYPE_ENUM_PREFIX)))
    {
      json_object *j_subtype = NULL;

      if (json_object_object_get_ex (j, "subtype", &j_subtype))
        {
          const char *subtype = json_object_get_string (j_subtype);

          printf ("subtype: %s\n", subtype);

          for (int i = SLACK_SUBTYPE_START; i != SLACK_SUBTYPE_END; i++)
            {
              printf ("IN SUBTYPE: %s vs %s and offset %d\n", subtype, SLACK_SUBTYPE_STRING[i], strlen (SLACK_SUBTYPE_ENUM_PREFIX));
              if (!strcmp (subtype, SLACK_SUBTYPE_STRING[i] + strlen (SLACK_SUBTYPE_ENUM_PREFIX)))
                {
                  return i;
                }
            }
        }
    }

  // Iterate through enum types, returning on match.
  for (int i = SLACK_TYPE_START; i != SLACK_TYPE_END; i++)
    {
      if (!strcmp (type, SLACK_TYPE_STRING[i] + strlen (SLACK_TYPE_ENUM_PREFIX)))
        {
          return i;
        }
    }

  return SLACK_TYPE_unknown;
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

/* Helper function to quickly get a key from the object. */
char *
json_get_string (json_object *j, const char *key)
{
  json_object *jobj = NULL;
  json_object_object_get_ex (j, key, &jobj);
  const char *val = json_object_get_string (jobj);

  return (char *) val;
}

typedef struct slack_user
{
  char *id;
  char *name;
  struct slack_user *next;
} slack_user_t;

slack_user_t dummy_user = { (char *) "0", (char *) "dummy", NULL };
slack_user_t *dummy = &dummy_user;

/* Find a user object based on the passed in id */
slack_user_t *
slack_user_get (char *id)
{
  for (slack_user_t *user = dummy; user != NULL; user = user->next)
    {
      if (!strcmp (id, user->id)) return user;
    }

  return NULL;
}

/* Add to the user list, always assume slack will give us a full list. */
int
slack_user_push (char *json)
{
  slack_user_t *current_user = dummy;
  json_object *j = json_to_object (json);
  json_object *j_results = NULL;

  if (! json_object_object_get_ex (j, "results", &j_results))
    {
      fprintf (stderr, "No 'results' property exists, fail!\n");

      exit (1);
    }

  // Shift to end of user chain.
  for (; current_user->next != NULL; current_user = current_user->next);

  int len = json_object_array_length (j_results);
  // array_list *results = json_object_get_array (j_results);

  // https://json-c.github.io/json-c/json-c-0.12.1/doc/html/json__object_8h.html#a5c9120a6d644ea12a61e2ec8520130c6
  for (int i = 0; i < len; i++)
    {
      json_object *j_user = json_object_array_get_idx (j_results, i);
      char *user_id = json_get_string (j_user, "id");

      if (slack_user_get (user_id) != NULL) continue;

      char *user_name = json_get_string (j_user, "name");

      slack_user_t *user = (slack_user_t *) calloc (sizeof (slack_user_t *),
                                                  1 * sizeof (slack_user_t *));
      user->id = (char *) user_id;
      user->name = (char *) user_name;
      user->next = NULL;

      printf ("Collected user name: %s, Id: %s\n", user->name, user->id);
      current_user->next = user;
      current_user = user;
    }

  return 0;
}

#endif

#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include <json-c/json_tokener.h>

#include "../json_handler.hpp"

int
assert_int (int line, const char *fn, const char *lbl, int expected, int received)
{
  if (expected != received)
    {
      fprintf
        (stderr,
         "%d:%s:%s Expected: %d, Received: %d\n",
         line, fn, lbl, expected, received);

      return 1;
    }

  return 0;
}

/* Test out various types of JSON object responses we may handle. */
int
test_get_types ()
{
  int res = 0;
  int result = 0;
  const char *json_type_unknown = "\"some-string\"";
  const char *json_type_message = "{\"type\": \"message\", \"a\": 1}";
  const char *json_type_pong = "{\"type\": \"pong\", \"a\": 1}";
  const char *json_type_flannel_user_query_response = "{\"type\": \"flannel\", \"subtype\": \"user_query_response\", \"a\": 1}";

  result = j_get_type (json_to_object ((char *) json_type_unknown));
  res += assert_int (__LINE__, __func__, "UNKNOWN", SLACK_TYPE_unknown, result);

  result = j_get_type (json_to_object ((char *) json_type_message));
  res += assert_int (__LINE__, __func__, "MESSAGE", SLACK_TYPE_message, result);

  result = j_get_type (json_to_object ((char *) json_type_pong));
  res += assert_int (__LINE__, __func__, "PONG", SLACK_TYPE_pong, result);

  result = j_get_type (json_to_object ((char *) json_type_flannel_user_query_response));
  res += assert_int (__LINE__, __func__, "USER_QUERY_RESPONSE", SLACK_SUBTYPE_user_query_response, result);

  return res;
}

int
test_get_names ()
{
  int res = 0;
  const char *json = "{\"type\":\"flannel\",\"subtype\":\"user_query_response\",\"ok\":true,\"next_marker\":\"jon smith\",\"results\":[{\"id\":\"USLACKBOT\",\"team_id\":\"omit\",\"name\":\"slackbot\",\"deleted\":false,\"color\":\"757575\",\"real_name\":\"slackbot\",\"tz\":null,\"tz_label\":\"PacificDaylightTime\",\"tz_offset\":-25200,\"profile\":{\"first_name\":\"slackbot\",\"last_name\":\"\",\"avatar_hash\":\"sv1444671949\",\"always_active\":true,\"display_name\":\"slackbot\",\"real_name\":\"slackbot\",\"real_name_normalized\":\"slackbot\",\"display_name_normalized\":\"slackbot\",\"fields\":null,\"team\":\"omit\"},\"is_admin\":false,\"is_owner\":false,\"is_primary_owner\":false,\"is_restricted\":false,\"is_ultra_restricted\":false,\"is_bot\":false,\"updated\":0,\"is_app_user\":false,\"presence\":\"active\"},{\"id\":\"AHU\",\"name\":\"Matthew Carter\"}]}";

  const char *json_next = "{\"type\":\"flannel\",\"subtype\":\"user_query_response\",\"ok\":true,\"next_marker\":\"jon smith\",\"results\":[{\"id\":\"ABC123\",\"team_id\":\"omit\",\"name\":\"alphabet\",\"deleted\":false,\"color\":\"757575\",\"real_name\":\"alphabet\",\"tz\":null,\"tz_label\":\"PacificDaylightTime\",\"tz_offset\":-25200,\"profile\":{\"first_name\":\"alpha\",\"last_name\":\"\",\"avatar_hash\":\"sv1444671949\",\"always_active\":true,\"display_name\":\"alpha\",\"real_name\":\"alpha\",\"real_name_normalized\":\"alpha\",\"display_name_normalized\":\"alpha\",\"fields\":null,\"team\":\"omit\"},\"is_admin\":false,\"is_owner\":false,\"is_primary_owner\":false,\"is_restricted\":false,\"is_ultra_restricted\":false,\"is_bot\":false,\"updated\":0,\"is_app_user\":false,\"presence\":\"active\"},{\"id\":\"XYZ123\",\"name\":\"Betalpha\"}]}";

  slack_user_push ((char *) json);
  slack_user_t *slackbot = slack_user_get ((char *) "USLACKBOT");

  if (strcmp ("USLACKBOT", slackbot->id))
    {
      fprintf (stderr, "Slackbot ID was wrong, saw :%s!!\n", slackbot->id);
      res++;
    }

  if (strcmp ("slackbot", slackbot->name))
    {
      fprintf (stderr, "Slackbot name was wrong, saw: %s!\n", slackbot->name);
      res++;
    }

  // we want to assert calling multiple times is just fine.
  slack_user_push ((char *) json);
  slack_user_push ((char *) json_next);
  slack_user_push ((char *) json);
  slack_user_t *ahu = slack_user_get ((char *) "AHU");

  if (strcmp ("AHU", ahu->id))
    {
      fprintf (stderr, "Ahu ID was wrong, saw :%s!!\n", ahu->id);
      res++;
    }

  if (strcmp ("Matthew Carter", ahu->name))
    {
      fprintf (stderr, "Ahu name was wrong, saw: %s!\n", ahu->name);
      res++;
    }

  slack_user_t *beta = slack_user_get ((char *) "XYZ123");

  if (strcmp ("XYZ123", beta->id))
    {
      fprintf (stderr, "Beta ID was wrong, saw :%s!!\n", beta->id);
      res++;
    }

  if (strcmp ("Betalpha", beta->name))
    {
      fprintf (stderr, "Beta name was wrong, saw: %s!\n", beta->name);
      res++;
    }

  return res;
}

int
test_get_user_query_request_json ()
{
  int res = 0;
  const char *exp = "{\"type\":\"flannel\", \"subtype\":\"user_query_request\", \"marker\":\"jon smith\"}";
  char *rec = get_user_query_request_json ((char *) "jon smith");

  if (strcmp (exp, rec))
    {
      fprintf (stderr, "Expected: %s, Received: %s\n", exp, rec);
      res++;
    }

  return res;
}

int
main (int argc, char *argv[])
{
  int res = 0;

  printf ("\n\nBegin tests...\n");

  res += test_get_names ();
  res += test_get_types ();
  res += test_get_user_query_request_json ();

  return res;
}

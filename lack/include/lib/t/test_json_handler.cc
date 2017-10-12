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
  assert_int (__LINE__, __func__, "UNKNOWN", unknown, result);

  result = j_get_type (json_to_object ((char *) json_type_message));
  assert_int (__LINE__, __func__, "MESSAGE", message, result);

  result = j_get_type (json_to_object ((char *) json_type_pong));
  assert_int (__LINE__, __func__, "PONG", pong, result);

  result = j_get_type (json_to_object ((char *) json_type_flannel_user_query_response));
  assert_int (__LINE__, __func__, "FLANNEL", flannel, result);

  return res;
}

int
main (int argc, char *argv[])
{
  int res = 0;

  printf ("\n\nBegin tests...\n");

  res += test_get_types ();

  return res;
}

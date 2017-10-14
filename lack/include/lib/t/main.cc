#include <stdlib.h>
#include <string.h>

#include "../ahungry_http_request.hpp"
#include "../slack_sdk.hpp"

int test_ahungry_http_request ()
{
  int res = 0;

  Http *http = new Http ("http://example.org");
  http->Get ();
  char *buf = http->Content ();

  // Just snag the title
  char *bufSlice = buf + 34;
  const char *e = "<title>Example Domain</title>";

  bufSlice[29] = '\0';

  if (strcmp(e, bufSlice))
    {
      fprintf (stderr, "Failed to match domain title:\n'%s' vs '%s'\n", e, bufSlice);
      res++;
    }

  printf ("buf was: %s\n", bufSlice);

  delete http;

  return res;
}

int test_slack_sdk_test ()
{
  int res = 0;

  SlackSdk *sdk = new SlackSdk ("fake");
  char *buf = sdk->GetTest ();

  const char *e = "{\"ok\":false,\"error\":\"invalid_auth\"}";
  if (strcmp (e, buf))
    {
      fprintf (stderr, "Failed to match slack response:\n'%s' vs '%s'", e, buf);
      res++;
    }

  printf ("buf was: %s\n", buf);

  delete sdk;

  return res;
}

int main ()
{
  int res = 0;

  printf ("\n\nBegin tests...\n");

  res += test_ahungry_http_request ();
  res += test_slack_sdk_test ();

  return res;
}

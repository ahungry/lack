#include <stdlib.h>
#include <string.h>

#include "../ahungry_http_request.hpp"

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

  printf ("buf was: %s", bufSlice);

  delete http;

  return res;
}

int main ()
{
  int res = 0;

  printf ("\n\nBegin tests...\n");

  res += test_ahungry_http_request ();

  return res;
}

#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include <json-c/json_tokener.h>

#include "../channel_container.hpp"

int
main (int argc, char *argv[])
{
  int res = 0;

  channel_push ((char *) "ABC123", (char *) "Hello World");
  channel_push ((char *) "XYZ456", (char *) "Bye World");
  channel_push ((char *) "ABC123", (char *) "I love programming!");
  channel_push ((char *) "ABC123", (char *) "it sure is fun");

  if (strcmp ("Hello World", channel_get ((char *) "ABC123")->buf[0]))
    {
      fprintf (stderr, "Failed to assert buf[0] = Hello World");
      res++;
    }

  if (strcmp ("I love programming!", channel_get ((char *) "ABC123")->buf[1]))
    {
      fprintf (stderr, "Failed to assert buf[1] = Hello World");
      res++;
    }

  printf ("Channel glue: %s\n", channel_glue ((char *) "ABC123", (char *) "..."));
  printf ("Channel glue: %s\n", channel_glue ((char *) "XYZ456", (char *) "\n"));
  printf ("Channel glue: %s\n", channel_glue ((char *) "ABC123", (char *) "\n"));

  if (strcmp ("Hello World...I love programming!...it sure is fun",
              channel_glue ((char *) "ABC123", (char *) "...")))
    {
      fprintf (stderr, "Failed to assert glue is working!");
      res++;
    }

  if (strcmp ("it sure is fun...I love programming!...Hello World",
              channel_glue_reverse ((char *) "ABC123", (char *) "...")))
    {
      fprintf (stderr, "Failed to assert glue is working!");
      res++;
    }

  return res;
}

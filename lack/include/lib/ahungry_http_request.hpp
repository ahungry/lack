#ifndef AHUNGRY_HTTP_REQUEST_H
#define AHUNGRY_HTTP_REQUEST_H

#include "nativeui/nativeui.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Network related
#include <curl/curl.h>

static char chatBuf[100000];

void *thread_fn(void *ptr);

class Http
{
  char *buf = NULL;
  //char buf[10000];
  const char *domain;

public:
  typedef struct cstring {
    char *ptr;
    size_t len;
  } cstring;

  Http (const char *domain);
  //~Http ();

  void Get ();
  char *Content ();

private:
  void InitString (Http::cstring *wt);

  // Adheres to curl callback signature
  static size_t Writefunc (void *ptr, size_t size, size_t nmemb, Http::cstring *s);
};

Http::Http (const char *domain)
{
  printf ("Start...");
  this->domain = domain;
}

size_t Http::Writefunc(void *ptr, size_t size, size_t nmemb, Http::cstring *s)
{
  size_t new_len = s->len + size*nmemb;

  // printf("slen: %d, new_len: %d", (int)s->len, (int)new_len);
  s->ptr = (char*)realloc(s->ptr, new_len + 1);

  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }

  // copy over the block size... makes sense
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

void
Http::InitString (Http::cstring *s)
{
  // @todo Yea yea, this should be done the c++ way with a 'new' keyword...
  s->len = 0;
  s->ptr = (char *) malloc (s->len + 1);

  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }

  s->ptr[0] = '\0';
}

void Http::Get ()
{
  CURL *curl;
  // CURLcode res;

  Http::cstring wt;
  this->InitString(&wt);

  curl = curl_easy_init();

  if (curl)
    {
      curl_easy_setopt(curl, CURLOPT_URL, this->domain);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, this->Writefunc);
      //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &wt);
    }

  // res = curl_easy_perform(curl);
  curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  this->buf = (char *) malloc (wt.len + 1);

  if (NULL == this->buf)
    {
      fprintf (stderr, "Failed to malloc() for this->buf in Http::Get\n");
      exit (EXIT_FAILURE);
    }

  memcpy (this->buf, wt.ptr, wt.len);
  this->buf[wt.len + 1] = '\0';
}

char *Http::Content ()
{
  return this->buf;
}

void *bye_label_ptr = NULL;

/* Set the ptr to reference the lifetime label. */
void
set_bye_label_ptr (void *ptr)
{
  bye_label_ptr = ptr;
}

int c = 0;

int
ahungry_http_task ()
{
  printf ("[%d] ahungry_http_task as a PostTask\n", c++);

  nu::Label *byeLabel = (nu::Label*) bye_label_ptr;

  const char *msg = "WOOT! %d";
  char *buf = (char *) malloc (sizeof (c) + strlen (msg));

  sprintf (buf, msg, c);
  byeLabel->SetText(buf);

  printf ("PostTask success!\n");

  return 0;
}

//void *thread_fn(void *ptr)
void*
thread_fn (void *ptr)
{
  sleep(1);

  nu::Lifetime *lifetime = (nu::Lifetime*) ptr;
  lifetime->PostTask (ahungry_http_task);

  char buf[100000];
  Http *http = new Http("http://example.org");

  printf ("Hello");

  http->Get();
  memcpy(buf, http->Content(), strlen(http->Content()));
  delete http;

  printf("Buflen is: %d\n", (int)strlen(buf));

  char *cur = (char*)chatBuf;
  size_t b_size = strlen(buf);
  size_t c_size = strlen(cur);

  // allocate just what we need
  char bufUp[c_size + b_size + 1];

  memcpy(bufUp, buf, b_size);
  bufUp[b_size] = '\n';
  memcpy(bufUp + b_size + 1, cur, c_size);
  bufUp[b_size + c_size + 1] = '\0';

  memcpy(chatBuf, bufUp, strlen(bufUp));
  //printf("Overall content is %s\n", chatBuf);

  //byeLabel->SetText(bufUp);

  // recurse, haha...
  pthread_t t;
  pthread_create(&t, NULL, thread_fn, ptr);
  pthread_join(t, NULL);

  /*
    for (;;)
    {
    sleep(1);
    //printf("%s \n", message);
    printf("%d\n", i++);
    char bufUp[10];
    sprintf(bufUp, "was: %d\n", i);
    byeLabel->SetText(bufUp);
    }

    return (void*)30;
  */
  return 0;
}
#endif

#ifndef AHUNGRY_HTTP_REQUEST_H
#define AHUNGRY_HTTP_REQUEST_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Network related
#include <curl/curl.h>

struct cstring {
  char *ptr;
  size_t len;
};

class Http
{
  char buf[10000];

public:
  typedef struct xstring {
    char *ptr;
    size_t len;
  } xstring;

  Http ();
  ~Http ();

  void Get ();
  void Content (char *buf);

private:
  void InitString (struct cstring *wt);

  // Adheres to curl callback signature
  size_t Writefunc (void *ptr, size_t size, size_t nmemb, struct cstring *s);
};

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct cstring *s)
{
  size_t new_len = s->len + size*nmemb;

  s->ptr = (char*)realloc(s->ptr, new_len + 1);

  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }

  memcpy(s->ptr + s->len, ptr, size*nmemb);

  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

void init_string(struct cstring *s) {
  s->len = 0;
  s->ptr = (char*)malloc(s->len + 1);

  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }

  s->ptr[0] = '\0';
}

void http_get (char *buf)
{
  //char *buf = this->buf;
  CURL *curl;
  CURLcode res;

  struct cstring wt;
  init_string(&wt);
  //this->InitString(&wt);

  curl = curl_easy_init();

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "http://ahungry.com");
    //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &Http::Writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &wt);
  }

  res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  memcpy(buf, wt.ptr, wt.len);
}

size_t Http::Writefunc(void *ptr, size_t size, size_t nmemb, struct cstring *s)
{
  size_t new_len = s->len + size*nmemb;

  printf("slen: %d, new_len: %d", s->len, new_len);
  //int *realloced = (int*) (realloc(s->ptr, new_len + 1));
  s->ptr = (char*)realloc(s->ptr, new_len + 1);
  //s->ptr = (char*)malloc(new_len + 1);

  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }

  // copy over the block size... makes sense
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  //printf("%s", s->ptr);

  return size*nmemb;
}

void Http::InitString (struct cstring *s)
{
  // @todo Yea yea, this should be done the c++ way with a 'new' keyword...
  s->len = 0;
  s->ptr = (char*)malloc(s->len + 1);

  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }

  s->ptr[0] = '\0';
}

void Http::Get ()
{
  char *buf = this->buf;
  CURL *curl;
  CURLcode res;

  struct cstring wt;
  init_string(&wt);
  //this->InitString(&wt);

  curl = curl_easy_init();

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "http://ahungry.com");
    //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &Http::Writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &wt);
  }

  res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  memcpy(buf, wt.ptr, wt.len);
}

void Http::Content (char *buf)
{
  memcpy(buf, this->buf, strlen(this->buf));
}

void *thread_fn(void *ptr);

static char chatBuf[100000];

void *thread_fn(void *ptr)
{
  sleep(1);
  // char *message;
  // message = (char *) ptr;
  // int i = 0;
  // nu::Label *byeLabel;
  // byeLabel = (nu::Label *) ptr;
  // scoped_refptr<nu::Label> byeLabel(new nu::Label("Goodbye World!"));

  char buf[100000];
  //sprintf(buf, "Clicked the button %d times!!\nYou sure are good at clicking!", ++clickCounter);
  http_get(buf);
  // Http *http = new Http();
  //Http *http = {};
  //http->Get();
  //http->Content(buf);

  printf("Buflen is: %d\n", (int)strlen(buf));
  //buf[strlen(buf)] = '\n';
  //buf[strlen(buf) + 0] = '\0';

  //std::cout << "The buffer was: \n" << buf << '\n' << " end buffer\n";
  // Append new stuff to the top
  //char *cur = (char*)byeLabel->GetText().c_str();
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
  printf("Overall content is %s\n", chatBuf);

  //byeLabel->SetText(bufUp);

  // recurse, haha...
  /*
  pthread_t t;
  pthread_create(&t, NULL, thread_fn, ptr);
  pthread_join(t, NULL);
  */


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

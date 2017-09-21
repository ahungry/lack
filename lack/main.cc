// http://libyue.com/docs/v0.2.0/cpp/api/button.html

#include "base/command_line.h"
#include "nativeui/nativeui.h"
#include "nativeui/menu_bar.h"
#include "nativeui/container.h"
#include "nativeui/button.h"
#include "nativeui/scroll.h"
#include "nativeui/text_edit.h"

// OS related
#include <iostream>
#include <string>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

// Network related
#include <curl/curl.h>

void *thread_fn(void *ptr);

static char chatBuf[100000];

struct cstring {
  char *ptr;
  size_t len;
};

// @todo Yea yea, this should be done the c++ way with a 'new' keyword...
void init_string(struct cstring *s) {
  s->len = 0;
  s->ptr = (char*)malloc(s->len + 1);

  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }

  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct cstring *s)
{
  size_t new_len = s->len + size*nmemb;

  //int *realloced = (int*) (realloc(s->ptr, new_len + 1));
  s->ptr = (char*)realloc(s->ptr, new_len + 1);

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

void getHttp(char *buf) {
  CURL *curl;
  CURLcode res;
  long lSize = 11024;
  // char *buffer;
  size_t result;

  struct cstring wt;
  init_string(&wt);

  // wt.ptr = buf;
  // wt.len = lSize;

  printf("Begin curl...\n");

  curl = curl_easy_init();

  if (curl) {
    printf("Stawt curl request...\n");
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:3001/version");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &wt);
    printf("End curl request...\n");
  }

  res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  // move the struct data into the buffer
  memcpy(buf, wt.ptr, wt.len);
  //printf("Buffer was; %s end buffer\n", buf);
}

#if defined(OS_WIN)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  base::CommandLine::Init(0, nullptr);
#else
int main(int argc, const char *argv[]) {
  base::CommandLine::Init(argc, argv);
#endif

  // Initialize the global instance of nativeui.
  nu::State state;

  // Create GUI message loop.
  nu::Lifetime lifetime;

  // Container to hold child elements.
  //nu::Container container;

  printf("Begin process...\n");

  scoped_refptr<nu::Container> container(new nu::Container());
  scoped_refptr<nu::Button> button(new nu::Button("Click me!", nu::Button::Type::Normal));
  scoped_refptr<nu::Label> byeLabel(new nu::Label("Goodbye World!"));
  scoped_refptr<nu::TextEdit> textEdit(new nu::TextEdit());
  scoped_refptr<nu::Scroll> scroll(new nu::Scroll);
  nu::Label *xLabel = new nu::Label("YEA");

  textEdit->SetStyle("flex", "1");
  textEdit->SetText("Hope this works!");

  printf("Text Editor text was: %s\n", textEdit->GetText().c_str());

  int clickCounter = 0;

  button->on_click.Connect([textEdit, byeLabel, &clickCounter](nu::Button*) {
      //char buf[100000];
      //sprintf(buf, "Clicked the button %d times!!\nYou sure are good at clicking!", ++clickCounter);
      //getHttp(buf);

      //printf("Buflen is: %d\n", (int)strlen(buf));
      //buf[strlen(buf)] = '\n';
      //buf[strlen(buf) + 0] = '\0';

      //std::cout << "The buffer was: \n" << buf << '\n' << " end buffer\n";
      // Append new stuff to the top
      /*
      char *cur = (char*)byeLabel->GetText().c_str();
      size_t b_size = strlen(buf);
      size_t c_size = strlen(cur);
      // allocate just what we need
      char bufUp[c_size + b_size + 1];

      memcpy(bufUp, buf, b_size);
      bufUp[b_size] = '\n';
      memcpy(bufUp + b_size + 1, cur, c_size);
      bufUp[b_size + c_size + 1] = '\0';
      */

      // byeLabel->SetText(bufUp);
      byeLabel->SetText(chatBuf);
      //textEdit->SetText(buf);
      //printf("Text Editor text was: %s\n", textEdit->GetText().c_str());
      // nu::Lifetime::GetCurrent()->Quit();
    });

  container->AddChildView(new nu::Label("Hello world"));

  scroll->SetStyle("flex", "1");
  scroll->SetContentSize(nu::SizeF(800,800));
  // scroll->SetContentView(textEdit.get());
  scroll->SetContentView(byeLabel.get());
  //scroll->SetContentView(xLabel);
  scroll->SetScrollbarPolicy(nu::Scroll::Policy::Automatic, nu::Scroll::Policy::Automatic);

  container->AddChildView(scroll.get());

  container->AddChildView(new nu::Label("Goodbye world"));
  container->AddChildView(button.get());

  // Create window with default options, and then show it.
  scoped_refptr<nu::Window> window(new nu::Window(nu::Window::Options()));
  window->SetContentSize(nu::SizeF(800, 800));
  window->SetContentView(container.get());
  window->Center();
  window->Activate();

  // Quit when window is closed.
  window->on_close.Connect([](nu::Window*) {
      nu::Lifetime::GetCurrent()->Quit();
    });

  // Fire off our own pthread
  pthread_t thread;
  int t_ret;

  const char *message = "Hello there...";
  // t_ret = pthread_create(&thread, NULL, thread_fn, (void*) message);

  // Enter message loop.
  // t_ret = pthread_create(&thread, NULL, thread_fn, (void*) byeLabel.get());
  //t_ret = pthread_create(&thread, NULL, thread_fn, (void*) xLabel);
  t_ret = pthread_create(&thread, NULL, thread_fn, NULL);
  lifetime.Run();

  return 0;
}


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
  getHttp(buf);

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

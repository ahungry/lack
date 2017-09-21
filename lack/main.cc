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

// Network related
#include <curl/curl.h>

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
    curl_easy_setopt(curl, CURLOPT_URL, "http://ahungry.com");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &wt);
    printf("End curl request...\n");
  }

  res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  // move the struct data into the buffer
  memcpy(buf, wt.ptr, wt.len);
  printf("Buffer was; %s end buffer\n", buf);
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

  nu::Container* container = new nu::Container();
  nu::Button* button = new nu::Button("Click me!", nu::Button::Type::Normal);

  // nu::Scroll* scroll = new nu::Scroll();
  nu::Label* byeLabel = new nu::Label("Goodbye World");
  //nu::TextEdit* textEdit = new nu::TextEdit();

  int clickCounter = 0;

  button->on_click.Connect([byeLabel, &clickCounter](nu::Button*) {
      char buf[100000];
      //sprintf(buf, "Clicked the button %d times!!\nYou sure are good at clicking!", ++clickCounter);
      getHttp(buf);

      std::cout << "The buffer was: \n" << buf << '\n' << " end buffer\n";

      byeLabel->SetText(buf);
      // nu::Lifetime::GetCurrent()->Quit();
    });

  // Hierarchy: window, container, scroll, textEdit
  // scroll->SetContentView(textEdit);
  // scroll->SetContentSize(nu::SizeF(400, 400));

  container->AddChildView(new nu::Label("Hello world"));
  container->AddChildView(byeLabel);
  container->AddChildView(new nu::Label("Goodbye world"));
  container->AddChildView(button);

  // Create window with default options, and then show it.
  scoped_refptr<nu::Window> window(new nu::Window(nu::Window::Options()));
  window->SetContentView(container);
  // window->SetContentView(new nu::Label("Hello world, goodbye world"));
  window->SetContentSize(nu::SizeF(800, 800));
  window->Center();
  window->Activate();

  // Quit when window is closed.
  window->on_close.Connect([](nu::Window*) {
      nu::Lifetime::GetCurrent()->Quit();
    });

  // Enter message loop.
  lifetime.Run();

  return 0;
}

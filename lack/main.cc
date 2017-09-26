// http://libyue.com/docs/v0.2.0/cpp/api/button.html

#include "base/command_line.h"
#include "nativeui/nativeui.h"
#include "nativeui/menu_bar.h"
#include "nativeui/container.h"
#include "nativeui/button.h"
#include "nativeui/scroll.h"
#include "nativeui/text_edit.h"
#include "nativeui/gfx/color.h"
#include "nativeui/gfx/font.h"

// OS related
#include <iostream>
#include <string>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

// Lack related
#include "lib/ahungry_http_request.hpp"

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
  textEdit->SetBackgroundColor(nu::Color(100, 100, 100));
  byeLabel->SetBackgroundColor(nu::Color(0, 0, 0));
  byeLabel->SetColor(nu::Color(255, 255, 255));

  // Change the font
  nu::App *app = nu::App::GetCurrent();
  nu::Font *font = app->GetDefaultFont();
  scoped_refptr<nu::Font> my_font(new nu::Font(font->GetName(), font->GetSize() * 2, font->GetWeight(), font->GetStyle()));
  byeLabel->SetFont(my_font.get());

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

  scroll->SetStyle("flex", 1, "flex-direction", "column");
  //scroll->SetContentSize(nu::SizeF(800,800));
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

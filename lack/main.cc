// http://libyue.com/docs/v0.2.0/cpp/api/button.html

#include "base/logging.h"
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
  //scoped_refptr<nu::Label> byeLabel(new nu::Label("Goodbye World!"));
  nu::Label *byeLabel = new nu::Label("Welcome to Lack...");
  //shared_refptr<nu::Label> byeLabel(new nu::Label("Goodbye World!"));

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

  int c = 0;

  container->on_draw.Connect ([&c] (nu::Container*, nu::Painter*, const nu::RectF)
                              {
                                printf ("In draw loop (%d)...\n", c++);

                                return false;
                              });

  button->on_click.Connect ([textEdit, byeLabel] (nu::Button*)
                            {
                              byeLabel->SetText(chatBuf);
                            });

  container->AddChildView(new nu::Label("Hello world"));

  scroll->SetStyle("flex", 1, "flex-direction", "column");
  //scroll->SetContentSize(nu::SizeF(800,800));
  // scroll->SetContentView(textEdit.get());

  //scroll->SetContentView(byeLabel.get());
  scroll->SetContentView(byeLabel);

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

  /**
  nu::Lifetime::GetCurrent()->on_ready.Connect([] {
      LOG(ERROR) << "OnReady";
    });
  */

  /*
  nu::Signal<void(const std::string&)> event;
  event.Connect([](const std::string&) {
      LOG(ERROR) << "OnEvent: " << arg;
    });

  event.Emit("Emitted");
  */

  // Fire off our own pthread
  pthread_t thread;
  int t_ret;

  const char *message = "Hello there...";
  // t_ret = pthread_create(&thread, NULL, thread_fn, (void*) message);

  // Enter message loop.
  // t_ret = pthread_create(&thread, NULL, thread_fn, (void*) byeLabel.get());
  //t_ret = pthread_create(&thread, NULL, thread_fn, (void*) xLabel);

  set_bye_label_ptr ((void*) byeLabel);
  t_ret = pthread_create (&thread, NULL, thread_fn, (void*) nu::Lifetime::GetCurrent());

  // Working empty thread
  //t_ret = pthread_create(&thread, NULL, thread_fn, NULL);

  // MAC only apparently...
  //lifetime.on_ready.Connect ([] { printf ("Lifetime is ready\n"); });

  //lifetime.PostTask (ahungry_http_task);
  lifetime.Run();

  return 0;
}

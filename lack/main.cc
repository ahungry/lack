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

  // Set up the main container and it's associated widgets (attach to window at end)
  scoped_refptr<nu::Container> main_container (new nu::Container ());
  main_container->SetStyle ("flex", 1, "flex-direction", "column", "align-items", "stretch");

  // Add the top banner to the container
  scoped_refptr<nu::Label> banner_label (new nu::Label ("Welcome to Lack!"));
  banner_label->SetStyle ("flex", 1, "justify-content", "center", "max-height", 50);
  main_container->AddChildView (banner_label.get ());

  // Create the chan_text_container area
  scoped_refptr<nu::Container> chan_text_container (new nu::Container ());
  chan_text_container->SetStyle ("flex", 1, "flex-direction", "row", "align-items", "stretch");
  main_container->AddChildView (chan_text_container.get ());

  // Create the interaction_container at the bottom
  scoped_refptr<nu::Container> interaction_container (new nu::Container ());
  interaction_container->SetStyle ("flex", 1, "flex-direction", "row", "max-height", 100, "align-items", "stretch");
  main_container->AddChildView (interaction_container.get ());

  // Now, add the channel_scroll and text_scroll parts to the chan_text_container
  scoped_refptr<nu::Scroll> channel_scroll (new nu::Scroll ());
  scoped_refptr<nu::Scroll> text_scroll (new nu::Scroll ());
  channel_scroll->SetStyle ("flex", 1, "flex-direction", "row", "align-items", "flex-start", "max-width", 200);
  text_scroll->SetStyle ("flex", 1, "flex-direction", "row");;
  channel_scroll->SetScrollbarPolicy(nu::Scroll::Policy::Automatic, nu::Scroll::Policy::Automatic);
  text_scroll->SetScrollbarPolicy(nu::Scroll::Policy::Never, nu::Scroll::Policy::Automatic);
  chan_text_container->AddChildView (channel_scroll.get ());
  chan_text_container->AddChildView (text_scroll.get ());

  // List of labels / buttons for the channel list, and add the container to the scroll
  scoped_refptr<nu::Container> channel_container (new nu::Container ());
  channel_container->SetStyle ("justify-content", "center");
  channel_container->AddChildView (new nu::Button ("First Channel", nu::Button::Type::Normal));
  channel_container->AddChildView (new nu::Button ("Second Channel", nu::Button::Type::Normal));
  channel_scroll->SetContentView (channel_container.get ());

  // Add the text to the text scroll area
  nu::Label *byeLabel = new nu::Label ("Welcome to Lack...");
  byeLabel->SetBackgroundColor (nu::Color(0, 0, 0));
  byeLabel->SetColor (nu::Color(255, 255, 255));
  byeLabel->SetStyle ("flex-direction", "row");

  // Change the font
  nu::App *app = nu::App::GetCurrent ();
  nu::Font *font = app->GetDefaultFont ();
  scoped_refptr<nu::Font> my_font (new nu::Font (font->GetName (), font->GetSize () * 2, font->GetWeight (), font->GetStyle ()));
  byeLabel->SetFont (my_font.get ());
  text_scroll->SetContentView (byeLabel);

  // Now, put some buttons in our bottom slot, the interaction_container
  scoped_refptr<nu::Entry> text_entry (new nu::Entry ());
  text_entry->SetStyle ("flex", 1);
  interaction_container->AddChildView (text_entry.get ());

  scoped_refptr<nu::Button> button (new nu::Button ("Send", nu::Button::Type::Normal));
  button->SetStyle ("max-width", 50);
  button->on_click.Connect ([byeLabel] (nu::Button*)
                            {
                              byeLabel->SetText (chatBuf);
                            });
  interaction_container->AddChildView (button.get ());

  scoped_refptr<nu::Button> help_button (new nu::Button ("Help", nu::Button::Type::Normal));
  help_button->SetStyle ("max-width", 50);
  interaction_container->AddChildView (help_button.get ());

  // scoped_refptr<nu::Container> container(new nu::Container());
  // //scoped_refptr<nu::Label> byeLabel(new nu::Label("Goodbye World!"));
  // //shared_refptr<nu::Label> byeLabel(new nu::Label("Goodbye World!"));

  // scoped_refptr<nu::TextEdit> textEdit(new nu::TextEdit());
  // scoped_refptr<nu::Scroll> scroll(new nu::Scroll);
  // nu::Label *xLabel = new nu::Label("YEA");

  // textEdit->SetStyle("flex", "1");
  // textEdit->SetText("Hope this works!");
  // textEdit->SetBackgroundColor(nu::Color(100, 100, 100));
  // printf("Text Editor text was: %s\n", textEdit->GetText().c_str());

  // int c = 0;

  // container->on_draw.Connect ([&c] (nu::Container*, nu::Painter*, const nu::RectF)
  //                             {
  //                               printf ("In draw loop (%d)...\n", c++);

  //                               return false;
  //                             });
  // container->AddChildView(new nu::Label("Hello world"));

  // scroll->SetStyle("flex", 1, "flex-direction", "row");
  // //scroll->SetContentSize(nu::SizeF(800,800));
  // // scroll->SetContentView(textEdit.get());

  // //scroll->SetContentView(byeLabel.get());
  // // scroll->SetContentView(byeLabel);

  // //scroll->SetContentView(xLabel);
  // scroll->SetScrollbarPolicy(nu::Scroll::Policy::Automatic, nu::Scroll::Policy::Automatic);

  // scoped_refptr<nu::Scroll> channel_scroll (new nu::Scroll);
  // channel_scroll->SetStyle ("flex", 1, "flex-direction", "row", "align-items", "stretch", "max-width", 100);
  // channel_scroll->SetContentView (channel_container.get ());

  // container->SetStyle ("flex-direction", "row");
  // container->AddChildView (channel_scroll.get ());
  // container->AddChildView (scroll.get ());

  // container->AddChildView(new nu::Label("Goodbye world"));
  // container->AddChildView(button.get());

  // Create window with default options, and then show it.
  scoped_refptr<nu::Window> window (new nu::Window (nu::Window::Options ()));
  window->SetContentSize (nu::SizeF (800, 800));
  window->SetContentView (main_container.get ());
  window->Center ();
  window->Activate ();

  // Quit when window is closed.
  window->on_close.Connect ([] (nu::Window*)
                            {
                              nu::Lifetime::GetCurrent ()->Quit ();
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
  lifetime.Run ();

  return 0;
}

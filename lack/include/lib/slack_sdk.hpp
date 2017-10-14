/* Serve as mapping for Slack endpoints */
#ifndef AHUNGRY_SLACK_SDK_H
#define AHUNGRY_SLACK_SDK_H

#include <iostream>
#include <string>
#include "ahungry_http_request.hpp"

using namespace std;

class SlackToken
{
public:
  static string token;
};

string SlackToken::token;

class SlackSdk
{
  const string root = "https://slack.com/api/";

public:
  SlackSdk ();
  ~SlackSdk ();

  char * GetTest ();
  char * GetChannelsList ();

private:
  string GenUrl (const string uri);
  char * HttpGetRequest (string uri);
  char * Get (string uri);
};

SlackSdk::SlackSdk ()
{
}

SlackSdk::~SlackSdk ()
{
}

string
SlackSdk::GenUrl (const string uri)
{
  return this->root + uri + "?token=" + SlackToken::token;
}

char *
SlackSdk::HttpGetRequest (string uri)
{
  cout << "Sending request to: " << uri << '\n';

  Http *http = new Http (uri.c_str ());
  http->Get ();

  char *buf = (char *) calloc (sizeof (char), (1 + strlen (http->Content())) * sizeof (char));
  memcpy (buf, http->Content (), 1 + strlen (http->Content ()));
  delete http;

  return buf;
}

char *
SlackSdk::Get (string uri)
{
  return this->HttpGetRequest (this->GenUrl (uri));
}

char *
SlackSdk::GetTest ()
{
  return this->Get ("api.test");
}

char *
SlackSdk::GetChannelsList ()
{
  return this->Get ("channels.list");
}

#endif /* end AHUNGRY_SLACK_SDK_H */

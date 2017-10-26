/* Serve as mapping for Slack endpoints */
#ifndef AHUNGRY_SLACK_SDK_H
#define AHUNGRY_SLACK_SDK_H

#include <vector>
#include <iostream>
#include <string>
#include "ahungry_http_request.hpp"
#include "urlencode.hpp"

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
  char * GetChannelsHistory (char *id);
  char * GetChannelsInfo (char *id);
  char * GetUsersInfo (char *id);
  char * ChatPostMessage (char *id, char *text);

private:
  string GenUrl (const string uri, vector<string> *args);
  char * HttpGetRequest (string uri);
  char * Get (string uri, vector<string> *args);
};

SlackSdk::SlackSdk ()
{
}

SlackSdk::~SlackSdk ()
{
}

string
SlackSdk::GenUrl (const string uri, vector<string> *args)
{
  string url = this->root + uri + "?token=" + SlackToken::token;

  if (NULL != args)
    {
      for (uint i = 0; i < args->size (); i += 2)
        {
          url += "&" + (*args)[i] + "=" + (*args)[i + 1];
        }
    }

  return url;
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
SlackSdk::Get (string uri, vector<string> *args)
{
  return this->HttpGetRequest (this->GenUrl (uri, args));
}

char *
SlackSdk::GetTest ()
{
  return this->Get ("api.test", NULL);
}

char *
SlackSdk::GetChannelsList ()
{
  vector<string> args = { "exclude_members", "true", "exclude_archived", "true" };

  return this->Get ("channels.list", &args);
}

char *
SlackSdk::GetChannelsHistory (char *id)
{
  string str (id);
  vector<string> args = { "channel", id, "count", "10" };

  return this->Get ("channels.history", &args);
}

char *
SlackSdk::GetChannelsInfo (char *id)
{
  string str (id);
  vector<string> args = { "channel", id };

  return this->Get ("conversations.info", &args);
}

char *
SlackSdk::GetUsersInfo (char *id)
{
  string str (id);
  vector<string> args = { "user", id };

  return this->Get ("users.info", &args);
}

char *
SlackSdk::ChatPostMessage (char *id, char *text)
{
  string s_id (id);
  string s_text (text);

  s_text = url_encode (s_text);

  vector<string> args = { "channel", id, "text", s_text };

  return this->Get ("chat.postMessage", &args);
}

#endif /* end AHUNGRY_SLACK_SDK_H */

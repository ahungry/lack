/* Serve as mapping for Slack endpoints */

#include <string>
#include "ahungry_http_request.hpp"

using namespace std;

static const char *slack_uri_channel_list = "https://slack.com/api/channels.list?token=%s";

class SlackSdk
{
  const string root = "https://slack.com/api/";
  const string uri_api_test = "api.test";
  const string uri_channels_list = "channels.list";
  string token;

public:
  SlackSdk (string token);
  ~SlackSdk ();

  char * GetTest ();
  char * GetChannelsList ();

private:
  string GenUrl (const string uri);
  char * HttpGetRequest (string uri);
  char * Get (string uri);
};

SlackSdk::SlackSdk (string token)
{
  this->token = token;
}

SlackSdk::~SlackSdk ()
{
}

string
SlackSdk::GenUrl (const string uri)
{
  return this->root + uri + "?token=" + this->token;
}

char *
SlackSdk::HttpGetRequest (string uri)
{
  Http *http = new Http (uri.c_str ());
  http->Get ();

  return http->Content ();
}

char *
SlackSdk::Get (string uri)
{
  return this->HttpGetRequest (this->GenUrl (uri));
}

char *
SlackSdk::GetTest ()
{
  return this->Get (this->uri_api_test);
}

char *
SlackSdk::GetChannelsList ()
{
  return this->Get (this->uri_channels_list);
}

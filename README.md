# Lack

A Slack GUI built using the Yue GUI framework (https://github.com/yue/yue)

# Purpose

To be a fast (non-bloated) Slack GUI.

# Usage

After cloning the repository, just run `./build.sh` to handle the
building process for you.

After this, try it out as such:

```sh
./build/lack xoxs-<your-slack-token-here> all # Run hooked to all channels
```

You can also run just against specific channels, as such (maybe your
org has hundreds and you only actively use 2 or so.

```sh
./build/lack xoxs-<your-slack-token-here> random,general # Run hooked to 2 channels
```

In either case, if you receive a direct message, a new channel will
open up automatically with the messaging user.

Speaking of messaging, you can open a channel with a fellow user as
such:

```sh
/msg <theirName> <your message>
```

Which will also pop open a new channel (sending them the initial message).

# Notes

This is *very* early/WIP, expect bugs, use at your own risk!

## Mac Support

### Requirements

* Brew

### How To

* Make sure Brew is up to date: `brew update`
* Install libwebsockets: `brew install websockets`
* Install cmake and such: `brew install autoconf automake` (may have to also install `cmake` itself)
* Install OpenSSL: `brew install openssl`
* Symlink the headers (otherwise they won't be seen) as regular user: cd /usr/local/include && ln -s ../opt/openssl/include/openssl .

Source for symlink matter: https://webcache.googleusercontent.com/search?q=cache:VmqURJIU6iAJ:https://solitum.net/openssl-os-x-el-capitan-and-brew/+&cd=1&hl=en&ct=clnk&gl=us

* Run `./build.sh`

# License

GPLv3

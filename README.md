# Lack

A Slack GUI built using the Yue (https://github.com/yue/yue)

# Purpose

To be a fast (non-bloated) Slack GUI.

# Note

This is *very* early/WIP, do not bother to use, just poking around the
framework for the most part.

Also - my C/C++ is a bit *LACK*ing (get it?  haha...how many puns can
we fit in here?).

# Building

At the moment the following libs will need to be installed (don't
worry, at some point their checks will be incorporated into the build
process and hopefully a real ./configure script/check).

- libwebsockets
- libcurl

# License

GPLv3

# Mac Support

## Requirements

* Brew

## How To

* Make sure Brew is up to date: `brew update`
* Install libwebsockets: `brew install websockets`
* Install cmake and such: `brew install autoconf automake` (may have to also install `cmake` itself)
* Install OpenSSL: `brew install openssl`
* Symlink the headers (otherwise they won't be seen) as regular user: cd /usr/local/include && ln -s ../opt/openssl/include/openssl .

Source for symlink matter: https://webcache.googleusercontent.com/search?q=cache:VmqURJIU6iAJ:https://solitum.net/openssl-os-x-el-capitan-and-brew/+&cd=1&hl=en&ct=clnk&gl=us

* Run `./build.sh`

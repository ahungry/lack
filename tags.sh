#!/bin/sh

find ./lack -type f -name "*.[ch]pp"  | etags -

#!/bin/sh

while sleep 30; do
    find ./lack -type f -name "*.[ch]pp"  | etags -
done

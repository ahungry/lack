#!/bin/sh

while sleep 1; do
    find ./lack -type f -name "*.[ch]pp"  | etags -
done

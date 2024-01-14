#!/bin/zsh
./nobuild run &
if [ $? -eq 0 ]; then
  fswatch -o ./src | xargs -n1 -I{} ./nobuild build
fi

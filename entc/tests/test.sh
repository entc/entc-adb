#!/bin/bash

/usr/bin/asciidoc --no-header-footer -o $1.html $1
echo $1.html

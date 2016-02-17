#!/bin/bash

git remote remove origin
git remote add origin lobo@10.8.8.1:/home/lobo/mirror/repositories/entc-adb
git push --all origin

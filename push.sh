#!/bin/bash

git remote remove origin
git remote add origin lobo@10.0.0.12:/home/lobo/mirror/projects/entc-adb
git push --all origin

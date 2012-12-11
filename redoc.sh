#!/bin/bash
cd gh-pages
git rm -rf *
cd ..
doxygen
cd gh-pages
git add -A
git commit -m"Update"
git push origin gh-pages

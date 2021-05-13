#!/bin/bash
pipenv install &&
pipenv run python step-to-ipt.py ./downloaded &&
rm -rf ../inventor/downloaded/ &&
mkdir "../inventor/downloaded/" &&
cp -a ./downloaded/. ../inventor/downloaded/ &&
cp -a ./existing/. ../inventor/downloaded/ &&
echo "DONE download and conversions.  Thanks for waiting."



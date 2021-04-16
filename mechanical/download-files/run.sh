#!/bin/bash
echo "Downloading required STEP files, please be very patient..." &&
rm -rf ./downloaded &&
mkdir "./downloaded" &&
pipenv install &&
pipenv run python ChromeDownloader.py &&
echo "DONE downloading STEP files, converting to IPT..." &&
pipenv run python step-to-ipt.py ./downloaded &&
rm -rf ../inventor/downloaded/ &&
mkdir "../inventor/downloaded/" &&
cp -a ./downloaded/. ../inventor/downloaded/ &&
cp -a ./existing/. ../inventor/downloaded/ &&
echo "DONE download and conversions.  Thanks for waiting."



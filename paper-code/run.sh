#!/bin/bash
echo "Recreating plots and outputs..." &&
pipenv install &&
pipenv run python calibration-analysis.py &&
echo "DONE."
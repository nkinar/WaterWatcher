#!/usr/bin/env python3

def strcmpi(str1, str2):
    """
    Case-insensitive string comparison routine
    """
    if str1.lower() == str2.lower():
        return True
    else:
        return False
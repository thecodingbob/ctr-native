#!/bin/sh
set -eu

exec python3 "$(dirname "$0")/tools/matching/match.py" "$@"

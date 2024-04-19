#!/bin/bash

bash toolchains/python/venv/create_venv.sh
source .venv/bin/activate
python3 toolchains/format/header_guard.py
deactivate

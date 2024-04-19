#!/bin/bash
set -e

REPO_ROOT=$(git rev-parse --show-toplevel)
THIS_FILE_DIR=$(dirname "$0")
VENV_DIR=$REPO_ROOT/.venv

python3 -m venv "$VENV_DIR"

source "$VENV_DIR/bin/activate"

pip install -r "$REPO_ROOT/$THIS_FILE_DIR/requirements.txt"

deactivate

echo "Virtual environment created at $VENV_DIR"

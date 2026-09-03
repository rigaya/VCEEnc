#!/bin/sh
set -eu

VERSION_FILE=${1:-VCECore/rgy_version.h}
VERSION=$(sed -n 's/^#define VER_STR_FILEVERSION[[:space:]]*"\([^"]*\)".*$/\1/p' "$VERSION_FILE" | tr -d '\r\n')
case "$VERSION" in
  ''|*[!0-9.]*)
    echo "バージョン定義を取得できません: ${VERSION_FILE}" >&2
    exit 1
    ;;
esac
printf '%s\n' "$VERSION"

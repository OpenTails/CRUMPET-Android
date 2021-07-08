#!/bin/bash
#originally a file from KStars

if [ $# -lt 1 ] ; then
  echo Usage: convert_translations.sh po_dir_location
  echo
  echo The script finds the downloaded translation files \(\*.po\) from the po subdirectory and it converts to
  echo mo format what can be loaded on Android directly.
  echo
  exit 1
fi

mkdir -p $1/locale
DIRS=$(ls -1 po)

echo Convert translations...
for dir in $DIRS; do
    # Don't convert the source pot file, let's just skip that one
    if [[ "$dir" == "digitail.pot" ]]; then
        continue
    fi
    NEW_DIR=$1/locale/$dir/LC_MESSAGES
    mkdir -p $NEW_DIR
    echo Convert po/$dir/digitail.po to $NEW_DIR/digitail.mo
    msgfmt po/$dir/digitail.po -o $NEW_DIR/digitail.mo
done

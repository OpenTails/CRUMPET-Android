#!/bin/sh
xgettext --c++ --kde --from-code=UTF-8 -ci18n  -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 -kki18n:1 -kki18nc:1c,2 -kki18np:1,2 -kki18ncp:1c,2,3 -kI18N_NOOP:1 -kI18NC_NOOP:1c,2 --copyright-holder="The Tail Company" --msgid-bugs-address="https://github.com/MasterTailer/CRUMPET/issues" `find . -name \*.qml -or -name \*.cpp` -o po/digitail.pot

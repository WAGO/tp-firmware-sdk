# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2018 WAGO Kontakttechnik GmbH & Co. KG

# call processing script if user selected a file

./wdialog "--infobox" "$TITLE" "Activate Uploads" " "  "Activation is in action - can take a few minutes..."  " "
./activate_download download-type="update-script"
ShowLastError


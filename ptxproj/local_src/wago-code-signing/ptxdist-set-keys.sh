#!/bin/bash
#-----------------------------------------------------------------------------#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2025 WAGO GmbH & Co. KG
#-----------------------------------------------------------------------------#

set -e

import_rauc_keys() {
	local r="update"
	cs_define_role "${r}"

	# SoftHSM use case
	cs_import_cert_from_pem "${r}" "${RAUC_CERT}"
	cs_import_key_from_pem "${r}" "${RAUC_KEY}"

	cs_append_ca_from_uri "${r}"
}

# SoftHSM use case
cs_init_softhsm
import_rauc_keys

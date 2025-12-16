//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
const form         = document.querySelector('form');
const center       = document.getElementsByClassName('center')[0]
const submitButton = form.querySelector('input[type=submit]');

const oldPassInput        = document.getElementById('old-password');
const oldPassError        = document.getElementById('old-password-error');
const newPassInput        = document.getElementById('new-password');
const newPassError        = document.getElementById('new-password-error');
const newPassConfirmInput = document.getElementById('new-password-confirmation');
const newPassConfirmError = document.getElementById('new-password-confirmation-error');

function checkNewPassConfirm() {
    let checkResult = true;
    newPassConfirmInput.setCustomValidity('');
    if (newPassConfirmInput.checkValidity()) {
        if (newPassInput.value != newPassConfirmInput.value) {
            checkResult = false;
            newPassConfirmInput.setCustomValidity('must match new password');
        }
    } else {
        checkResult = false;
    }
    return checkResult;
}

oldPassInput.addEventListener('change', () => {
    if (oldPassInput.validity.valid) {
        oldPassError.classList.add('hidden');
    }
    oldPassInput.reportValidity();
});
newPassInput.addEventListener('input', () => {
    if (newPassInput.validity.valid) {
        newPassError.classList.add('hidden');
    }
    newPassInput.reportValidity();
    // also check "password confirmation" which could become valid now
    if (!newPassConfirmError.classList.contains('hidden') //only when error is displayed
        && checkNewPassConfirm()) {
        newPassConfirmError.classList.add('hidden');
    }
});
newPassConfirmInput.addEventListener('change', () => {
    if (checkNewPassConfirm()) {
        newPassConfirmError.classList.add('hidden');
    }
    newPassConfirmInput.reportValidity();
});

oldPassInput.addEventListener('invalid', () => {
    oldPassError.classList.remove('hidden')
});
newPassInput.addEventListener('invalid', () => {
    newPassError.classList.remove('hidden')
});
newPassConfirmInput.addEventListener('invalid', () => {
    newPassConfirmError.classList.remove('hidden')
});

form.addEventListener('submit', (e) => {
    submitButton.disabled = true;
    if (!checkNewPassConfirm()) { // check custom validity
        e.preventDefault();
        submitButton.disabled = false;
        form.reportValidity();
        return false;
    }
    center.classList.add('sending');
    return true;
});

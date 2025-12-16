//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2023-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
const form         = document.querySelector('form')
const center       = document.getElementsByClassName('center')[0]
const submitButton = form.querySelector('input[type=submit]')

form.addEventListener('submit', () => {
  center.classList.add('sending')
  submitButton.disabled = true
}, {
  passive: true
})

const ua= navigator.userAgent;
if(ua.includes("QtWebEngine") && ua.includes("Wago"))
{
  let viewPort = document.getElementsByClassName('viewport')[0];
  viewPort.style.cssText = "user-select: none;";
}

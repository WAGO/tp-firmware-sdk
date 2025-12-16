//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

/**
 * @param {String} HTML representing a single node (which might be an Element,
                   a text node, or a comment).
 * @return {Node}
 */
function htmlToNode(html) {
    const template = document.createElement('template');
    template.innerHTML = html.trim();
    const nNodes = template.content.childNodes.length;
    if (nNodes !== 1) {
        throw new Error(`html parameter must represent a single node; got ${nNodes}.`);
    }
    return template.content.firstChild;
}

let messages_container = document.getElementById("messages-container");
/**
 * hides all message containers without any content and shows all other containers
 */
export function update_open_messages() {
    let message_is_open = false;
    let messages = document.querySelectorAll("#messages-container > div");
    for (let message of messages) {
        let message_content = message.getElementsByClassName("message-content")[0];
        if (message_content.innerText) {
            message_is_open = true;
            message.classList.remove("hidden");
        } else {
            message.classList.add("hidden");
        }
    }
    if (message_is_open) {
        messages_container.classList.remove("hidden");
    } else {
        messages_container.classList.add("hidden");
    }
}

// handle empty cancel / continue links by closing current tab (tab must have been opened by an other webpage programmatically)
let cancel_continue_links = document.querySelectorAll(".buttons a.cancel-link, .buttons a.continue-link")
for (let cancel_continue_link of cancel_continue_links) {
    // empty links are interpreted as closing tab
    if(cancel_continue_link.getAttribute("href") === "") {
        cancel_continue_link.innerText = "Close";

        // detect if closable by script as spezified in https://developer.mozilla.org/en-US/docs/Web/API/Window/close
        if(window.opener != null || window.history.length == 1){
            cancel_continue_link.addEventListener("click", function(e) {
                e.preventDefault();
                window.close();
                return false;
            });
        } else { // show that closing must be done by user
            cancel_continue_link.remove();
            messages_container.appendChild(htmlToNode(`
                <div class="info-message hidden">
                    <span class="message-content" taid="not-closable-message">
                        To return to the previous application close this tab.
                    </span>
                    <button class="close-message-btn" taid="close-info-message"></button>
                </div>
            `));
        }
    }
}

// open all messages with content 
update_open_messages();

// close button for message (deletes message content to automatically close it)
let message_close_buttons = document.getElementsByClassName("close-message-btn");
for (let button of message_close_buttons) {
    button.addEventListener("click", function (e) {
        e.preventDefault();
        button.parentElement.getElementsByClassName("message-content")[0].innerText = "";
        update_open_messages();
    });
}

// focus first text input field if not on WAGO panels
if(!navigator.userAgent.includes('Wago')) {
    let allTextinputs = document.querySelectorAll(".textfield input")
    if (allTextinputs.length != 0) allTextinputs[0].focus();
}

// highlight textinput field when label or field is selected (better touch panel usability)
let textinputfields = document.getElementsByClassName("textfield");
for (let field of textinputfields) {
    field.addEventListener("click", function (e) {
        e.preventDefault();
        field.getElementsByTagName("input")[0].focus();
    });
}

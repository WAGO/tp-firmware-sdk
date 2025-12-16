<?php
//------------------------------------------------------------------------------
// Copyright (c) 2023 WAGO GmbH & Co. KG
//
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//------------------------------------------------------------------------------

function get_access_token() {
    $auth_header_value = $_SERVER['HTTP_AUTHORIZATION'];
    $space_pos = strpos($auth_header_value, " ");
    $access_token_type = substr($auth_header_value, 0, $space_pos);
    $access_token_value = substr($auth_header_value, $space_pos + 1);
    return $access_token_value;
}

function verify_access_token($access_token) {
    $url = 'http://localhost/auth/verify';
    $data = array(
        'token' => $access_token
    );

    $options = array(
        'http' => array(
            'header'  => "Content-Type: application/x-www-form-urlencoded\r\n",
            'method'  => 'POST',
            'content' => http_build_query($data)
        )
    );
    $context  = stream_context_create($options);
    $result = file_get_contents($url, false, $context);
    if ($result === false) {
        return false;    
    }
    $json_result = json_decode($result, true);
    if ($json_result['active'] == true && str_contains($json_result['scope'], "mrs")) {
        return true;
    } else {
        return false;
    }
}


$access_token = get_access_token();
if (!verify_access_token($access_token)) {
    header('HTTP/1.1 401 Unauthorized');
    exit();
}

echo "Protected Resource!!!!";

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

function request_resource_server($access_token) {
    $url = 'http://localhost/mrs.php';

    $options = array(
        'http' => array(
            'header'  => "Authorization: Bearer $access_token\r\n",
            'method'  => 'GET'
        )
    );
    $context = stream_context_create($options);
    $result = file_get_contents($url, false, $context);
    return $result;
}


function retrieve_access_token($authorization_code, $code_verifier) {
    $url = 'http://localhost/auth/token';
    $data = array(
        'grant_type'    => 'authorization_code',
        'code'          => $authorization_code,
        'client_id'     => 'mcs',
        'code_verifier' => $code_verifier
    );

    $options = array(
        'http' => array(
            'header'  => "Content-Type: application/x-www-form-urlencoded\r\n",
            'method'  => 'POST',
            'content' => http_build_query($data)
        )
    );
    $context = stream_context_create($options);
    $result = file_get_contents($url, false, $context);
    var_dump($result);
    if ($result === false) {
        return false;    
    }
    $json_result = json_decode($result, true);
    if (isset($json_result['access_token'])) {
        $_SESSION['access_token'] = $json_result['access_token'];
        return true;
    }
    return false;
}

function base64url_encode($data) {
    return rtrim(strtr(base64_encode($data), '+/', '-_'), '=');
}

session_name('mcs');
session_start();


if (isset($_GET['error'])) {
    $error = $_GET['error'];
    $resource_server_response = "Error: $error";
    if (isset($_GET['error_description'])) {
        $error_description = $_GET['error_description'];
        $resource_server_response .= "\n$error_description";
    }
}
else if (!isset($_SESSION['access_token'])) {
    if (!isset($_GET['code'])) {

        $code_verifier = 'my_code_verifier';
        $_SESSION['code_verifier'] = $code_verifier;
        $code_challenge = base64url_encode(hash('sha256', $code_verifier, true));

        header("Location: /auth/authorize?client_id=mcs&response_type=code&scope=mrs&code_challenge_method=S256&code_challenge=$code_challenge");
        exit();
    }
    if (!retrieve_access_token($_GET['code'], $_SESSION['code_verifier'])) {
        header('HTTP/1.1 401 Unauthorized');
        exit();
    }
    header('Location: /mcs.php');
    exit();
}
else {
    $access_token = $_SESSION['access_token'];
    $resource_server_response = request_resource_server($access_token);
}

?>


<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>My Client Service</title>
</head>
<body>
<h1>My Client Service</h1>
<h2>Resource Server Response:</h2>
<textarea>
<?php echo $resource_server_response; ?>
</textarea>
</body>
</html>

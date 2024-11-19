#!/usr/bin/php

<?php

  $plcListRoot = '/var/www/plclist/';


  // loading moustache
  require  '/usr/share/mustache/src/Mustache/Autoloader.php';

  Mustache_Autoloader::register();

  // first parse the config file to an array
  try {
	$str_conf = file_get_contents('/etc/specific/plcselect.conf');
  }
  catch (Exception $e) {
    error_log('[PLC List]: error reading plcselect.conf file');
  }
  $lines_conf = preg_split('/\\r\\n|\\r|\\n/', $str_conf);
  $plc_settings = array();
  $plc_listdata = array();
  foreach($lines_conf as $elem) {
    $data = explode('=', $elem, 2);
    if($data[0]) {
      $plc_settings[$data[0]] = $data[1];
    }
  }
  $default = $plc_settings['plc_selected'];
  
  // first parse gesture.conf to an array
  try {
	$str_conf = file_get_contents('/etc/specific/gesture.conf');
  }
  catch (Exception $e) {
    error_log('[PLC List]: error reading gesture.conf file');
  }
  $lines_conf = preg_split('/\\r\\n|\\r|\\n/', $str_conf);
  $gesture_settings = array();
  $gesture_listdata = array();
  foreach($lines_conf as $elem) {
    $data = explode('=', $elem, 2);
    if($data[0]) {
      $gesture_settings[$data[0]] = $data[1];
    }
  }
  $wbm_allowed = $gesture_settings['btn0'];
  
  

  // 0: wbm, 
  $wbm_data = array(
          'url' => $plc_settings['url00'],
          'vkb' => $plc_settings['vkb00'],
          'mic' => $plc_settings['mic00'],
          'text' => $plc_settings['txt00'],
          'default' => (0 == $default)
        );
  // 1: plc-selection (ignore), 
  // 2-11: plc-entries
  for ($i = 2; $i <= 11; $i++) {
    $num_padded = sprintf("%02d", $i);
    if($i !== 1) {
      if (
			isset($plc_settings['url' . $num_padded]) &&
			$plc_settings['url' . $num_padded] &&
			$plc_settings['txt' . $num_padded]
		) {
        $microbrowser = $plc_settings['mic' . $num_padded];
        if ($microbrowser == 0)
        {
          //list without microbrowser entries
          $plc_listdata[] = array(
            'url' => $plc_settings['url' . $num_padded],
            'vkb' => $plc_settings['vkb' . $num_padded],
            'mic' => $plc_settings['mic' . $num_padded],
            'text' => $plc_settings['txt' . $num_padded],
            'default' => ($i == $default)
          );
        }
      }
    }
  }

  //hide wbm button if configured
  if ($wbm_allowed == 0)
  {
    $maindata = array(
      'entries' => $plc_listdata
    );
  }
  else
  {
    $maindata = array(
      'entries' => $plc_listdata,
      'wbm' => $wbm_data
    );
  }

  // instanciating mustache and rendering of the final result
  $mustache = new Mustache_Engine(array(
    'loader' => new Mustache_Loader_FilesystemLoader($plcListRoot . 'views'),
  ));


  //rendering and writing the final plc list
  $fh = fopen($plcListRoot . 'plclist.html', 'w');
  $dataToWrite = $mustache->render('main', $maindata);
  fwrite($fh, $dataToWrite);
  fflush($fh);
  fclose($fh);

?>

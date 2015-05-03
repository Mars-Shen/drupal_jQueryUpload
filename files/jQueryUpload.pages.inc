<?php

/**
 * @file
 * Page callback file for the performance review module.
 */

function jQueryUploadURL($variables) {
    dd("request========================");
    dd($_REQUEST);
    dd('files=======================');
    dd($_FILES);

    if (array_key_exists('files', $_FILES)) {

        global $base_url, $user;

        //check for folders
        $current_directory = getcwd();
        if (!file_exists($current_directory . '/' . variable_get('file_public_path', conf_path() . '/files') . '/file_uploads')) {
            $result = mkdir($current_directory . '/' . variable_get('file_public_path', conf_path() . '/files') . '/file_uploads');
        }
        if (!file_exists($current_directory . '/' . variable_get('file_public_path', conf_path() . '/files') . '/file_uploads/' . $user->uid)) {
            $result = mkdir($current_directory . '/' . variable_get('file_public_path', conf_path() . '/files') . '/file_uploads/' . $user->uid);
        }
        //TODO change this part to read from configuration
        $extensions = "gif jpeg jpg png tiff asf avi mpeg mpg wmv vob mov mp4 flv txt doc docx xls xlsx ppt pptx";
        $secure_file_name = file_munge_filename($_FILES['files']['name'][0], $extensions);

        $destination_uri = file_build_uri('file_uploads/' . $user->uid);

        
        $validators ['file_validate_extensions'] = array();
        $validators ['file_validate_extensions'][0] = $extensions;

        dd('user=================');
        dd($user);

        $ok = file_save_upload(0, $validators, $destination_uri);

        dd('$ok=======================');
        dd($ok);

       if ($ok) {
              // Create our message variables.
              $files = array('file' => $ok);
              // Make sure at least one module implements our hook.
              if (sizeof(module_implements('process_file')) > 0) {
                // Call all modules that implement the hook, and let them make changes to $variables.
                saveAndRetrieve_session(FILE_UPLOAD_KEY.$user->uid,0,'processing file...');
                $result = module_invoke_all('process_file', $files);
              }
              return drupal_json_output(array($files));
        }else {
            # code...
        }
    }
}

/**
*
*/
function get_process_bar_number(){
    global $user;
    $data = saveAndRetrieve_session(FILE_UPLOAD_KEY.$user->uid);
    $dataArr = explode("_", $data);
    $number = $dataArr[0];
    $message = $dataArr[1];
    if(is_numeric($number)){
        $number = $number>50?50:$number;
        $number = $number<0?0:$number;
        $jsonArr = array('num' => $number, 'msg' => $message);
        return drupal_json_output($jsonArr);
    }else{
        $jsonArr = array('num' => 0 , 'msg' => 'processing...');

        return drupal_json_output($jsonArr);
    }
}


function saveAndRetrieve_session($key, $percentage = NULL,$message = NULL) {
    $cache_key = md5(serialize(($key)));
    if($percentage||$message) {
      $value = $percentage.'_'.$message;
      cache_set($cache_key, $value);
    } else {
        if($cached = cache_get($cache_key)){
            $data = $cached->data;
            return $data;
        }
    }
}
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
        $extensions = "gif jpeg jpg png tiff asf avi mpeg mpg wmv vob mov mp4 flv txt doc docx xls xlsx ppt pptx zip";

        $destination_uri = file_build_uri('file_uploads/' . $user->uid);

        
        $validators ['file_validate_extensions'] = array();
        $validators ['file_validate_extensions'][0] = $extensions;

        dd('user=================');
        dd($user);

        $ok = file_save_upload_for_jquery_upload(0, $validators, $destination_uri);

        dd('$ok=======================');
        dd($ok);

       if ($ok['status']) {
              // Create our message variables.
              $files = array('file' => $ok['file']);
              // Make sure at least one module implements our hook.
              if (sizeof(module_implements('process_file')) > 0) {
                // Call all modules that implement the hook, and let them make changes to $variables.
                $result = module_invoke_all('process_file', $files);
              }
              $return_valArr =   array('status'=>'success','file'=>$file);
              saveAndRetrieve_session(FILE_UPLOAD_KEY.$user->uid,0,'processing file...');
              return drupal_json_output($return_valArr);
        }else {
            # code...
           $message_val = '';
           foreach($ok['message'] as $value){
               $message_val .= $value . '<br>';
           }
           dd('message...........................');
           dd($ok['message']);
           $return_valArr =   array('status'=>'error','msg'=>$message_val);
           dd('error...........................');
           return drupal_json_output($return_valArr);
        }
    }else{
        $return_valArr =   array('status'=>'error','msg'=>'error happended when upload file...');
        dd('no file error...........................');
        return drupal_json_output($return_valArr);
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
function file_save_upload_for_jquery_upload($form_field_name, $validators = array(), $destination = FALSE, $replace = FILE_EXISTS_RENAME) {
    global $user;
    static $upload_cache;
    $return_val_arr = array('status'=>false,'message'=>array());

    // Return cached objects without processing since the file will have
    // already been processed and the paths in _FILES will be invalid.
    if (isset($upload_cache [$form_field_name])) {
        return $upload_cache [$form_field_name];
    }

    // Make sure there's an upload to process.
    if (empty($_FILES ['files']['name'][$form_field_name])) {
        $return_val_arr['status'] = false;
        $return_val_arr['file'] = NULL;
        return $return_val_arr;
    }

    // Check for file upload errors and return FALSE if a lower level system
    // error occurred. For a complete list of errors:
    // See http://php.net/manual/features.file-upload.errors.php.
    switch ($_FILES ['files']['error'][$form_field_name]) {
        case UPLOAD_ERR_INI_SIZE:
        case UPLOAD_ERR_FORM_SIZE:
                $return_val_arr['status'] = false;
                $return_val_arr['file'] = NULL;
                $return_val_arr['message'][0] = t('The file %file could not be saved, because it exceeds %maxsize, the maximum allowed size for uploads.', array('%file' => $_FILES ['files']['name'][$form_field_name], '%maxsize' => format_size(file_upload_max_size())));
        return  $return_val_arr;

        case UPLOAD_ERR_PARTIAL:
        case UPLOAD_ERR_NO_FILE:
                $return_val_arr['status'] = false;
                $return_val_arr['file'] = NULL;
                $return_val_arr['message'][0] =t('The file %file could not be saved, because the upload did not complete.', array('%file' => $_FILES ['files']['name'][$form_field_name]));
        return  $return_val_arr;


        case UPLOAD_ERR_OK:
            // Final check that this is a valid upload, if it isn't, use the
            // default error handler.
            if (is_uploaded_file($_FILES ['files']['tmp_name'][$form_field_name])) {
                break;
            }

        // Unknown error
        default:
                $return_val_arr['status'] = false;
                $return_val_arr['file'] = NULL;
                $return_val_arr['message'][0] =t('The file %file could not be saved. An unknown error has occurred.', array('%file' => $_FILES ['files']['name'][$form_field_name]));
            return  $return_val_arr;

    }

    // Begin building file object.
    $file = new stdClass();
    $file->uid = $user->uid;
    $file->status = 0;
    $file->filename = trim(drupal_basename($_FILES ['files']['name'][$form_field_name]), '.');
    $file->uri = $_FILES ['files']['tmp_name'][$form_field_name];
    $file->filemime = file_get_mimetype($file->filename);
    $file->filesize = $_FILES ['files']['size'][$form_field_name];

    $extensions = '';
    if (isset($validators ['file_validate_extensions'])) {
        if (isset($validators ['file_validate_extensions'][0])) {
            // Build the list of non-munged extensions if the caller provided them.
            $extensions = $validators ['file_validate_extensions'][0];
        }
        else {
            // If 'file_validate_extensions' is set and the list is empty then the
            // caller wants to allow any extension. In this case we have to remove the
            // validator or else it will reject all extensions.
            unset($validators ['file_validate_extensions']);
        }
    }
    else {
        // No validator was provided, so add one using the default list.
        // Build a default non-munged safe list for file_munge_filename().
        $extensions = 'jpg jpeg gif png txt doc xls pdf ppt pps odt ods odp';
        $validators ['file_validate_extensions'] = array();
        $validators ['file_validate_extensions'][0] = $extensions;
    }

    if (!empty($extensions)) {
        // Munge the filename to protect against possible malicious extension hiding
        // within an unknown file type (ie: filename.html.foo).
        $file->filename = file_munge_filename($file->filename, $extensions);
    }

    // Rename potentially executable files, to help prevent exploits (i.e. will
    // rename filename.php.foo and filename.php to filename.php.foo.txt and
    // filename.php.txt, respectively). Don't rename if 'allow_insecure_uploads'
    // evaluates to TRUE.
    if (!variable_get('allow_insecure_uploads', 0) && preg_match('/\.(php|pl|py|cgi|asp|js)(\.|$)/i', $file->filename) && (substr($file->filename, -4) != '.txt')) {
        $file->filemime = 'text/plain';
        $file->uri .= '.txt';
        $file->filename .= '.txt';
        // The .txt extension may not be in the allowed list of extensions. We have
        // to add it here or else the file upload will fail.
        if (!empty($extensions)) {
            $validators ['file_validate_extensions'][0] .= ' txt';
            $return_val_arr['message'][] =t('For security reasons, your upload has been renamed to %filename.', array('%filename' => $file->filename));
        }
    }

    // If the destination is not provided, use the temporary directory.
    if (empty($destination)) {
        $destination = 'temporary://';
    }

    // Assert that the destination contains a valid stream.
    $destination_scheme = file_uri_scheme($destination);
    if (!$destination_scheme || !file_stream_wrapper_valid_scheme($destination_scheme)) {
        $return_val_arr['status'] = false;
        $return_val_arr['file'] = NULL;
        $return_val_arr['message'][] = t('The file could not be uploaded, because the destination %destination is invalid.', array('%destination' => $destination));
        return $return_val_arr;
    }

    $file->source = $form_field_name;
    // A URI may already have a trailing slash or look like "public://".
    if (substr($destination, -1) != '/') {
        $destination .= '/';
    }
    $file->destination = file_destination($destination . $file->filename, $replace);
    // If file_destination() returns FALSE then $replace == FILE_EXISTS_ERROR and
    // there's an existing file so we need to bail.
    if ($file->destination === FALSE) {
        $return_val_arr['status'] = false;
        $return_val_arr['file'] = NULL;
        $return_val_arr['message'][] = t('The file %source could not be uploaded because a file by that name already exists in the destination %directory.', array('%source' => $form_field_name, '%directory' => $destination));
        return $return_val_arr;
    }

    // Add in our check of the file name length.
    $validators ['file_validate_name_length'] = array();

    // Call the validation functions specified by this function's caller.
    $errors = file_validate($file, $validators);

    // Check for errors.
    if (!empty($errors)) {
        $message = t('The specified file %name could not be uploaded.', array('%name' => $file->filename));
        if (count($errors) > 1) {
            $message .= theme('item_list', array('items' => $errors));
        }
        else {
            $message .= ' ' . array_pop($errors);
        }
        form_set_error($form_field_name, $message);
        $return_val_arr['status'] = false;
        $return_val_arr['file'] = NULL;
        $return_val_arr['message'][] = $message;
        return $return_val_arr;
    }

    // Move uploaded files from PHP's upload_tmp_dir to Drupal's temporary
    // directory. This overcomes open_basedir restrictions for future file
    // operations.
    $file->uri = $file->destination;
    if (!drupal_move_uploaded_file($_FILES ['files']['tmp_name'][$form_field_name], $file->uri)) {
        form_set_error($form_field_name, t('File upload error. Could not move uploaded file.'));
        watchdog('file', 'Upload error. Could not move uploaded file %file to destination %destination.', array('%file' => $file->filename, '%destination' => $file->uri));
        $return_val_arr['status'] = false;
        $return_val_arr['file'] = NULL;
        $return_val_arr['message'][] = t('File upload error. Could not move uploaded file.');
        return $return_val_arr;
    }

    // Set the permissions on the new file.
    drupal_chmod($file->uri);

    // If we are replacing an existing file re-use its database record.
    if ($replace == FILE_EXISTS_REPLACE) {
        $existing_files = file_load_multiple(array(), array('uri' => $file->uri));
        if (count($existing_files)) {
            $existing = reset($existing_files);
            $file->fid = $existing->fid;
        }
    }

    // If we made it this far it's safe to record this file in the database.
    if ($file = file_save($file)) {
        // Add file to the cache.
        $return_val_arr['status'] = true;
        $return_val_arr['file'] = $file;
        $upload_cache [$form_field_name] = $return_val_arr;
        return $return_val_arr;
    }
    $return_val_arr['status'] = false;
    $return_val_arr['file'] = NULL;
    return $return_val_arr;
}
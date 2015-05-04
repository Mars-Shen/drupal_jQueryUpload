# jQueryUpload
This is a module for drupal7, add a customize form type which is called 'jQuery_upload', then we can upload file by jQuery upload's way

## Required
[jQuery Update module](https://www.drupal.org/project/jquery_update)


## How to use
In your form:
```
$form['image_example_image_fid'] = array(
                                          '#title' => t('title'),
                                          '#type' => 'jQuery_upload',
                                          '#description' => t('description'),
                                         );
```
In you module:

you can implement **hook_process_file** to process your own file process logic, in **hook_process_file** 
you can call **saveAndRetrieve_session(key,percentage,message)** to log you process percentage, 
it will show in the front page.
```
function  UploadBar_process_file($files){
    global $user;
    sleep(5);
    saveAndRetrieve_session(FILE_UPLOAD_KEY.$user->uid,30,'process file...');
    dd(saveAndRetrieve_session(FILE_UPLOAD_KEY.$user->uid));
    sleep(7);
    saveAndRetrieve_session(FILE_UPLOAD_KEY.$user->uid,40,'move to other place...');
    sleep(3);
    return true;
}
```

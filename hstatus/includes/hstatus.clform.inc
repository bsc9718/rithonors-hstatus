<?php

//works better than constants
function _hstatus_timetosave(){return '6 months';}

/**
 * Convenience function to build complearning form
 */

function hstatus_build_clform(){
    return drupal_get_form('hstatus_forms_cl',(func_num_args() > 0)?func_get_arg(0):'first');
}

/**
 * Complearning form
 */
function hstatus_forms_cl(&$form_state,$page){
    if(isset($form_state['storage']['data']['count'])){
        $data = $form_state['storage']['data'];
    }else{
        $data = _hstatus_clform_loaddata();
    }
    
    //DEBUG
    $form = array();
    $form['currentstate'] = array(
        '#prefix' => '<pre>',
        '#suffix' => '</pre>',
        '#value' => print_r($data,true),
    );
    
    //if we are coming from another page, we need to get it from the
    //storage rather than from the args.
    //we also need to store it back so that our page forward and page
    //back will work.
    $page = (isset($form_state['storage']['page']))?$form_state['storage']['page']:$page;
    $form_state['storage']['page'] = $page;
    
    //ensure that data is loaded if we are going to a page other than 'first'
    if(isset($data['count']) && $data['count'] > 0){
        $form_state['storage']['count'] = $data['count'];
    }else{
        //DEBUG, but possibly useful anyway...
        if($page != 'first')drupal_set_message(t('Number of activities unknown.  Going to first page'));
        $page = 'first';
    }
    
    $n = intval($page);
    //DEBUG
    drupal_set_message("rendering page $n ($page)");
    //if $page is a string, $n will be 0
    if($n > 0){
        //$form = array();
        $form['page'] = array(
            '#type' => 'value',
            '#value' => $n,
        );
        $form['heading'] = array(
            '#value' => '<h3>'.t('Activity !n',array('!n'=>$n)).'</h3>',
        );

        $form['name'] = array(
            '#type' => 'textarea',
            '#required' => true,
            '#title' => t('Activity Name'),
        );
        $form['desc'] = array(
            '#type' => 'textarea',
            '#required' => true,
            '#title' => t('Activity Description'),
        );
        $form['lead'] = array(
            '#type' => 'textarea',
            '#required' => true,
            '#title' => t('Leadership/Citizenship'),
        );
        $form['time'] = array(
            '#type' => 'textarea',
            '#required' => true,
            '#title' => t('Time Commitment'),
        );
        $form['contact'] = array(
            '#type' => 'textarea',
            '#required' => true,
            '#title' => t('Contact Information'),
        );
        $form['comment'] = array(
            '#type' => 'textarea',
            '#title' => t('Comments'),
        );
        $form['back'] = array(
          '#type' => 'submit',
          '#value' => t('< Prev Page'),
          '#id' => 'back',
        );
        $form['submit'] = array(
          '#type' => 'submit',
          '#value' => t('Next Page >'),
        );
        
        $form['name']['#default_value'] = $data[$n]['name'];
        $form['desc']['#default_value'] = $data[$n]['desc'];
        $form['lead']['#default_value'] = $data[$n]['lead'];
        $form['time']['#default_value'] = $data[$n]['time'];
        $form['contact']['#default_value'] = $data[$n]['contact'];
        $form['comment']['#default_value'] = $data[$n]['commment'];
        return $form;
    }
    
    if($page == 'last'){
        //last page
        $form['page'] = array(
            '#type' => 'value',
            '#value' => 'last',
        );
        $form['information'] = array(
            '#value' => '<p>'.t("Your responses have been saved as a draft.
            They will be stored for up to "._hstatus_timetosave()."
            If you are satisfied with your submission,
            you may submit the form now by clicking the button below.").'</p>',
        );
        $form['back'] = array(
          '#type' => 'submit',
          '#value' => t('< Prev Page'),
          '#id' => 'back',
        );
        $form['submit'] = array(
          '#type' => 'submit',
          '#value' => t('Done, Submit Now'),
        );
        return $form;
    }else{
        //first page -- acts as a catch-all, rather than returning a 404
        $form['page'] = array(
            '#type' => 'value',
            '#value' => 'first',
        );
        $form['count'] = array(
            '#type' => 'textfield',
            '#title' => t('Activities'),
            '#description' => t('Number of activities that compose this submission.'),
            '#required' => true,
            '#element_validate' => array('hstatus_form_cl_count_validate'),
            '#default_value' => (isset($data['count']))?$data['count']:1,
            //HTML5 attributes
            '#attributes' => array('type' => 'number', 'min' => '1'),
        );
        $form['back'] = array(
          '#type' => 'submit',
          '#value' => t('< Prev Page'),
          '#id' => 'back',
        );
        $form['submit'] = array(
          '#type' => 'submit',
          '#value' => t('Next Page >'),
        );
    }
    return $form;
}

/*
 * hstatus_form_cl_count_validate
 * 
 * Validates the count field on the first page of the form
 */
function hstatus_form_cl_count_validate($element, &$form_state){
    //activites has to be a positive number
	$v = trim($element['#value']);
    if(empty($v) || intval($v) < 1){
        //flags the field as incorrect
        form_error($element, t('You must have a positive integer number of activities.'));
    }
    //ensure it is actually an integer
    $integer = true;
    //probably faster than preg_match
    foreach(str_split($v) as $c){
        //if there are decimal points, they will not be numeric.
        $integer = is_numeric($c);
        if(!$integer){
            form_error($element, t('You must have a positive integer number of activities.'));
            return;
        }
    }
}

/*
 * hstatus_forms_cl_submit
 * 
 * Called whenever the form is submitted.
 * This function does not actually handle the data but instead passes
 * it on to helper functions.
 */

function hstatus_forms_cl_submit($form, &$form_state){
    $page = $form_state['values']['page'];
    $back = ($form_state['clicked_button']['#id'] == 'back');
    
    if(is_numeric($page)){
        _hstatus_clform_savepage($form_state,$back);
    }elseif($page == 'first'){
        _hstatus_clform_newentry($form_state);
    }elseif($page == 'last'){
        if($back){
            drupal_set_message('going back to '.$form_state['storage']['count']);
            _hstatus_clform_redirect($form_state,$form_state['storage']['count'],false);
        }else{
            _hstatus_clform_savefinal($form_state);
        }
    }else{
        _hstatus_clform_redirect($form_state,false,false);
    }
}

//====================================
//===== Private/Helper Functions =====
//====================================

//note the Drupal convention of making 
//"private" functions start with an underscore

/*
 * Helper function for the first page
 */
function _hstatus_clform_newentry(&$form_state){
    //DEBUG
    drupal_set_message('recording new entry');
    
    if(isset($form_state['storage']['data']['count'])){
        $data = $form_state['storage']['data'];
    }else{
        $data = _hstatus_clform_loaddata();
    }
    
    $n = $form_state['values']['count'];
    $form_state['storage']['count'] = $n;
    $data['count'] = $n;
    
    _hstatus_clform_saveandredirect($form_state,$data,1);
}

/*
 * Helper for all pages except first and last
 */

function _hstatus_clform_savepage(&$form_state,$back=false){
    
    $data = $form_state['storage']['data'];
    if(!isset($data['count'])){
        $data = _hstatus_clform_loaddata();
    }
    
    //DEBUG
    drupal_set_message('Loading data: '.($data)?'Success.':'Failure.');
    
    $vals = $form_state['values']; //note that PHP arrays are NOT assigned by reference
    $n = $vals['page'];
    unset($vals['page']);
    
    $data[$n] = $vals;
    
    file_put_contents('/home/regdoug/hstatus.savepage.log',print_r($data,true));
    
    $nextpage = $back?'<':'>';
    _hstatus_clform_saveandredirect($form_state,$data,$nextpage);
}

/*
 * Helper for last page
 */
function _hstatus_clform_savefinal(&$form_state){
    global $user;
    module_load_include('inc','hstatus','hstatus.db');
    if(hstatus_clsubmit($user->uid)){
        drupal_set_message(t('Complementary Learning sucessfully submitted.'));
        unset($form_state['storage']);
        unset($form_state['rebuild']);
        $form_state['redirect'] = '<front>';
    }else{
        drupal_set_message(t('Failed to submit Complementary Learning.'),'error');
        _hstatus_clform_redirect($form_state,'last',false);
    }
}

/*
 * Saves the $data passed to it and calls the redirect helper
 * with $page if it is successful.  If unsucessful, the redirect
 * helper is called with false.
 */

function _hstatus_clform_saveandredirect(&$form_state,$data,$page){
    global $user;
    module_load_include('inc','hstatus','hstatus.db');
    $success = hstatus_save_cldraft($user->uid,$data);
    if($success)$form_state['storage']['data'] = $data;
    $page = ($success)?$page:false;
    _hstatus_clform_redirect($form_state,$page);
} 

/*
 * The redirect helper.
 * 
 * Not actually a true redirect because the page doesn't change, but
 * will reload the form to the $page requested.  If $page is false,
 * the current page of the form is reloaded.
 */

function _hstatus_clform_redirect(&$form_state,$page='>',$message=true){
    if($page == '>' || $page == '<'){
        $n = $form_state['storage']['page'];
        //DEBUG
        drupal_set_message("current page: $n");
        if(is_numeric($n)){
            if($page == '>'){
                //DEBUG
                drupal_set_message('going forward');
                $page = ($n < $form_state['storage']['count'])?$n+1:'last';
            }else{
                //DEBUG
                drupal_set_message('going backward');
                $page = ($n > 1)?$n-1:'first';
            }
        }else{
            $page = false;
        }
    }
    
    //message could be better.    
    if($page){
        $form_state['storage']['page'] = $page;
        if($message){drupal_set_message(t('Data saved successfully'));}
    }else{
        $form_state['redirect'] = false;
        if($message){drupal_set_message(t('Failed to save data.'),'error');}
    }
}

//this gets done often enough that it needs it's own helper function
function _hstatus_clform_loaddata(){
    global $user;
    module_load_include('inc','hstatus','hstatus.db');
    return hstatus_load_cldraft($uid,_hstatus_timetosave());
}

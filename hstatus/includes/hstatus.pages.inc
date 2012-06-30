<?php
/* 
 * hstatus.pages.inc
 * 
 * Implements the page display functions for the hStatus module.
 * 
 * Forms are not included in this page.  For forms, see hstatus.forms.inc
 */

/**
 * Function defined by hstatus_menu to generate the page.
 * 
 * All it does is start drupal's theme system
 */
function hstatus_user_page($user){
    return theme('hstatus_mystatus',$user);
}

/**
 * Preprocess function for the theme hook 'hstatus_mystatus'.
 * 
 * It loads the database api from hstatus.db.inc and gets a user object
 */
function template_preprocess_hstatus_mystatus(&$variables){
    // this shouldn't happen unless a null value (or no value) is passed to
    // the theme function.  i.e. theme('hstatus_mystatus');
    if(is_null($variables['user'])){
        $variables['displaynone'] = true;
    }else{
        $username = $variables['user']->name;
        
        //loads the database API
        if(module_load_include('inc', 'hstatus', 'includes/hstatus.db') === FALSE){
            $variables['displaynone'] = true;
            return;
        }
        
        //gets a user object
        $huser = hstatus_load_user($username);
        // $huser will be false if the load failed.
        if($huser){
        
            //get all the variables first, then assign them to the $variables array
            
            $status = $huser->continuation['status'];
            switch($status){
                case 0: $status_str = 'Continuted';break;
                case 1: $status_str = 'Conditional Continuation';break;
                case 2: $status_str = 'Warning';break;
                case 3: $status_str = 'Withdrawn';break;
                case 4: $status_str = 'Graduated';break;
                case 5: $status_str = 'Withdrawn due to BS/MS';break;
                case 6: $status_str = 'Graduated with BS/MS';break;
                case 7: $status_str = 'Conditional Continuation (BS/MS)';break;
                default: $status_str = 'Unknown';
            }
            
            $points = $huser->points['total'];
            
            $cl = $huser->continuation['cl'];
            switch($cl){
                case 0: $cl_str = 'Pending'; break;
                case 1: $cl_str = 'Approved'; break;
                case 2: $cl_str = 'Rejected'; break;
                default: $cl_str = 'Unknown';
            };
            
            $fullname = ucwords(implode(' ',$huser->basic['name']));
            //Guaranteed Variables
            $variables['username'] = $username;
            $variables['status'] = $status;
            $variables['status_string'] = $status_str;
            $variables['points_total'] = $points;
            $variables['complearning_status'] = $cl;
            $variables['complearning_status_string'] = $cl_str;
            
            //Optional variables
            $variables['name'] = $fullname;
            $variables['gpa'] = $huser->basic['gpa'];
            $variables['year'] = $huser->basic['year'];
            $variables['college'] = $huser->basic['college'];
            $variables['department'] = $huser->basic['department'];
            $variables['program'] = $huser->basic['program'];
            
        }else{ //the load failed.
            //signal that the display should be nothing
            $variables['displaynone'] = true;
        }
    }
}

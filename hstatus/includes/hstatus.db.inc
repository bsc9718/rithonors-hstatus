<?php
/**
 * @file hstatus.db.inc
 * Manages the database queries for the hstatus module.
 * 
 * I chose to put this in a separate file for two reasons.
 *  - It keeps the code cleaner in the other files.
 *  - It gives more flexibility if we should change the db structure.
 */

/**
 * hstatus_load_user
 * 
 * Arguments:
 *  1. $username - RIT DCE
 *  2. $loadCourses - NOT YET SUPPORTED
 *  3. $loadSubmissions - NOT YET SUPPORTED
 *  4. $reset - forces loading of a new user object instead of using the cache.
 * 
 * Loads an hstatus user object.  This function is very complex because it
 * uses a smart caching system to improve performance.
 * 
 * Structure
 *      -basic
 *          -name
 *              -firstname
 *              -lastname
 *          -active
 *          -college
 *          -department
 *          -program
 *          -gpa
 *      -continuation
 *          -status
 *          -cl
 *          -college
 *          -courses
 *          -gpa
 *      -points
 *          -points
 *          -waivers
 *          -courses
 *          -total
 *      -courses (optional, NOT YET SUPPORTED)
 *          array of (course, name, professor, quarter, grade, credits, honors)
 *      -submissions (optional, NOT YET SUPPORTED)
 *          -to be determined
 */
 
function hstatus_load_user($username, $loadCourses = false, $loadSubmissions = false, $reset = false){
    //allows short-term caching (within each page load)
    static $husers;
    
    /*check the short term cache in the static variable $husers.
    * The decision making process is as follows:
    *   1. check if caches should be used at all from the $reset variable.
    *   2. check to see if the user is available in the array
    *   3. if we don't have to load courses or submissions then it is guaranteed
    *       to be everything that we need in the object.
    */
    if(!$reset && isset($husers[$username]) 
            && !($loadCourses || $loadSubmissions)){
        return $husers[$username];
    }
    
    //Query Definitions 
    $q_basic = 
    "SELECT 
        student.firstname fname, 
        student.lastname lname, 
        student.college college, 
        student.department dpt, 
        student.program program, 
        student.active active, 
        gpas.gpa gpa
     FROM {h_students} student
     JOIN {h_students_gpa} gpas
        ON student.username = gpas.username
     WHERE student.username = '%s'
     LIMIT 1";
    $q_pts = "SELECT username, sum(numPoints) total
                FROM {h_students_points}
                WHERE username = '%s'
                GROUP BY username";
    $q_cpts = "SELECT username, sum(credits) total
                FROM {h_students_courses}
                WHERE username = '%s'
                GROUP BY username";
    $q_wpts = "SELECT username, sum(credits) total
                FROM h_students_waivers
                WHERE username = '%s'
                GROUP BY username";
    $q_continue = 
    "SELECT status, comp_learn, college, courses, gpa
     FROM {h_continuation}
     WHERE username = '%s'
     LIMIT 1";
    $q_courses = 
    "SELECT course, name, professor, quarter, grade, credits, honors
     FROM {h_students_courses} courses
     WHERE courses.username = '%s'
     LIMIT 1";
    $q_submissions = ""; //TODO: write this.
    
    // is it in the short term cache?
    if(!$reset && isset($husers[$username])){
        $u = $husers[$username];
    // is it in the long term (database) cache?
    } else if(!$reset 
            && ($cache = cache_get("hstatus_user:$username",'cache_honors')) 
            && !empty($cache->data)) {
        // all cached data in the db cache is serialized.
        $u = unserialize($cache->data);
    // fetch new data.
    // Note that this will fail if the user lacks an entry in either the h_students
    // or the h_continuation table.
    } else {
        // Fetch Basic Info
        $rs = db_query($q_basic, $username,$username,$username,$username);
        $arry = db_fetch_array($rs);
        if($arry){
            $u['basic'] = array(
                'name' => array(
                    'firstname' => $arry['fname'],
                    'lastname' => $arry['lname'],
                ),
                'active' => $arry['active'],
                'college' => $arry['college'],
                'department' => $arry['dpt'],
                'program' => $arry['program'],
                'gpa' => $arry['gpa'],
            );
        }else{
            //DEBUG, but we should maybe return error statuses instead of just 'false' 
            drupal_set_message(t('@username is not in Honors',array('@username'=>$username)),'warning');
            return false; 
        }
        
        // Fetch Continuation Info
        $rs = db_query($q_continue, $username);
        $arry = db_fetch_array($rs);
        if($arry){
            $u['continuation'] = array(
                'status' => $arry['status'],
                'cl' => $arry['comp_learn'],
                'college' => $arry['college'],
                'courses' => $arry['courses'],
                'gpa' => $arry['gpa'],
            );
        }else{
            //DEBUG, but we should maybe return error statuses instead of just 'false'
            drupal_set_message(t('@username has no continuation entry',array('@username'=>$username)),'warning');
            return false;
        }
        
        // Fetch Points
        // All of this might get changed later if we add columns to the h_students
        // table that store this information (those would be updated via cron)
        $u['points'] = array(
            'points' => 0,
            'courses' => 0,
            'waivers' => 0,
            'total' => 0,
        );
        //keep track of total points
        $total = 0;
        //points from point submissions like trips, research, etc
        $rs = db_query($q_pts, $username);
        $arry = db_fetch_array($rs);
        if($arry){
            $u['points']['points'] = $arry['total'];
            $total += $arry['total'];
        }
        //points from courses
        $rs = db_query($q_cpts, $username);
        $arry = db_fetch_array($rs);
        if($arry){
            $u['points']['courses'] = $arry['total'];
            $total += $arry['total'];
        }
        //points from waivers
        $rs = db_query($q_wpts, $username);
        $arry = db_fetch_array($rs);
        if($arry){
            $u['points']['waivers'] = $arry['total'];
            $total += $arry['total'];
        }
        //total points
        $u['points']['total'] = $total;
        
        //store data in cache with 2 hour lifetime.
        //this comes before the db queries for courses and submissions because we
        //don't cache the courses and submissions because they aren't always loaded.
        cache_set("hstatus_user:$username", serialize($u), 'cache_honors', time() + 2*60*60);
    }
    
                    // below needed in case of static variable load
    if($loadCourses && ($reset || !isset($u['courses']))){
        //TODO: implement
    }
    
    if($loadSubmissions && ($reset || !isset($u['submissions']))){
        //TODO: implement
            // Date, status, and link should be sufficient info.
            // 1. 'complearn' => submitted to h_complearning.  This year only.
            // 2. 'cldrafts' => h_cldrafts submissions.  link goes to form.  not expired only.
            // 3. 'points' => ???
    }
    
    $husers[$username] = $u;  //update static variable for future calls

    return (object) $u;
}

/**
 * hstatus_load_cldraft
 * 
 * @param $uid The Drupal user id to search for
 * @param $expire A time length (to be passed to strtotime) specifying the
 *  maximum age of the data.  Defaults to 1 year
 * 
 * this fails and returns false if the data cannot be loaded, if it is
 * older than the expire length or if $data['count'] is not set.
 */
function hstatus_load_cldraft($uid, $expire = '1 year'){
    //basic caching
    static $cldrafts;
    if(isset($cldrafts[$uid]) && isset($cldrafts[$uid]['count'])){
        return $cldrafts[$uid];
    }
    
    $q = 'SELECT updatetime, data FROM {h_cldrafts} WHERE uid=%d';
    $rs = db_query($q,$uid);
    $obj = db_fetch_object($rs);
    
    if($obj && $obj->updatetime > strtotime('-'.trim($expire))){
        $data = unserialize($obj->data);
        if(isset($data['count'])){
            $cldrafts[$uid] = $data;
            return $data;
        }
    }
    return false;
}

/**
 * hstatus_save_cldraft
 * 
 * Saves $data to the h_cldrafts db table.
 */
function hstatus_save_cldraft($uid,$data){
    //basic caching
    static $cldrafts;
    $cldrafts[$uid] = $data;
    
    $check = "SELECT uid FROM {h_cldrafts} WHERE uid=%d";
    $rs = db_query($check,$uid);
    $obj = (object) array('uid'=>$uid,'updatetime'=>time(),'data'=>serialize($data));
    if($rs && db_fetch_array($rs)){
        return drupal_write_record('h_cldrafts',$obj,'uid');
    }else{
        //return drupal_write_record('h_cldrafts',$obj);
        db_query('INSERT INTO {h_cldrafts} (uid,updatetime,data) VALUES (%d,%d,%b)',
                $obj->uid,$obj->updatetime,$obj->data);
        return true;
    }
}

/*
 * hstatus_clsubmit
 * 
 * Saves a draft to the h_complearning database
 */
function hstatus_clsubmit($uid){
    $user = user_load($uid);
    $huser = hstatus_load_user($user->name);
    if(!$huser){
        drupal_set_message('Failed to load your profile.','error');
        return false;
    }elseif($huser->continuation['comp_learn'] == 1){
        drupal_set_message(t('Not submitted. You have already completed Comp Learning for this year.'),'warning');
        return false;
    }elseif($huser->continuation['status'] > 2){
        drupal_set_message(t('Not submitted. You are not active in the Honors Program'),'warning');
        return false;
    }elseif(isset($huser->submissions['complearn']) && is_array($huser->submissions['complearn'])){
        foreach($huser->submissions['complearn'] as $sub){
            if($sub['status'] == -2){
                drupal_set_message(t('Not submitted. You already have a pending submission.'),'warning');
                return false;
            }
        }
    }
    
    $data = hstatus_load_cldraft($uid);
    
    $n = isset($data['count'])?intval($data['count']):0;
    $desc='';$lead='';$time='';$contact='';$comment='';
    if($n > 0){
        // $data is not zero-indexed.
        for($i = 1; $i <= $n; $i++){
            $prefix = "======  Activity $i =====".PHP_EOL;
            $desc .= $prefix.check_plain(strip_tags($data[$i]['desc'])).PHP_EOL;
            $lead .= $prefix.check_plain(strip_tags($data[$i]['lead'])).PHP_EOL;
            $time .= $prefix.check_plain(strip_tags($data[$i]['time'])).PHP_EOL;
            $contact .= $prefix.check_plain(strip_tags($data[$i]['contact'])).PHP_EOL;
            $comment .= $prefix.check_plain(strip_tags($data[$i]['comment'])).PHP_EOL;
        }
    }else{
        drupal_set_message(t('Not submitted.  You seem to have an invalid number of activities.'),'error');
        return false;
    }
    
    $submission = new stdClass();
    $submission->dce = $user->name;
    $submission->title = "Complearning for ".$huser->basic['name']['firstname'].' '.
                        $huser->basic['name']['lastname'].'. '.strftime('%b %a, %Y');
    $submission->submitted_date = strftime('%Y-%m-%d %H:%M:%S');
    $submission->description = $desc;
    $submission->reflection = $lead;
    $submission->date = $time;
    $submission->contact = $contact;
    $submission->comments = $comment;
    
    return drupal_write_record('h_complearning',$submission);
}

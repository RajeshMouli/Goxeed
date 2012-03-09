<?php

/**
 * This file contains functions handeling the users of the GoXeed system.
 *
 * Written 2012 by Nicolas Kristoffersen (nicolaskristoffersen.com) at Wemind (wemind.dk)
 */


/**
 * Implements hook_user_login 
 * 
 * Update information about user from LinkedIn every time a users log in.
 */
function goxeed_user_login(&$edit, $account) {												// Upon login ...
  goxeed_update_profile_data_with_linkedin();													// ... update the user profile with date from LinkedIn
}

/**
 * Implements hook_user_delete()
 */
function goxeed_user_delete($account)
{
  $user_account = goxeed_user_secure_load ($account->uid);
  
  $items = $user_account->field_goxeed_game_user_nid['und'];
  foreach ($items as $item)
  {
    node_delete ($item['value']);
  }
}

/**
 * Implement hook_username_alter()
 * 
 * The user name provided by LinkedIn is stored in two separate fields for first and last name on the user object.
 * This function combines the two and returns the full name
 * (Note: The Drupal name field is the full name with the LinkedIn id and a time code added, because of the risk of having two identical users)
 *
 * 
 * Note: This function seems to fail, so instead of relying totally on this function
 * the internal GoXeed function calls the goxeed_real_name function when the real name is needed for the user interface
 */
function goxeed_username_alter (&$name, $account)
{
  $account = goxeed_user_secure_load ($account->uid);
  $items_first_name = goxeed_field_secure_get_items ("user", $account, "field_goxeed_first_name");				// Get the first name items
  $items_last_name = goxeed_field_secure_get_items ("user", $account, "field_goxeed_last_name");				// Get the last name items
  $user_name = $items_first_name[0]["value"] .  " " .$items_last_name[0]["value"];		// Combine first and last name
  if (strlen($user_name)>1) $name = $user_name;											// If the name exists, use it instead of the the Drupal user name
}

/**
 * Provide the full user name
 * 
 * Also see goxeed_username_alter
 * 
 * @param $account
 *   The user account
 *   
 * @return string
 *   Full user name
 */
function goxeed_real_name ($account)
{
  $name = format_username ($account);
  $account = goxeed_user_secure_load ($account->uid);
  $items_first_name = goxeed_field_secure_get_items ("user", $account, "field_goxeed_first_name");				// Get the first name items
  $items_last_name = goxeed_field_secure_get_items ("user", $account, "field_goxeed_last_name");				// Get the last name items
  $user_name = $items_first_name[0]["value"] .  " " .$items_last_name[0]["value"];		// Combine first and last name
  if (strlen($user_name)>1) $name = $user_name;											// If the name exists, use it instead of the the Drupal user name
  return $name;
}

/**
 * Provide first name of the specified user account
 * 
 * @param $account
 *   The user account
 *   
 * @return string
 *   The users first name
 */
function goxeed_first_name ($account)
{
  $name = format_username ($account);
  $account = goxeed_user_secure_load ($account->uid);
  $items_first_name = goxeed_field_secure_get_items ("user", $account, "field_goxeed_first_name");				// Get the first name items
  $user_name = $items_first_name[0]["value"];														// Combine first and last name
  if (strlen($user_name)>1) $name = $user_name;													// If the name exists, use it instead of the the Drupal user name
  return $name;
}

/**
 * Options callback for simple_content_type_checkboxes field
 * 
 * @return
 *   Indexed array as integer => string
 */
function goxeed_goxeed_user_type_options_list()  {
  $options = array(
      1 => 'GoXeed Admin',
  );
  return $options;
}

// *
// * DRUPAL GOTO
// * Handle the login process by catching redirects to certain pages to handle the login process
// * This is done by implementing hook_drupal_goto_alter
// *

/**
 * Implement hook_drupal_goto_alter
 * 
 * Handle the login process by catching redirects to certain pages to handle the login process
 */
function goxeed_drupal_goto_alter(&$path, &$options, &$http_response_code)
{
   
  // USER/REGISTER																		                // Automatically register a new Drupal user for a LinkedIn account

  if ($path == "user/register")															          // Create user automatically if LinkedIn profile has no account linked to it.
  {																						                        // Note: If no user account exists the Linkedin module redirects to "user/register".
  		$userinfo = array(
  		    'name' => 'auto'.time().rand(),										          // Create a random name - real name will be set in field_first_name and field_last_name from LinkedIn later
  		    'pass' => "auto".rand().rand(),									            // Create a random password
  		    'status' => 1																	              // Enable the account
  		);
  		$account = user_save(NULL, $userinfo);								          // Save the new user

  		if ($account)																		                // If account was successfully created
  		{
  		  $uid = user_authenticate($account->name, $account->pass);			// Authenticate user
  		  $form_state['uid'] = $account->uid;												    // Create an internal form state with the user id ...
  		  user_login_submit(array(), $form_state);										  //  ... log in as the just created user
  		  $path = "linkedin/token/".$account->uid;										  // Call LinkedIn authorization by redirecting to a path that will be handled by the LinkedIn module
  		}
  		else																				                    // If the account could not be created
  		{
  		  drupal_set_message (t("Error creating user account!"));				// Write a message if an error occured during user_save().
  		  $path = "";																		                // And go to the front page.
  		}
  }

  // USER/XX/EDIT/LINKEDIN																            // Prevent editing of LinkedIn settings - to avoid unlink profile to LinkedIn account - this is important because users do not know own username and e-mail-address is not known by the system.
   
  global $user;
  if ($path == "user/".$user->uid."/edit/linkedin")										// Catch this if sent to this path by LinkedIn module or a user attempts to go to the path directly.
  {
  		goxeed_update_profile_data_with_linkedin();												    // Update Drupal profile with user data from LinkedIn profile
  		$path = "";																			                // Go to the front page
  }
   
   
}

/**
 * Implement hook_form_alter for the user profile
 * 
 * This is done to hide the references to game user object to anyone but the Drupal admin
 */
function goxeed_goxeed_user_profile_form_alter($form)
{
  global $user; if ($user->uid>1)
  {
  		$form["field_goxeed_game_user_nid"]["#attributes"]["style"] = "display: none;";
  }
  return $form;
}

/**
 * Add a game user node reference to the user profile
 * 
 * @param $user_uid
 *   User id
 *   
 * @param $game_user_nid
 *   Node id of the game user node to be added
 */
function goxeed_goxeed_user_profile_game_user_add ($user_uid, $game_user_nid)
{
  $user_profile = goxeed_user_secure_load ($user_uid);
  $user_profile->field_goxeed_game_user_nid["und"][] = array("value" => $game_user_nid);
  user_save ($user_profile);
}

/**
 * Remove a game user node reference from the user profile
 * 
 * @param $user_uid
 *   User id
 *   
 * @param $game_user_nid
 *   Node id of the game user node to be removed
 */
function goxeed_user_profile_game_user_remove ($user_uid, $game_user_nid)
{
  $user_profile = goxeed_user_secure_load ($user_uid);
  $items = goxeed_field_secure_get_items ('user', $user_profile, 'field_goxeed_game_user_nid');
  foreach ($items as $key => $item)
  {
    if ($item["value"] == $game_user_nid)
    {
      unset($user_profile->field_goxeed_game_user_nid['und'][$key]);
    }
  }
  user_save ($user_profile);
}

/**
 * Get data from LinkedIn
 *
 * Get the profile data from the authorized LinkedIn account and write it to the Drupal account
 */
function goxeed_update_profile_data_with_linkedin()
{

  global $user;
  if ($user->uid > 1)									 									// Do not save LinkedIn information for admin
  {
    $linkedin_get_profile_fields = linkedin_get_profile_fields	($user->uid,
        array("first-name", "last-name", "id", "summary", "picture-url", "public-profile-url"));	// Get relevant information from LinkedIn
     
    $summary = "";

    if (key_exists('summary', $linkedin_get_profile_fields))
    {
      if ($linkedin_get_profile_fields["summary"])
      {
        $summary = truncate_utf8($linkedin_get_profile_fields["summary"], 250, TRUE, TRUE);
      }
    }

    if (array_key_exists("last-name", $linkedin_get_profile_fields))					// Only update, if fields actually found
    {
      // Create array containing information
      $userinfo = array(
        		'name' => $linkedin_get_profile_fields["first-name"] . "_" .
          $linkedin_get_profile_fields["last-name"] . "_" .
          $linkedin_get_profile_fields["id"] . "_" . time(),  					// Create a unique user name from real name, LinkedIn id and he time
        		'mail' => $linkedin_get_profile_fields["id"] . "_" . time().
          "_goxeed@nicolaskristoffersen.com",						  					// Create a unique email from real name, LinkedIn id and he time
          'field_goxeed_description' => array('und' => array(0 =>
              array('value' => $summary),),),		// Store first name - this field will be used in the front end instead of default Drupal user name
          'field_goxeed_first_name' => array('und' => array(0 =>
              array('value' => $linkedin_get_profile_fields["first-name"]),),),		// Store first name - this field will be used in the front end instead of default Drupal user name
          'field_goxeed_last_name' => array('und' => array(0 =>
              array('value' => $linkedin_get_profile_fields["last-name"]),),),		// Store last name - this field will be used in the front end instead of default Drupal user name
          'field_goxeed_li_pic_url' => array('und' => array(0 =>
              array('value' => $linkedin_get_profile_fields["picture-url"]),),),		// Store url for users LinkedIn profile picture - this will be used instead of internal Drupal picture
          'field_goxeed_li_prof_url' => array('und' => array(0 =>
              array('value' => $linkedin_get_profile_fields["public-profile-url"]),),),	// Store url for users public profile on LinkedIn
      );
      $account = goxeed_user_secure_load($user->uid);												// Load the current user object
      $account = user_save($account, $userinfo);										// Save the user object with updates
    }
  }
}

/**
 * Implement hook_user_view
 * 
 * Alter the presentation of users
 */
function goxeed_user_view($account, $view_mode, $langcode)
{
  
  $result = "";
   
  $user_image_html = "";
  $user_object = goxeed_user_secure_load($account->uid);
  $first_name_items = goxeed_field_secure_get_items ("user", $user_object, "field_goxeed_first_name");
  $first_name = $first_name_items[0]["value"];
  if (isset ($user_object->field_goxeed_li_pic_url["und"]))
  		$user_view_mode_top_image = $user_object->field_goxeed_li_pic_url["und"][0]["value"];
  if (isset ($user_object->field_goxeed_li_prof_url["und"]))
  		$user_linkedin_profile_url = $user_object->field_goxeed_li_prof_url["und"][0]["value"];
  if (isset($user_view_mode_top_image))
    $user_image_html = '<a href="'. url("") .'"><img src="'. $user_view_mode_top_image.'"></a>';
  $items = goxeed_field_secure_get_items ("user", $user_object, "field_goxeed_user_points");			// Get the user point field
  if ($items) $user_points = $items[0]["value"];									// If present stor the point value

  // ** Print the LinkedIn image with a link to the users linked in public profile

  $user_points = goxeed_user_profile_points ($account->uid);


  $result = $result . "<ul class='user profile'>";
  $result = $result . $user_image_html;
  drupal_set_title ("");
  $result = $result . "<div class='username'><h2>".goxeed_real_name ($user_object)."</h2></div>";
  $items = goxeed_field_secure_get_items ("user", $user_object, "field_goxeed_description");
  $description = $items[0]["value"];
   
  $result = $result . "<div class='linkedin description clearfix'><p>".$description."</p></div>";
   
  if (isset($user_linkedin_profile_url)) $result = $result . "<div class='linkedin profile link'>".l(t("View")." ".$first_name."'s ".t("LinkedIn profile"), $user_linkedin_profile_url, array ("attributes" => array("class" => "linkedin","target" => "_blank"))) . "</div>";
   
  $result = $result . goxeed_mission_badges_html ($account->uid, -1);
  $result = $result . "<div id='experiences'>";
  $items = goxeed_field_secure_get_items ("user", $user_object, "field_goxeed_first_name");
  $first_name = $items[0]["value"];
   
  $result = $result."<h3>" . goxeed_first_name($user_object)."'s " . t("experiences") . "</h3>";
   
  $position = 0;
  
   
  $challenge_user_nids = array();
  $game_user_nids = goxeed_field_secure_get_items ("user", $user_object, "field_goxeed_game_user_nid");
  if ($game_user_nids) foreach ($game_user_nids as $game_user_nid)
  {
  		$challenge_user_nids = array_merge ($challenge_user_nids, goxeed_game_user_challenge_users($game_user_nid["value"]));
  }
   
  $list = "";

  foreach ($challenge_user_nids as $challenge_user_nid)
  {
    $node = goxeed_node_secure_load ($challenge_user_nid);
    $items = goxeed_field_secure_get_items ('node', $node, 'field_goxeed_challenge_nid');
    $challenge_nid = $items[0]['value'];
    $comment_items = goxeed_field_secure_get_items ("node", $node, "field_goxeed_challenge_comment");
    $comment = $comment_items[0]["value"];
    if (strlen($comment)>0)
    {
      $image_url_items = goxeed_field_secure_get_items ("user", $user_object, "field_goxeed_li_pic_url");
      $image_url = $image_url_items[0]["value"];
      global $user;
      $challenge_user_current_user = goxeed_challenge_challenge_user_find($challenge_nid, $user->uid);
      if ($challenge_user_current_user)    // If the current user has a challenge user for the challenge, show the experience. If not, the current user is not playing the game and should not see this experience
      {
        $challenge_user_current_user = 'node/'.$challenge_user_current_user;
        $date_time = format_date($node->changed, 'custom', 'm.d.Y - H:i');
        $list = $list . "<li class='comment'><a href='".url("user/".$user_object->uid)."'><img src='".$image_url."' /></a><b>".goxeed_first_name($user_object).":</b><p>" .$comment. ' '. l('Go to challenge ...', $challenge_user_current_user). '</p><p class="datetime">'.$date_time.'</p></li>';
      }
    }
  }

  if ($list == "")
  {
    $list = "<div class='messages warning'>" . t("No experiences yet") . "</div>";
  }
  else
  {
    $result = $result . "<ul class='comments'>";
    $result = $result . $list;
    $result = $result . "</ul>";
  }
  
   
  $result = $result . "</div>";
   
  unset($account->content);                                // Remove Drupal standard user profile content
  $account->content["goxeed"][0]["#markup"] = $result;     // Set Goxeed user profile content
  $account->content["goxeed"]["#weight"] = 99;
}

/*
 * Users badges
 */

function goxeed_user_profile_badges ($uid)
{
  $result = array();
  $user = goxeed_user_secure_load ($uid);
  $items = goxeed_field_secure_get_items ('user', $user, 'field_goxeed_game_user_nid');
  if ($items)
  foreach ($items as $item)
  {
    $result = array_merge ($result, goxeed_game_user_badges ($item['value']));
  }
  
  return $result;
}

/**
 * Update user points of the specified user
 * 
 * @param $user_uid
 *   User id
 */
function goxeed_user_profile_user_points_update($user_uid)
{
  $points = 0;
  $user = goxeed_user_secure_load ($user_uid);
  $game_user_items = goxeed_field_secure_get_items ("user", $user, "field_goxeed_game_user_nid");
  if ($game_user_items) foreach ($game_user_items  as $game_user_item)
  {
  		$points = $points + goxeed_game_user_points ($game_user_item["value"]);
  }
  $items = goxeed_field_secure_get_items("user", $user, "field_goxeed_user_points");
  $memory = $items[0]["value"];
  if ($points != $memory)
  {
  		$user->field_goxeed_user_points["und"][0]["value"] = $points;
  		user_save ($user);
  }
}

/**
 * Get current point score of a specified user
 * 
 * @param $user_nid
 *   User id
 *   
 * @return int
 *   Current point score of the specified user
 */
function goxeed_user_profile_points($user_nid)
{
  $user = user_load ($user_nid);
  $fields = field_get_items ("user", $user, "field_goxeed_user_points");
  return $fields[0]["value"];
}



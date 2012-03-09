<?php

/**
 *   Functions handeling GoXeed Game User Nodes in the GoXeed module
 *
 *   Written 2011 by Nicolas Kristoffersen @ Wemind (wemind.dk) for Schneider Electric
 */
 
 
 
/**
 * Create a game user
 *
 * Create a new game user node for the specified game and the current user
 *
 * @param $game_nid
 *   Node id of the game
 * @param $user_uid
 *   The user id of user this game user belongs to
 * @param $active
 *   Specifies weather the game user is initailly active in the game
 *
 * @return array
 *   Returns the node object of the newly created game user
 */
function goxeedmodule_game_user_create($game_nid, $user_uid, $active = true)
{

  // Check if the user exists
  $game_user_exists = FALSE;
  $game = goxeedmodule_node_secure_load ($game_nid);
  $game_user_items = goxeedmodule_field_secure_get_items ("node", $game, "field_goxeed_game_user_nid");
  if ($game_user_items) foreach ($game_user_items as $game_user_item)
  {
    $game_user = goxeedmodule_node_secure_load ($game_user_item["value"]);
    $user_items = goxeedmodule_field_secure_get_items ("node", $game_user, "field_goxeed_user_uid");
    if ($user_items[0]["value"] == $user_uid) $game_user_exists = TRUE;
  }

  if (!$game_user_exists && $user_uid>1)		// If the user is not the admin go on to create a new user
  {
    $user_local = goxeedmodule_user_secure_load($user_uid);													// Load the full user object

    $game_user = new stdClass();															// Create an empty object
    $game_user->type = "goxeedmodule_game_user";														// Set the content type to game_user
    node_object_prepare($game_user);														// Prepare the node

    $game = goxeedmodule_node_secure_load ($game_nid);

    $game_user->title = $game->title . " ".t("for")." ".goxeedmodule_real_name($user_local);	// Set the title to a combination of the game name and the user name
    $game_user->field_goxeed_user_uid["und"][0]["value"] = $user_uid;									// Set the user id to the currently logged in user
    $game_user->field_goxeed_game_nid["und"][0]["value"] = $game_nid;							// Set the game id to the current game

    $active_local = "0";																// Assume that the game user is inactive
    if ($active) $active_local = "1";													// Set active if active is set in the function call
    $game_user->field_goxeed_active["und"][0]["value"] = $active_local;							// Store the activity setting as 0 or 1
    $game_user->field_goxeed_user_points["und"][0]["value"] = 0;								// Set user points to 0 as this is a new game user.
    node_save($game_user);																// Save the node


    // TODO: Implement path auto
    // $node_new->node_url = pathauto_create_alias("node", "insert", "node/".$node_new->nid, array());
    // node_save($node_new);

    goxeedmodule_goxeedmodule_user_profile_game_user_add ($user_uid, $game_user->nid);
    goxeedmodule_game_game_user_add ($game_nid, $game_user->nid);

    return $game_user;																	// Return the node
  }
  else
  {
    return null;																		// Return null if the user is the administrator; hence no game user is created
  }
}

/**
 * Get the user id of a game user node
 *
 * @param int
 *   Node id of the game user node
 *
 * @return int
 *   User id of the user belonging to the game user node
 */
function goxeedmodule_game_user_user_uid($nid)
{
  $result = NULL;
  $node = goxeedmodule_node_secure_load ($nid);
  $items = goxeedmodule_field_secure_get_items ('node', $node, 'field_goxeed_user_uid');
  $result = $items[0]['value'];
  return $result;
}

/**
 * Implements hook_delete() for GoXeed Games Users
 *
 * This hook is invoked by the actual hook in goxeed.module
 */
function goxeedmodule_goxeedmodule_goxeedmodule_game_user_delete ($node)
{
  $node = goxeedmodule_node_secure_load ($node->nid);
  $items = goxeedmodule_field_secure_get_items ('node', $node, 'field_goxeed_mission_user_nid');
  if ($items) foreach ($items as $item)
  {
    node_delete($item['value']);
  }
  $items = goxeedmodule_field_secure_get_items ('node', $node, 'field_goxeed_user_uid');
  if ($items) foreach ($items as $item)
  {
    goxeedmodule_user_profile_game_user_remove ($item['value'], $node->nid);  //  Remove the reference in the user profile
  }
  $items = goxeedmodule_field_secure_get_items ('node', $node, 'field_goxeed_game_nid');
  if ($items) foreach ($items as $item)
  {
    goxeedmodule_game_game_user_remove ($item['value'], $node->nid);
  }
}

/**
 * Add a mission user to the game user
 *
 * @param $game_user_nid
 *   Node id of the game user node
 * @param $mission_user_nid
 *   Node if of the mission user node
 */
function goxeedmodule_game_user_mission_user_add ($game_user_nid, $mission_user_nid)
{
  $game_user = goxeedmodule_node_secure_load ($game_user_nid);
  if (!array_key_exists ("und", $game_user->field_goxeed_mission_user_nid))
    $game_user->field_goxeed_mission_user_nid["und"] = array();
  $found = false;
  foreach ($game_user->field_goxeed_mission_user_nid["und"] as $mission_user_nid_current)
  {
    if ($mission_user_nid_current["value"] == $mission_user_nid) $found = true;
  }
  if (!$found)
  {
    array_unshift($game_user->field_goxeed_mission_user_nid["und"], array ("value" => $mission_user_nid));
    node_save ($game_user);
  }
}

/**
 * Remove a mission user from the game user
 *
 * @param $nid
 *   Node id of the game user
 * @param $target_nid
 *   Node id of the mission user to remove
 */
function goxeedmodule_game_user_mission_user_remove ($nid, $target_nid)
{
  $node = goxeedmodule_node_secure_load ($nid);
  $items = goxeedmodule_field_secure_get_items ('node', $node, 'field_goxeed_mission_user_nid');
  foreach ($items as $key => $item)
  {
    if ($item["value"] == $target_nid)
    {
      unset($node->field_goxeed_mission_user_nid['und'][$key]);
    }
  }
  node_save ($node);
}

/**
 * Implement hook_access() for game user nodes
 *
 * Invoked by the actual Drupal hook in goxeed.module
 */
function goxeedmodule_goxeedmodule_game_user_access($node, $op, $account)
{
  $result = NODE_ACCESS_IGNORE;
  $node = goxeedmodule_node_secure_load ($node->nid);
  $game_items = goxeedmodule_field_secure_get_items ("node", $node, "field_game_nid");
  $game = goxeedmodule_node_secure_load($game_items[0]["value"]);
  if ($op == "view")
  {
    if (node_access ("update", $game, $account))
    {
      $result = NODE_ACCESS_ALLOW;
    }
    else if (goxeedmodule_game_user_nid ($game_items[0]["value"], $account->uid) > -1)
    {
      $result = NODE_ACCESS_ALLOW;
    }
  }
  /*if ($op == "update")
   {
  if (node_access ("update", $game, $account))
    $result = NODE_ACCESS_ALLOW;
  }*/
  return $result;
}

/*
 * Badges earned in this game
 */
function goxeedmodule_game_user_badges($nid)
{
  $result = array();
  $node = goxeedmodule_node_secure_load ($nid);
    $items = goxeedmodule_field_secure_get_items ('node', $node, "field_goxeed_mission_user_nid");
    if ($items)
  {
    foreach ($items as $item)
    {
      $current = goxeedmodule_mission_user_badge($item['value']);
      if (strlen($current)>0)
      {
        $result[] = $current;
      }
    }
  }
  return $result;
}

/**
 * Implement hook_preprocess_node for goxeedmodule_game_user nodes
 */
function goxeedmodule_goxeedmodule_preprocess_goxeedmodule_game_user($variables)
{
  global $user;
  $result = "";
  $node = $variables["node"];
  $game_user = goxeedmodule_node_secure_load ($node->nid);

  $game_items = goxeedmodule_field_secure_get_items("node", $game_user, "field_goxeed_game_nid");
  $game = goxeedmodule_node_secure_load($game_items[0]["value"]);

  $user_items = goxeedmodule_field_secure_get_items ("node", $game_user, "field_goxeed_user_uid");
  $game_user_user_uid = $user_items[0]["value"];
  
  $is_admin = node_access ("update", $game);
  
  
  if ($is_admin)																	// If user is administrator of the game ...
  {
    if (arg(2) == "active" && (arg(3) == "0" || arg(3) == "1"))					// ... and if data has been posted ...
    {
      $game_user->field_goxeed_active["und"][0]["value"] = arg(3);			    // Set activity to new value ...
      node_save ($game_user);											// ... and save node
    }
     
    if (arg(2) == "mission_create")
    {
      $mission = goxeedmodule_goxeedmodule_mission_add($game);
      drupal_goto ("node/".$mission->nid."/edit");								// Go to edit the new node
    }
    $active_items = goxeedmodule_field_secure_get_items ("node", $game_user, "field_goxeed_active");				// Get active field ...
    $status_string = t("active"); $status_number = "1";
    if ($active_items[0]["value"] == "1") {
      $status_string = t("inactive"); $status_number = "0";					// .. and change to active, if set to active
    }
    $result = $result . "<div class='goxeed btn admin'>";
    $result = $result .l(t("edit this game"), "node/".
        $game->nid."/edit");
/*
    $result = $result .l(t("change user status to") . " " . $status_string,
        "node/".$game_user->nid."/active/".$status_number);
        */
    $result = $result . "</div>";
  }
  
  if ($user->uid != $game_user_user_uid)
  {
    drupal_goto ('node/'.$game->nid);
  }
  

  /*
  		if ($user_items[0]["value"] != $user->uid)
  {
    			$user_logged_in = goxeedmodule_user_secure_load($user->uid);
    			$game_user_user = goxeedmodule_user_secure_load ($game_user_user_uid);
    			$result = $result . "<div id='goxeedmodule_show_extra'>". t("Show this game for"). " ".l(goxeedmodule_real_name($user_logged_in), "node/".$game->nid)." / ";
    			$result = $result .t("Show profile for "). " ".l(goxeedmodule_real_name($game_user_user), "user/".$game_user_user->uid)."</div>";
  }
  */


  if (array_key_exists ("und", $game->field_goxeed_description))
  {
    $items = goxeedmodule_field_secure_get_items ('node', $game, 'field_goxeed_description');
    $description = $items[0]["value"];
    //if (strpos($description, ">")<1)
    $description = nl2br($description);
    $result = $result . "<div id='goxeed description'>".$description."</div>";
  }

  $points = goxeedmodule_game_user_points ($game_user->nid);
  //		$result = $result . "<div class='goxeed game_user points'>".t("You've got ").$points." ".t("points in this game").".</div>";

  $items = goxeedmodule_field_secure_get_items ("node", $game, "field_goxeed_closing");
  $closing = $items[0]["value"];
  $current = date('U');

  $result = $result . goxeedmodule_game_user_countdown ($closing - $current);

  // MISSIONS

  if (arg(2) != 'list')
  {
    $result = $result . "<h2>".t("Missions in this game")."</h2>";
    if ($is_admin) $result = $result . "<div class='goxeed btn admin'>".l(t("create mission"), "node/".$game_user->nid."/mission_create")."</div>";
    //$view = views_get_view_result("game_user_mission_user", "block", $node_local->nid);

    $view = goxeedmodule_field_secure_get_items("node", $node, "field_goxeed_mission_user_nid");
    $result = $result . "<ul class='goxeed game_user missions'>";

    if ($view)
      foreach ($view as $mission_user)
      {
         
        $mission_user = goxeedmodule_node_secure_load ($mission_user["value"]);
        $mission_items = goxeedmodule_field_secure_get_items ("node", $mission_user, "field_goxeed_mission_nid");
        $mission = goxeedmodule_node_secure_load ($mission_items[0]["value"]);
        if ($mission)
        {
          $mission_color_string = "";
          $mission_color_items = goxeedmodule_field_secure_get_items ("node", $mission, "field_goxeed_color");
          if ($mission_color_items){
            //					$mission_color_string = " style='background-color: #".$mission_color_items[0]["jquery_colorpicker"]."'";
          }



          // Mission Icon
          $mission_icon_string = "";
          $mission_icon_class = "";
          $mission_div_class = "";
          $mission_icon_items = goxeedmodule_field_secure_get_items ("node", $mission, "field_goxeed_icon");
          if ($mission_icon_items)
          {
            $mission_icon_class = $mission_icon_items[0]["value"];
            $mission_div_class = " icon ".$mission_icon_class;
            $mission_icon_title_options = goxeedmodule_goxeedmodule_mission_icons();
            $mission_icon_title = $mission_icon_title_options[$mission_icon_class];
          }

          /*
           $mission_icon_string = "";
          $mission_icon_items = goxeedmodule_field_secure_get_items ("node", $mission, "field_goxeed_icon");
          if (FALSE && $mission_icon_items)
          {
          $mission_icon_node = goxeedmodule_node_secure_load ($mission_icon_items[0]["nid"]);
          $mission_icon_title = $mission_icon_node->title;
          $mission_icon_class = str_replace (" ", "-", strtolower($mission_icon_title));
          $mission_icon_string = "<span class='icon category ".$mission_icon_class."'>".$mission_icon_node->title."</span>";
          }
          */

          $challenges = goxeedmodule_goxeedmodule_mission_user_challenges ($mission_user->nid);
          $challenges_completed = goxeedmodule_goxeedmodule_mission_user_challenges_completed($mission_user->nid);
          $points =  goxeedmodule_mission_user_points($mission_user->nid);
          if ($points > 0) $points = $points . " points";
          else $points = "";
          $challenges_remaining = $challenges - $challenges_completed;

          $url = url ("node/".$mission_user->nid);
          $result = $result . "<li class='mission".$mission_div_class."' onclick='location.href=\"".$url."\"' class='pointer color white'>";
          $result = $result . "<h3>" . $mission->title . "</h3>";
          $result = $result . $mission_icon_string;
          $result = $result . "<div class='mission property challenges'>".$challenges." challenges</div>";
          $result = $result . "<div class='mission property challenges-remaining'>".$challenges_remaining." remaining challenges</div>";
          $result = $result . "<div class='mission property points'>".$points."</div>";
        }

      }

      $result = $result . "</ul>";
    }

    // LEADERBOARD

    if ($is_admin || arg(2) == 'list')
    {

      $result = $result . "<h2>";
      if ($is_admin)
      {
        $result = $result . t("User list (admin)");
      }
      else
      {
        $result = $result . t("All players");
      }
      $result = $result ."</h2>";

      /*if ($is_admin && arg(2) != "all") $result = $result .
       "<div class='goxeed btn admin'>".l(t("show all"), "node/".$game_user->nid."/all")."</div>";
      if ($is_admin && arg(2) == "all") $result = $result .
      "<div class='goxeed btn admin'>".l(t("show normal"), "node/".$game_user->nid)."</div>";
      */
       
      //$game_user_items = goxeedmodule_field_secure_get_items ("node", $game, "field_game_user_nid");

      $game_user_items = goxeedmodule_game_game_user_sorted ($game->nid);
       
      $position = 1;
      $rows = array();
      foreach ($game_user_items as $game_user)
      {
        $node_game_user = goxeedmodule_node_secure_load ($game_user["value"]);

        $items = goxeedmodule_field_secure_get_items ("node", $node_game_user, "field_goxeed_user_uid");
        $user_local_leaderboard = goxeedmodule_user_secure_load ($items[0]["value"]);


        //				if (($is_admin && arg(2) == "all") || $position < 11 || $user_local_leaderboard->uid == $user->uid)
        if (($is_admin) || $position < 11 || $user_local_leaderboard->uid == $user->uid)
        {
          $active_items = goxeedmodule_field_secure_get_items ("node", $node_game_user, "field_goxeed_active");				// Get active field ...
          $status_string = t("activate"); $status_number = "1";
          if ($active_items[0]["value"] == "1") {
            $status_string = t("deactivate"); $status_number = "0";					// .. and change to active, if set to active
          }
          
          $status_string = l($status_string,
              "node/".$node_game_user->nid."/active/".$status_number);
          
          
          $active = false;
          $active_label = " (".$status_string.")";
          if (!$is_admin) $active_label = "";
          
          $position_label = "#";
          $class_add = array();
          if ($user_local_leaderboard->uid == $user->uid) $class_add[] = "current";
          $items_active = goxeedmodule_field_secure_get_items ("node", $node_game_user, "field_goxeed_active");
          if ($items_active[0]["value"] == "1")
          {
            //$active_label = "";
            $position_label = $position++;
            $active = true;
            $class_add[] = "active";
          }
          else
          {
            $class_add[] = "inactive";
          }
          if ($is_admin || $active)
          {
            
            // $link_string = "node/".$node_game_user->nid;
            // if (!$is_admin) 
            $link_string = "user/".goxeedmodule_game_user_user_uid ($node_game_user->nid);
            
            $name_string_local =l(goxeedmodule_real_name($user_local_leaderboard), $link_string). $active_label;
            
            $rows[] = array("class" => $class_add,"data" => array (array("class" => "position", "data" => $position_label), array("class" => "name", "data" => $name_string_local), array("class" => "points", "data" => $game_user["points"]." points")));
          }
        }
      }
       
      $result = $result . theme ("table", array ("rows" => $rows, "attributes" => array ("class" => "goxeed game leaderboard table reset")));
       
    }

    unset($variables["content"]);
    $variables["content"]["goxeed"][0]["#markup"] = $result;
    $variables["content"]["goxeed"]["#weight"] = 99;

    return $variables;
}

/**
 * Get game node id for spefied game user node id
 * 
 * @param $nid
 *   Node id of game user node
 *   
 * @return
 *   int Node id of the game
 */
function goxeedmodule_game_user_game ($nid)
{
  $result = NULL;
  $node = goxeedmodule_node_secure_load ($nid);
  $items = goxeedmodule_field_secure_get_items ('node', $node, "field_goxeed_game_nid");
  if ($items)
  {
    $result = $items[0]['value'];
  }
  return $result;
}

/**
 * Get game title
 * 
 * @param $game_user_nid
 *   Node id of the game user node
 *   
 * @return string
 *   Game title of the game 
 */
function goxeedmodule_game_user_game_title($game_user_nid)
{
  $result = "";
  $node = goxeedmodule_node_secure_load ($game_user_nid);
  $items = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_game_nid");
  $node = goxeedmodule_node_secure_load ($items[0]["value"]);
  if ($node) $result = $node->title;
  //$items = goxeedmodule_field_secure_get_items("node", $node, "title");
  return $result;
}



/* CAN BE DELETED
function goxeedmodule_game_user_challenges_remaining ($mission_user)
{
  $points = 0;
  $challenges_remaining = 0;
  $challenge_user_items = goxeedmodule_field_secure_get_items ("node", $mission_user, "field_challenge_user_nid");
  foreach ($challenge_user_items as $challenge_user_item)
  {
    $challenge_user = goxeedmodule_node_secure_load ($challenge_user_item["value"]);
    $points_new = goxeedmodule_challenge_user_points ($challenge_user->nid);
    $points = $points + $points_new;
    if ($points_new == 0) $challenges_remaining++;
  }
  return $challenges_remaining;
}
*/


/**
 * Challenges remaining for the the specified mission
 * 
 * @param $mission_user
 *   Mission user object
 *   
 * @return int
 *   Number of challenges remaining
 */
function goxeedmodule_game_user_challenges_remaining ($game_user_nid)
{
  $result = 0;
  $game_user = goxeedmodule_node_secure_load ($game_user_nid);
  $mission_user_items = goxeedmodule_field_secure_get_items ("node", $game_user, "field_goxeed_mission_user_nid");
  if ($mission_user_items) foreach ($mission_user_items as $mission_user_item)
  {
    $challenges = goxeedmodule_goxeedmodule_mission_user_challenges ($mission_user_item["value"]);
    $challenges_completed = goxeedmodule_goxeedmodule_mission_user_challenges_completed($mission_user_item["value"]);
    $result = $result + ($challenges - $challenges_completed);
  }

  return $result;
}

/**
 * Get number of missions for a specified game user
 * 
 * @param $game_user_nid
 *   Node id of the game user
 *   
 * @return
 *   Number of missions
 */
function goxeedmodule_game_user_missions ($game_user_nid)
{
  $result = 0;
  $game_user = goxeedmodule_node_secure_load ($game_user_nid);
  $mission_user_items = goxeedmodule_field_secure_get_items ("node", $game_user, "field_mission_user_nid");
  if ($mission_user_items) $result = count ($mission_user_items);
  return $result;
}

/**
 * Update points score for a specified game user node
 * 
 * @param $game_user_nid
 *   Node id of the game user
 */
function goxeedmodule_game_user_points_update($game_user_nid)
{
  $points = 0;
  $node = goxeedmodule_node_secure_load ($game_user_nid);
  $mission_user_items = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_mission_user_nid");
  if ($mission_user_items) foreach ($mission_user_items as $mission_user_item)
  {
    $points = $points + goxeedmodule_mission_user_points ($mission_user_item["value"]);
  }
  $items = goxeedmodule_field_secure_get_items("node", $node, "field_goxeed_user_points");
  $memory = $items[0]["value"];
  if ($points != $memory)
  {
    $node->field_goxeed_user_points["und"][0]["value"] = $points;
    node_save ($node);
    $game = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_game_nid");
    goxeedmodule_game_top_players_update($game[0]["value"]);
    $user = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_user_uid");
    goxeedmodule_user_profile_user_points_update($user[0]["value"]);
  }
}

/**
 * Get current point score for a given game user
 * 
 * @param $game_user_nid
 *   Node id of the game user
 *   
 * @return int
 *   Point score
 */
function goxeedmodule_game_user_points($game_user_nid)
{
  $node = goxeedmodule_node_secure_load ($game_user_nid);
  $fields = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_user_points");
  return $fields[0]["value"];
}


/**
 * Get challenge users of the specified game user
 * 
 * @param $nid
 *   Node id of game user
 *   
 * @return array
 *   Array of challenge users
 *   
 * The function returns all challenge user from all missions in the game
 */
function goxeedmodule_game_user_challenge_users($nid)
{
  $challenge_users = array();
  $node = goxeedmodule_node_secure_load ($nid);
  $mission_user_nids = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_mission_user_nid");
  if ($mission_user_nids) foreach ($mission_user_nids as $mission_user_nid)
  {
    $challenge_users = array_merge ($challenge_users, goxeedmodule_mission_user_challenge_users($mission_user_nid["value"]));
  }
  return $challenge_users;
}

/**
 * Implements hook_preprocess_page for goxeedmodule_game_user nodes
 */
function goxeedmodule_goxeedmodule_preprocess_page_goxeedmodule_game_user ($variables)
{

  $variables["display_submitted"] = FALSE;
   
  global $user;
  $node = $variables["node"];
  $node = goxeedmodule_node_secure_load ($node->nid);
  // EDIT

  $items = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_game_nid");				// Get the game field and ..
  $game_local = goxeedmodule_node_secure_load ($items[0]["value"]);									// .. load the corresponding game node.
  if ($user->uid > 1 && arg(2) == "edit")											// If edit selected and not Drupal admin
    drupal_goto ("node/".$game_local->nid."/edit");									// .. go to edit the game
   
  // TITLE SET
   
  $items_user = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_user_uid");				// Get user id field items
  $user_local = goxeedmodule_user_secure_load ($items_user[0]["value"]);
  $items_node = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_game_nid");				// Get game id field items
  $title_new = $game_local->title;												// Title is initially just the title of the game
  if ($user->uid != $items_user[0]["value"])										// If the user in not the logged in user
  {
    $title_new = $title_new . " " . t("for") . " " .
        goxeedmodule_real_name($user_local);											// Add the name of the user to the title
  }
  drupal_set_title ($title_new);													// Set the changed title in Drupal

  return NULL; //$variables;
}

/**
 * Check if game user is active
 * 
 * @param $game_user_nid
 *   The node id of the game user node
 * 
 * @return bool
 *   Returns true if user is active in the game and false if not.
 */
function goxeedmodule_game_user_active ($game_user_nid)
{
  $game_user = goxeedmodule_node_secure_load ($game_user_nid);
  $active_items = goxeedmodule_field_secure_get_items ("node", $game_user, "field_goxeed_active");				// Get active field ...
  return $active_items[0]["value"];
}

/**
 * Get HTML and JavaScript for the counter
 * 
 * @param $seconds
 *   Number of seconds until the game closes
 *   
 * @return string
 *   HTML to include in the page rendering
 */
function goxeedmodule_game_user_countdown($seconds = 3673)
{
  $result = '<div id="javascript_countdown_time"></div>
  <script type="text/javascript">
  var javascript_countdown = function () {
  var time_left = 10; //number of seconds for countdown
  var output_element_id = \'javascript_countdown_time\';
  var keep_counting = 1;
  var no_time_left_message = \'Game has closed!\';

  function countdown() {
  if(time_left < 2) {
  keep_counting = 0;
}

time_left = time_left - 1;
}

function add_leading_zero(n) {
//if(n.toString().length < 2) {
//return \'0\' + n;
//} else {
return n;
//}
}

function format_output() {
var hours, minutes, seconds;
seconds = time_left % 60;
minutes = Math.floor(time_left / 60) % 60;
hours = Math.floor(time_left / 3600) % 24;
days = Math.floor(time_left / (24 * 3600));


var seconds10, seconds1, minutes10, minutes1, hours10, hours1, days1000, days100, days10, days1;
seconds10 = Math.floor (seconds / 10);
seconds1 = seconds - (seconds10 * 10);
minutes10 = Math.floor (minutes / 10);
minutes1 = minutes - (minutes10 * 10);
hours10 = Math.floor (hours / 10);
hours1 = hours - (hours10 * 10);
days1000 = Math.floor (days / 1000) % 1000;
days = days - (days1000 * 1000);
days100 = Math.floor (days / 100) % 100;
days = days - (days100 * 100);
days10 = Math.floor (days / 10) % 10;
days1 = days - (days10 * 10);


var result = "";
if (days1000 > 0) result = result + "<span class=\'icon number"+days1000+"\'>"+days1000+"</span>";
if (days1000 > 0 || days100 > 0) result = result + "<span class=\'icon number"+days100+"\'>"+days100+"</span>";
if (days1000 > 0 || days100 > 0 || days10 > 0) result = result + "<span class=\'icon number"+days10+"\'>"+days10+"</span>";
result = result + "<span class=\'icon number"+days1+"\'>"+days1+"</span>";
result = result + " ";
result = result + "<span class=\'icon number"+hours10+"\'>"+hours10+"</span>";
result = result + "<span class=\'icon number"+hours1+"\'>"+hours1+"</span>";
result = result + " ";
result = result + "<span class=\'icon number"+minutes10+"\'>"+minutes10+"</span>";
result = result + "<span class=\'icon number"+minutes1+"\'>"+minutes1+"</span>";
result = result + " ";
result = result + "<span class=\'icon number"+seconds10+"\'>"+seconds10+"</span>";
result = result + "<span class=\'icon number"+seconds1+"\'>"+seconds1+"</span>";
return result;

}

function show_time_left() {
document.getElementById(output_element_id).innerHTML = format_output();//time_left;
}

function no_time_left() {
document.getElementById(output_element_id).innerHTML = no_time_left_message;
}

return {
count: function () {
countdown();
show_time_left();
},
timer: function () {
javascript_countdown.count();

if(keep_counting) {
setTimeout("javascript_countdown.timer();", 1000);
} else {
no_time_left();
}
},
setTimeLeft: function (t) {
time_left = t;
if(keep_counting == 0) {
javascript_countdown.timer();
}
},
init: function (t, element_id) {
time_left = t;
output_element_id = element_id;
javascript_countdown.timer();
}
};
}();

//time to countdown in seconds, and element ID
javascript_countdown.init('.$seconds.', \'javascript_countdown_time\');

</script>';

  return $result;
}



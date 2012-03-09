<?php

/**
 *   Function handeling GoXeed Game Nodes in the GoXeed module
 *
 *   Written 2011 by Nicolas Kristoffersen @ Wemind (wemind.dk) for Schneider Electric


/**
 * Implements hook_delete() for game content type
 *
 * The function is invoked by the actual hook in goxeed.module
 *
 * The function deletes all missions and all game users associated with the game
 *
 */
function goxeed_goxeed_goxeed_game_delete($node)													// If a game has been deleted ...
{
  $node = goxeed_node_secure_load ($node->nid);
  $mission_items = goxeed_field_secure_get_items ("node", $node, "field_goxeed_mission_nid");
  if ($mission_items)
  {
  foreach ($mission_items as $mission_item)
  {
      node_delete ($mission_item["value"]);
    }
  }
  $game_user_items = goxeed_field_secure_get_items ("node", $node, "field_goxeed_game_user_nid");
  if ($game_user_items)
  {
    foreach ($game_user_items as $game_user_item)
    {
      node_delete ($game_user_item["value"]);
    }
  }
}

/**
 * Add a mission to the game
 *
 * @param int $game_nid
 *   The node id of the game
 * @param int $mission_nid
 *   The node id of the mission to add
 */
function goxeed_game_mission_add ($game_nid, $mission_nid)
{
  $game = goxeed_node_secure_load ($game_nid);
  $game->field_goxeed_mission_nid["und"][] = array ("value" => $mission_nid);
  node_save ($game);
}

/**
 * Remove mission reference from game
 *
 * @param int $nid
 *   The node id of the game
 * @param int $target_nid
 *   The node id of the mission reference to remove
 */
function goxeed_game_mission_remove ($nid, $target_nid)
{
  $node = goxeed_node_secure_load ($nid);
  $items = goxeed_field_secure_get_items ('node', $node, 'field_goxeed_mission_nid');
  if ($items) foreach ($items as $key => $item)
  {
    if ($item["value"] == $target_nid)
    {
      unset($node->field_goxeed_mission_nid['und'][$key]);
    }
  }
  node_save ($node);
}

/**
 * Add a game user to the game
 *
 * @param $game_nid
 *   The node id of the game
 * @param $game_user_nid
 *   The node id of the game user to add
 */
function goxeed_game_game_user_add ($game_nid, $game_user_nid)								// Takes game node id and game user node id
{
  $found = false;																			// Initially assume user not alreay registered for this game
  $game = goxeed_node_secure_load($game_nid);															// Load the game
  $game_user_items = goxeed_field_secure_get_items ("node", $game, "field_goxeed_game_user_nid");					// Get current game users
  if ($game_user_items)																	// If any exists ..
    foreach ($game_user_items as $game_user_item)										// .. loop though them ...
    if ($game_user_item["value"] == $game_user_nid)	$found = true;					// .. if new user is found set flag for found
  if (!$found)																			// If not already found ..
  {
    $game->field_goxeed_game_user_nid["und"][] = array ("value" => $game_user_nid);			// .. add the user
    node_save ($game);																	// .. and save the game.
  }
}

/**
 * Remove game user reference from game
 *
 * @param int $nid
 *   The node id of the game
 * @param int $target_nid
 *   The node id of the game user reference to remove
 */
function goxeed_game_game_user_remove ($nid, $target_nid)
{
  $node = goxeed_node_secure_load ($nid);
  $items = goxeed_field_secure_get_items ('node', $node, 'field_goxeed_game_user_nid');
  foreach ($items as $key => $item)
  {
    if ($item["value"] == $target_nid)
    {
      unset($node->field_goxeed_game_user_nid['und'][$key]);
    }
  }
  node_save ($node);
}

/**
 *  Update game user references in the game to reflect point score
 *
 *  @param int $nid
 *    The node id of the game to update
 *
 *  The function sorts the refenreces and saves them.
 */
function goxeed_game_top_players_update($nid)
{
  $node = goxeed_node_secure_load ($nid);
  $items = array();
  $game_user_nids = goxeed_field_secure_get_items ("node", $node, "field_goxeed_game_user_nid");
   
  if ($game_user_nids) foreach ($game_user_nids as $game_user_nid)
  {
    $points = goxeed_game_user_points ($game_user_nid["value"]);
    $items[] = array ("points" => $points, "value" => $game_user_nid);
  }
  rsort ($items);
  unset ($node->field_goxeed_game_user_nid["und"]);
  foreach ($items as $item)
  {
  		$node->field_goxeed_game_user_nid["und"][] = $item["value"];
  }
  node_save ($node);
}


/**
 * Top players of the game
 *
 * @param int $nid
 *   The node id of the game
 *
 * @return string
 *   HTML markup for the block displaying the top players
 */
function goxeed_game_top_players($nid)
{
  $result = "<ul>";
  $node = goxeed_node_secure_load ($nid);
  $game_users = goxeed_field_secure_get_items ("node", $node, "field_goxeed_game_user_nid");
  $position = 0;
  $current_points = -1;
  $postion_real = 0;
  if ($game_users) foreach ($game_users as $game_user)
  {
    $node = goxeed_node_secure_load ($game_user["value"]);
    if ($node)
    {
      $game_user_id = $node->nid;
      $items = goxeed_field_secure_get_items ('node', $node, 'field_goxeed_active');
      if ($items[0]['value'] == "1")
      {
        $items = goxeed_field_secure_get_items ("node", $node, "field_goxeed_user_points");
        $game_user_points = $items[0]["value"];
        $position++;
        if ($current_points != $game_user_points)
        {
          $position_local = $position;
          $current_points = $game_user_points;
        }
        $current_points = $game_user_points;
        $items = goxeed_field_secure_get_items ("node", $node, "field_goxeed_user_uid");
        $user = goxeed_user_secure_load ($items[0]["value"]);
        $user_name = goxeed_real_name($user);
        $result = $result . "<li><span class='position'>".$position_local."</span>".l($user_name, "user/".$user->uid)."<span class='points'>".$game_user_points."</span></li>";
      }
    }
    else
    {
      watchdog ('goxeed','Missing node ' . $game_user["value"]. ' referenced in node '.$nid, 'warning', array(), WATCHDOG_ERROR);
    }
   }
  return $result."</ul>";
}



/**
 * Implements hook_form_alter() for goxeed games
 *
 * Invoked by the actual implementation in goxeed.module
 */
function goxeed_goxeed_goxeed_game_node_form_alter($form)
{
  global $user; if ($user->uid>1)
  {
  		$form["field_goxeed_mission_nid"]["#attributes"]["style"] = array('display:none;');
  		$form["field_goxeed_game_user_nid"]["#attributes"]["style"] = array('display:none;');
  }
  return $form;
}

/**
 * Implements hook_node_access for goxeed_game nodes
 */
function goxeed_goxeed_goxeed_game_access($node, $op, $account)
{
  $result = NODE_ACCESS_IGNORE;															// Default to access defined by Drupal admin
  if ($op == "view")																		// If requesting access for viewing a game ..
  {
  		$result = NODE_ACCESS_ALLOW;														// ...always allow, as a code will be required, if not a part of the game
  }
  else if ($op == "update")																// If requesting access for updating a game
  {
  		$account_full = goxeed_user_secure_load ($account->uid);
  		$field_goxeed_admin_a = goxeed_field_secure_get_items("user", $account_full, "field_goxeed_admin_a");
  		if ($field_goxeed_admin_a[0]["value"])
  		{
    		$result = NODE_ACCESS_ALLOW;
  		}
  			
  		$items = goxeed_field_secure_get_items ("node", $node, "field_goxeed_game_administrator");					// Get administrator fields
  		if ($items) foreach ($items as $administrator_current)								// If items exist loop through them ...
  		{
        if ($administrator_current["uid"] == $account->uid)								// ... as if one of them has the id of the current user ...
  		    $result = NODE_ACCESS_ALLOW;												// ... allow access
  		}
  }
  else if ($op == "create")
  {
    $account_full = goxeed_user_secure_load ($account->uid);
    $field_goxeed_admin_a = goxeed_field_secure_get_items("user", $account_full, "field_goxeed_admin_a");
    if ($field_goxeed_admin_a[0]["value"])
    {
      $result = NODE_ACCESS_ALLOW;
    }
  }
  return $result;
}

/**
 * Get games users sorted according to current point score
 * 
 * @param $game_nid
 *   Node id of the game
 *   
 * @return
 *   Sorted array of game user node ids
 */
function goxeed_game_game_user_sorted ($game_nid)
{
  $sort_array = array();
  $game = goxeed_node_secure_load ($game_nid);
  $game_user_items = goxeed_field_secure_get_items ("node", $game, "field_goxeed_game_user_nid");
  foreach ($game_user_items as $game_user_item)
  {
    $points = goxeed_game_user_points ($game_user_item["value"]);
    $sort_array[] = array ("points" => $points, "value" => $game_user_item["value"]);
  }
  rsort ($sort_array);
  return $sort_array;
}

/**
 * Check if all game administrators have a game user node
 * 
 * @param $game
 *   Node id of the game to check
 * 
 * This function checks if all administrators have game user node.
 * If this is not the case then create one
 */
function goxeed_game_administrators_game_user_check($game)
{
  if (array_key_exists("und",  $game->field_goxeed_game_administrator))
  {
    $game_administrators = $game->field_goxeed_game_administrator["und"];							// Get administrator items ..
    foreach ($game_administrators as $game_administrator)								// .. and loop through them
    {
      $found = false;
      if (isset ($game->field_goxeed_game_user_nid["und"]))
      {
        $game_users = $game->field_goxeed_game_user_nid["und"];
        foreach ($game_users as $game_user)
        {
          if ($game_user["value"] == $game_administrator["uid"])
            $found = true;
        }
      }
      if (!$found)																	// If the game_user does not exist ..
      {
        $game_user = goxeed_game_user_create
        ($game->nid, $game_administrator["uid"], $active = false);				// .. create an inactive game user for each of them
         
      }
    }
  }
}


/**
 * Implements hook_preprocess game node
 */
function goxeed_goxeed_preprocess_goxeed_game($variables)
{

  global $user;																			// Make the global user object available
  $result = "";																			// Start with an empty result of the processing
  $node = $variables["node"];																// Get the node object from variables
  $view_mode = $variables["view_mode"];
  $game = goxeed_node_secure_load ($node->nid);															// Load the full game node to make sure all fields are loaded

  if ($view_mode == "teaser")
  {
    $variables["title"] = "";
    unset($variables["content"]["links"]);
  }
  else
  {
    goxeed_game_administrators_game_user_check($game);															// Check if admin users have a game user node

    // Hanndle signup
    if (array_key_exists("signup_code", $_POST))											// If a signup code has been entered
    {
      $items = goxeed_field_secure_get_items ("node", $game, "field_goxeed_signup_code");						// Get the game signup code field
      if ($_POST["signup_code"] == $items[0]["value"])									// If the entered signup code is equal to the game signup code ..
      {
        $game_user = goxeed_game_user_create
        ($game->nid, $user->uid, true);												// ... create an active game user node for the current game and current user ..
         
        $mission_items = goxeed_field_secure_get_items ("node", $game, "field_goxeed_mission_nid");
        if ($mission_items)
          foreach ($mission_items as $mission_item)
          {
            $mission_user = goxeed_mission_user_create ($game_user->nid);
            goxeed_goxeed_mission_user_mission_nid_set ($mission_user->nid, $mission_item["value"]);
            $mission = goxeed_node_secure_load ($mission_item["value"]);
            goxeed_goxeed_mission_mission_user_add($mission, $mission_user->nid);
            node_save ($mission);
            $challenge_items = goxeed_field_secure_get_items ("node", $mission, "field_goxeed_challenge_nid");
            if ($challenge_items)
              foreach ($challenge_items as $challenge_item)
              {
                $challenge_user = goxeed_challenge_user_create ($mission_user->nid);
                goxeed_challenge_user_challenge_nid_set ($challenge_user->nid, $challenge_item["value"]);
                $challenge = goxeed_node_secure_load ($challenge_item["value"]);
                goxeed_challenge_challenge_user_add($challenge, $challenge_user->nid);
                node_save ($challenge);
              }
          }
          drupal_goto ("node/".$game_user->nid);											// ... and go to the page of the game user
      }
      else																				// If the entered signup code is not correct ...
      {
        drupal_set_message (t("The signup code was not correct."));						// Write an error message ...
        $show_game_signup = true;														// ... and put the signup code form on the page again
      }
    }
     
    $game_user_nid = goxeed_game_user_nid($game->nid, $user->uid);							// Get nid of game_user object (-1 if none)
     
    if (node_access ("update", $game))														// If the user has editing rights for the game ...
    {
      if ($game_user_nid<0 && arg(2) != "delete") 										// ... and if a game_user exists and make sure, the administrator is not trying to delete the game ...
      {
        $game = goxeed_game_user_create
        ($game->nid, $user->uid, false);											// ... create a game_user node for the current game, but make it inactive to hide the administrator from palyers of the game.
        if ($game) drupal_goto ("node/".$game->nid);								// ... and ff the new game_user was created succesfully, go to the page of the new game_user
      }
      if (arg(2) != "delete" && arg(2) !="edit")
        drupal_goto("node/".$game_user_nid);											// ... go to the game_user page
    }
    else																					// If it is a normal user ...
    {
      if ($game_user_nid>-1)																// ... and if a game_user node exists  ..
      {
        drupal_goto("node/".$game_user_nid);											// ... go to the game_user page
      }
      else																				// If a game_user does not exist ...
      {
        $show_game_signup = true;														// ... show the game signup form
      }
    }
     
    if ($show_game_signup)
    {
      unset($variables["content"]);

      $content = '<p>' . (t("Welcome to GoXeed.")). '</p>';
      $content = $content . '<p>' . (t("You need to enter your signup code before you can enter this game.")) . '</p>';
      $content = $content . '<p>' . (t("Please look for the code in the invitation e-mail.")) . '</p>';
      $content = $content . '<form name="input" action="" method="post">Signup-code: <input type="text" name="signup_code" /><input type="submit" value="Submit" /></form>';
      $variables["content"]["system_main"]["goxeed"][0]["#markup"] = $content;
    }
  }

  $variables["display_submitted"] == "false";
  $variables["content"]["goxeed"][0]["#markup"] = $result;
  $variables["content"]["goxeed"]["#weight"] = 99;

  return $variables;
}

/**
 * Implements hook_preprocess_page for game nodes
 */
function goxeed_goxeed_preprocess_page_game ($variables)
{
  $variables["display_submitted"] == FALSE;

  global $user;
  $node = $variables["node"];
  $game_user_nid = goxeed_game_user_nid($node->nid, $user->uid);			// Get nid of game_user object (-1 if none)
  if ($game_user_nid <0) drupal_set_title (t("Game signup"));
  return $variables;
}

/**
 * Find the game user node id for at specified game and user
 * 
 * @param $game_nid
 *   Node id of the game
 *   
 * @param $user_uid
 *   User id
 *   
 * @return int
 *   Node id of the game user node or -1 if not found
 */
function goxeed_game_user_nid ($game_nid, $user_uid)
{
$result = -1;																			// Assume the user is not signed up for the game

  $game = goxeed_node_secure_load ($game_nid);
  $game_user_items = goxeed_field_secure_get_items ("node", $game, "field_goxeed_game_user_nid");

  if ($game_user_items)
    foreach ($game_user_items as $game_user_nid)
    {
      $game_user_node = goxeed_node_secure_load ($game_user_nid["value"]);										// Load the current game_user node
      $game_user_user_items = goxeed_field_secure_get_items ("node", $game_user_node, "field_goxeed_user_uid");	// Get the game_user user reference items
      if ($game_user_user_items[0]["value"] == $user_uid)									// If game_user holds the right user reference and game reference ..
        $result = $game_user_node->nid;														// .. store the node id as the result
    }
    return $result;																			// Return the result
}

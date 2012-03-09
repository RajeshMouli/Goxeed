<?php

/**
 * This file contains functions handeling the mission user elements of the GoXeed system.
 *
 * Written 2012 by Nicolas Kristoffersen (nicolaskristoffersen.com) at Wemind (wemind.dk)
 */


/**
 * Create a mission user node
 *
 * @param $game_user_nid
 *   Node id of the game_user
 *
 * @return
 *   The new mission user node
 */
function goxeedmodule_mission_user_create($game_user_nid)
{
  $node_new = new stdClass();															// Create an empty object
  $node_new->type = "goxeedmodule_mission_user";													// Set the content type to mission_user
  node_object_prepare($node_new);														// Prepare the node

  $node_new->title =  "new";					// Set the title to a combination of the mission title and the game user title
  $node_new->language = LANGUAGE_NONE;
  $node_new->field_goxeed_game_user_nid["und"][0]["value"] = $game_user_nid;						// Set the game_user reference

  node_save($node_new);																// Save the node

  goxeedmodule_game_user_mission_user_add ($game_user_nid, $node_new->nid);

  return $node_new;																	// Return the node
}

/**
 * Implement hook_node_delete() for goxeedmodule_mission_user
 */
function goxeedmodule_goxeedmodule_goxeedmodule_mission_user_delete($node)
{
  $node = goxeedmodule_node_secure_load ($node->nid);
  $items = goxeedmodule_field_secure_get_items ('node', $node, 'field_goxeed_challenge_user_nid');
  if ($items) foreach ($items as $item)
  {
    node_delete($item['value']);
  }
  $items = goxeedmodule_field_secure_get_items ('node', $node, 'field_goxeed_game_user_nid');
  foreach ($items as $item)
  {
    goxeedmodule_game_user_mission_user_remove ($item['value'], $node->nid);
  }
  $items = goxeedmodule_field_secure_get_items ('node', $node, 'field_goxeed_mission_nid');
  foreach ($items as $item)
  {
    goxeedmodule_mission_mission_user_remove ($item['value'], $node->nid);
  }
}

/**
 * Set mission node id for the mission user
 *
 * @param $mission_user_nid
 *   Mission user node id
 * @param $mission_nid
 *   Mission node id
 */
function goxeedmodule_goxeedmodule_mission_user_mission_nid_set ($mission_user_nid, $mission_nid)
{
  $mission_user = goxeedmodule_node_secure_load ($mission_user_nid);
  $mission_user->field_goxeed_mission_nid["und"][0]["value"] = $mission_nid;
  node_save ($mission_user);
}

function goxeedmodule_mission_user_challenge_user_add ($mission_user_nid, $challenge_user_nid)
{
  $mission_user = goxeedmodule_node_secure_load ($mission_user_nid);
  if (!array_key_exists("und", $mission_user->field_goxeed_challenge_user_nid))
    $mission_user->field_goxeed_challenge_user_nid["und"] = array();
  $found = false;
  foreach ($mission_user->field_goxeed_challenge_user_nid["und"] as $challenge_user_nid_current)
  {
    if ($challenge_user_nid_current["value"] == $challenge_user_nid) $found = true;
  }
  if (!$found)
  {
    array_unshift($mission_user->field_goxeed_challenge_user_nid["und"], array ("value" => $challenge_user_nid));
    node_save ($mission_user);
  }
}

/**
 * Remove challenge user from mission user
 *
 * @param int $nid
 *   The node id of the mission user
 * @param int $target_nid
 *   The node id of challenge user to remove
 */
function goxeedmodule_mission_user_challenge_user_remove ($nid, $target_nid)
{
  $node = goxeedmodule_node_secure_load ($nid);
  $items = goxeedmodule_field_secure_get_items ('node', $node, 'field_goxeed_challenge_user_nid');
  foreach ($items as $key => $item)
  {
    if ($item["value"] == $target_nid)
    {
      unset($node->field_goxeed_challenge_user_nid['und'][$key]);
    }
  }
  node_save ($node);
}

/*

// *
// * MISSION USER CHALLENGE USER ADD
// * Add a challenge user to a mission user, to allow quick update
// *
function mission_user_challenge_user_add($challenge_user)
{
$mission_items = goxeedmodule_field_secure_get_items("node", $challenge_user, "field_goxeed_mission_user_nid");
$mission_user_node = goxeedmodule_node_secure_load($mission_items[0]["nid"]);
$reference_items = goxeedmodule_field_secure_get_items("node", $mission_user_node, "field_goxeed_challenge_user_nid");
$found = false;
if ($reference_items)
  foreach ($reference_items as $reference)
  {
if ($reference["value"] == $challenge_user->nid) $found == true;
}
if (!$found)
{
$mission_user_node->field_goxeed_challenge_user_nid["und"][]
= array ("value" => $challenge_user->nid);
node_save ($mission_user_node);
}
}

*/

/**
 * List the possible icons for mission
 *
 * @return array
 *   Return array of missions icons
 */
function goxeedmodule_goxeedmodule_mission_icons()
{
  $options = array(
      'mouth' => '<span class="category icon mouth">Mouth</span>',
      'ear' => '<span class="category icon ear">Ear</span>',
      'light' => '<span class="category icon light">Light bulb</span>',
      'graph_up' => '<span class="category icon graph">Graph</span>',
      'education' => '<span class="category icon education">Education</span>',
      'star_2' => '<span class="category icon star">Star</span>',
      'gift' => '<span class="category icon gift">Gift</span>',
      'electrical_energy' => '<span class="category icon flash">Flash</span>',
      'time' => '<span class="category icon time">Time</span>',
      'earth_america' => '<span class="category icon category globe">Globe</span>',
      'suitcase' => '<span class="category icon suitcase">Suitcase</span>',
      'pen' => '<span class="category icon pen">Pen</span>',
      'book' => '<span class="category icon book">Book</span>',
      'tools' => '<span class="category icon tools">Tools</span>',
      'key' => '<span class="category icon key">Key</span>',
      'ruler' => '<span class="category icon ruler">Ruler</span>',
      'microphone' => '<span class="category icon microphone">Microphone</span>',
      'seat' => '<span class="category icon chair">Chair</span>',
      'umbrella' => '<span class="category icon umbrella">Umbrella</span>',
      'food_and_beverages' => '<span class="category icon food">Food</span>',
  );
  return $options;
}

// *
// * MISSION USER FIND
// * This function finds the mission user object for a given mission.
// *
/**
 * Find mission user node id
 *
 * @param $mission_nid
 *   Mission user node id
 *
 * @param $user_uid
 *   User id
 *
 * @return int
 *   Node id of the mission user node for the specified mission and user. Return -1 if not found.
 */
function goxeedmodule_goxeedmodule_module_mission_user_nid ($mission_nid, $user_uid)
{
  $result = -1;																			// Assume the user is not signed up for the game
  $mission = goxeedmodule_node_secure_load ($mission_nid);
  $mission_user_items = goxeedmodule_field_secure_get_items ("node", $mission, "field_goxeed_mission_user_nid");
  if ($mission_user_items) foreach ($mission_user_items as $mission_user_item)
  {
    $mission_user_nid = goxeedmodule_goxeedmodule_mission_user_game_user ($mission_user_item["value"]);
    $game_user = goxeedmodule_node_secure_load ($mission_user_nid);
    $user_items = goxeedmodule_field_secure_get_items ("node", $game_user, "field_goxeed_user_uid");
    if ($user_items[0]["value"] == $user_uid) $result = $mission_user_item["value"];
  }
  return $result;
}

/**
 * Find game user node for a give challenge user node
 *
 * @param $mission_user_nid
 *   Node id of the mission user
 *
 * @return int
 *   Node id of the game user
 */
function goxeedmodule_goxeedmodule_mission_user_game_user ($mission_user_nid)
{
  $mission_user = goxeedmodule_node_secure_load ($mission_user_nid);
  $game_user_items = goxeedmodule_field_secure_get_items ("node", $mission_user, "field_goxeed_game_user_nid");
  return $game_user_items[0]["value"];
}

/**
 * Implements hook_node_access for goxeedmodule_mission_user nodes
 */
function goxeedmodule_goxeedmodule_goxeedmodule_mission_user_access($node, $op, $account)
{
  $result = NODE_ACCESS_IGNORE;
  if ($op == "view")
  {
    $items = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_game_user_nid");
    $game_user_local = goxeedmodule_node_secure_load ($items[0]["value"]);
    if (node_access("view", $game_user_local, $account))
      $result = NODE_ACCESS_ALLOW;
  }
  return $result;
}

/**
 * Implements hook_node_preprocess for goxeedmodule_mission_user nodes
 */
function goxeedmodule_goxeedmodule_preprocess_goxeedmodule_mission_user($variables)
{
  global $user;

  $result = "";

  $node = $variables["node"];
  $mission_user = goxeedmodule_node_secure_load ($node->nid);

  if (node_access("view", $mission_user))
  {
    unset($variables["content"]);
     
    $mission_items = goxeedmodule_field_secure_get_items("node", $mission_user, "field_goxeed_mission_nid");
    $mission = goxeedmodule_node_secure_load($mission_items[0]["value"]);
    $game_user_items = goxeedmodule_field_secure_get_items("node", $mission_user, "field_goxeed_game_user_nid");
    $game_user = goxeedmodule_node_secure_load($game_user_items[0]["value"]);
     
    $user_items = goxeedmodule_field_secure_get_items ("node", $game_user, "field_goxeed_user_uid");
    $user_local = goxeedmodule_user_secure_load($user_items[0]["value"]);
    $is_admin = node_access ("update", $mission);
     
    if ($is_admin)																	// If user is administrator of the game ...
    {
      if (arg(2) == "challenge_create")
      {
        $challenge = goxeedmodule_challenge_add($mission);
        drupal_goto ("node/".$challenge->nid."/edit");								// Go to edit the new node
      }
      $result = $result . "<div class='goxeed btn admin'>";
      $result = $result . l(t("edit this mission"), "node/". $mission->nid."/edit") ."</div>";
    }
     
    
    if ($user_local->uid != $user->uid)
    {
      drupal_goto ('node/'.$mission->nid);
    }
    /*
    			if ($user_local->uid != $user->uid)
    			{
    				$user_logged_in = goxeedmodule_user_secure_load($user->uid);
    				$result = $result . "<div id='goxeedmodule_show_extra'>". t("Show this mission for"). " ".l(goxeedmodule_real_name($user_logged_in), "node/".$mission->nid)." / ";
    				$result = $result .t("Show profile for "). " ".l(goxeedmodule_real_name($user_local), "user/".$user_local->uid)."</div>";
    			}
*/
     
     
    $points = goxeedmodule_mission_user_points ($mission_user->nid);
     
    // $result = $result . "<div class='goxeed mission_user points'>".t("You've got ").$points." ".t("points in this mission").".</div>";

    /*

    // Mission Color
    $mission_color_string = "";
    $mission_color_items = goxeedmodule_field_secure_get_items ("node", $mission, "field_goxeed_color");
    if ($mission_color_items){
    $mission_color_string = " style='background-color: #".$mission_color_items[0]["jquery_colorpicker"]."'";
    }
    */
     
     
    // Mission Icon
    $mission_icon_string = "";
    $mission_icon_items = goxeedmodule_field_secure_get_items ("node", $mission, "field_goxeed_icon");
    if ($mission_icon_items)
    {
      $mission_icon_class = $mission_icon_items[0]["value"];
      $mission_icon_title_options = goxeedmodule_goxeedmodule_mission_icons();
      $mission_icon_title = $mission_icon_title_options[$mission_icon_class];
      $result = $result . $mission_icon_title;

    }
     
     
    if (array_key_exists ("und", $mission->field_goxeed_description))
    {
      $items = goxeedmodule_field_secure_get_items ('node', $mission, 'field_goxeed_description');
      $description = $items[0]["value"];
      //if (strpos($description, ">")<1)
      $description = nl2br($description);
      $result = $result . "<div id='goxeed description'>".$description."</div>";
    }
     
    //$result = $result . "<div class='color white goxeed description mission'>" . $mission_icon_string . drupal_render(node_view ($mission, "teaser")) . "</div>";
     
    // CHALLENGES

    $result = $result . "<h2>".t("Challenges")."</h2>";
    if ($is_admin) $result = $result . "<div class='goxeed btn admin''>".l(t("create challenge"), "node/".$mission_user->nid."/challenge_create")."</div>";
     
    $view = goxeedmodule_field_secure_get_items("node", $mission_user, "field_goxeed_challenge_user_nid");
     
    $rows = array();
    if ($view)
      foreach ($view as $challenge_user)
      {
        $node_challenge_user = goxeedmodule_node_secure_load ($challenge_user["value"]);
        $points = goxeedmodule_challenge_user_points ($node_challenge_user->nid);
        $check = "uncheck";
        $Check = "Uncheck";
        if ($points > 0)
        {
          $check = "check";
          $Check = "Check";
          $points = $points . " points";
        }
        else
        {
          $points = "";
        }
        $items = goxeedmodule_field_secure_get_items ("node", $node_challenge_user, "field_goxeed_challenge_nid");
        $node_challenge = goxeedmodule_node_secure_load ($items[0]["value"]);
        $url = url("node/".$node_challenge_user->nid);
        $rows[] = array ("onclick" => "location.href=\"".$url."\"", "data" => array(array("class" => "name", "data" => $node_challenge->title),array("class" => "points", "data" => $points), array("class" => "icon", "data" => "<span class='challenge status ".$check."'>".$Check."</span>")));
      }
      $result = $result . theme("table", array ("rows" => $rows, "attributes" => array ("class" => "goxeed mission challenges table reset")));
       
  }


  $variables["content"]["goxeed"][0]["#markup"] = $result;
  $variables["content"]["goxeed"]["#weight"] = 99;

  return $variables;
}

function goxeedmodule_mission_user_badge ($nid)
{
  $result = "";
  $node = goxeedmodule_node_secure_load ($nid);
  $items_mission = goxeedmodule_field_secure_get_items ('node', $node, 'field_goxeed_mission_nid');
  $mission = goxeedmodule_node_secure_load($items_mission[0]['value']);
  if ($node)
  {
    $items_icon = goxeedmodule_field_secure_get_items ('node', $mission, 'field_goxeed_icon');
    if ($items_icon)
    {
      $result = $items_icon[0]['value'];
      if (strlen($result)>0)
      {
        $challenges = goxeedmodule_goxeedmodule_mission_user_challenges($nid);
        $challenges_completed = goxeedmodule_goxeedmodule_mission_user_challenges_completed($nid);
        if ($challenges-$challenges_completed > 0)
        {
          $result = "";
        }
      }
    }
  }
  else
  {
    watchdog('goxeed' , 'Node '.$nid.' not found in function goxeedmodule_mission_badge.');
  }
  return $result;
  
}

/**
 * Update user points for the mission_user
 *
 * @param int $mission_user_nid
 *   Node id of the mission user
 */
function goxeedmodule_mission_user_points_update($mission_user_nid)
{
  $node = goxeedmodule_node_secure_load ($mission_user_nid);
  $points = 0;
  $mission_user = goxeedmodule_node_secure_load ($mission_user_nid);
  $challenge_user_items = goxeedmodule_field_secure_get_items ("node", $mission_user, "field_goxeed_challenge_user_nid");
  if ($challenge_user_items)
  {
    foreach ($challenge_user_items as $challenge_user_item)
    {
      $points = $points + goxeedmodule_challenge_user_points ($challenge_user_item["value"]);
    }
  }
  $items = goxeedmodule_field_secure_get_items("node", $node, "field_goxeed_user_points");
  $memory = $items[0]["value"];
  if ($points != $memory)
  {
    $node->field_goxeed_user_points["und"][0]["value"] = $points;
    node_save ($node);
    $game_users = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_game_user_nid");
    goxeedmodule_game_user_points_update($game_users[0]["value"]);
  }
}

/**
 * Get mission user points for the given mission user
 * 
 * @param int $mission_user_nid
 *   Node id of the mission user node
 *   
 * @return int
 *   Current point score
 */
function goxeedmodule_mission_user_points($mission_user_nid)
{
  $node = goxeedmodule_node_secure_load ($mission_user_nid);
  $fields = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_user_points");
  return $fields[0]["value"];
}

/**
 * Get number of challenges of the mission user
 * 
 * @param int
 *   Mission user node id
 *   
 * @return
 *   Number of challenges
 */
function goxeedmodule_goxeedmodule_mission_user_challenges ($mission_user_nid)
{
  $mission_user = goxeedmodule_node_secure_load ($mission_user_nid);
  $challenge_user_items = goxeedmodule_field_secure_get_items ("node", $mission_user, "field_goxeed_challenge_user_nid");
  return count ($challenge_user_items);
}

/**
 * Get number of completed challenges for the specified mission user
 * 
 * @param int $mission_user_nid
 *   Node id of the mission user
 *   
 * @return int
 *   Number of challenges completed
 */
function goxeedmodule_goxeedmodule_mission_user_challenges_completed ($mission_user_nid)
{
  $completed = 0;
  $mission_user = goxeedmodule_node_secure_load ($mission_user_nid);
  $challenge_user_items = goxeedmodule_field_secure_get_items ("node", $mission_user, "field_goxeed_challenge_user_nid");
  if ($challenge_user_items)
  {
    foreach ($challenge_user_items as $challenge_user_item)
    {
      if (goxeedmodule_challenge_user_completed ($challenge_user_item["value"])) $completed++;
    }
  }
  return $completed;
}

/**
 * Get challenge users of the specified mission user
 * 
 * @param int $nid
 *   Node id of the mission user
 *   
 * @return array
 *   Array of challenge users
 */
function goxeedmodule_mission_user_challenge_users($nid)
{
  $challenge_users = array();
  $node = goxeedmodule_node_secure_load ($nid);
  $challenge_user_nids = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_challenge_user_nid");
  if ($challenge_user_nids) foreach ($challenge_user_nids as $challenge_user_nid)
  {
    $challenge_users[] = $challenge_user_nid["value"];
  }
  return $challenge_users;
}

/**
 * Implements hook_preprocess_page() for goxeedmodule_mission_user nodes
 */
function goxeedmodule_goxeedmodule_preprocess_page_goxeedmodule_mission_user ($variables)
{
  global $user;
  $node_local = $variables["node"];

  // EDIT

  $items = goxeedmodule_field_secure_get_items ("node", $node_local, "field_goxeed_mission_nid");				// Get the game field and ..
  $mission_local = goxeedmodule_node_secure_load ($items[0]["value"]);										// .. load the corresponding game node.
  if ($user->uid > 1 && arg(2) == "edit")											// If edit selected and not Drupal admin
    drupal_goto ("node/".$mission_local->nid."/edit");									// .. go to edit the game

  // TITLE SET

  $items_game_user = goxeedmodule_field_secure_get_items ("node", $node_local, "field_goxeed_game_user_nid");
  $game_user_local = goxeedmodule_node_secure_load ($items_game_user[0]["value"]);
  $items_user = goxeedmodule_field_secure_get_items ("node", $game_user_local, "field_goxeed_user_uid");			// Get user id field items
  $user_local = goxeedmodule_user_secure_load($items_user[0]["value"]);									// Load user object
  $title_new = $mission_local->title;												// Title is initially just the title of the game
  global $user;
  if ($user->uid != $items_user[0]["value"])										// If the user in not the logged in user
  {
    $title_new = $title_new . " " . t("for") . " " .
        goxeed_real_name($user_local);											// Add the name of the user to the title
  }
  drupal_set_title ($title_new);  // Set the changed title in Drupal
  
  return $variables;
}
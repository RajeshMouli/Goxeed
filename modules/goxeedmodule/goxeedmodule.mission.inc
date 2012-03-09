<?php
/**
 * This file contains functions handeling the mission elements of the GoXeed system.
 *
 * Written 2012 by Nicolas Kristoffersen (nicolaskristoffersen.com) at Wemind (wemind.dk)
 */

/**
 * Create a mission that belongs to the specified game
 * 
 * @param $game
 *   Node object of the game
 *   
 * @return
 *   Node object of the new mission
 *
 */

function goxeed_goxeed_mission_add($game)
{
  $mission = new stdClass();												// Create a standard object
  $mission->type = "goxeed_mission";											// Make it a mission node
  node_object_prepare($mission);											// Prepare the node
  $mission->title = t("Mission") . " - ". $game->title;					// Set the title to be the game name and "New Mission" added
  $mission->field_goxeed_game_nid["und"][0]["value"] = $game->nid;		// Set game reference to the game of this node
  node_save($mission);													// Save the node
  goxeed_game_mission_add ($game->nid, $mission->nid);
  return $mission;
}

/**
 * Add a challenge reference to the specified mission
 * 
 * @param $mission_nid
 *   Node id of the mission
 *   
 * @param $challenge_nid
 *   Node id of the challenge
 */
function goxeed_goxeed_mission_challenge_add ($mission_nid, $challenge_nid)
{
  $mission = goxeed_node_secure_load ($mission_nid);
  $mission->field_goxeed_challenge_nid["und"][] = array ("value" => $challenge_nid);
  node_save ($mission);
}


/**
 * Remove a challenge from the mission
 *
 * @param $nid
 *   Node id of the mission
 * @param $target_nid
 *   Node id of the challenge to remove
 */
function goxeed_mission_challenge_remove ($nid, $target_nid)
{
  $node = goxeed_node_secure_load ($nid);
  $items = goxeed_field_secure_get_items ('node', $node, 'field_goxeed_challenge_nid');
  foreach ($items as $key => $item)
  {
    if ($item["value"] == $target_nid)
    {
      unset($node->field_goxeed_challenge_nid['und'][$key]);
    }
  }
  node_save ($node);
}

/**
 * Remove a mission user from the mission
 *
 * @param $nid
 *   Node id of the mission
 * @param $target_nid
 *   Node id of the mission user to remove
 */
function goxeed_mission_mission_user_remove ($nid, $target_nid)
{
  $node = goxeed_node_secure_load ($nid);
  $items = goxeed_field_secure_get_items ('node', $node, 'field_goxeed_mission_user_nid');
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
 * Implement hook_node_presave for goxeed_mission nodes
 */
function goxeed_goxeed_goxeed_mission_presave($node)
{

  if (!isset($node->nid))
  {
    $game = goxeed_node_secure_load ($node->field_goxeed_game_nid["und"][0]["value"]);
    $game_user_items = $game->field_goxeed_game_user_nid["und"];
     
    foreach ($game_user_items as $game_user_nid)
    {

      $mission_user = goxeed_mission_user_create($game_user_nid["value"]);
      goxeed_goxeed_mission_mission_user_add($node, $mission_user->nid);
    }
  }
}

function goxeed_goxeed_goxeed_mission_node_form_alter($form)
{
  global $user; if ($user->uid>1)
  {
    $form["field_goxeed_game_nid"]["#attributes"]["style"] = array('display:none;');
    $form["field_goxeed_mission_user_nid"]["#attributes"]["style"] = array('display:none;');
    $form["field_goxeed_challenge_nid"]["#attributes"]["style"] = array('display:none;');
  }
  return $form;
}

/**
 * Add mission user
 * 
 * @param $mission
 *   Node object of the mission
 * @param $mission_user_nid
 *   Node id of the mission user node
 */
function goxeed_goxeed_mission_mission_user_add (&$mission, $mission_user_nid)								// Takes game node id and game user node id
{
  $found = false;																			// Initially assume user not alreay registered for this game
  $mission_user_items = goxeed_field_secure_get_items ("node", $mission, "field_goxeed_mission_user_nid");					// Get current game users
  if ($mission_user_items)																	// If any exists ..
    foreach ($mission_user_items as $mission_user_item)										// .. loop though them ...
    if ($mission_user_item["value"] == $mission_user_nid)	$found = true;					// .. if new user is found set flag for found
  if (!$found)																			// If not already found ..
  {
    $mission->field_goxeed_mission_user_nid["und"][] = array ("value" => $mission_user_nid);			// .. add the user
  }
}

/**
 * Implement hook_node_insert for mission nodes
 */
function goxeed_goxeed_goxeed_mission_insert($node)
{
  $mission_user_items = goxeed_field_secure_get_items ("node", $node, "field_goxeed_mission_user_nid");
  foreach ($mission_user_items as $mission_user_item)
  {
    goxeed_goxeed_mission_user_mission_nid_set ($mission_user_item["value"], $node->nid);
  }
}

/**
 * Implement hook_node_delete for goxeed_mission node
 */
function goxeed_goxeed_goxeed_mission_delete($node)
{
  $node = goxeed_node_secure_load ($node->nid);
  $items = goxeed_field_secure_get_items ('node', $node, 'field_goxeed_challenge_nid');
  if ($items) foreach ($items as $item)
  {
    node_delete ($item['value']);
  }
  $items = goxeed_field_secure_get_items ('node', $node, 'field_goxeed_mission_user_nid');
  if ($items) foreach ($items as $item)
  {
    node_delete ($item['value']);
  }
  $items = goxeed_field_secure_get_items ('node', $node, 'field_goxeed_game_nid');
  if ($items) foreach ($items as $item)
  {
    goxeed_game_mission_remove ($item['value'], $node->nid);
  }

}

/**
 * Implements hook_node_access for goxeed_mission nodes
 */
function goxeed_goxeed_goxeed_mission_access($node, $op, $account)
{
  $result = NODE_ACCESS_IGNORE;
  //if ($op == "create") $result = NODE_ACCESS_DENY;
  if ($op == "update" || $op == "delete")
  {
    $items_game = goxeed_field_secure_get_items ("node", $node, "field_goxeed_game_nid");
    $game_local = goxeed_node_secure_load($items_game[0]['value']);
    if (node_access ("update", $game_local, $account))
      $result = NODE_ACCESS_ALLOW;
  }

  return $result;
}


/**
 *  Get HTML for badges belonging to the specified user
 *
 * @param int $uid
 *  User id of the user
 *
 *  @param int $number
 *  Number of bedges to return (maximum)
 */
  function goxeed_mission_badges_html ($uid, $number)
  {
    $badges_html = "";
    $badges = goxeed_user_profile_badges ($uid);    // Get array of badges earned by the current user
    $badges = array_unique($badges);
    $badges_in_header = $number;  
    $badge_keys = array();
    if ($badges_in_header < 0)
    {
      $badges_in_header = count($badges);
    }
    else
    {
      if (count($badges) < $badges_in_header) $badges_in_header = count($badges);
    }
    if ($badges_in_header > 0) $badge_keys = array_rand($badges, $badges_in_header);
    if (count($badge_keys) > 1)
    {
      shuffle ($badge_keys);
    }
    $icons = goxeed_goxeed_mission_icons();                       // Get array of available badges
    $badges_html = '';
    foreach ($badge_keys as $badge_key)
    {
      $icon = $icons[$badges[$badge_key]];
      $badges_html = $badges_html . '<li class="icon '.$badges[$badge_key].'">'.$icon.'</li>';
    }
    return $badges_html;
  }

/**
 * Implements hook_preprocess_nodes for goxeed_mission nodes
 */
function goxeed_goxeed_preprocess_goxeed_mission($variables)
{
  global $user;

  $node_local = $variables["node"];
  $result = "";

  {
    if (node_access("view", $node_local))
    {
      switch ($variables["view_mode"])
      {
        case "teaser":
          $variables["title"] = "";
          unset($variables["content"]["links"]);
          break;
           
        default:
           
          $mission_user_nid = goxeed_goxeed_module_mission_user_nid($node_local->nid, $user->uid);	// Get nid of game_user object (-1 if none)

        if (node_access ("update", $node_local))										// If the user has editing rights for the mission ...
        {
          if (arg(2) != "delete" && arg(2) !="edit")
            drupal_goto("node/".$mission_user_nid);									// ... go to the mission_user page
        }
        else																			// If it is a normal user ...
        {
          if ($mission_user_nid>-1)														// ... and if a game_user node exists  ..
          {
            drupal_goto("node/".$mission_user_nid);									// ... go to the game_user page
          }
          else																		// If a game_user does not exist ...
          {
            $items = goxeed_field_secure_get_items("node", $node_local, "field_goxeed_game_nid");
            drupal_goto("node/".$items[0]["nid"]);									// ... show the game signup form
          }
        }

        break;
      }
    }
  }
   
  $variables["content"]["goxeed"][0]["#markup"] = $result;
  $variables["content"]["goxeed"]["#weight"] = 99;


  return $variables;
}
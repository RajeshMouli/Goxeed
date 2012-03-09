<?php

/**
 * This file contains functions handeling the challenge elements of the GoXeed system.
 * 
 * Written 2012 by Nicolas Kristoffersen (nicolaskristoffersen.com) at Wemind (wemind.dk)
 */


/**
 * Add a challenge to the database
 * 
 * @param $mission
 *   Reference for the mission node object
 *   
 * @return
 *   The challenge node object
 */
function goxeedmodule_challenge_add($mission)
{
  
  $challenge = new stdClass();											                        // Create a standard object
  $challenge->type = "goxeedmodule_challenge";										                // Make it a challenge node
  node_object_prepare($challenge);										                      // Prepare the node
  $challenge->title = t("Challenge - ").$mission->title;			              // Set the title
  $challenge->field_goxeed_mission_nid["und"][0]["value"] = $mission->nid;	// Set game reference to the game of this node
  node_save($challenge);                                                    // Save the node
  goxeedmodule_goxeedmodule_mission_challenge_add($mission->nid, $challenge->nid);		  // Add the challenge to the mission

return $challenge;
}

/**
 * Handle the hook_presave for goxeedmodule_challenge nodes
 */
function goxeedmodule_goxeedmodule_goxeedmodule_challenge_presave($node)
{
  if (!isset($node->nid))                                                                          // If it is a new node
  {
  		$mission_items = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_mission_nid");               // Get mission node id
  		$mission = goxeedmodule_node_secure_load ($mission_items[0]["value"]);                                          // Load the mission node
  		$mission_user_items = goxeedmodule_field_secure_get_items ("node", $mission, "field_goxeed_mission_user_nid");  // Get mission users
  		if ($mission_user_items) foreach ($mission_user_items as $mission_user_nid)                 // Loop through the mission users
  		{
  		  $challenge_user = goxeedmodule_challenge_user_create($mission_user_nid["value"]);               // Create a challenge user
  		  goxeedmodule_challenge_challenge_user_add($node, $challenge_user->nid);                         // Add the challenge user to this challenge
  		}
  }
}

/**
 * Implement hook_form_alter for goxeed challenge nodes
 * 
 * Remove the node id reference fields from the backend user interface for all users except the Drupal admin
 */
function goxeedmodule_goxeedmodule_goxeedmodule_challenge_node_form_alter($form)
{
  global $user; if ($user->uid>1)
  {
    $form["field_goxeed_mission_nid"]["#attributes"]["style"] = array('display:none;');
    $form["field_goxeed_challenge_user_nid"]["#attributes"]["style"] = array('display:none;');
  }
  return $form;
}

/**
 * Add a challenge user reference to the challenge
 * 
 * @param $challenge
 *   The challenge node
 *   
 * @param $challenge_user_nid
 *   The challange user node id to add to the challenge node
 */
function goxeedmodule_challenge_challenge_user_add (&$challenge, $challenge_user_nid)					        			// Takes game node id and game user node id
{
  $found = false;																			                                                // Initially assume user not alreay registered for this game
  $challenge_user_items = goxeedmodule_field_secure_get_items ("node", $challenge, "field_goxeed_challenge_user_nid");		// Get current game users
  if ($challenge_user_items)																	                                        // If any exists ..
    foreach ($challenge_user_items as $challenge_user_item)										                        // .. loop though them ...
    if ($challenge_user_item["value"] == $challenge_user_nid)	$found = true;				                	// .. if new user is found set flag for found
  if (!$found)																			                                                  // If not already found ..
  {
    $challenge->field_goxeed_challenge_user_nid["und"][] = array ("value" => $challenge_user_nid);		// .. add the user
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
function goxeedmodule_challenge_challenge_user_remove ($nid, $target_nid)
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


/**
 * Implement hook_insert for goxeed challenge nodes
 * 
 * Set the reference to the challenge on all challenge user node belonging to it
 */
function goxeedmodule_goxeedmodule_goxeedmodule_challenge_insert($node)
{
  $challenge_user_items = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_challenge_user_nid");
  if ($challenge_user_items) foreach ($challenge_user_items as $challenge_user_item)
  {
    $challenge_user = goxeedmodule_node_secure_load($challenge_user_item["value"]);
    $challenge_user->field_goxeed_challenge_nid["und"][0]["value"] = $node->nid;
    node_save ($challenge_user);
  }
}

/**
 * Implement hook_delete()
 */
function goxeedmodule_goxeedmodule_goxeedmodule_challenge_delete($node)
{
  $node = goxeedmodule_node_secure_load($node->nid);
  $items = goxeedmodule_field_secure_get_items ('node', $node, 'field_goxeed_challenge_user_nid');
  foreach ($items as $item)
  {
    node_delete ($item['value']);
  }
  $items = goxeedmodule_field_secure_get_items ('node', $node, 'field_goxeed_mission_nid');
  foreach ($items as $item)
  {
    goxeedmodule_mission_challenge_remove ($item['value'], $node->nid);
  }
}

/**
 * Implement hook_access for goxeedmodule_challenge nodes
 */
function goxeedmodule_goxeedmodule_goxeedmodule_challenge_access($node, $op, $account)
{
  $result = NODE_ACCESS_IGNORE;
  if ($op == "update" || $op == "delete")
  {
    $items_mission = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_mission_nid");
    $mission_local = goxeedmodule_node_secure_load ($items_mission[0]["value"]);
    if (node_access ("update", $mission_local, $account))
    {
      $result = NODE_ACCESS_ALLOW;
    }
  }
  return $result;
}

/**
 * Find the challenge user node for at given challenge and user
 * 
 * @param int
 *   Node id of the challenge node
 * @param int
 *   User id of the user
 *   
 * @return int
 *   Returns the node id of the challenge user node, that represents the specified challenge for the specified user
 */
function goxeedmodule_challenge_challenge_user_find($nid, $uid)
{
  $result = NULL;
  $node = goxeedmodule_node_secure_load($nid);
  if ($node)
  {
    $items = goxeedmodule_field_secure_get_items('node', $node, 'field_goxeed_challenge_user_nid');
    if ($items)
    {
      foreach ($items as $item)
      {
        $game_user_nid = goxeedmodule_challenge_user_game_user($item['value']);
        $uid_temp = goxeedmodule_game_user_user_uid($game_user_nid);
        if ($uid_temp == $uid) $result = $item['value'];
      }
    }
  }
  return $result;
}

/**
 * Implement hook_preprocess_node for challenge nodes
 * 
 * @param $variables
 *   Varibles array
 */
function goxeedmodule_goxeedmodule_preprocess_goxeedmodule_challenge($variables)
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
           
          $challenge_user_nid = goxeedmodule_module_challenge_user_nid ($node_local->nid, $user->uid);	// Get nid of challenge user object (-1 if none)

        if (node_access ("update", $node_local))										                              // If the user has editing rights for the mission ...
        {
          if (arg(2) != "delete" && arg(2) !="edit")
            drupal_goto("node/".$challenge_user_nid);									                            // ... go to the mission_user page
        }
        else																			                                                // If it is a normal user ...
        {
          if ($challenge_user_nid>-1)														                                  // ... and if a game_user node exists  ..
          {
            drupal_goto("node/".$challenge_user_nid);									                            // ... go to the game_user page
          }
          else																		                                                // If a game_user does not exist ...
          {
            $items = goxeedmodule_field_secure_get_items("node", $node_local, "field_goxeed_mission_nid");
            drupal_goto("node/".$items[0]["value"]);									                            // ... show the game signup form
          }
        }
        break;
      }
    }
  }
   
  $variables["content"]["goxeed"][0]["#markup"] = $result;                                      // Add content to the variables array
  $variables["content"]["goxeed"]["#weight"] = 99;

  return $variables;

}


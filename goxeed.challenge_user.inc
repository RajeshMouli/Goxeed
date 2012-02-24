<?php

/**
 * This file contains functions handeling challenge user nodes of the GoXeed system.
 * 
 * Written 2012 by Nicolas Kristoffersen (nicolaskristoffersen.com) at Wemind (wemind.dk)
 */


/**
 * Create a challenge user node
 *
 * @param $mission_user_nid
 *   Node id of the mission use the challenge user belongs to
 *   
 * @return
 *   array The new node
 */
function goxeed_challenge_user_create($mission_user_nid)
{

  $node_new = new stdClass();															                            // Create an empty object
  $node_new->type = "goxeed_challenge_user";													                // Set the content type to challenge_user
  node_object_prepare($node_new);														                          // Prepare the node
   
  $node_new->title =  "challenge";													                          // Set the title
  $node_new->language = LANGUAGE_NONE;
  $node_new->field_goxeed_mission_user_nid["und"][0]["value"] = $mission_user_nid;		// Set the game_user reference
   
  node_save($node_new);																                                // Save the node
   
  goxeed_mission_user_challenge_user_add ($mission_user_nid, $node_new->nid);
   
  return $node_new;																	                                  // Return the node
}

/**
 * Set the reference to the challenge this challenge user belongs to
 * 
 * @param $challenge_user_nid
 *   Node id of the challenge user
 *   
 * @param $challenge_nid
 *   Node id of the challenge
 */
function goxeed_challenge_user_challenge_nid_set ($challenge_user_nid, $challenge_nid)
{
  $challenge_user = goxeed_node_secure_load ($challenge_user_nid);
  $challenge_user->field_goxeed_challenge_nid["und"][0]["value"] = $challenge_nid;
  node_save ($challenge_user);
}


/**
 * Implements hook_node_delete for goxeed_challenge_user nodes
 */
function goxeed_goxeed_goxeed_challenge_user_delete($node)
{
  $node = goxeed_node_secure_load ($node->nid);
  $items = goxeed_field_secure_get_items('node', $node, 'field_goxeed_mission_user_nid');
  foreach ($items as $item)
  {
    goxeed_mission_user_challenge_user_remove($item['value'], $node->nid);
  }
  $items = goxeed_field_secure_get_items('node', $node, 'field_goxeed_challenge_nid');
  foreach ($items as $item)
  {
    goxeed_challenge_challenge_user_remove($item['value'], $node->nid);
  }
}

/**
 * Update the challenge user points
 * 
 * @param $challenge_user_nid
 *   Node id of the challenge user to update
 */
function goxeed_challenge_user_points_update($challenge_user_nid)
{
  $node = goxeed_node_secure_load ($challenge_user_nid);
  $points = "0";
  $comment_current_items = goxeed_field_secure_get_items("node", $node, "field_goxeed_challenge_comment");
  $comment_current = $comment_current_items[0]["value"];
  if (strlen ($comment_current) > 0) $points = "10";
  $items = goxeed_field_secure_get_items("node", $node, "field_goxeed_user_points");
  $memory = $items[0]["value"];
  if ($points != $memory)
  {
    $node->field_goxeed_user_points["und"][0]["value"] = $points;
    $mission_users = goxeed_field_secure_get_items ("node", $node, "field_goxeed_mission_user_nid");
    node_save ($node);
    goxeed_mission_user_points_update($mission_users[0]["value"]);
  }
}

/**
 * Get the current points score of the specified challenge user
 * 
 * @param $challenge_user_nid
 *   Node id of the challenge user
 */
function goxeed_challenge_user_points($challenge_user_nid)
{
  $result = '';
  $node = goxeed_node_secure_load ($challenge_user_nid);
  $fields = goxeed_field_secure_get_items ("node", $node, "field_goxeed_user_points");
  $result = $fields[0]["value"];
  return $result;
}


// *
// * CHALLENGE USER FIND
// * This function finds the challenge user object for a given game.
// *

/**
 * Find challenge user node for a given challenge and user
 * 
 * @param $challenge_nid
 *   Node id of the challenge node
 *   
 * @param $user_uid
 *   User id
 *   
 * @return
 *   integer Node id of the challenge user or -1 if none exists
 *   
 * This function is called if the system needs to display a challenge for a known user.
 */
function goxeed_module_challenge_user_nid ($challenge_nid, $user_uid)
{
  $result = -1;																			// Assume the user is not signed up for the game
  $challenge = goxeed_node_secure_load ($challenge_nid);
  $challenge_user_items = goxeed_field_secure_get_items ("node", $challenge, "field_goxeed_challenge_user_nid");
  if ($challenge_user_items) foreach ($challenge_user_items as $challenge_user_item)
  {
    $game_user_nid = goxeed_challenge_user_game_user ($challenge_user_item["value"]);
    $game_user = goxeed_node_secure_load ($game_user_nid);
    $user_items = goxeed_field_secure_get_items ("node", $game_user, "field_goxeed_user_uid");
    if ($user_items[0]["value"] == $user_uid) $result = $challenge_user_item["value"];
  }
  return $result;
}

/**
 * Create the challenge form
 */
function goxeed_challenge_user_form($form, &$form_state) {
  $form['comment'] = array(
      '#type' => 'textarea',
  );
  // unset ($form['comment']['#cols']);
  $form['submit'] = array(
      '#type' => 'submit',
      //	    '#value' => t('Complete and post to Chatter'),
      '#value' => t('Complete challenge'),
  );
  return $form;
}

/**
 * Handle submittet data for the challenge user node
 */
function goxeed_challenge_user_form_submit($form, &$form_state)
{
  global $user;
  $node_local = goxeed_node_secure_load (arg(1));
  $mission_user_items = goxeed_field_secure_get_items("node", $node_local, "field_goxeed_mission_user_nid");
  $mission_user_node = goxeed_node_secure_load ($mission_user_items[0]["value"]);
  $game_user_items = goxeed_field_secure_get_items ("node", $mission_user_node, "field_goxeed_game_user_nid");
  $game_user_node = goxeed_node_secure_load ($game_user_items[0]["value"]);
  $user_local_items = goxeed_field_secure_get_items ("node", $game_user_node, "field_goxeed_user_uid");
  if ($user_local_items[0]["value"] == $user->uid)
  {
    if (strlen($form_state['values']['comment'])>0)
    {
      $node_local->field_goxeed_challenge_comment["und"][0]["value"] = $form_state["values"]["comment"];
      node_save ($node_local);
      goxeed_challenge_user_points_update($node_local->nid);
      $items = goxeed_field_secure_get_items ('node', $node_local, 'field_goxeed_mission_user_nid');
      drupal_goto ('node/'.$items[0]['value']);
    }
    else
    {
      drupal_set_message (t('You need to write down your experience to complete the challenge'));
    }
  }
}

/**
 * Implement hook_node_access for challenge user nodes.
 */
function goxeed_goxeed_goxeed_challenge_user_access($node, $op, $account)
{
  $result = NODE_ACCESS_IGNORE;
  if ($op != "create")
  {
    $node = goxeed_node_secure_load ($node->nid);
    $result = NODE_ACCESS_DENY;
    if ($op == "view")
    {
      $items = goxeed_field_secure_get_items ("node", $node, "field_goxeed_mission_user_nid");
      $game_user_local = goxeed_node_secure_load ($items[0]["value"]);
      if (node_access("view", $game_user_local, $account))
        $result = NODE_ACCESS_ALLOW;
    }
  }
  return $result;
}

/**
 * Check if the challenge has been completed for this challenge user node
 * 
 * @param $challenge_user_nid
 *   Node id of the challenge user
 *   
 * @return boolean
 *   True if completed and false if not completed
 */
function goxeed_challenge_user_completed($challenge_user_nid)
{
  $completed = false;
  $challenge_user = goxeed_node_secure_load ($challenge_user_nid);
  $comment_current_items = goxeed_field_secure_get_items("node", $challenge_user, "field_goxeed_challenge_comment");
  $comment_current = $comment_current_items[0]["value"];
  if (strlen($comment_current) > 2) $completed = true;
  return $completed;
}

/**
 * Implement preprocess for goxeed_challenge_user nodes
 */
function goxeed_goxeed_preprocess_goxeed_challenge_user($variables)
{
  global $user;

  $node_local = $variables["node"];
  $node = $node_local;
  $result = "";
  $challenge_user = goxeed_node_secure_load ($node->nid);
  if ($challenge_user)
  if (node_access("view", $node_local))
  {
    switch ($variables["view_mode"])
    {
      case "teaser":
        $items_challenge = goxeed_field_secure_get_items ("node", $node_local, "field_goxeed_challenge_nid");
        $items_user_points = goxeed_field_secure_get_items ("node", $node_local, "field_goxeed_user_points");
        $challenge_local = goxeed_node_secure_load($items_challenge[0]["nid"]);
        $text_local = $challenge_local->title; // . " " . $items_user_points[0]["value"];
        $result = $result . (l($text_local, "node/".$node_local->nid));
        break;

      default:

      $items = goxeed_field_secure_get_items("node", $node_local, "field_goxeed_challenge_nid");
      $challenge_local = goxeed_node_secure_load($items[0]["value"]);
      $challenge = $challenge_local;
      $items_mission_user = goxeed_field_secure_get_items ("node", $node_local, "field_goxeed_mission_user_nid");
      $mission_user_local = goxeed_node_secure_load($items_mission_user[0]["value"]);
      $items_game_user = goxeed_field_secure_get_items("node", $mission_user_local, "field_goxeed_game_user_nid");
      $game_user_local = goxeed_node_secure_load($items_game_user[0]["value"]);
      $items_user = goxeed_field_secure_get_items ("node", $game_user_local, "field_goxeed_user_uid");
      $user_local = goxeed_user_secure_load($items_user[0]["value"]);

      //					$temp = goxeed_field_secure_get_items ("node", $mission_user, "field_mission_nid");
      //					$node_temp = goxeed_node_secure_load ($
      $is_admin = node_access ("update", $challenge);

      if ($is_admin)																	// If user is administrator of the game ...
      {
        $result = $result . "<div class='goxeed btn admin'>";
        $result = $result . l(t("edit this challenge"), "node/". $challenge_local->nid."/edit");
        $result = $result . "</div>";
      }
       
                if ($user_local->uid != $user->uid)
      {
                    $user_logged_in = goxeed_user_secure_load($user->uid);
                    $result = $result . "<div id='goxeed_show_extra'>". t("Show this challenge for"). " ".l(goxeed_real_name($user_logged_in), "node/".$challenge_local->nid)." / ";
                    $result = $result .t("Show profile for "). " ".l(goxeed_real_name($user_local), "user/".$user_local->uid)."</div>";
      }
       
      // $result = $result . "<div class='goxeed description challenge'>". drupal_render(node_view ($challenge_local, "teaser"))."</div>";
       
      if (array_key_exists ("und", $challenge->field_goxeed_description))
      {
        $items = goxeed_field_secure_get_items ('node', $challenge, 'field_goxeed_description');
        $description = $items[0]["value"];
        //if (strpos($description, ">")<1)
        $description = nl2br($description);
        $result = $result . "<div id='goxeed description'>".$description."</div>";
      }
       
       
       
      $comment_current_items = goxeed_field_secure_get_items("node", $node_local, "field_goxeed_challenge_comment");
      $comment_current = $comment_current_items[0]["value"];
       
      if (strlen($comment_current) < 1 && ($items_user[0]["value"] == $user->uid))
      {
        $result = $result . "<h3>".t("Write down your experience completing this challenge")."</h3>";
        $result = $result . drupal_render(drupal_get_form('goxeed_challenge_user_form'));
      }
      else
      {
        //$result = $result . "<h3>".t("Write down your experience completing this challenge")."</h3>";
      }

       
      if (strlen($comment_current) > 0)
      {
        //$result = $result . "<p>" . $comment_current . "</p>";
        if ($is_admin)
        {
          if (arg(2) == "revoke")
          {
            unset($challenge_user->field_goxeed_challenge_comment["und"]);
            node_save ($challenge_user);
            goxeed_challenge_user_points_update($challenge_user->nid);
            drupal_goto ("node/". $challenge_user->nid);
          }

          /*
           $result = $result . "<div class='goxeed btn admin'>";
          $result = $result . l(t("revoke this comment"), "node/". $challenge_user->nid."/revoke");
          $result = $result . "</div>";
          */
        }
      }
      
      if ($user->uid != $user_local->uid)
      {
        drupal_goto ('node/'.$challenge_local->nid);
      }
      
       
      if (strlen($comment_current) == 0) if ($items_user[0]["value"] != $user->uid)
      {
        $result = $result . t("No comment written yet");
      }

      // Others comments
       
      $comments_html = "";
      $challenge_user_items = goxeed_field_secure_get_items ("node", $challenge, "field_goxeed_challenge_user_nid");
      foreach ($challenge_user_items as $challenge_user_item)
      {
        //if ($challenge_user_item["value"] != $challenge_user->nid)
        {
          $challenge_user_nid = $challenge_user_item["value"];
          $game_user_nid = goxeed_challenge_user_game_user ($challenge_user_nid);
          if (goxeed_game_user_active ($game_user_nid) == "1")
          {
            $challenge_user_current = goxeed_node_secure_load ($challenge_user_item["value"]);
            $comment_items = goxeed_field_secure_get_items ("node", $challenge_user_current, "field_goxeed_challenge_comment");
            $comment = $comment_items[0]["value"];
            if (strlen($comment)>0)
            {
              $game_user = goxeed_node_secure_load ($game_user_nid);
              $user_items = goxeed_field_secure_get_items("node", $game_user, "field_goxeed_user_uid");
              $user_current = goxeed_user_secure_load ($user_items[0]["value"]);
              $user_current_name = goxeed_real_name($user_current);
              //$comments_html = $comments_html . "<li>" . l($comment, "node/".$challenge_user_current->nid) . " ".t("by") . " " . l($user_current_name, "user/".$user_current->uid) . "</li>";

              $image_url_items = goxeed_field_secure_get_items ("user", $user_current, "field_goxeed_li_pic_url");
              $image_url = $image_url_items[0]["value"];
              $class_own = "";
              if ($challenge_user_item["value"] != $challenge_user->nid)
              {
                $class_own = "active ";
              }
              
              $date_time = format_date($challenge_user_current->changed, 'custom', 'm.d.Y - H:i');
              
              $comments_html = $comments_html . "<li class='".$class_own."comment'><a href='".url("user/".$user_current->uid)."'><img src='".$image_url."' /></a><b>".goxeed_first_name($user_current).":</b><p>" .$comment. '</p><p class="datetime">'.$date_time.'</p></li>';
              //if ($challenge_user_item["value"] == $challenge_user->nid)
              if ($is_admin)
              {
                $comments_html = $comments_html . "<div class='goxeed btn admin'>";
                $comments_html = $comments_html . l(t("revoke the above comment"), "node/". $challenge_user_nid."/revoke");
                $comments_html = $comments_html . "</div>";
              }
            }
          }
        }
      }
       
      if (strlen($comments_html) > 0)
      {
        $result = $result . "<div id='experiences' class='challenge'><h3>".t("Experiences with this challenge")."</h3><ul class='goxeed comments user-profile'>";
        $result = $result . $comments_html;
        $result = $result . "</ul></div>";
      }
      break;
    }
  }

  unset($variables["content"]);
  $variables["content"]["goxeed"][0]["#markup"] = $result;
  $variables["content"]["goxeed"]["#weight"] = 99;

  return $variables;

}

// Get the game user node id corrensponding to a given challenge user node id
/**
 * Find the game user node id for a given challenge user node
 * 
 * @param $challenge_user_nid
 *   Node id of the challenge user
 *   
 * @return
 *   Node id of the game user
 */
function goxeed_challenge_user_game_user ($challenge_user_nid)
{
  $result = "";
  $challenge_user = goxeed_node_secure_load ($challenge_user_nid);
  $mission_user_items = goxeed_field_secure_get_items ("node", $challenge_user, "field_goxeed_mission_user_nid");
  $mission_user = goxeed_node_secure_load ($mission_user_items[0]["value"]);
  $game_user_items = goxeed_field_secure_get_items ("node", $mission_user, "field_goxeed_game_user_nid");
  $result =$game_user_items[0]["value"];
  return $result;
}

/**
 * Get the game the challenge user belongs to
 * 
 * @param $challenge_user_nid
 *   Node id if the challenge user
 *   
 * @return
 *   Node id of the game
 */
function goxeed_challenge_user_game ($challenge_user_nid)
{
  $game_user_nid = goxeed_challenge_user_game_user ($challenge_user_nid);
  $game_nid = goxeed_game_user_game ($game_user_nid);
  return $game_nid;
}

/**
 * Implement hook_preprocess_page for goxeed_challenge_user nodes
 */
function goxeed_goxeed_preprocess_page_goxeed_challenge_user ($variables)
{

  global $user;

  $challenge_user = $variables["node"];
  $challenge_user = goxeed_node_secure_load ($challenge_user->nid);
  
  // EDIT
   
  $challenge_items = goxeed_field_secure_get_items ("node", $challenge_user, "field_goxeed_challenge_nid");
  $challenge = goxeed_node_secure_load ($challenge_items[0]["value"]);
  if ($user->uid > 1 && arg(2) == "edit")											// If edit selected and not Drupal admin
    drupal_goto ("node/".$challenge->nid."/edit");									// .. go to edit the game

  // TITLE SET
   
  $mission_user_items = goxeed_field_secure_get_items ("node", $challenge_user, "field_goxeed_mission_user_nid");
  $mission_user = goxeed_node_secure_load ($mission_user_items[0]["value"]);
  $game_user_items = goxeed_field_secure_get_items ("node", $mission_user, "field_goxeed_game_user_nid");
  $game_user = goxeed_node_secure_load ($game_user_items[0]["value"]);
  $user_items = goxeed_field_secure_get_items ("node", $game_user, "field_goxeed_user_uid");			// Get user id field items
  $user_local = goxeed_user_secure_load($user_items[0]["value"]);									// Load user object
  $title = $challenge->title;											// Title is initially just the title of the game
  global $user;
  if ($user->uid != $user_items[0]["value"])										// If the user in not the logged in user
  {
    $title = $title . " " . t("for") . " " .
        goxeed_real_name($user_local);											// Add the name of the user to the title
  }
  drupal_set_title ($title);													// Set the changed title in Drupal
   
  $game_items = goxeed_field_secure_get_items ("node", $game_user, "field_goxeed_game_nid");
  $game = node_load ($game_items[0]["value"]);

  return NULL; //$variables;
  
}
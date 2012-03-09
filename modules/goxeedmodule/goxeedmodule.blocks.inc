<?php

/**
 * This file defines the blocks of the GoXeed system.
 * 
 * Written 2012 by Nicolas Kristoffersen (nicolaskristoffersen.com) at Wemind (wemind.dk)
 */


/**
 * Implements hook_block_info()
 */
function goxeedmodule_block_info() {
  
  $blocks = array();
  $blocks['goxeedmodule_points'] = array(
      'info' => t('GoXeed Points'),
      'region' => 'sidebar_second',
      'weight' => -1,
      'status' => 1,
      'cache' => DRUPAL_NO_CACHE,
  );
  $blocks['goxeedmodule_currently_playing'] = array(
      'info' => t('GoXeed Currently Playing'),
      'region' => 'sidebar_second',
      'status' => 1,
      'cache' => DRUPAL_NO_CACHE,
  );
  $blocks['goxeedmodule_has_completed'] = array(
      'info' => t('GoXeed Has Completed'),
      'region' => 'sidebar_second',
      'status' => 1,
      'cache' => DRUPAL_NO_CACHE,
  );
  $blocks['goxeedmodule_video'] = array(
      'info' => t('GoXeed Video'),
      'region' => 'sidebar_second',
      'status' => 1,
      'cache' => DRUPAL_NO_CACHE,
  );
  $blocks['goxeedmodule_links'] = array(
      'info' => t('GoXeed Links'),
      'region' => 'sidebar_second',
      'status' => 1,
      'cache' => DRUPAL_NO_CACHE,
  );
  $blocks['goxeedmodule_nps'] = array(
      'info' => t('GoXeed NPS'),
      'region' => 'sidebar_second',
      'status' => 1,
      'cache' => DRUPAL_NO_CACHE,
  );
  $blocks['goxeedmodule_top_players'] = array(
      'info' => t('GoXeed Top Players'),
      'region' => 'sidebar_second',
      'status' => 1,
      'cache' => DRUPAL_NO_CACHE,
  );
  $blocks['goxeedmodule_header'] = array(
      'info' => t('GoXeed Header'),
      'region' => 'header',
      'status' => 1,
      'cache' => DRUPAL_NO_CACHE,
  );
  $blocks['goxeedmodule_breadcrumbs'] = array(
      'info' => t('GoXeed Breadcrumbs'),
      'region' => 'featured',
      'status' => 1,
      'cache' => DRUPAL_NO_CACHE,
  );
  return $blocks;
}

/**
 * Impements hook_block_view()
 */
function goxeedmodule_block_view($delta = '')
{
  
  $block = array();
  switch ($delta) {
    case 'goxeedmodule_points': $block = goxeedmodule_goxeedmodule_block_points(); break;
    case 'goxeedmodule_header': $block = goxeedmodule_goxeedmodule_block_header(); break;
    case 'goxeedmodule_breadcrumbs': $block = goxeedmodule_goxeedmodule_block_breadcrumbs(); break;
    case 'goxeedmodule_currently_playing': $block = goxeedmodule_goxeedmodule_block_currently_playing(); break;
    case 'goxeedmodule_has_completed': $block = goxeedmodule_goxeedmodule_block_has_completed(); break;
    case 'goxeedmodule_video': $block = goxeedmodule_goxeedmodule_block_video(); break;
    case 'goxeedmodule_nps': $block = goxeedmodule_goxeedmodule_block_nps(); break;
    case 'goxeedmodule_links': $block = goxeedmodule_goxeedmodule_block_links(); break;
    case 'goxeedmodule_top_players': $block = goxeedmodule_goxeedmodule_block_top_players(); break;
  }
  return $block;
}

/**
 * Create output for the points block
 * 
 * @return array
 *   Block array
 */
function goxeedmodule_goxeedmodule_block_points()
{
  $block = array();
  $name = t("I");
  $points = "";
  $points_temp = "";
  if (arg(0) == "node")
  {
    $node = goxeedmodule_node_secure_load (arg(1));
    if ($node)
    {
      if ($node->type == "goxeedmodule_game_user" || $node->type == "goxeedmodule_mission_user" || $node->type == "goxeedmodule_challenge_user")
      {
        global $user;
        $user_uid_current_page = goxeedmodule_goxeedmodule_user_uid_current_page();
        if ($user->uid != $user_uid_current_page)
        {
          $user_object = goxeedmodule_user_secure_load($user_uid_current_page);
          $name = goxeedmodule_real_name ($user_object);
        }
      }
      if ($node->type == "goxeedmodule_game_user")
      {
        $points = t("in this game");
        $points_temp = goxeedmodule_game_user_points($node->nid);
      }
      if ($node->type == "goxeedmodule_challenge_user")
      {
        //$points = t("in this challenge");
        //$points_temp = goxeedmodule_challenge_user_points($node->nid);
        $items = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_mission_user_nid");
        $node = goxeedmodule_node_secure_load ($items[0]["value"]);
      }

      if ($node->type == "goxeedmodule_mission_user")
      {
        $points = t("in this mission");
        $points_temp = goxeedmodule_mission_user_points($node->nid);
      }

    }
  }


  if(arg(0) == "user") if(is_numeric(arg(1)))
  {
    global $user;
    $user_uid_current_page = goxeedmodule_goxeedmodule_user_uid_current_page();
    $user_object = goxeedmodule_user_secure_load($user_uid_current_page);
    if ($user->uid != $user_uid_current_page)
    {
      $name = goxeedmodule_real_name ($user_object);
    }
    $items = goxeedmodule_field_secure_get_items ("user", $user_object, "field_goxeed_user_points");
    $points_temp = $items[0]["value"];
    $points = t("in total");
  }

  if (strlen ($points) > 0)
  {
    if (strlen($points_temp)<1) $points_temp = 0;

    $block['subject'] = t('Points');
    $block['content'] = "<p>".t("So far")." ".$name." ".t("earned")." <span>".$points_temp. "</span> ".t("points")." ".$points;
  }
  return $block;
}

/**
 * Create output for the GoXeed header block
 * 
 * @return array
 *   Block array
 */
function goxeedmodule_goxeedmodule_block_header()
{
  global $user;
  $block = array();
  unset($block['subject']);
  $user_image_html = "";
  $user_object = goxeedmodule_user_secure_load($user->uid);
  if (isset ($user_object->field_goxeed_li_pic_url["und"]))
    $user_view_mode_top_image = $user_object->field_goxeed_li_pic_url["und"][0]["value"];
  if (isset ($user_object->field_li_prof_url["und"]))
    $user_linkedin_profile_url = $user_object->field_li_prof_url["und"][0]["value"];
  if (isset($user_view_mode_top_image))
    $user_image_html = '<a href="'. url("") .'"><img src="'. $user_view_mode_top_image.'"></a>';

  $items_points = goxeedmodule_field_secure_get_items ("user", $user_object, "field_goxeed_user_points");
  //$user_points = $items_points[0]["value"];
  $user_points = goxeedmodule_user_profile_points ($user_object->uid);

  $header = "";
  
//  $badges_html = goxeedmodule_mission_badges_html ($user_object->uid, 5);
  
/*  if (strlen($badges_html)>0)
  {
      $badges_html = '<ul class ="header block badges">'.$badges_html.'</ul>';
  }
*/  
  if ($user->uid > 0)
  {
    $header = $header . $user_image_html;
    $header = $header . "<ul>";
    $header = $header . "<li class='name'>" . l(goxeedmodule_real_name($user_object), "user/".$user_object->uid) . "</li>";
    //$header = $header . "<li class='points'>" .$user_points." ". t("points") . "</li>";
    if(isset($user_linkedin_profile_url))  $header = $header . "<li class='linkedin profile'>" . l(t("LinkedIn profile"), $user_linkedin_profile_url, array ("attributes" => array("target" => "_blank"))) . "</li>";
    $header = $header . "<li class='logout'>" . l(t("Log out"), 'user/logout')."</li>";
    $header = $header . "</ul>";
 //   $header = $header . $badges_html;
  }

  $block['content'] = $header;
  return $block;
}

/**
 * Create output for the GoXeed breadcrumbs block
 * 
 * @return array
 *   Block array
 */
function goxeedmodule_goxeedmodule_block_breadcrumbs()
{
  $block = array();
  $result = "";
  if (arg(0) == "node") if (is_numeric(arg(1)))
  {
    $node = goxeedmodule_node_secure_load (arg(1));
    if ($node->type == "goxeedmodule_challenge_user")
    {
      $link = "node/".$node->nid;
      $mission_user = goxeedmodule_field_secure_get_items("node", $node, "field_goxeed_mission_user_nid");
      $challenge = goxeedmodule_field_secure_get_items("node", $node, "field_goxeed_challenge_nid");
      $node = goxeedmodule_node_secure_load ($challenge[0]["value"]);
      $result = "<li class='goxeed trail challenge'>".t("Challenge").": ".l($node->title, $link)."</li>";
      $node = goxeedmodule_node_secure_load ($mission_user[0]["value"]);
    }
    if ($node->type == "goxeedmodule_mission_user")
    {
      $link = "node/".$node->nid;
      $game_user = goxeedmodule_field_secure_get_items("node", $node, "field_goxeed_game_user_nid");
      $mission = goxeedmodule_field_secure_get_items("node", $node, "field_goxeed_mission_nid");
      $node = goxeedmodule_node_secure_load ($mission[0]["value"]);
      $result = "<li class='goxeed trail mission'>".t("Mission").": ".l($node->title, $link)."</li>".$result;
      $node = goxeedmodule_node_secure_load ($game_user[0]["value"]);
    }
    if ($node->type == "goxeedmodule_game_user")
    {
      $link = "node/".$node->nid;
      $game = goxeedmodule_field_secure_get_items("node", $node, "field_goxeed_game_nid");
      $node = goxeedmodule_node_secure_load ($game[0]["value"]);
      $result = "<li class='goxeed trail game'>".t("Game").": ".l($node->title, $link)."</li>".$result;
    }
  }
  if (strlen($result) > 0) $result = "<ul id='goxeed breadcrumb'>" . $result ."</ul>";
  $block['content'] = $result;
  return $block;
}

/**
 * Get user id of the current page
 * 
 * Helper function for various blocks.
 * 
 * @return
 *   Returns a user id of the current node, by reading its references back to a user.
 */
function goxeedmodule_goxeedmodule_user_uid_current_page()
{
  global $user;
  $user_uid = $user->uid;
  if (arg(0)=="user")
  {
    $arg1 = arg(1);
    if (is_numeric ($arg1)) $user_uid = $arg1;
  }
  if (arg(0)=="node")
  {
    if (is_numeric (arg(1)))
    {
      $node = goxeedmodule_node_secure_load (arg(1));
      if ($node->type == "goxeedmodule_challenge_user")
      {
        $items = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_mission_user_nid");
        $node = goxeedmodule_node_secure_load ($items[0]["value"]);
      }
      if ($node->type == "goxeedmodule_mission_user")
      {
        $items = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_game_user_nid");
        $node = goxeedmodule_node_secure_load ($items[0]["value"]);
      }
      if ($node->type == "goxeedmodule_game_user")
      {
        $items = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_user_uid");
        $user_uid = $items[0]["value"];
      }
    }
  }
  return $user_uid;
}

/**
 * Get current game node id.
 * 
 * This is a helper function for various block functions
 * 
 * @param $node_nid int
 *   Node id of the node currently shown
 *   
 * @return int
 *   Node id of the current game
 */
function goxeedmodule_goxeedmodule_game_current($node_nid)
{
  $result = NULL;
  $node = goxeedmodule_node_secure_load ($node_nid);
  if ($node->type == "goxeedmodule_challenge_user")
  {
    $items = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_challenge_nid");
    $node = goxeedmodule_node_secure_load ($items[0]["value"]);
  }
  if ($node->type == "goxeedmodule_challenge" || $node->type == "goxeedmodule_mission_user")
  {
    $items = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_mission_nid");
    $node = goxeedmodule_node_secure_load ($items[0]["value"]);
  }

  if ($node->type == "goxeedmodule_mission" || $node->type == "goxeedmodule_game_user")
  {
    $items = goxeedmodule_field_secure_get_items ("node", $node, "field_goxeed_game_nid");
    $result = $items[0]["value"];
  }
  if ($node->type == "goxeedmodule_game")
  {
    $result = $node_nid;
  }
  return $result;
}

/**
 * Create output for block showing games currently being played by the active player
 * 
 * @return array
 *   Block array
 */
function goxeedmodule_goxeedmodule_block_currently_playing()
{
  $block = array();
  if (arg(0) == "user") if (is_numeric(arg(1)))
  {
    $result = "";
    $user_object = goxeedmodule_user_secure_load (goxeedmodule_goxeedmodule_user_uid_current_page());
    $game_users = goxeedmodule_field_secure_get_items ("user", $user_object, "field_goxeed_game_user_nid");
    if ($game_users) foreach ($game_users as $game_user)
    {
      if (goxeedmodule_game_user_challenges_remaining($game_user["value"]) > 0)
        $result = $result . "<li>" . l(goxeedmodule_game_user_game_title($game_user["value"]), "node/".$game_user["value"]) . "</li>";

    }
    if ($result != "")
    {
      $name = t("I am currently playing");
      global $user;
      if ($user_object->uid != $user->uid) $name = goxeedmodule_real_name($user_object) . " " .t("is currently playing");
      $block["title"] = $name;
      $block["content"] = "<ul>".$result."</ul>";
    }
  }
  return $block;
}

/**
 * Create output for block showing games completed by the active player
 * 
 * @return array
 *   Block array
 */
function goxeedmodule_goxeedmodule_block_has_completed()
{
  $block = array();
  if (arg(0) == "user") if (is_numeric(arg(1)))
  {
    $result = "";
    $user_object = goxeedmodule_user_secure_load (goxeedmodule_goxeedmodule_user_uid_current_page());
    $game_users = goxeedmodule_field_secure_get_items ("user", $user_object, "field_goxeed_game_user_nid");
    if ($game_users) foreach ($game_users as $game_user)
    {
      if (goxeedmodule_game_user_challenges_remaining($game_user["value"]) == 0)
        $result = $result . "<li>" . l(goxeedmodule_game_user_game_title($game_user["value"]), "node/".$game_user["value"]) . "</li>";
    }
    if ($result != "")
    {
      $name = t("I have completed");
      global $user;
      if ($user_object->uid != $user->uid) $name = goxeedmodule_real_name($user_object) . " " .t("has completed");
      $block["title"] = $name;
      $block["content"] = "<ul>".$result."</ul>";
    }
  }
  return $block;
}

/**
 * Create output for block videos attached to the current game
 *
 * @return array
 *   Block array
 */
function goxeedmodule_goxeedmodule_block_video()
{
  $block = array();
   
  if (arg(0) == "node")
  {
    if (is_numeric(arg(1)))
    {
      $game_nid = goxeedmodule_goxeedmodule_game_current(arg(1));

      if ($game_nid)
      {
        $game = goxeedmodule_node_secure_load($game_nid);
        $items = goxeedmodule_field_secure_get_items ("node", $game, "field_goxeed_video");
        $html = $items[0]["value"];
        if (strpos ($html, "youtube.com") > 0)
        {
          $html = str_replace ("width", "_w", $html);
          $html = str_replace ("height", "_h", $html);
          $html = str_replace ("<iframe", "<iframe height='181' width='242'", $html);

        }
        $block["title"] = t("Video for the game");
        $block["content"] = $html;
      }
    }
  }

  return $block;
}

/**
 * Create output for block showing nps score for the current game
 *
 * @return array
 *   Block array
 */
function goxeedmodule_goxeedmodule_block_nps()
{
  $block = array();
   
  if (arg(0) == "node")
  {
    if (is_numeric(arg(1)))
    {
      $game_nid = goxeedmodule_goxeedmodule_game_current(arg(1));

      if ($game_nid)
      {
        $game = goxeedmodule_node_secure_load($game_nid);
        $items = goxeedmodule_field_secure_get_items ("node", $game, "field_goxeed_nps_score");
        $html = $items[0]["value"];
        $block["title"] = t("NPS of the game");
        $block["content"] = $html;
      }
    }
  }

  return $block;
}

/**
 * Create output for block showing links defined for the current game
 *
 * @return array
 *   Block array
 */
function goxeedmodule_goxeedmodule_block_links()
{
  $block = array();
   
  if (arg(0) == "node")
  {
    if (is_numeric(arg(1)))
    {
      $game_nid = goxeedmodule_goxeedmodule_game_current(arg(1));

      if ($game_nid)
      {
        $game = goxeedmodule_node_secure_load($game_nid);
         
        $html = "";
        $items = goxeedmodule_field_secure_get_items ("node", $game, "field_goxeed_link");
        if ($items) foreach ($items as $link)
        {
          $attribute_html = "";
          $attributes = $link['attributes'];
          foreach ($attributes as $key => $attribute)
          {
            $attribute_html = $key.'="'.$attribute.'" ';
          }
          $title = $link["title"];
          $url = $link["url"];
          if (strlen($title)<1) $title = $url;

          if (strlen($url)>0)
          {
            $html = $html . '<li><a ' . $attribute_html . 'href="'.$url.' ">'.$title.'</a></li>';
          }
          //						$html = '<p><a target="_blank" href="http://www.politiken.dk">politiken.dk</a></p><p><a target="_blank" href="http://www.dr.dk">dr.dk</a></p>';
        }
        if (strlen($html) > 0)
        {
          $html = '<ul class="links">'.$html .'</ul>';
        }
        $block["title"] = t("Links of the game");
        $block["content"] = $html;
      }
    }
  }

  return $block;
}

/**
 * Create output for block showing top players of the current game
 *
 * @return array
 *   Block array
 */
function goxeedmodule_goxeedmodule_block_top_players()
{
  $block = array();
   
  if (arg(0) == "node")
  {
    if (is_numeric(arg(1)))
    {
      $node = goxeedmodule_node_secure_load (arg(1));
      if ($node->type == "goxeedmodule_game_user")
      {
        $game_nid = goxeedmodule_goxeedmodule_game_current(arg(1));
         
        if ($game_nid)
        {
          $game = node_load($game_nid);
          $block["title"] = t("Top players of the game");
          $block["content"] = goxeedmodule_game_top_players($game_nid);
        }
      }
    }
  }

  return $block;
}

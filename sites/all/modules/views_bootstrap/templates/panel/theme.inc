<?php

/**
 * @file
 * Preprocessors and helper functions for panel theming.
 */

/**
 * Implementation of template preprocess for the view.
 */
function template_preprocess_views_bootstrap_panel_plugin_style(&$vars) {
  $view = &$vars['view'];

  $heading_field = $vars['options']['heading_field'];
  $body_field = $vars['options']['body_field'];
  $footer_field = $vars['options']['footer_field'];

  $vars['context'] = $vars['options']['context'];
  $vars['title_flag'] = $vars['options']['title'];
  $vars['title_tag'] = $vars['options']['title_tag'];

  $vars['headers'] = $vars['bodys'] = $vars['footers'] = array();

  // Get headers.
  if (isset($view->field[$heading_field])) {
    foreach (array_keys($vars['rows']) as $key) {
      $vars['headers'][$key] = $view->style_plugin->get_field($key, $heading_field);
    }
  }

  // Get bodys.
  if (isset($view->field[$body_field])) {
    foreach (array_keys($vars['rows']) as $key) {
      $vars['bodys'][$key] = $view->style_plugin->get_field($key, $body_field);
    }
  }

  // Get footers.
  if (isset($view->field[$footer_field])) {
    foreach (array_keys($vars['rows']) as $key) {
      $vars['footers'][$key] = $view->style_plugin->get_field($key, $footer_field);
    }
  }

}

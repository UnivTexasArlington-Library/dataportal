<?php
/**
 * @file
 * Hooks and preprocess functions for the Slick views module.
 */

/**
 * Implements hook_preprocess_slick_views().
 */
function template_preprocess_slick_views(&$variables) {
  $settings = $variables['options'];

  // Prepare the Slick wrappers for asNavFor.
  $variables['wrapper_prefix'] = '';
  $variables['wrapper_suffix'] = '';

  // Build the Slick wrappers for asNavFor.
  if (!empty($settings['nav'])) {
    // Mimick the Slick wrapper attributes.
    $attributes['class'] = array('slick-wrapper', 'slick-wrapper--asnavfor');
    if ($settings['skin'] && $settings['skin'] != 'asnavfor') {
      $attributes['class'][] = str_replace('_', '-', 'slick-wrapper--' . $settings['skin']);
    }

    $variables['wrapper_prefix'] = '<div' . drupal_attributes($attributes) . '>';
    $variables['wrapper_suffix'] = '</div>';
  }
}

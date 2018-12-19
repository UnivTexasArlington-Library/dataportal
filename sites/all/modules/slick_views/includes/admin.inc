<?php

/**
 * @file
 * Contains functions called only by admin pages.
 */

/**
 * Returns available fields for select options.
 */
function slick_views_get_defined_field_options($view) {
  $fields = $view->display_handler->get_handlers('field');

  $options = array();
  $classes = array(
    'list_text',
    'entityreference',
    'taxonomy_term_reference',
    'text',
  );
  foreach ($fields as $field => $handler) {
    if (isset($handler->field_info)) {
      $type = $handler->field_info['type'];

      switch ($type) {
        case 'file':
        case 'image':
        case 'youtube':
        case 'video_embed_field':
          $options['images'][$field] = $handler->ui_name();
          $options['overlays'][$field] = $handler->ui_name();
          $options['thumbnails'][$field] = $handler->ui_name();
          break;

        case 'list_text':
          $options['layouts'][$field] = $handler->ui_name();
          break;

        case 'entityreference':
        case 'text':
        case 'text_long':
        case 'text_with_summary':
        case 'link_field':
          $options['links'][$field] = $handler->ui_name();
          $options['titles'][$field] = $handler->ui_name();
          if ($type != 'link_field') {
            $options['thumb_captions'][$field] = $handler->ui_name();
          }
          break;
      }
      if (in_array($type, $classes)) {
        $options['classes'][$field] = $handler->ui_name();
      }
    }

    // Content: title is not really a field, unless title.module installed.
    if ($handler->field == 'title') {
      $options['classes'][$field] = $handler->ui_name();
      $options['titles'][$field] = $handler->ui_name();
      $options['thumb_captions'][$field] = $handler->ui_name();
    }

    if ($handler->field == 'nothing') {
      $options['classes'][$field] = $handler->ui_name();
      $options['links'][$field] = $handler->ui_name();
    }

    if ($handler->field == 'view_node') {
      $options['links'][$field] = $handler->ui_name();
    }

    // Caption can be anything to get custom works going.
    $options['captions'][$field] = $handler->ui_name();
  }

  return $options;
}

/**
 * Off-loaded SlickViews::options_form().
 */
function _slick_views_options_form(&$form, &$form_state, $view, $settings) {
  $elements = array();

  $fields = array(
    'captions',
    'classes',
    'fieldable',
    'images',
    'layouts',
    'links',
    'overlays',
    'thumbnails',
    'thumb_captions',
    'titles',
  );

  $options = slick_views_get_defined_field_options($view);
  $definition = array();
  foreach ($fields as $key) {
    $definition[$key] = isset($options[$key]) ? $options[$key] : array();
  }

  slick_get_top_elements($elements, $settings, $form_state);
  slick_get_grid_elements($elements, $settings, $form_state);

  $elements['slide_field_wrapper'] = array(
    '#type'        => 'checkbox',
    '#title'       => t('Slick markups'),
    '#description' => t('If checked, Slick will provide its markups for the following fields, ignoring Views row/field template suggestions. This is relevant for the provided skins as Slick needs to know what to style. Simply uncheck it to turn all Views row/field templates back on with your custom works. For more complex slide (asNavFor + nested/overlay + main BG altogether), be sure to apply filter criteria and skin properly.'),
  );

  slick_get_fieldable_elements($elements, $settings, $definition);

  // http://en.wikipedia.org/wiki/List_of_common_resolutions
  $ratio = array('1:1', '3:2', '4:3', '8:5', '16:9', 'fluid');
  $elements['aspect_ratio'] = array(
    '#type'        => 'select',
    '#title'       => t('Aspect ratio'),
    '#options'     => drupal_map_assoc($ratio),
    '#description' => t('Aspect ratio to get consistently responsive images within responsive layout. This also fixes layout reflow and excessive height issues with lazyload ondemand. <a href="@dimensions" target="_blank">Image styles and video dimensions</a> must <a href="@follow" target="_blank">follow the ratio</a>, otherwise your images will be unexpectedly distorted. <a href="@link" target="_blank">Learn more</a>, or leave empty if you care not for aspect ratio, or prefer to DIY, etc. Choose fluid if unsure. Requires Image formatter for Main Image option, and the default <strong>Use field template</strong> unchecked under its <strong>Style settings</strong>, ignore if you don\'t know.', array(
      '@dimensions' => '//size43.com/jqueryVideoTool.html',
      '@follow'     => '//en.wikipedia.org/wiki/Aspect_ratio_%28image%29',
      '@link'       => '//www.smashingmagazine.com/2014/02/27/making-embedded-content-work-in-responsive-design/',
    )),
    '#weight'       => 62,
    '#states'       => array(
      'visible' => array(
        ':input[name*="[slide_image]"]' => array('!value' => ''),
      ),
    ),
  );

  $elements['slide_image']['#description'] = t('Main image, treated as background if overlay is provided. It will be lazy-loaded if the Formatter is <strong>Image</strong> and lazyLoad is enabled. Only one image is displayed per slide, even if it is a multi-value field. Enable <strong>Aspect ratio</strong> below to fix empty space issue with lazyloaded images.');

  $elements['slide_overlay']['#description'] = t('For audio/video, be sure the display is not image. For nested slicks, use the Slick carousel formatter for this field. Zebra layout is reasonable for overlay and captions.');

  $elements['slide_thumbnail']['#description'] = t("Only needed if <em>Option set thumbnail</em> is provided. Maybe the same field as the main image, only different instance. Leave empty to not use thumbnail pager.");

  $elements['slide_classes']['#description'] = t('If provided, individual slide will have this class, e.g.: to have different background with transparent images and skin Split, or Slick filtering. Be sure to have a Key, Label or Plain text under Formatter without links accordingly. Supported fields: Node title, Entityreference, Taxonomy term, List, Text. Only works if using Fields under Format > Show.');

  $elements['id'] = array(
    '#type'         => 'textfield',
    '#title'        => t('Element ID'),
    '#size'         => 40,
    '#maxlength'    => 255,
    '#field_prefix' => '#',
    '#description'  => t("Manually define the Slick carousel container ID. <em>This ID is used for the cache identifier, so be sure it is unique</em>. Leave empty to have a guaranteed unique ID managed by the module."),
  );

  slick_get_elements($elements, $settings, $form_state);

  drupal_alter('slick_views_options_form', $elements, $settings);

  slick_get_admin_assets($elements, $settings);

  $form['slick'] = array(
    '#type' => 'fieldset',
    '#title' => t('Slick carousel'),
    '#attributes' => array('class' => array('form--slick form--compact form--field form--views has-tooltip')),
    '#description' => t('Leave anything empty/unchecked, except Optionset, if working with custom markups. <br /><small>Alternatively check <strong>Slick markups</strong> to get consistent markups with Field formatter, be sure to add supported fields below. Views preview works with jQuery > 1.7.</small>'),
  );

  $grids = array(
    'grid_medium',
    'grid_small',
    'visible_slides',
    'preserve_keys',
  );

  $no_markups = array(
    'optionset',
    'skin',
    'skin_arrows',
    'skin_dots',
    'slide_classes',
    'id',
    'grid',
    'slide_field_wrapper',
    'override',
    'overridables',
    'cache',
  ) + $grids;

  foreach ($elements as $key => $element) {
    $form['slick'][$key] = $element;
    if (isset($elements[$key]['#states'])) {
      continue;
    }
    if (!isset($elements[$key]['#type'])) {
      continue;
    }
    if (!in_array($key, $no_markups)) {
      $form['slick'][$key]['#states'] = array(
        'visible' => array(
          ':input[name*="[slide_field_wrapper]"]' => array('checked' => TRUE),
        ),
      );
    }

    if (in_array($key, array('thumbnail_caption'))) {
      $form['slick'][$key]['#states'] = array(
        'invisible' => array(
          array('select[name*="[optionset_thumbnail]"]' => array('value' => '')),
          array(':input[name*="[slide_field_wrapper]"]' => array('checked' => FALSE)),
        ),
      );
    }

    if (in_array($key, $grids)) {
      $form['slick'][$key]['#states'] = array(
        'invisible' => array(
          array('select[name$="[grid]"]' => array('value' => '')),
        ),
      );
    }

    if ($key == 'overridables') {
      $form[$key]['#states'] = array(
        'visible' => array(
          ':input[name$="[override]"]' => array('checked' => TRUE),
        ),
      );
    }
  }
}

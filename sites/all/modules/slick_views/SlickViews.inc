<?php
/**
 * @file
 * Slick style plugin for the Views module.
 */

/**
 * Implements a style type plugin for the Views module.
 */
class SlickViews extends views_plugin_style {

  /**
   * {@inheritdoc}
   */
  public function init(&$view, &$display, $options = NULL) {
    parent::init($view, $display, $options);
    // Even empty active to call render; where library is attached if required.
    $this->definition['even empty'] = TRUE;
  }

  /**
   * Provides default options.
   */
  public function option_definition() {
    module_load_include('inc', 'slick', 'includes/slick.global');
    $options = array(
      'slide_thumbnail'     => array('default' => ''),
      'slide_field_wrapper' => array('default' => FALSE),
      'id'                  => array('default' => ''),
    );

    foreach (slick_get_global_default_settings() as $key => $value) {
      $options[$key] = array('default' => $value);
    }

    drupal_alter('slick_views_options_info', $options);
    return $options + parent::option_definition();
  }

  /**
   * Shows a form to edit the style options.
   */
  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    ctools_form_include($form_state, 'slick.admin', 'slick');
    ctools_form_include($form_state, 'admin', 'slick_views');

    _slick_views_options_form($form, $form_state, $this->view, $this->options);
  }

  /**
   * Performs some cleanup tasks on the options array before saving it.
   */
  public function options_submit(&$form, &$form_state) {
    $options = &$form_state['values']['style_options'];

    // The form is a #tree, but the expected output is a flattened array.
    if (!empty($options['slick'])) {
      // Pull the fieldset values one level up.
      $options += $options['slick'];
      unset($options['slick']);
    }
  }

  /**
   * Renders the slick instances.
   */
  public function renderSlick($slick, $settings) {
    return array(
      '#theme'   => $this->theme_functions(),
      '#view'    => $this->view,
      '#options' => $settings,
      '#rows'    => $slick,
    );
  }

  /**
   * Renders the display in this style.
   */
  public function render() {
    global $language;
    $langcode  = isset($language->language) ? $language->language : LANGUAGE_NONE;
    $view      = $this->view;
    $settings  = $this->options;
    $view_name = $view->name;
    $current   = $view->current_display;
    $id        = slick_html_id("slick-views-{$view_name}", $settings['id']);
    $thumb_id  = $id . '-thumbnail';
    $asnavfor  = $settings['optionset_thumbnail'];
    $attach    = $build = array();

    // Renders slicks quickly from cache if any, as render cache is just cache.
    $settings['count'] = count($view->result);
    $settings['nav']   = $asnavfor && $settings['count'] > 1;
    $settings['cid']   = $settings['optionset'] . $settings['skin'] . $view_name . $current . $langcode;

    if (!empty($settings['cache']) && $cache = slick_render_cache($settings)) {
      return $this->renderSlick($cache, $settings);
    }

    // Otherwise do the routines before a cache stored, or when disabled.
    module_load_include('inc', 'slick', 'includes/slick.global');
    $optionset           = slick_optionset_load($settings['optionset']);
    $optionset_thumbnail = !$settings['nav'] ? NULL : slick_optionset_load($asnavfor);
    $attachments         = slick_attach($attach, $settings);
    $lazy                = $optionset->options['settings']['lazyLoad'];

    $settings['blazy']             = $lazy == 'blazy' || !empty($settings['breakpoints']);
    $settings['lazy']              = $settings['blazy'] ? 'blazy' : $lazy;
    $settings['has_pattern']       = !empty($optionset->options['general']['goodies']['pattern']);
    $settings['view_name']         = $view_name;
    $settings['current_view_mode'] = $current;
    $settings['current_display']   = 'main';
    $settings['ratio']             = !empty($settings['aspect_ratio']) ? $settings['aspect_ratio'] : FALSE;

    foreach ($this->render_grouping($view->result, $settings['grouping']) as $rows) {
      $js = array();
      if ($settings['nav']) {
        $js['asNavFor'] = "#{$thumb_id}-slider";
      }

      // Build the single/main display Slick.
      $items = $this->buildElements($rows, $settings);
      $slick[0] = slick_build($items, $js, $settings, $attachments, $id, $optionset);

      if ($settings['nav']) {
        $settings['optionset']       = $asnavfor;
        $settings['current_display'] = 'thumbnail';
        $js['asNavFor']              = "#{$id}-slider";

        // Build the thumbnail+/text navigation Slick.
        $thumbs = $this->buildElements($rows, $settings, TRUE);
        $slick[1] = slick_build($thumbs, $js, $settings, array(), $id, $optionset_thumbnail);
        unset($thumbs, $optionset_thumbnail);
      }

      // Attach library if there is no results and ajax is active,
      // otherwise library will not be attached on ajax callback.
      if (empty($view->result) && $view->use_ajax) {
        $slick[0]['#attached'] = $attachments;
      }

      // Build the Slick.
      $build = $this->renderSlick($slick, $settings);
      unset($slick, $items, $optionset, $settings, $view);
    }
    return $build;
  }

  /**
   * Returns slick contents.
   */
  public function buildElements($rows, $settings = array(), $nav = FALSE) {
    $build = $dimensions = array();
    $view  = $this->view;
    $keys  = array_keys($view->field);
    $stage = $settings['slide_image'];
    $class = $settings['slide_classes'];

    // Defines image dimensions once if applicable to reduce function calls.
    // Think of tons of images with Slick grid.
    if (!empty($stage)) {
      $image = $this->getFieldData($rows[0], $stage);

      // Not-empty behavior and filter fail, so add own check here.
      if (isset($image['rendered']) && $this->get_field(0, $stage)) {
        $rendered = $image['rendered'];
        $supported = isset($rendered['#theme']) && $rendered['#theme'] == 'image_formatter';
        if (isset($image['raw']) && $supported) {
          $settings['image_style'] = isset($rendered['#image_style']) ? $rendered['#image_style'] : '';
          if (!empty($settings['image_style'])) {
            $dimensions = slick_get_dimensions($image['raw'], $settings['image_style']);
          }
        }
      }
    }

    foreach ($rows as $index => $row) {
      $view->row_index = $index;

      $thumb = array();
      $slide = array(
        'caption' => array(),
        'slide'   => array(),
      );

      $settings['delta'] = $index;

      // Uses all Views markups, ignoring Slick markups.
      if (empty($settings['slide_field_wrapper'])) {
        $slide['slide'] = $view->style_plugin->row_plugin->render($row);
      }
      else {
        // Add layout field, may be a list field, or builtin layout options.
        if ($layout = $settings['slide_layout']) {
          if (strpos($layout, 'field_') !== FALSE) {
            $settings['slide_layout'] = check_plain($this->get_field($index, $layout));
          }
          $settings['layout'] = $settings['slide_layout'];
        }

        // Add main image field if so configured.
        if (!empty($stage)) {

          // Not-empty behavior and filter fail, so add own check here.
          if ($rendered_image = $this->get_field($index, $stage)) {

            // See if we have renderable array to work with.
            $image = $this->getFieldData($row, $stage);

            if (isset($image['rendered'])) {
              $media = $dimensions;
              $rendered = $image['rendered'];
              $settings['type'] = 'image';

              // Only lazyLoad known formatter: image_formatter.
              $supported = isset($rendered['#theme']) && $rendered['#theme'] == 'image_formatter';
              if (!empty($settings['lazy']) && isset($image['raw']) && $supported) {
                $settings['image_style'] = isset($rendered['#image_style']) ? $rendered['#image_style'] : '';

                if (isset($rendered['#path']['path'])) {
                  $settings['media_switch'] = 'content';
                  $settings['url'] = $settings['entity_uri'] = $rendered['#path']['path'];
                }

                $slide['slide'] = slick_get_image($settings, $media, $image['raw']);
              }
              else {
                $slide['slide'] = $rendered;
              }
            }
            else {
              // Otherwise fallback to rendered image.
              $slide['slide'] = $rendered_image;
            }
          }
        }

        // Add all caption fields if so configured.
        if ($captions = $settings['slide_caption']) {
          $captions = is_array($captions) ? array_filter($captions) : (array) $captions;

          $caption_items = array();
          foreach ($captions as $key => $caption) {
            $caption_rendered = $this->get_field($index, $caption);
            if (empty($caption_rendered)) {
              continue;
            }

            if (in_array($caption, array_values($keys))) {
              $caption_items[$key]['#markup'] = $caption_rendered;
            }
          }
          if ($caption_items) {
            $slide['caption']['data'] = $caption_items;
            unset($caption_items);
          }
        }

        // Add caption fields if so configured.
        $slide['caption']['title']   = empty($settings['slide_title']) ? array() : $this->getFieldRendered($index, $settings['slide_title'], TRUE);
        $slide['caption']['link']    = empty($settings['slide_link']) ? array() : $this->getFieldRendered($index, $settings['slide_link']);
        $slide['caption']['overlay'] = empty($settings['slide_overlay']) ? array() : $this->getFieldRendered($index, $settings['slide_overlay']);

        // Add thumbnail navigation if so configured.
        if ($slide_thumbnail = $settings['slide_thumbnail']) {
          $thumbnail = $this->getFieldData($row, $slide_thumbnail);
          if (isset($thumbnail['rendered']) && $thumbnail_rendered = $this->get_field($index, $slide_thumbnail)) {
            $thumb['slide'] = $thumbnail['rendered'];
          }
        }

        // Allows text-only thumbnail navigation, like regular tabs.
        // Use Views UI "Rewrite results" to sanitize the caption.
        if ($thumbnail_caption = $settings['thumbnail_caption']) {
          $thumb['caption']['data'] = $this->getFieldRendered($index, $thumbnail_caption);
          if (!isset($thumb['slide'])) {
            $thumb['slide'] = array();
            $settings['type'] = 'text';
          }
        }
      }

      if (!empty($class)) {
        if ($this->get_field($index, $class) && $classes = $this->getFieldValue($row, $class, $index)) {
          $classes = array_filter($classes);
          $settings['slide_classes'] = empty($classes[$index]) ? '' : $classes[$index];
        }
        else {
          $settings['slide_classes'] = '';
        }
      }

      // Build thumbnail slide items, otherwise main.
      $slide['settings'] = $settings;
      $build[] = $nav ? $thumb : $slide;
      unset($thumb, $slide);
    }
    unset($view->row_index);
    return $build;
  }

  /**
   * Returns the rendered field, either string or array.
   */
  public function getFieldRendered($index, $field_name = '', $restricted = FALSE) {
    if (!empty($field_name) && $output = $this->get_field($index, $field_name)) {
      return is_array($output) ? $output : array('#markup' => ($restricted ? filter_xss_admin($output) : $output));
    }
    return array();
  }

  /**
   * Gets renderable array of field containing rendered and raw data.
   */
  public function getFieldData($row, $field_name, $multiple = FALSE) {
    $field = $this->view->field[$field_name]->handler_type . '_' . $field_name;
    return $multiple && isset($row->{$field}) ? $row->{$field} : (isset($row->{$field}[0]) ? $row->{$field}[0] : '');
  }

  /**
   * Returns the values for the expected Title, ER, List, Term.
   */
  public function getFieldValue($row, $field_name, $index) {
    $values = array();

    // Content title/List/Text, either as link or plain text.
    // @todo recheck multi-values.
    if ($value = $this->get_field_value($index, $field_name)) {
      if (is_array($value)) {
        foreach (array_filter($value) as $key => $val) {
          $v = isset($val['value']) ? $val['value'] : $val;
          $value[$key] = drupal_clean_css_identifier(drupal_strtolower($v));
        }
      }

      $string = is_string($value) ? $value : implode(' ', $value);
      $values[$index] = empty($string) ? '' : $string;
    }

    // Term reference/ER, either as link or plain text.
    elseif ($renderable = $this->getFieldData($row, $field_name, TRUE)) {
      $value = array();
      foreach ($renderable as $key => $render) {
        $class = '';
        if (isset($render['rendered']['#title'])) {
          $class = $render['rendered']['#title'];
        }
        elseif (isset($render['raw']['value'])) {
          $class = $render['raw']['value'];
        }
        $class = $class ? $class : drupal_render($render['rendered']);
        $class = strip_tags($class);
        $value[$key] = drupal_clean_css_identifier(drupal_strtolower($class));
      }
      $values[$index] = empty($value) ? '' : implode(' ', $value);
    }

    return $values;
  }

}

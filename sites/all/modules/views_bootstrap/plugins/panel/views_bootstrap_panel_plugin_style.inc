<?php

/**
 * @file
 * Definition of views_bootstrap_plugin_style.
 */

/**
 * Class to define a style plugin handler.
 */
class ViewsBootstrapPanelPluginStyle extends views_plugin_style {

  /**
   * Definition.
   */
  public function option_definition() {
    $options = parent::option_definition();
    $options['heading_field'] = array('default' => NULL);
    $options['title'] = array('default' => FALSE);
    $options['title_tag'] = array('default' => 'h3');
    $options['body_field'] = array('default' => NULL);
    $options['footer_field'] = array('default' => NULL);
    $options['context'] = array('default' => 'default');
    return $options;
  }

  /**
   * Options form.
   */
  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    if (isset($form['grouping'])) {
      $options = array();
      foreach (element_children($form['grouping']) as $key => $value) {
        if (!empty($form['grouping'][$key]['field']['#options']) && is_array($form['grouping'][$key]['field']['#options'])) {
          $options = array_merge($options, $form['grouping'][$key]['field']['#options']);
        }
      }

      $form['heading_field'] = array(
        '#type' => 'select',
        '#title' => t('Heading field'),
        '#options' => $options,
        '#default_value' => $this->options['heading_field'],
        '#description' => t('Select the field that will be used as the heading.'),
        '#id' => 'heading-field',
      );

      $field_labels = $this->display->handler->get_field_labels(TRUE);
      $form['title'] = array(
        '#type' => 'checkbox',
        '#title' => t('Use title.'),
        '#default_value' => $this->options['title'],
        '#dependency' => array(
          'heading-field' => array_keys($field_labels),
        ),
        '#id' => 'title-checkbox',
      );
      $t_options = array(
        'h1' => t('h1'),
        'h2' => t('h2'),
        'h3' => t('h3'),
        'h4' => t('h4'),
        'h5' => t('h5'),
        'h6' => t('h6'),
        'div' => t('div'),
      );
      $form['title_tag'] = array(
        '#type' => 'select',
        '#title' => t('Title tag'),
        '#options' => $t_options,
        '#default_value' => $this->options['title_tag'],
        '#description' => t('Select the tag to use around the title.'),
        '#dependency' => array(
          'heading-field' => array_keys($field_labels),
        ),
      );

      $form['body_field'] = array(
        '#type' => 'select',
        '#title' => t('Body field'),
        '#options' => $options,
        '#required' => TRUE,
        '#default_value' => $this->options['body_field'],
        '#description' => t('Select the field that will be used as the body.'),
      );
      $form['footer_field'] = array(
        '#type' => 'select',
        '#title' => t('Footer field'),
        '#options' => $options,
        '#default_value' => $this->options['footer_field'],
        '#description' => t('Select the field that will be used as the footer.'),
      );

      $c_options = array(
        'default' => 'default',
        'primary' => 'primary',
        'success' => 'success',
        'info' => 'info',
        'warning' => 'warning',
        'danger' => 'danger',
      );
      $form['context'] = array(
        '#type' => 'select',
        '#title' => t('Context'),
        '#options' => $c_options,
        '#default_value' => $this->options['context'],
        '#description' => t('Contextual state.'),
      );
    }
  }

}

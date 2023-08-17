<?php

/**
 * @file
 * Default theme implementation for the Slick carousel template.
 *
 * Available variables:
 * - $items: The array of items containing main image/video/audio, and optional
 *     image/video/audio overlay and captions.
 * - $settings: A cherry-picked settings that mostly defines the slide HTML or
 *     layout, and none of JS settings/options which are defined at data-slick.
 * - $attributes: The array of attributes to hold the container classes, and id.
 * - $content_attributes: The array of attributes to hold the slick-slider and
 *     data-slick containing JSON object aka JS settings the Slick expects to
 *     override default options. We don't store these JS settings in the normal
 *     <head>, but inline within data-slick attribute instead.
 */
?>
<div<?php print $attributes; ?>>
  <?php if (empty($settings['unslick'])): ?>
    <div<?php print $content_attributes; ?>>
  <?php endif; ?>

    <?php foreach ($items as $item): ?>
      <?php print render($item); ?>
    <?php endforeach; ?>

  <?php if (empty($settings['unslick'])): ?>
    </div>
    <nav class="slick__arrow<?php print $settings['skin_arrows']; ?>">
      <button type="button" data-role="none" class="slick-prev" aria-label="<?php print $settings['prev_arrow']; ?>" tabindex="0"><?php print $settings['prev_arrow']; ?></button>
      <?php isset($arrow_down) && print $arrow_down; ?>
      <button type="button" data-role="none" class="slick-next" aria-label="<?php print $settings['next_arrow']; ?>" tabindex="0"><?php print $settings['next_arrow']; ?></button>
    </nav>
  <?php endif; ?>
</div>

/**
 * @file
 * Provides Slick loader.
 */

(function ($, Drupal, drupalSettings) {

  'use strict';

  /**
   * Slick utility functions.
   *
   * @param {int} i
   *   The index of the current element.
   * @param {HTMLElement} elm
   *   The slick HTML element.
   */
  function doSlick(i, elm) {
    var t = $('> .slick__slider', elm).length ? $('> .slick__slider', elm) : $(elm);
    var a = $('> .slick__arrow', elm);
    var o = t.data('slick') ? $.extend({}, drupalSettings.slick, t.data('slick')) : drupalSettings.slick;
    var r = $.type(o.responsive) === 'array' && o.responsive.length ? o.responsive : false;
    var d = o.appendDots;
    var b;
    var isBlazy = o.lazyLoad === 'blazy' && Drupal.blazy;
    var unSlick = t.hasClass('unslick');

    // Populate defaults + globals into each breakpoint.
    if (!unSlick) {
      o.appendDots = d === '.slick__arrow' ? a : (d || $(t));
    }

    if (r) {
      for (b in r) {
        if (r.hasOwnProperty(b) && r[b].settings !== 'unslick') {
          r[b].settings = $.extend({}, drupalSettings.slick, globals(o), r[b].settings);
        }
      }
    }

    // Update the slick settings object.
    t.data('slick', o);
    o = t.data('slick');

    /**
     * The event must be bound prior to slick being called.
     */
    function beforeSlick() {
      if (o.randomize && !t.hasClass('slick-initiliazed')) {
        randomize();
      }

      // Puts dots in between arrows for easy theming like this: < ooooo >.
      if (!unSlick) {
        t.on('init.sl', function (e, slick) {
          // Puts dots in between arrows for easy theming like this: < ooooo >.
          if (d === '.slick__arrow') {
            $(slick.$dots).insertAfter(slick.$prevArrow);
          }

          // Fixes for slidesToShow > 1, centerMode, clones with Blazy IO.
          var $src = t.find('.slick-cloned.slick-active .b-lazy:not(.b-loaded)');
          if (isBlazy && $src.length) {
            Drupal.blazy.init.load($src);
          }
        });
      }

      // Lazyload ahead with Blazy integration.
      if (isBlazy) {
        t.on('beforeChange.sl', function () {
          preloadBlazy(true);
        });
      }
      else {
        // Useful to hide caption during loading, but watch out setBackground().
        $('.media--loading', t).closest('.slide__content').addClass('is-loading');
      }

      t.on('setPosition.sl', function (e, slick) {
        setPosition(slick);
      });
    }

    /**
     * Blazy is not loaded on slidesToShow > 1, reload.
     */
    function preloadBlazy(ahead) {
      if (t.find('.b-lazy:not(.b-loaded)').length) {
        var $src = t.find(ahead ? '.slide:not(.slick-cloned) .b-lazy:not(.b-loaded)' : '.slick-active .b-lazy:not(.b-loaded)');
        if ($src.length) {
          Drupal.blazy.init.load($src);
        }
      }
    }

    /**
     * Reacts on Slick afterChange event.
     */
    function afterChange() {
      if (isBlazy) {
        preloadBlazy(false);
      }
    }

    /**
     * The event must be bound after slick being called.
     */
    function afterSlick() {
      // Arrow down jumper.
      t.parent().on('click.sl', '.slick-down', function (e) {
        e.preventDefault();
        var b = $(this);
        $('html, body').stop().animate({
          scrollTop: $(b.data('target')).offset().top - (b.data('offset') || 0)
        }, 800, 'easeOutQuad' in $.easing && o.easing ? o.easing : 'swing');
      });

      if (o.mouseWheel) {
        t.on('mousewheel.sl', function (e, delta) {
          e.preventDefault();
          return t.slick(delta < 0 ? 'slickNext' : 'slickPrev');
        });
      }

      if (!isBlazy) {
        t.on('lazyLoaded lazyLoadError', function (e, slick, img) {
          setBackground(img);
        });
      }

      t.on('afterChange.sl', afterChange);
    }

    /**
     * Turns images into CSS background if so configured.
     *
     * @param {HTMLElement} img
     *   The image HTML element.
     */
    function setBackground(img) {
      var $img = $(img);
      var $bg = $img.closest('.media--background');
      var p = $img.closest('.slide') || $img.closest('.unslick');

      $img.parentsUntil(p).removeClass(function (index, css) {
        return (css.match(/(\S+)loading/g) || []).join(' ');
      });

      if ($bg.length && $bg.find('> img').length) {
        $bg.css('background-image', 'url(' + $img.attr('src') + ')');
        $bg.find('> img').remove();
        $bg.removeAttr('data-lazy');
      }
    }

    /**
     * Randomize slide orders, for ads/products rotation within cached blocks.
     */
    function randomize() {
      t.children().sort(function () {
        return 0.5 - Math.random();
      })
      .each(function () {
        t.append(this);
      });
    }

    /**
     * Updates arrows visibility based on available options.
     *
     * @param {Object} slick
     *   The slick instance object.
     *
     * @return {String}
     *   The visibility of slick arrows controlled by CSS class visually-hidden.
     */
    function setPosition(slick) {
      // Use the options that applies for the current breakpoint and not the
      // variable "o".
      // @see https://www.drupal.org/project/slick/issues/2480245
      var less = slick.slideCount <= slick.options.slidesToShow;
      var hide = less || slick.options.arrows === false;

      // Be sure the most complex slicks are taken care of as well, e.g.:
      // asNavFor with the main display containing nested slicks.
      if (t.attr('id') === slick.$slider.attr('id')) {
        // Removes padding rules, if no value is provided to allow non-inline.
        if (!slick.options.centerPadding || slick.options.centerPadding === '0') {
          slick.$list.css('padding', '');
        }

        // @todo: Remove temp fix for when total <= slidesToShow at 1.6.1+.
        // Ensures the fix doesn't break responsive options.
        // @see https://github.com/kenwheeler/slick/issues/262
        if (less && slick.$slideTrack.width() <= slick.$slider.width()) {
          slick.$slideTrack.css({left: '', transform: ''});
        }

        // Do not remove arrows, to allow responsive have different options.
        a[hide ? 'addClass' : 'removeClass']('element-invisible');
      }
    }

    /**
     * Declare global options explicitly to copy into responsive settings.
     *
     * @param {Object} o
     *   The slick options object.
     *
     * @return {Object}
     *   The global options common for both main and responsive displays.
     */
    function globals(o) {
      return unSlick ? {} : {
        slide: o.slide,
        lazyLoad: o.lazyLoad,
        dotsClass: o.dotsClass,
        rtl: o.rtl,
        prevArrow: $('.slick-prev', a),
        nextArrow: $('.slick-next', a),
        appendArrows: a,
        customPaging: function (slick, i) {
          var container = slick.$slides.eq(i).find('[data-thumb]') || null;
          var img = '<img alt="' + Drupal.t(container.find('img').attr('alt')) + '" src="' + container.data('thumb') + '">';
          var dotsThumb = container.length && o.dotsClass.indexOf('thumbnail') > 0 ?
            '<div class="slick-dots__thumbnail">' + img + '</div>' : '';
          var paging = slick.defaults.customPaging(slick, i);
          return dotsThumb ? paging.add(dotsThumb) : paging;
        }
      };
    }

    // Build the Slick.
    beforeSlick();
    t.slick(globals(o));
    afterSlick();

    // Destroy Slick if it is an enforced unslick.
    // This allows Slick lazyload to run, but prevents further complication.
    // Should use lazyLoaded event, but images are not always there.
    if (t.hasClass('unslick')) {
      t.slick('unslick');
    }

    // Add helper class for arrow visibility/ RTL as they are outside slider.
    $(elm).addClass('slick--initialized');
  }

  /**
   * Attaches slick behavior to HTML element identified by CSS selector .slick.
   *
   * @type {Drupal~behavior}
   */
  Drupal.behaviors.slick = {
    attach: function (context) {
      $('.slick', context).once('slick', doSlick);
    }
  };

})(jQuery, Drupal, Drupal.settings);

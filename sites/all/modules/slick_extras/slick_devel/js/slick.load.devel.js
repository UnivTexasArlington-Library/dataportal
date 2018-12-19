/**
 * @file
 * Provides Slick loader.
 */

/* global jQuery:false, Drupal:false */
/* jshint -W072 */
/* eslint max-params: 0, consistent-this: [0, "_"] */
(function ($, Drupal) {

  "use strict";

  Drupal.behaviors.slick = {
    attach: function (context, settings) {
      var _ = this;

      $(".slick:not(.unslick)", context).once("slick", function () {
        var b,
          t = $("> .slick__slider", this),
          a = $("> .slick__arrow", this),
          o = $.extend({}, settings.slick, t.data("slick"));

        // Populate defaults + globals into each breakpoint.
        if ($.type(o.responsive) === "array" && o.responsive.length) {
          for (b in o.responsive) {
            if (o.responsive.hasOwnProperty(b)
              && o.responsive[b].settings !== "unslick") {
                o.responsive[b].settings = $.extend(
                  {},
                  settings.slick,
                  _.globals(t, a, o),
                  o.responsive[b].settings);
            }
          }
        }

        // Update the slick settings object.
        t.data("slick", o);
        o = t.data("slick");

        // Build the Slick.
        _.beforeSlick(t, a, o);
        t.slick(_.globals(t, a, o));
        _.afterSlick(t, o);
      });

      $(".unslick .media--background", context).once("media-background", function () {
        _.setBackground($(this).find("img"));
      });
    },

    /**
     * The event must be bound prior to slick being called.
     */
    beforeSlick: function (t, a, o) {
      var _ = this,
        r = $(".slide--0 .media--ratio", t);

      _.randomize(t, o);

      // Fixed for broken slick with Blazy, aspect ratio, hidden containers.
      if (r.length && r.is(":hidden")) {
        r.removeClass("media--ratio").addClass("js-media--ratio");
      }

      t.on("setPosition.slick", function (e, slick) {
        _.setPosition(t, a, o, slick);
      });

      $(".media--loading", t).closest(".slide").addClass("slide--loading");
    },

    /**
     * The event must be bound after slick being called.
     */
    afterSlick: function (t, o) {
      var _ = this,
        slick = t.slick("getSlick"),
        $ratio = $(".js-media--ratio", t);

      // Arrow down jumper.
      t.parent().on("click.slick.load", ".slick-down", function (e) {
        e.preventDefault();
        var b = $(this);
        $("html, body").stop().animate({
          scrollTop: $(b.data("target")).offset().top - (b.data("offset") || 0)
        }, 800, o.easing);
      });

      if (o.mousewheel) {
        t.on("mousewheel.slick.load", function (e, delta) {
          e.preventDefault();
          return (delta < 0) ? t.slick("slickNext") : t.slick("slickPrev");
        });
      }

      // Fixed for broken slick with Blazy, aspect ratio, hidden containers.
      if ($ratio.length) {
        // t[0].slick.refresh();
        t.trigger("resize");
        $ratio.addClass("media--ratio").removeClass("js-media--ratio");
      }

      t.on("lazyLoaded lazyLoadError", function (e, slick, img) {
        _.setBackground(img);
      });

      t.trigger("afterSlick", [_, slick, slick.currentSlide]);
    },

    /**
     * Turns images into CSS background if so configured.
     */
    setBackground: function (img) {
      var $img = $(img),
        $bg = $img.closest(".media--background");

      $img.closest(".media").removeClass("media--loading").addClass("media--loaded");
      $img.closest(".slide--loading").removeClass("slide--loading");

      if ($bg.length) {
        $bg.css("background-image", "url(" + $img.attr("src") + ")");
        $bg.find("> img").remove();
        $bg.removeAttr("data-lazy");
      }
    },

    /**
     * Randomize slide orders, for ads/products rotation within cached blocks.
     */
    randomize: function (t, o) {
      if (o.randomize && !t.hasClass("slick-initiliazed")) {
        t.children().sort(function () {
          return 0.5 - Math.random();
        })
        .each(function () {
          t.append(this);
        });
      }
    },

    /**
     * Updates arrows visibility based on available options.
     */
    setPosition: function (t, a, o, slick) {
      // Be sure the most complex slicks are taken care of as well, e.g.:
      // asNavFor with the main display containing nested slicks.
      if (t.attr("id") === slick.$slider.attr("id")) {
        // Removes padding rules, if no value is provided to allow non-inline.
        if (!o.centerPadding || o.centerPadding === "0") {
          slick.$list.css("padding", "");
        }

        // Do not remove arrows, to allow responsive have different options.
        return slick.slideCount <= o.slidesToShow || o.arrows === false
          ? a.addClass("element-hidden") : a.removeClass("element-hidden");
      }
    },

    /**
     * Declare global options explicitly to copy into responsive settings.
     */
    globals: function (t, a, o) {
      return {
        slide: o.slide,
        lazyLoad: o.lazyLoad,
        dotsClass: o.dotsClass,
        rtl: o.rtl,
        appendDots: o.appendDots === ".slick__arrow"
          ? a : (o.appendDots || $(t)),
        prevArrow: $(".slick-prev", a),
        nextArrow: $(".slick-next", a),
        appendArrows: a,
        customPaging: function (slick, i) {
          var tn = slick.$slides.eq(i).find("[data-thumb]") || null,
            alt = Drupal.t(tn.attr("alt")) || "",
            img = "<img alt='" + alt + "' src='" + tn.data("thumb") + "'>",
            dotsThumb = tn.length && o.dotsClass.indexOf("thumbnail") > 0 ?
              "<div class='slick-dots__thumbnail'>" + img + "</div>" : "";
          return slick.defaults.customPaging(slick, i).add(dotsThumb);
        }
      };
    }
  };

})(jQuery, Drupal);

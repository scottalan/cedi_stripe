/*
 @file
 Handles communication with Stripe.com
 */

var Stripe = Stripe || {};

(function($) {

  Drupal.behaviors.cedi_stripe = {
    attach: function (context, settings) {
      // If we already have a token then they have made a payment.
      if (settings.cedi_stripe.payment) {
        // Hide the payment button for good measure.
        $('form#stripe-button button', context).hide();
      }
    }
  };

})(jQuery);

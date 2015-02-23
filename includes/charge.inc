<?php

/**
 * @param string $id
 *   The id representing what to charge. For us this is a customer but could
 *   also be a card.
 * @param int $amount
 *   The amount to charge in cents.
 *
 * @return Exception|Stripe_CardError|Stripe_Charge
 *   on success: a charge object
 *   on error: an error object
 */
function cedi_stripe_create_charge($id, $amount) {
  // Set your secret key: remember to change this to your live secret key in production
  Stripe::setApiKey(TEST_SECRET_KEY);

  $e = FALSE;

  // Build a unique key. @see https://stripe.com/docs/api#idempotency
  $key = cedi_stripe_verification_hmac(0, REQUEST_TIME);

  // Create the charge on Stripe's servers - this will charge the user's card
  try {
    $charge = Stripe_Charge::create(
      array(
        "amount" => $amount, # amount in cents, again
        "currency" => "usd",
        "customer" => $id,
      ), array(
        'idempotency_key' => $key,
      )
    );
  } catch(Stripe_CardError $e) {
    // The card has been declined
  } catch (Stripe_InvalidRequestError $e) {
    // Invalid parameters were supplied to Stripe's API
  } catch (Stripe_AuthenticationError $e) {
    // Authentication with Stripe's API failed
    // (maybe you changed API keys recently)
  } catch (Stripe_ApiConnectionError $e) {
    // Network communication with Stripe failed
  } catch (Stripe_Error $e) {
    // Display a very generic error to the user, and maybe send
    // yourself an email
  } catch (Exception $e) {
    // Something else happened, completely unrelated to Stripe
  }

  $result = isset($charge) ? $charge : $e;

  return $result;
}

function cedi_stripe_idempotent_charge() {

}

/**
 * Logs an error to the database so we have a record of the failed charge attempt.
 *
 * @param string $type
 *   A label for the watchdog error.
 * @param stdClass $charge
 *   The complete charge object. In this case an error.
 * @param string $mail
 *   The email address of the person that tried to create an account.
 */
function cedi_stripe_charge_error($type, $charge, $mail) {
  $body = $charge->jsonBody['error'];
  $err_message = $body['message'];
  $err_type = $body['type'];
  $err_code = $body['code'];
  $err_card = $body['charge'];

  watchdog($type, "For $mail: $charge->httpStatus. $err_message, $err_type, $err_code");
}

/**
 * Helper function to generate HMAC.
 *
 * @param int $uid
 * @param int $timestamp
 *
 * @return string
 *  A base-64 encoded, URL-safe sha-256 hmac
 */
function cedi_stripe_verification_hmac($uid, $timestamp) {
  $string = drupal_get_hash_salt() . $uid . variable_get('reg_wizard_verification_salt', 'salt');
  return drupal_hmac_base64($timestamp . $uid, $string);
}

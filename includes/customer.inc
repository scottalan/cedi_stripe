<?php

/**
 * Creates a customer. This does not charge the card yet.
 *
 * @param string $token
 *   The token representing the customer, used for recurring charges. We pass
 *   this in as the 'card' and Stripe knows we want to create a customer and
 *   not just charge the card.
 * @param string $mail
 *   The customer's email address.
 * @param $description
 *   The description that is added to the customer. This is for us.
 *
 * @return bool|Stripe_Customer
 *   A valid customer.
 */
function cedi_stripe_customer_create($token, $mail, $description) {
  // Load the Stripe library.
  if (!cedi_stripe_load_library()) {
    return FALSE;
  }

  $e = FALSE;

  // Set your secret key: remember to change this to your live secret key in production
  Stripe::setApiKey(TEST_SECRET_KEY);

  try{
    // Create a Customer
    $customer = Stripe_Customer::create(array(
        "card" => $token,
        "description" => $description . '|' . $mail)
    );
  } catch (Stripe_Error $e) {
    // Display a very generic error to the user, and maybe send
    // yourself an email
  } catch (Exception $e) {
    // Something else happened, completely unrelated to Stripe
  }

  $result = isset($customer) ? $customer : $e;

  return $result;

}

/**
 * Retrieves information about the customer.
 *
 * @param string $customer_id
 *   The unique customer id.
 *
 * @return bool|null
 */
function cedi_stripe_customer_retrieve($customer_id) {
  // Load the Stripe library.
  if (!cedi_stripe_load_library()) {
    return FALSE;
  }
  // Set your secret key: remember to change this to your live secret key in production
  Stripe::setApiKey(TEST_SECRET_KEY);

  // Create a Customer
  $customer = Stripe_Customer::retrieve($customer_id);

  return $customer;
}

function cedi_stripe_customer_charge($customer, $amount) {
  // Check for errors so we don't try to charge.
  if (!isset($_SESSION['messages']['error'])) {
    // Load the Stripe library. If it doesn't load it's logged to the database
    // and we set a message here.
    if (!cedi_stripe_load_library()) {
      drupal_set_message(t('There was an error charging the card. Please contact us.'));
      return FALSE;
    }
    return cedi_stripe_create_charge($customer->id, $amount);
  }
  else {
    return FALSE;
  }
}

function cedi_stripe_customer_delete($customer) {
  $cu = Stripe_Customer::retrieve($customer->id);
  $cu->delete();
}

/**
 * Logs an error to the database so we have a record of the failed charge attempt.
 *
 * @param string $type
 *   A label for the watchdog error.
 * @param stdClass $error
 *   The error stack.
 * @param string $mail
 *   The email address of the person that tried to create an account.
 */
function cedi_stripe_customer_error($type, $error, $mail) {

  $err_type = $error['type'];
  $err_message = $error['message'];

  watchdog($type, "For $mail: $err_type, $err_message");
}

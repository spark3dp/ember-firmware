// jQuery plugin to prevent double submission of forms
jQuery.fn.preventDoubleSubmission = function(submittedValue) {
  $(this).on('submit',function(e){
    var $form = $(this);

    if ($form.data('submitted') === true) {
      // Previously submitted - don't submit again
      e.preventDefault();
    } else {
      // Mark it so that the next submit can be ignored
      $form.data('submitted', true);
      // Disable button
      $button = $form.find('button[type=submit]');
      $button.attr('disabled', true);
      // Update button text if provided with argument
      if (arguments.length > 0) {
        $button.attr('value', submittedValue);
        $button.html(submittedValue);
      }
    }
  });

  // Keep chainability
  return this;
};

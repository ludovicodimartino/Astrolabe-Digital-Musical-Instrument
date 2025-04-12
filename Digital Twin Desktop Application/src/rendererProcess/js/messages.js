/**
 * The DOM element containing all the alerts.
 */
const alertPlaceholder = document.getElementById('liveAlertPlaceholder');

/**
 * The DOM element containing the button to toggle the alerts on and off.
 */
const toggleAlertsCheckbox = document.getElementById('toggleAlertCheckbox');

/**
 * The DOM element containing the div of the checkbox to show and disable the alerts visibility
 */
const toggleAlertsDiv = document.getElementById('toggleAlertsDiv');

/**
 * This messages display an alert to the user.
 * @param {String} msg The text message to show as an alert.
 * @param {String} type The bootstrap message type.
 */
export function showErrorMessage(msg, type='danger'){
  // Create the DOM element
  const wrapper = document.createElement('div')

  // Write the inner HTML to display the bootstrap alert
  wrapper.innerHTML = [
    `<div class="alert alert-${type} fade show" role="alert">`,
    `   <div>${msg}</div>`,
    '</div>'
  ].join('')

  // If type is success, dismiss all previous danger alerts
  if(type == 'success') dismissAllDangerAlerts();
  
  // Append the new alert to the DOM
  alertPlaceholder.append(wrapper);

  // Warnings and Success alerts are shown for 5 seconds
  if(type == "success" || type == "warning"){
    setTimeout(() => wrapper.remove(), 5000);
  }
}

/**
 * This function dismiss all the danger alerts.
 */
function dismissAllDangerAlerts(){
  alertPlaceholder.querySelectorAll(".alert-danger").forEach(node => {
    node.remove();
  });
}

/**
 * This function hides or shows the checkbox to toggle the alerts on and off.
 */
export function checkboxSelectorVisibility(){
  toggleAlertsDiv.style.display = toggleAlertsDiv.style.display == 'none' ? 'block' : 'none';
}

/**
 * The toggle button event listener.
 */
toggleAlertsCheckbox.addEventListener('change', () => {
  if (toggleAlertsCheckbox.checked) {
    alertPlaceholder.style.display = 'block';
    showErrorMessage("Alerts are enabled", 'warning');
  } else {
    alertPlaceholder.style.display = 'none';
  }
});
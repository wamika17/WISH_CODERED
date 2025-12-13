const MIN_REQUIRED_CASHIERS = 2;
let model;
const video = document.getElementById("webcam");

/* -------- ALERT -------- */
function showAlert(msg) {
  const box = document.getElementById("alertBox");
  document.getElementById("alertText").innerText = msg;
  box.classList.remove("hidden");
  setTimeout(() => box.classList.add("hidden"), 4000);
}

/* -------- UI UPDATE -------- */
function updateDashboard(count) {
  document.getElementById("cashiersDetected").innerText = count;

  const statusText = document.getElementById("statusText");
  const badge = document.getElementById("statusBadge");

  if (count < MIN_REQUIRED_CASHIERS) {
    statusText.innerText = "Understaffed";
    statusText.className = "text-2xl font-bold text-red-600";
    badge.innerText = "Action Required";
    badge.className =
      "inline-block mt-3 px-4 py-1 rounded-full text-sm font-semibold bg-red-100 text-red-600";
    showAlert("⚠️ Low cashier count detected");
  } else {
    statusText.innerText = "Optimal";
    statusText.className = "text-2xl font-bold text-green-600";
    badge.innerText = "Balanced";
    badge.className =
      "inline-block mt-3 px-4 py-1 rounded-full text-sm font-semibold bg-green-100 text-green-600";
  }
}

/* -------- WEBCAM -------- */
async function setupCamera() {
  const stream = await navigator.mediaDevices.getUserMedia({ video: true });
  video.srcObject = stream;
  return new Promise(resolve => {
    video.onloadedmetadata = () => resolve(video);
  });
}

/* -------- DETECTION LOOP -------- */
async function detectCashiers() {
  const predictions = await model.detect(video);

  const persons = predictions.filter(p => p.class === "person");
  updateDashboard(persons.length);

  requestAnimationFrame(detectCashiers);
}

/* -------- INIT -------- */
async function init() {
  await setupCamera();
  model = await cocoSsd.load();
  detectCashiers();
}

init();

from flask import Flask, jsonify
import threading
import time

app = Flask(__name__)

# -------- GLOBAL REAL-TIME VALUES --------
queue_length = 0
available_seats = 0
active_counters = 1

# -------- SIMULATED READERS (REPLACE WITH REAL) --------
def read_ir_and_force_sensors():
    global queue_length, available_seats
    while True:
        # Replace this with Arduino serial read
        queue_length = 7
        available_seats = 5
        time.sleep(1)

def read_webcam_count():
    global active_counters
    while True:
        # Replace with your OpenCV count
        active_counters = 2
        time.sleep(1)

# -------- API FOR DASHBOARD --------
@app.route("/data")
def get_data():
    return jsonify({
        "queueLength": queue_length,
        "availableSeats": available_seats,
        "activeCounters": active_counters
    })

if __name__ == "__main__":
    threading.Thread(target=read_ir_and_force_sensors, daemon=True).start()
    threading.Thread(target=read_webcam_count, daemon=True).start()
    app.run(port=5000)

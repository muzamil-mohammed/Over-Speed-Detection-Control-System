"""
speed_calc_simulation.py
-------------------------
A standalone simulation/verification script for the speed-calculation
logic used in the Arduino firmware. Useful for unit-testing the math
independent of the hardware, and for demonstrating the algorithm
(e.g., in a Jupyter notebook or CI pipeline).

Usage:
    python speed_calc_simulation.py
"""

SENSOR_DISTANCE_M = 0.20   # distance between sensors in meters
SPEED_LIMIT_KMPH = 5.0     # speed limit threshold


def calculate_speed_kmph(distance_m: float, time_seconds: float) -> float:
    """Calculate speed in km/h given distance (m) and elapsed time (s)."""
    if time_seconds <= 0:
        raise ValueError("time_seconds must be greater than 0")
    speed_mps = distance_m / time_seconds
    speed_kmph = speed_mps * 3.6
    return speed_kmph


def evaluate_speed(speed_kmph: float, limit_kmph: float = SPEED_LIMIT_KMPH) -> str:
    """Return the system action that would be taken for a given speed."""
    if speed_kmph > limit_kmph:
        return "OVER SPEED -> buzzer ON, LED ON, motor PWM reduced"
    return "Normal -> buzzer OFF, LED OFF, motor PWM normal"


def run_test_cases():
    test_cases_seconds = [0.30, 0.15, 0.10, 0.08, 0.05]

    print(f"{'Time (s)':>10} | {'Speed (km/h)':>13} | Action")
    print("-" * 60)
    for t in test_cases_seconds:
        speed = calculate_speed_kmph(SENSOR_DISTANCE_M, t)
        action = evaluate_speed(speed)
        print(f"{t:>10.2f} | {speed:>13.2f} | {action}")


if __name__ == "__main__":
    run_test_cases()

"""
PID controller in Python.

Drives a system from an initial state to a target (setpoint) by
continuously computing the error and adjusting the control output.
"""


class PID:
    def __init__(self, kp, ki, kd, setpoint, output_limits=(None, None)):
        self.kp, self.ki, self.kd = kp, ki, kd
        self.setpoint = setpoint
        self.min_out, self.max_out = output_limits
        self._integral = 0.0
        self._prev_error = None

    def reset(self):
        self._integral = 0.0
        self._prev_error = None

    def update(self, measurement, dt):
        """Compute control output from the current measurement."""
        error = self.setpoint - measurement

        # Proportional
        p = self.kp * error

        # Integral (with anti-windup clamping)
        self._integral += error * dt
        i = self.ki * self._integral

        # Derivative
        if self._prev_error is None:
            d = 0.0
        else:
            d = self.kd * (error - self._prev_error) / dt
        self._prev_error = error

        output = p + i + d

        # Clamp output and stop integral from winding up when saturated
        if self.max_out is not None and output > self.max_out:
            output = self.max_out
            self._integral -= error * dt
        elif self.min_out is not None and output < self.min_out:
            output = self.min_out
            self._integral -= error * dt

        return output


def simulate(initial_state=0.0, target=100.0, kp=2.0, ki=1.5, kd=0.1,
             dt=0.05, duration=10.0, tolerance=0):
    a, b = 0.5, 1.0
    pid = PID(kp, ki, kd, setpoint=target, output_limits=(-100, 100))

    x = initial_state
    t = 0.0
    times, states, outputs = [], [], []

    while t < duration:
        u = pid.update(x, dt)           # controller output
        x += (-a * x + b * u) * dt      # plant responds
        times.append(t)
        states.append(x)
        outputs.append(u)
        t += dt

    settled = next((tt for tt, xx in zip(times, states)
                    if abs(target - xx) <= tolerance), None)
    return times, states, outputs, settled


if __name__ == "__main__":
    TARGET = 100.0
    times, states, outputs, settled = simulate(initial_state=200.0, target=TARGET, kp=0.5, ki=3.5, kd=0.1)

    print(f"Start: {states[0]:.2f}  Final: {states[-1]:.2f}  Target: {TARGET}")
    start = 200.0
    if start > TARGET:   # approaching from above -> overshoot = dip below target
        os_ = max(0.0, TARGET - min(states))
    else:                # approaching from below -> overshoot = rise above target
        os_ = max(0.0, max(states) - TARGET)
    print(f"Overshoot: {os_:.2f}")
    print(f"First within tolerance at t = {settled:.2f}s" if settled is not None
          else "Did not reach target - retune gains")

    # Print the values shown on the graphs (every PRINT_EVERY-th sample)
    PRINT_EVERY = 10   # dt = 0.05, so 10 -> one row every 0.5 s. Use 1 for all rows.
    print(f"\n{'Time(s)':>8} {'State':>10} {'Output':>10} {'Error':>10}")
    print("-" * 42)
    for k in range(0, len(times), PRINT_EVERY):
        print(f"{times[k]:8.2f} {states[k]:10.3f} {outputs[k]:10.3f} {TARGET - states[k]:10.3f}")

    try:
        import matplotlib.pyplot as plt
        fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True)
        ax1.plot(times, states, label="State")
        ax1.axhline(TARGET, color="r", linestyle="--", label="Target")
        ax1.legend()
        ax1.set_ylabel("Value")
        ax2.plot(times, outputs, color="g")
        ax2.set_ylabel("Control output")
        ax2.set_xlabel("Time (s)")
        plt.tight_layout()
        plt.savefig("pid_graph.png", dpi=150)
        print("\nGraph saved as pid_graph.png")
        plt.show()
    except ImportError:
        print("\nmatplotlib not installed -> no graph. Run: pip install matplotlib")

/**
 * Madgwick filter implementation for sensor fusion.
 * This code is a JavaScript port of the original C code by Sebastian Madgwick.
 * 
 * It works with an IMU with accelerometer, gyroscope and magnetometer.
 */
class Madgwick {
  constructor(beta = 0.06) {
    this.beta = beta;
    this.q0 = 1.0;
    this.q1 = 0.0;
    this.q2 = 0.0;
    this.q3 = 0.0;
    this.lastUpdateTime = Date.now();
  }

  update(gx, gy, gz, ax, ay, az, mx, my, mz) {
    const now = Date.now();
    const dt = (now - this.lastUpdateTime) / 1000;
    // const dt = 1/20; // Assuming a fixed time step of 50ms (20Hz) for simplicity
    this.lastUpdateTime = now;
    let recipNorm;
    let s0, s1, s2, s3;
    let qDot1, qDot2, qDot3, qDot4;
    let hx, hy;
    let v2q0mx, v2q0my, v2q0mz, v2q1mx, v2bx, v2bz, v4bx, v4bz, v2q0, v2q1, v2q2, v2q3, v2q0q2, v2q2q3;
    let q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;

    // Rate of change of quaternion from gyroscope
    qDot1 = 0.5 * (-this.q1 * gx - this.q2 * gy - this.q3 * gz);
    qDot2 = 0.5 * (this.q0 * gx + this.q2 * gz - this.q3 * gy);
    qDot3 = 0.5 * (this.q0 * gy - this.q1 * gz + this.q3 * gx);
    qDot4 = 0.5 * (this.q0 * gz + this.q1 * gy - this.q2 * gx);

    // Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
    if (!(ax === 0.0 && ay === 0.0 && az === 0.0)) {
      // Normalise accelerometer measurement
      recipNorm = (ax * ax + ay * ay + az * az) ** -0.5;
      ax *= recipNorm;
      ay *= recipNorm;
      az *= recipNorm;

      // Normalise magnetometer measurement
      recipNorm = (mx * mx + my * my + mz * mz) ** -0.5;
      mx *= recipNorm;
      my *= recipNorm;
      mz *= recipNorm;

      // Auxiliary variables to avoid repeated arithmetic
      v2q0mx = 2.0 * this.q0 * mx;
      v2q0my = 2.0 * this.q0 * my;
      v2q0mz = 2.0 * this.q0 * mz;
      v2q1mx = 2.0 * this.q1 * mx;
      v2q0 = 2.0 * this.q0;
      v2q1 = 2.0 * this.q1;
      v2q2 = 2.0 * this.q2;
      v2q3 = 2.0 * this.q3;
      v2q0q2 = 2.0 * this.q0 * this.q2;
      v2q2q3 = 2.0 * this.q2 * this.q3;
      q0q0 = this.q0 * this.q0;
      q0q1 = this.q0 * this.q1;
      q0q2 = this.q0 * this.q2;
      q0q3 = this.q0 * this.q3;
      q1q1 = this.q1 * this.q1;
      q1q2 = this.q1 * this.q2;
      q1q3 = this.q1 * this.q3;
      q2q2 = this.q2 * this.q2;
      q2q3 = this.q2 * this.q3;
      q3q3 = this.q3 * this.q3;

      // Reference direction of Earth's magnetic field
      hx = mx * q0q0 - v2q0my * this.q3 + v2q0mz * this.q2 + mx * q1q1 + v2q1 * my * this.q2 + v2q1 * mz * this.q3 - mx * q2q2 - mx * q3q3;
      hy = v2q0mx * this.q3 + my * q0q0 - v2q0mz * this.q1 + v2q1mx * this.q2 - my * q1q1 + my * q2q2 + v2q2 * mz * this.q3 - my * q3q3;
      v2bx = Math.sqrt(hx * hx + hy * hy);
      v2bz = -v2q0mx * this.q2 + v2q0my * this.q1 + mz * q0q0 + v2q1mx * this.q3 - mz * q1q1 + v2q2 * my * this.q3 - mz * q2q2 + mz * q3q3;
      v4bx = 2.0 * v2bx;
      v4bz = 2.0 * v2bz;

      // Gradient decent algorithm corrective step
      s0 =
        -v2q2 * (2.0 * q1q3 - v2q0q2 - ax) +
        v2q1 * (2.0 * q0q1 + v2q2q3 - ay) -
        v2bz * this.q2 * (v2bx * (0.5 - q2q2 - q3q3) + v2bz * (q1q3 - q0q2) - mx) +
        (-v2bx * this.q3 + v2bz * this.q1) * (v2bx * (q1q2 - q0q3) + v2bz * (q0q1 + q2q3) - my) +
        v2bx * this.q2 * (v2bx * (q0q2 + q1q3) + v2bz * (0.5 - q1q1 - q2q2) - mz);
      s1 =
        v2q3 * (2.0 * q1q3 - v2q0q2 - ax) +
        v2q0 * (2.0 * q0q1 + v2q2q3 - ay) -
        4.0 * this.q1 * (1 - 2.0 * q1q1 - 2.0 * q2q2 - az) +
        v2bz * this.q3 * (v2bx * (0.5 - q2q2 - q3q3) + v2bz * (q1q3 - q0q2) - mx) +
        (v2bx * this.q2 + v2bz * this.q0) * (v2bx * (q1q2 - q0q3) + v2bz * (q0q1 + q2q3) - my) +
        (v2bx * this.q3 - v4bz * this.q1) * (v2bx * (q0q2 + q1q3) + v2bz * (0.5 - q1q1 - q2q2) - mz);
      s2 =
        -v2q0 * (2.0 * q1q3 - v2q0q2 - ax) +
        v2q3 * (2.0 * q0q1 + v2q2q3 - ay) -
        4.0 * this.q2 * (1 - 2.0 * q1q1 - 2.0 * q2q2 - az) +
        (-v4bx * this.q2 - v2bz * this.q0) * (v2bx * (0.5 - q2q2 - q3q3) + v2bz * (q1q3 - q0q2) - mx) +
        (v2bx * this.q1 + v2bz * this.q3) * (v2bx * (q1q2 - q0q3) + v2bz * (q0q1 + q2q3) - my) +
        (v2bx * this.q0 - v4bz * this.q2) * (v2bx * (q0q2 + q1q3) + v2bz * (0.5 - q1q1 - q2q2) - mz);
      s3 =
        v2q1 * (2.0 * q1q3 - v2q0q2 - ax) +
        v2q2 * (2.0 * q0q1 + v2q2q3 - ay) +
        (-v4bx * this.q3 + v2bz * this.q1) * (v2bx * (0.5 - q2q2 - q3q3) + v2bz * (q1q3 - q0q2) - mx) +
        (-v2bx * this.q0 + v2bz * this.q2) * (v2bx * (q1q2 - q0q3) + v2bz * (q0q1 + q2q3) - my) +
        v2bx * this.q1 * (v2bx * (q0q2 + q1q3) + v2bz * (0.5 - q1q1 - q2q2) - mz);
      recipNorm = (s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3) ** -0.5; // normalise step magnitude
      s0 *= recipNorm;
      s1 *= recipNorm;
      s2 *= recipNorm;
      s3 *= recipNorm;

      // Apply feedback step
      qDot1 -= this.beta * s0;
      qDot2 -= this.beta * s1;
      qDot3 -= this.beta * s2;
      qDot4 -= this.beta * s3;
    }

    // Integrate rate of change of quaternion to yield quaternion
    this.q0 += qDot1 * dt;
    this.q1 += qDot2 * dt;
    this.q2 += qDot3 * dt;
    this.q3 += qDot4 * dt;

    // Normalise quaternion
    recipNorm = (this.q0 * this.q0 + this.q1 * this.q1 + this.q2 * this.q2 + this.q3 * this.q3) ** -0.5;
    this.q0 *= recipNorm;
    this.q1 *= recipNorm;
    this.q2 *= recipNorm;
    this.q3 *= recipNorm;
  }

  getQuaternion() {
    return {
      w: this.q0,
      x: this.q1,
      y: this.q2,
      z: this.q3
    };
  }

  getAngles() {
    const q0 = this.q0;
    const q1 = this.q1;
    const q2 = this.q2;
    const q3 = this.q3;

    const ww = q0 * q0,
    xx = q1 * q1,
    yy = q2 * q2,
    zz = q3 * q3;

    return {
      roll: Math.atan2(2 * (q2 * q3 + q1 * q0), -xx - yy + zz + ww),
      pitch: Math.asin(-2 * (q1 * q3 - q2 * q0)),
      yaw: Math.atan2(2 * (q1 * q2 + q3 * q0), xx - yy - zz + ww),
    };
  }
}

const mf = new Madgwick();

module.exports = { mf };
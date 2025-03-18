// Timestamp of the last data reception
let lastReceivedDataTS = Date.now();

// Complementary filter variables initialization
let angleX = 0, angleY = 0, angleZ = 0;
const defaultAlpha = 0.98;

/**
 * Calculates the complementary filter with dynamic alpha adjustment.
 * @param {number} accelAngle - Angle from accelerometer.
 * @param {number} gyroRate - Rate from gyroscope.
 * @param {number} dt - Time delta.
 * @param {number} currentAngle - Current angle.
 * @param {number} alpha - Weighting factor for the complementary filter.
 * @returns {number} - Filtered angle.
 */
function complementaryFilter(accelAngle, gyroRate, dt, currentAngle, alpha = defaultAlpha) {
    return alpha * (currentAngle + gyroRate * dt) + (1 - alpha) * accelAngle;
}

/**
 * Normalizes a vector to ensure consistent magnitude.
 * @param {object} vector - The vector to normalize.
 * @returns {object} - Normalized vector.
 */
function normalizeVector(vector) {
    const magnitude = Math.sqrt(vector.x ** 2 + vector.y ** 2 + vector.z ** 2);
    return {
        x: vector.x / magnitude,
        y: vector.y / magnitude,
        z: vector.z / magnitude
    };
}

/**
 * Calculates tilt-compensated yaw using magnetometer data.
 * @param {object} mag - Magnetometer data.
 * @param {number} pitch - Pitch angle.
 * @param {number} roll - Roll angle.
 * @returns {number} - Tilt-compensated yaw.
 */
function calculateTiltCompensatedYaw(mag, pitch, roll) {
    const magX = mag.x * Math.cos(pitch) + mag.z * Math.sin(pitch);
    const magY = mag.x * Math.sin(roll) * Math.sin(pitch) + mag.y * Math.cos(roll) - mag.z * Math.sin(roll) * Math.cos(pitch);
    return Math.atan2(magY, magX);
}

/**
 * Calculate the rotation to be used in the frontend based on the data from the sensors.
 * The calculation is done using the complementary filter.
 * @param {object} sensorData - Sensor data containing accelerometer, gyroscope, and magnetometer readings.
 * @returns {object} - Calculated angles (pitch, roll, yaw).
 */
function calculateRotation(sensorData) {
    const { acc, gyro, mag } = sensorData;

    // Normalize accelerometer data
    const normalizedAcc = normalizeVector(acc);

    // Calculate pitch and roll from accelerometer
    const accPitch = Math.atan2(normalizedAcc.y, Math.sqrt(normalizedAcc.x ** 2 + normalizedAcc.z ** 2));
    const accRoll = Math.atan2(-normalizedAcc.x, normalizedAcc.z);

    // Calculate tilt-compensated yaw from magnetometer
    const magYaw = calculateTiltCompensatedYaw(mag, accPitch, accRoll);

    // Update time interval
    const now = Date.now();
    let dt = (now - lastReceivedDataTS) / 1000;
    lastReceivedDataTS = now;

    // Safeguard for dt
    if (dt <= 0 || dt > 1) {
        dt = 0.01; // Default to 10ms if dt is invalid
    }

    // Apply complementary filter
    angleX = complementaryFilter(accPitch, gyro.x, dt, angleX);
    angleY = complementaryFilter(accRoll, gyro.y, dt, angleY);
    angleZ = complementaryFilter(magYaw, gyro.z, dt, angleZ);

    return {
        angleX,
        angleY,
        angleZ
    };
}

exports.calculateRotation = calculateRotation;
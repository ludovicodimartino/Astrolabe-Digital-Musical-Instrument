// Timestamp of the last data reception
let lastReceivedDataTS = Date.now(); 

// Complementary filter variables initialization
let angleX = 0, angleY = 0, angleZ = 0;
const alpha = 0.98;

/**
 * Calculates the complementary filter.
 * @param {*} accelAngle 
 * @param {*} gyroRate 
 * @param {*} dt 
 * @param {*} currentAngle 
 * @returns
 */
function complementaryFilter(accelAngle, gyroRate, dt, currentAngle) {
    return alpha * (currentAngle + gyroRate * dt) + (1 - alpha) * accelAngle;
}

function calculateYaw(mag) {
    return Math.atan2(mag.y, mag.x);
}

/**
 * Calculate the rotation to be used in the frontend based on the data from the sensors.
 * The calculation is done using the <strong>complementary filter</strong>.
 * @param {*} sensorData 
 */
function calculateRotation(sensorData) {
    const { acc, gyro, mag } = sensorData;
    
    const accPitch = Math.atan2(acc.y, Math.sqrt(acc.x * acc.x + acc.z * acc.z));
    const accRoll = Math.atan2(-acc.x, acc.z);

    const magYaw = calculateYaw(mag);

    // Update time interval
    const dt = (Date.now() - lastReceivedDataTS) / 1000;
    lastReceivedDataTS = Date.now();

    angleX = complementaryFilter(accPitch, gyro.x, dt, angleX);
    angleY = complementaryFilter(accRoll, gyro.y, dt, angleY);
    angleZ = complementaryFilter(magYaw, gyro.z, dt, angleZ);

    return {
        angleX,
        angleY,
        angleZ
    }
}

exports.calculateRotation = calculateRotation;
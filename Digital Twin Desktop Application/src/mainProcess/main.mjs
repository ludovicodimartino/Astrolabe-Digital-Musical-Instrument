import {app, BrowserWindow, ipcMain} from "electron";
import { UdpOscListen } from "./listenUdp.mjs";
import { mf } from "./madgwickFilter.mjs";
import electronSquirrelStartup from "electron-squirrel-startup";

if(electronSquirrelStartup) app.quit();

/**
 * The main window of the application
 */
let mainWindow;

/**
 * This function creates the main window of the application and initializes the IPC channels.
 */
const createWindow = () => {
    mainWindow = new BrowserWindow({
        width: 800,
        height: 600,
        fullscreenable: true,
        autoHideMenuBar: true,
        frame: false,
        webPreferences: {
            nodeIntegration: true,
            contextIsolation: false
        }
    });

    mainWindow.loadFile("./src/rendererProcess/index.html");

    ipcMain.on('close-app', () => {
        mainWindow.close();
    });

    ipcMain.on('minimize-app', () => {
        mainWindow.minimize();
    });

    ipcMain.on('fullscreen-app', () => {
        if (mainWindow.isFullScreen()) {
            mainWindow.setFullScreen(false);
        } else {
            mainWindow.setFullScreen(true);
        }
    });

    ipcMain.on('exit-fullscreen-app', () => {
        mainWindow.setFullScreen(false);
    });
}

/** 
 * An instance of the UdpOscListen class that listens for OSC messages.
 */
const udpOscListener = new UdpOscListen();

/**
 * Tracks the timestamp of the last function call in milliseconds.
 * Used by the `throttle` function to limit the frequency of function executions.
 * @type {number}
 */
let lastCall = 0;

/**
 * Limits the execution of a function to at most once every specified time interval.
 * 
 * @param {Function} func - The function to be throttled.
 * @param {number} limit - The minimum time interval (in milliseconds) between consecutive executions of the function.
 */
const throttle = (func, limit) => {
    const now = Date.now();
    if (now - lastCall >= limit) {
        lastCall = now;
        func();
    }
}

/**
 * When a new OSC bundle with a message containing IMU information is received.
 */ 
udpOscListener.on("OSCBundleReceived", (msg) => {
    throttle(() => {
        if (msg.length !== 9){
            console.log("Illegal Arguments received as OSCBundle");
            throw new Error("Illegal Arguments");
        } 
        const sensors = {
            acc: { x: msg[0].args[0]/9.81, y: msg[1].args[0]/9.81, z: msg[2].args[0]/9.81 },
            mag: { x: msg[3].args[0], y: msg[4].args[0], z: msg[5].args[0] },
            gyro: { x: msg[6].args[0], y: msg[7].args[0], z: msg[8].args[0] }
        };
        
        // Update the Madgwick filter with the new sensor data
        mf.update(sensors.gyro.x, sensors.gyro.y, sensors.gyro.z, sensors.acc.x, sensors.acc.y, sensors.acc.z, sensors.mag.x, sensors.mag.y, sensors.mag.z);
        
        // Get the angles from the Madgwick filter
        let quaternion = mf.getQuaternion();

        // Send the angles to the renderer process
        mainWindow.webContents.send("rotation:data", quaternion);
    }, 33);
})

/**
 * When a new OSC message containing the encoder information is received
 */
let radianAngle = 0;
udpOscListener.on("OSCMessageReceived", (msg) => {
    switch(msg.address){
        case "/encA":
            radianAngle = -(msg.args[0] % 30) * (Math.PI / 15);
            mainWindow.webContents.send("alidade:data", radianAngle);
            break;
        case "/encB":
            radianAngle = -(msg.args[0] % 30) * (Math.PI / 15);
            mainWindow.webContents.send("rete:data", radianAngle);
            break;
        case "/reset":
            mainWindow.webContents.send("reset");
            break;
    }
})

udpOscListener.on("timeout", (msg) => {
    mainWindow.webContents.send("ctrlMsg", formatCtrlMessage(msg, 'danger'));
})

udpOscListener.on("newDataComing", () => {
    mainWindow.webContents.send("ctrlMsg", formatCtrlMessage("Receiving new data from the sensors.", 'success'));
})

udpOscListener.on("listenError", (msg) => {
    mainWindow.webContents.send("ctrlMsg", formatCtrlMessage(msg, 'warning'));
})

/**
 * This function is called when the application is ready.
 * It creates the main window and initializes the UDP listener.
 */
app.whenReady().then(() => {
    createWindow();
    udpOscListener.init();
});

/**
 * This function is called when the application is closed.
 * It closes the UDP listener and quits the application.
 */
app.on('window-all-closed', () => {
    udpOscListener.close();
    app.quit();
})

/**
 * This function formats the control messages sent to the renderer process.
 * 
 * @param {string} msgText the text content of the message.
 * @param {'success' | 'danger' | 'warning'} type - The Bootstrap type of the message.
 *   - `'success'`: Indicates a successful operation or status.
 *   - `'danger'`: Indicates an error or critical issue.
 *   - `'warning'`: Indicates a warning or non-critical issue.
 * @returns {{type: string, msg: string}}
 */
const formatCtrlMessage = (msgText, type) => {
    return {
        type,
        msg: msgText
    };
};
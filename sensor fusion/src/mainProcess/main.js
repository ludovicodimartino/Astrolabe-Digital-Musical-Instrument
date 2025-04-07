const {app, BrowserWindow, ipcMain} = require("electron");
const { UdpOscListen } = require("./listenUdp");
const { calculateRotation } = require("./sensorFusion");
const os = require('os');

if(require('electron-squirrel-startup')) app.quit();

let mainWindow;

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

function getWiFiIPv4() {
    const interfaces = os.networkInterfaces();
    
    for (const iface of Object.values(interfaces)) {
        for (const info of iface) {
            if (info.family === 'IPv4' && !info.internal && info.mac !== '00:00:00:00:00:00') {
                return info.address; // Returns the first valid IPv4
            }
        }
    }
    return null; // No valid WiFi IPv4 found
}

const udpOscListener = new UdpOscListen(getWiFiIPv4());

const throttle = (func, limit) => {
    let lastCall = 0;
    return (...args) => {
        const now = Date.now();
        if (now - lastCall >= limit) {
            lastCall = now;
            func(...args);
        }
    };
}

// Throttle the rotation updates to 30 FPS (33ms interval)
const sendRotationData = throttle((calculatedRotation) => {
    mainWindow.webContents.send("rotation:data", calculatedRotation);
}, 33);

// When a new OSC bundle with a message containing IMU information is received
udpOscListener.on("OSCBundleReceived", (msg) => {
    if (msg.length !== 9) throw new Error("Illegal Arguments");
    msg = {
        acc: { x: msg[0].args[0], y: msg[1].args[0], z: msg[2].args[0] },
        mag: { x: msg[3].args[0], y: msg[4].args[0], z: msg[5].args[0] },
        gyro: { x: msg[6].args[0], y: msg[7].args[0], z: msg[8].args[0] }
    };
    const calculatedRotation = calculateRotation(msg);
    sendRotationData(calculatedRotation); // Throttled function
})

// When a new OSC message containing the encoder information is received
udpOscListener.on("OSCMessageReceived", (msg) => {
    switch(msg.address){
        case "/encA":
            mainWindow.webContents.send("alidada:data", msg.args[0]);
            break;
        case "/encB":
            mainWindow.webContents.send("rete:data", msg.args[0]);
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

app.whenReady().then(() => {
    createWindow();
    udpOscListener.init();
});

// Final operations
app.on('window-all-closed', () => {
    udpOscListener.close();
    app.quit();
})

const formatCtrlMessage = (msgText, type) => {
    return {
        type,
        msg: msgText
    };
};
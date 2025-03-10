const {app, BrowserWindow} = require("electron");
const { UdpOscListen } = require("./listenUdp");
const { calculateRotation } = require("./sensorFusion");

let mainWindow;

const createWindow = () => {
    mainWindow = new BrowserWindow({
        width: 800,
        height: 600,
        fullscreenable: true,
        autoHideMenuBar: false,
        webPreferences: {
            nodeIntegration: true,
            contextIsolation: false
        }
    });

    mainWindow.loadFile("./src/public/index.html")
}

const udpOscListener = new UdpOscListen("192.168.43.238");

// When a new OSC bundle with a message containing IMU information is received
udpOscListener.on("OSCBundleReceived", (msg) => {
    if (msg.length !== 9) throw new Error("Illegal Arguments");
    msg = {
      acc: { x: msg[0].args[0], y: msg[1].args[0], z: msg[2].args[0] },
      mag: { x: msg[3].args[0], y: msg[4].args[0], z: msg[5].args[0] },
      gyro: { x: msg[6].args[0], y: msg[7].args[0], z: msg[8].args[0] }
    };
    const calculatedRotation = calculateRotation(msg);
    // console.log(calculatedRotation);
    mainWindow.webContents.send("rotation:data", calculatedRotation);
})

// When a new OSC message containing the encoder information is received
udpOscListener.on("OSCMessageReceived", (msg) => {
    console.log(msg);
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
})

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
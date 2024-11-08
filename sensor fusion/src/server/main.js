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

const udpOscListener = new UdpOscListen();

udpOscListener.on("OSCMessageReceived", (msg) => {
    const calculatedRotation = calculateRotation(msg);
    console.log(calculatedRotation);
    mainWindow.webContents.send("rotation:data", calculatedRotation);
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
const {app, BrowserWindow} = require("electron");
const { initUDPListener, closeUDPListening } = require("./listenUdp");
const { calculateRotation } = require("./sensorFusion");

let mainWindow;

const createWindow = () => {
    mainWindow = new BrowserWindow({
        width: 800,
        height: 600,
        webPreferences: {
            nodeIntegration: true,
            contextIsolation: false
        }
    });

    console.log(__dirname);

    mainWindow.loadFile("./src/public/index.html")
}


app.whenReady().then(() => {
    createWindow()
    initUDPListener((msg) => {
        const calculatedRotation = calculateRotation(msg);
        console.log(calculatedRotation);
        mainWindow.webContents.send("rotation:data", calculatedRotation);
    })
})

// Final operations
app.on('window-all-closed', () => {
    closeUDPListening();
    app.quit();
})


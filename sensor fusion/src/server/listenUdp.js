const dgram = require('node:dgram');
const server = dgram.createSocket('udp4');
const osc = require('osc');
const PORT = 9999;

let udpPort;

function parseOSCMessage(msg) {
  if(msg.length != 9) throw new Error("Illegal Arguments");
  return {
    acc: {x: msg[0].args[0], y: msg[1].args[0], z: msg[2].args[0]},
    mag: {x: msg[3].args[0], y: msg[4].args[0], z: msg[5].args[0]},
    gyro: {x: msg[6].args[0], y: msg[7].args[0], z: msg[8].args[0]}
  }
}

function initUDPListener(callback){
  udpPort = new osc.UDPPort({
      localAddress: "0.0.0.0",
      localPort: PORT
  });

  udpPort.on("bundle", function (bundle) {
    /*{
  timeTag: { raw: [ 0, 0 ], native: -2208988800000 },
  packets: [
    { address: '/acc/x', args: [Array] },
    { address: '/acc/y', args: [Array] },
    { address: '/acc/z', args: [Array] },
    { address: '/mag/x', args: [Array] },
    { address: '/mag/y', args: [Array] },
    { address: '/mag/z', args: [Array] },
    { address: '/gyro/x', args: [Array] },
    { address: '/gyro/y', args: [Array] },
    { address: '/gyro/z', args: [Array] }
  ]
}*/
    callback(parseOSCMessage(bundle.packets));
  });

  udpPort.open();
}

// Close the UDP port
function closeUDPListening(){
  udpPort.close();
}

exports.initUDPListener = initUDPListener;
exports.closeUDPListening = closeUDPListening;
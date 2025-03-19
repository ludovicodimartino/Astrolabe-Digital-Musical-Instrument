const dgram = require('node:dgram');
const os = require('os');
const server = dgram.createSocket('udp4');
const osc = require('osc');


/**
 * UdpOscListen class extends osc.UDPPort to provide a specialized OSC (Open Sound Control) UDP listener
 * that listens for OSC messages on a specified port. It includes custom event handling for
 * timeout, data receipt, and error scenarios.
 *
 * @class UdpOscListen
 * @extends osc.UDPPort
 * @author Ludovico Di Martino
 * @example
 * const listener = new UdpOscListen("192.168.43.238", 9999);
 * listener.on("timeout", (msg) => console.log(msg));
 * listener.on("newDataComing", () => console.log("Data is coming in"));
 * listener.init();
 */
class UdpOscListen extends osc.UDPPort {
  #TIMEOUT_MS = 5000;
  #timeoutId;
  #noDataComing = false;
  #timeoutHandler = (msg) => { };
  #newDataComingHandler = () => { };
  #errorHandler = (msg) => { };
  #OSCBundleReceivedHandler = (msg) => { };
  #OSCMessageReceivedHandler = (msg) => { };

  /**
   * Constructs a UdpOscListen instance listening on the specified port.
   * @constructor
   * @param {String} localAddress - The interface local address on which you want to listen.
   * @param {number} [port=9999] - The port to listen for incoming OSC messages.
   */
  constructor(localAddress, port = 9999) {
    if (localAddress === null) throw new Error("Invalid local address.");
    super({
      localAddress: localAddress,
      multicastMembership: [{
        address: "239.255.0.1",
        interface: localAddress
      }],
      localPort: port
    });
  }

  /**
   * Initializes the UDP listener, setting up a timeout mechanism and beginning
   * to listen for OSC message bundles and simple messages.
   * - If an error occurs during parsing, the ```errorHandler``` is invoked.
   * - If a OSC message is received, the ```OSCMessageReceivedHandler``` is invoked.
   * - If an OSC bundle is received, the ```OSCBundleReceivedHandler``` is invoked.
   */
  init() {
    this.#resetTimeout();

    // Modifying the OSC library to avoid emitting messages for the boundles
    let modOSC = require("../../node_modules/osc/src/osc.js")
    modOSC.fireBundleEvents = function (port, bundle, timeTag, packetInfo) {
      port.emit("bundle", bundle, timeTag, packetInfo);
      for (var i = 0; i < bundle.packets.length; i++) {
        var packet = bundle.packets[i];
        // osc.firePacketEvents(port, packet, bundle.timeTag, packetInfo);
      }
    };
    
    // When a bundle is received
    this.on("bundle", (bundle) => {
      this.#resetTimeout();
      try {
        this.#OSCBundleReceivedHandler(bundle.packets);
      } catch (e) {
        this.#errorHandler("Received corrupted OSC IMU message!");
      }
    });

    // When a simple message is received
    this.on("message", (msg) => {
      this.#resetTimeout();
      try {
        this.#OSCMessageReceivedHandler(msg);
      } catch (e) {
        this.#errorHandler("Received corrupted OSC Encoder message!");
      }
    });

    // Start UDP listening
    this.open();
  }

  /**
   * Resets the timeout for incoming data. If no data is received within the specified
   * timeout period, the timeoutHandler is invoked with a "No data coming from the sensor" message.
   * If data is received before the timeout, the ```newDataComingHandler``` is invoked.
   * @private
   */
  #resetTimeout() {
    if (this.#noDataComing) {
      this.#noDataComing = false;
      this.#newDataComingHandler();
    }
    clearTimeout(this.#timeoutId);
    this.#timeoutId = setTimeout(() => {
      this.#noDataComing = true;
      this.#timeoutHandler("No data coming from the sensor");
    }, this.#TIMEOUT_MS);
  }

  /**
   * Sets custom event handlers for specific events.
   *
   * @param {string} eventName - The name of the event. Possible values:
   *                             'timeout', 'newDataComing', 'OSCBundleReceived', 'listenError', 'OSCMessageReceived'.
   * @param {Function} handler - The callback function to handle the event.
   */
  on(eventName, handler) {
    switch (eventName) {
      case 'timeout':
        this.#timeoutHandler = handler;
        break;
      case 'newDataComing':
        this.#newDataComingHandler = handler;
        break;
      case 'OSCBundleReceived':
        this.#OSCBundleReceivedHandler = handler;
        break;
      case 'OSCMessageReceived':
        this.#OSCMessageReceivedHandler = handler;
        break;
      case 'listenError':
        this.#errorHandler = handler;
        break;
      default:
        super.on(eventName, handler);
    }
  }

}

exports.UdpOscListen = UdpOscListen;
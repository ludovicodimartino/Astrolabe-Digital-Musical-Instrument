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
  #timeoutHandler = (msg) => {};
  #newDataComingHandler = () => {};
  #errorHandler = (msg) => {}; 
  #OSCMsgReceivedHandler = (msg) => {};

  /**
   * Constructs a UdpOscListen instance listening on the specified port.
   * @constructor
   * @param {String} localAddress - The interface local address on which you want to listen.
   * @param {number} [port=9999] - The port to listen for incoming OSC messages.
   */
  constructor(localAddress, port = 9999) {
    super({
      localAddress: localAddress,
      multicastMembership: [{address: "225.0.0.1", 
        interface: localAddress
      }],
      localPort: port
    });
  }

  /**
   * Initializes the listener, setting up a timeout mechanism and beginning
   * to listen for OSC message bundles.
   * If an error occurs during parsing, the errorHandler is invoked.
   * If a valid OSC message is received, the OSCMsgReceivedHandler is invoked.
   */
  init() {
    this.#resetTimeout();
    this.on("bundle", (bundle) => {
      this.#resetTimeout();
      try {
        this.#OSCMsgReceivedHandler(this.#parseOSCMessage(bundle.packets));
      } catch (e) {
        this.#errorHandler("Received corrupted OSC message!");
      }
    });

    // Start UDP listening
    this.open();
  }

  /**
   * Resets the timeout for incoming data. If no data is received within the specified
   * timeout period, the timeoutHandler is invoked with a "No data coming from the sensor" message.
   * If data is received before the timeout, the newDataComingHandler is invoked.
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
   * Parses incoming OSC messages, expecting a specific 9-element format for accelerometer,
   * magnetometer, and gyroscope data. Throws an error if the message does not match
   * the expected format.
   * 
   * @private
   * @param {Array} msg - The incoming OSC message array.
   * @returns {Object} Parsed message data containing acc, mag, and gyro data objects.
   * @throws {Error} If the message does not contain exactly 9 elements.
   */
  #parseOSCMessage(msg) {
    if (msg.length !== 9) throw new Error("Illegal Arguments");
    return {
      acc: { x: msg[0].args[0], y: msg[1].args[0], z: msg[2].args[0] },
      mag: { x: msg[3].args[0], y: msg[4].args[0], z: msg[5].args[0] },
      gyro: { x: msg[6].args[0], y: msg[7].args[0], z: msg[8].args[0] }
    };
  }

  /**
   * Sets custom event handlers for specific events.
   *
   * @param {string} eventName - The name of the event. Possible values:
   *                             'timeout', 'newDataComing', 'OSCMessageReceived', 'listenError'.
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
      case 'OSCMessageReceived':
        this.#OSCMsgReceivedHandler = handler;
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
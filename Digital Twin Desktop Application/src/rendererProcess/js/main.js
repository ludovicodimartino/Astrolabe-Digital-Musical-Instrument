import * as THREE from 'three';
import { OrbitControls } from 'orbitControls';
import { GLTFLoader } from 'GLTFLoader';
const { ipcRenderer } = require('electron')
import { showErrorMessage, checkboxSelectorVisibility } from './messages.js'

/** Manage the top bar */
const customTopBarDiv = document.getElementById('customTopBar');
const bodyElement = document.body;

// Close Button
document.getElementById('closeButton').addEventListener('click', () => {
  ipcRenderer.send('close-app');
});

// Minimize Button
document.getElementById('minimizeButton').addEventListener('click', () => {
  ipcRenderer.send('minimize-app');
});

// Track fullscreen state
let isFullscreen = false;

// Toggle fullscreen mode
document.getElementById('fullscreenButton').addEventListener('click', () => {
  ipcRenderer.send('fullscreen-app');
  isFullscreen = !isFullscreen;
  if (isFullscreen) {
    bodyElement.classList.add('fullscreen-mode');
  } else {
    bodyElement.classList.remove('fullscreen-mode');
  }
  console.log("Fullscreen mode toggled:", isFullscreen);
  customTopBarDiv.style.display = isFullscreen ? 'none' : 'flex'; // Hide the top bar in fullscreen mode
  checkboxSelectorVisibility(); // Hide the checkbox when in fullscreen mode
});

// Listen for keydown events (Escape key to exit fullscreen)
document.addEventListener('keydown', (event) => {
  if (isFullscreen && event.key === 'Escape') {
    ipcRenderer.send('exit-fullscreen-app');
    isFullscreen = false;
    customTopBarDiv.style.display = 'flex';
    checkboxSelectorVisibility();
    bodyElement.classList.remove('fullscreen-mode');
  }
});

const canvas = document.getElementById("threeCanvas");
const renderer = new THREE.WebGLRenderer({ antialias: true, canvas });
renderer.outputColorSpace = THREE.SRGBColorSpace;

renderer.setSize(window.innerWidth, window.innerHeight);
renderer.setClearColor(0x000000); //bkg color
renderer.setPixelRatio(window.devicePixelRatio);

renderer.shadowMap.enabled = true;
renderer.shadowMap.type = THREE.PCFSoftShadowMap;
// renderer.shadowMap.type = THREE.BasicShadowMap;

document.body.appendChild(renderer.domElement);

// Scene definition
const scene = new THREE.Scene();

// Camera definition
const camera = new THREE.PerspectiveCamera(45, window.innerWidth / window.innerHeight, 1, 1000);
camera.position.set(4, 5, 11);

// Controls to move the view
const controls = new OrbitControls(camera, renderer.domElement);
controls.enableDamping = true;
controls.enablePan = false;
controls.minDistance = 5;
controls.maxDistance = 20;
controls.minPolarAngle = 0.5;
controls.maxPolarAngle = 1.5;
controls.autoRotate = false;
controls.target = new THREE.Vector3(0, 1, 0);
controls.update();

/** 
 * The THREE points representing the stars.
 */
let starsObject = null;

// --- Starry background ---
function addStars(scene, starCount = 1000, radius = 100) {
  const geometry = new THREE.BufferGeometry();
  const positions = [];
  for (let i = 0; i < starCount; i++) {
    const phi = Math.acos(2 * Math.random() - 1);
    const theta = 2 * Math.PI * Math.random();
    const r = radius * (0.8 + 0.2 * Math.random());
    positions.push(
      r * Math.sin(phi) * Math.cos(theta),
      r * Math.sin(phi) * Math.sin(theta),
      r * Math.cos(phi)
    );
  }
  geometry.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
  const material = new THREE.PointsMaterial({ color: 0xffffff, size: 0.7, sizeAttenuation: true });
  starsObject = new THREE.Points(geometry, material);
  scene.add(starsObject);
}

addStars(scene);


// Light
const spotLight = new THREE.SpotLight(0xffffff, 3000, 100, 0.32, 1);
spotLight.position.set(0, 25, 0);
spotLight.castShadow = true;
spotLight.shadow.bias = -0.0001;
scene.add(spotLight);

const sunLight = new THREE.DirectionalLight(0xfff7e0, 1.5); // Warm sunlight color and intensity
sunLight.position.set(10, 30, 10);
sunLight.castShadow = true;
sunLight.shadow.mapSize.width = 2048;
sunLight.shadow.mapSize.height = 2048;
sunLight.shadow.bias = -0.0001;
scene.add(sunLight);

// Group to hold models
const modelGroup = new THREE.Group();
scene.add(modelGroup);

// Define the filenames of the models
const modelsFileNames = ['madre_timpano_bullone.glb', 'rete.glb', 'alidada.glb'];
const modelMeshes = {};

// GLTF Loader with model-loading function
const loader = new GLTFLoader().setPath('./3dModels/');
let modelCount = 0; // Counter to keep track of loaded models
const loadModel = (fileName) => {
  loader.load(
    fileName,
    (gltf) => {
      console.log(`Model loaded: ${fileName}`);
      modelMeshes[fileName] = gltf.scene;

      // Enable shadows for meshes
      modelMeshes[fileName].traverse((child) => {
        if (child.isMesh) {
          child.castShadow = true;
          child.receiveShadow = true;
        }
      });

      // Add model to the group
      modelGroup.add(modelMeshes[fileName]);

      modelCount++;

      // Start the animation loop when all models are loaded
      if (modelCount === modelsFileNames.length) {
        animate();
      }
    }
  );
};

// Load models
modelsFileNames.forEach(loadModel);

// Variables to store target rotations for smooth interpolation
let targetGroupRotation = { x: 0, y: 0, z: 0 };
let targetAlidadeRotation = 0;
let targetReteRotation = 0;

// Current rotations for interpolation
let currentGroupQuaternion = new THREE.Quaternion(0, 0, 0, 1);
let currentAlidadeRotation = 0;
let currentReteRotation = 0;

// Update group rotation with IMU sensor data
ipcRenderer.on('rotation:data', (event, data) => {
  currentGroupQuaternion.set(-data.y, data.z, -data.x, data.w);
});

// Update alidade rotation with rotary encoder data
ipcRenderer.on('alidade:data', (event, data) => {
  targetAlidadeRotation = data;
});

// Update rete rotation with rotary encoder data
ipcRenderer.on('rete:data', (event, data) => {
  targetReteRotation = data;
});

// Reset rotation with rotary encoder data
ipcRenderer.on('reset', (event, data) => {
  console.log('Reset received');
  currentAlidadeRotation = currentReteRotation = 0;
  targetAlidadeRotation = targetReteRotation = 0;
  modelMeshes['alidada.glb'].rotation.y = 0;
  modelMeshes['rete.glb'].rotation.y = 0;
});

// Base transformation for the group
modelGroup.position.set(0, 1, 0);
modelGroup.scale.set(25, 25, 25);

// Window resizing
window.addEventListener('resize', () => {
  camera.aspect = window.innerWidth / window.innerHeight;
  camera.updateProjectionMatrix();
  renderer.setSize(window.innerWidth, window.innerHeight);
});

// Animation loop
let lastFrameTime = 0;
const targetFPS = 30; // Target 30 FPS
const animate = (time) => {
  const delta = time - lastFrameTime;
  if (delta >= 1000 / targetFPS) {
    lastFrameTime = time;

    // Smoothly interpolate group rotation
    modelGroup.quaternion.slerp(currentGroupQuaternion, 0.1);

    // Smoothly interpolate alidada rotation
    currentAlidadeRotation = circularLerp(currentAlidadeRotation, targetAlidadeRotation, 0.1, 2 * Math.PI);
    modelMeshes['alidada.glb'].rotation.y = currentAlidadeRotation;

    // Smoothly interpolate rete rotation
    currentReteRotation = circularLerp(currentReteRotation, targetReteRotation, 0.1, 2 * Math.PI);
    modelMeshes['rete.glb'].rotation.y = currentReteRotation;

    // Update the stars' positions when the rete is moved
    if (starsObject) {
      starsObject.rotation.y = currentReteRotation;
    }

    // Update controls and render the scene
    controls.update();
    renderer.render(scene, camera);
  }
  requestAnimationFrame(animate);
};

// Helper function for circular interpolation
function circularLerp(start, end, t, maxValue) {
  const delta = (end - start + maxValue) % maxValue; // Calculate the shortest path
  const shortestDelta = delta > maxValue / 2 ? delta - maxValue : delta; // Adjust for circular wrap-around
  return start + shortestDelta * t;
}

ipcRenderer.on("ctrlMsg", (event, data) => {
  console.log(data);
  showErrorMessage(data.msg, data.type);
});

// Test the rete rotation using the arrow keys
window.addEventListener('keydown', (event) => {
  if (event.key === 'ArrowLeft') {
    targetReteRotation -= Math.PI / 15; // Decrease angle
  } else if (event.key === 'ArrowRight') {
    targetReteRotation += Math.PI / 15; // Increase angle
  }
});

import * as THREE from 'three'
import { GLTFLoader } from 'three/addons/loaders/GLTFLoader.js'
import { OrbitControls } from 'three/addons/controls/OrbitControls.js'
const { ipcRenderer } = require('electron') 
import { showErrorMessage } from './messages.js'

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

// Ground plane
const groundGeometry = new THREE.PlaneGeometry(20, 20, 32, 32);
groundGeometry.rotateX(-Math.PI / 2);
const groundMaterial = new THREE.MeshStandardMaterial({
  color: 0x555555,
  side: THREE.DoubleSide
});
const groundMesh = new THREE.Mesh(groundGeometry, groundMaterial);
groundMesh.castShadow = false;
groundMesh.receiveShadow = true;
scene.add(groundMesh);


// Light
const spotLight = new THREE.SpotLight(0xffffff, 3000, 100, 0.32, 1);
spotLight.position.set(0, 25, 0);
spotLight.castShadow = true;
spotLight.shadow.bias = -0.0001;
scene.add(spotLight);

// Group to hold models
const modelGroup = new THREE.Group();
scene.add(modelGroup);

// Define the filenames of the models
const modelsFileNames = ['madre_timpano_bullone.glb', 'rete.glb', 'alidada.glb'];
const modelMeshes = {};

// GLTF Loader with model-loading function
const loader = new GLTFLoader().setPath('./models_test/');
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
    },
    (xhr) => {
      console.log(`Loading progress (${fileName}): ${(xhr.loaded / xhr.total * 100).toFixed(2)}%`);
    },
    (error) => {
      console.error(`Error loading ${fileName}:`, error);
    }
  );
};

// Load models
modelsFileNames.forEach(loadModel);

// Update group rotation with IMU sensor data
let lastUpdateTime = 0;
const updateInterval = 50; // Update every 50ms
ipcRenderer.on('rotation:data', (event, data) => {
  const now = Date.now();
  if (now - lastUpdateTime >= updateInterval) {
    lastUpdateTime = now;
    modelGroup.rotation.set(-data.angleY, data.angleZ, data.angleX);
  }
});

// Helper function for linear interpolation
function lerp(start, end, t) {
  return start + (end - start) * t;
}

// Update alidada rotation with rotary encoder data
ipcRenderer.on('alidada:data', (event, data) => {
  const targetRotation = -(data % 30) * (Math.PI / 15);
  const currentRotation = modelMeshes['alidada.glb'].rotation.y;

  // Interpolate between current and target rotation
  const duration = 200; // Transition duration in milliseconds
  const steps = 30; // Number of interpolation steps
  const stepTime = duration / steps;
  let step = 0;

  const interval = setInterval(() => {
    if (step >= steps) {
      clearInterval(interval);
      modelMeshes['alidada.glb'].rotation.y = targetRotation; // Ensure final value is set
    } else {
      const t = step / steps;
      modelMeshes['alidada.glb'].rotation.y = lerp(currentRotation, targetRotation, t);
      step++;
    }
  }, stepTime);
});

// Update rete rotation with rotary encoder data
ipcRenderer.on('rete:data', (event, data) => {
  modelMeshes['rete.glb'].rotation.y = -(data%30) * (Math.PI / 15);
});

// Reset rotation with rotary encoder data
ipcRenderer.on('reset', (event, data) => {
  console.log('Reset received');
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

// Updates the rotation of the model based on the received data
const updateGroupRotation = (rotationData) => {
    group.rotation.set(-rotationData.angleY, rotationData.angleZ, rotationData.angleX);
}

// Animation loop
let lastFrameTime = 0;
const targetFPS = 30; // Target 30 FPS
const animate = (time) => {
  const delta = time - lastFrameTime;
  if (delta >= 1000 / targetFPS) {
    lastFrameTime = time;
    controls.update();
    renderer.render(scene, camera);
  }
  requestAnimationFrame(animate);
};
animate();

ipcRenderer.on("ctrlMsg", (event, data) => {
  console.log(data);
  showErrorMessage(data.msg, data.type);
})

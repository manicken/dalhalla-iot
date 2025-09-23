const fs = require('fs');
const WebSocket = require('ws');

const wss = new WebSocket.Server({port:8081});

const { spawn } = require('child_process');

let serverProcess;

// Function to start the server
function startServer() {
  serverProcess = spawn('node', ['server.js'], { stdio: 'inherit' });

  serverProcess.on('close', (code) => {
    if (code === 8) {
      console.error('Error detected, waiting for changes...');
    }
  });
}

// Function to watch for changes in files
function watchFiles() {
  const watcher = fs.watch('./server.js', { recursive: false }, (event, filename) => {
    if (filename && (filename.endsWith('.js') || filename.endsWith('.json'))) {
      console.log(`File ${filename} changed, restarting server...`);

    // Perform any cleanup tasks or other actions here
    wss.clients.forEach((client) => {
        if (client.readyState === WebSocket.OPEN) {
        client.send('{"cmd":"reload","timeout":2000}'); // Send a message to reload the page after a time
        }
    });
      serverProcess.kill('SIGTERM');
      startServer();
    }
  });

  watcher.on('error', (error) => {
    console.error('Watcher error:', error);
  });
}

// Handle WebSocket connections
wss.on('connection', function connection(ws) {
    console.log('server_mon Client connected');
    
    ws.on('message', function incoming(message) {
      console.log('server_mon Received: %s', message);
    });
  
    ws.send('{"connected":"Hello, Client from server_mon!"}');
  });
  
  // Display a message when the server starts listening
  wss.on('listening', () => {
    console.log('server_mon WebSocket server is listening on port 8080');
  });

// Start the server
startServer();

// Watch for changes in files
watchFiles();
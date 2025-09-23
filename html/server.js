// this is only to simulate the server side for the scheduler

// Importing required modules
const express = require('express');
const WebSocket = require('ws');
const chokidar = require('chokidar');
const multer = require('multer');
const fs = require('fs');
const path = require('path');

const webserver = express();
const upload = multer();
const port = 80;
const wss = new WebSocket.Server({port:8080});
/*
TimeAlarmsFromJson::NameToFunction nameToFunctionList[4] = {
  //   name         , onTick            , onTickExt
      {"ntp_sync"   , &Timer_SyncTime   , nullptr           },
      {"sendEnvData", &Timer_SendEnvData, nullptr           },
      {"fan"        , nullptr           , &Alarm_SetFanSpeed},
      {"rf433"      , nullptr           , &Alarm_SendToRF433}
  };
  */
const functionNames = {"ntp_sync":"","sendEnvData":"","fan":"p","rf433":"p"};
const shortDows = ["mon","tue", "wed", "thu", "fri", "sat", "sun"];

let execFileChange = true;
// Watch for file changes
chokidar.watch('./schedule').on('change', (path) => {
  if (execFileChange == false) return;

  // Trigger rebuild process
  console.log("file changed: "+  path);
  // Notify connected clients
  wss.clients.forEach((client) => {
    if (client.readyState === WebSocket.OPEN) {
      client.send('{"cmd":"reload"}'); // Send a message to reload the page immediately
    }
  });
});
chokidar.watch('./schedule2').on('change', (path) => {
  if (execFileChange == false) return;

  // Trigger rebuild process
  console.log("file changed: "+  path);
  // Notify connected clients
  wss.clients.forEach((client) => {
    if (client.readyState === WebSocket.OPEN) {
      client.send('{"cmd":"reload"}'); // Send a message to reload the page immediately
    }
  });
});

webserver.use(express.json());
webserver.use(express.urlencoded({extended: true}));

// Middleware to inject JavaScript
webserver.use((req, res, next) => {
  
  // Intercept HTML responses
  res.oldSend = res.send;
  res.send = function(data) {
    //console.log(res.get('Content-Type'));

    // Check if response is HTML
    if (typeof data === 'string' && res.get('Content-Type') && res.get('Content-Type').includes('text/html')) {
      // Read the JavaScript file
      const script = fs.readFileSync('autoRefresh.js', 'utf8');
      console.log("injecting javascript into: ", req.url ? req.url : 'unknown');
      // Inject JavaScript into HTML
      data = data.replace('</head>', `<script>${script}</script></head>`);
    }
    // Call the original send function
    res.oldSend(data);
  };
  next();
});

// Define the storage engine
const storage = multer.diskStorage({
  destination: function (req, file, cb) {
    // Specify the destination folder for uploads
    cb(null, './');
  },
  filename: function (req, file, cb) {
    execFileChange = false; // set this to avoid file changed event, set to true in webserver.post('/edit')
    cb(null, sanitizeFilePath(file.originalname));
  }
});

function sanitizeFilePath(filepath) {
  // Remove any leading drive letters (e.g., 'C:\')
  filepath = filepath.replace(/^[A-Za-z]:[\\\/]?/, '');

  // Replace any redundant directory traversal characters with a single slash
  filepath = filepath.replace(/[\\\/]+/g, '/');

  // Remove any leading directory traversal sequences (e.g., '../')
  filepath = filepath.replace(/^(\.\.\/)+/, '');

  // Remove any leading slashes or backslashes
  filepath = filepath.replace(/^[\\\/]+/, '');

  console.log("sanitized FilePath: " + filepath);
  return filepath;
}

// Set up multer with custom options
const uploadFile = multer({
  storage: storage,
  preservePath: true
});

// Route to handle file upload
webserver.post('/edit', uploadFile.single("data"), (req, res) => {
  execFileChange = true; // set back to normal operation
  if (!req.file) {
    return res.status(400).send('No file uploaded.');
  }
  console.log(req.file);
  // This function will be executed after the file has been saved to disk
  console.log('File saved successfully');
  return res.status(200).send('File uploaded successfully.');
  
});

webserver.get('/schedule/refresh', (req, res) => {
  res.contentType("text/plain").send("schedule load json OK");
});

webserver.get('/schedule/getFunctionNames', (req, res) => {
    res.contentType("application/json").send(JSON.stringify(functionNames));
});

webserver.get('/schedule/getShortDows', (req, res) => {
    res.contentType("application/json").send(JSON.stringify(shortDows));
});

function getAnyIndexFileFromDir(req, res, dirPath) {
    console.log("requesting index file @ " + dirPath);

    let indexHtml = path.join(dirPath, 'index.html');
    if (fileExists(indexHtml)) {
        req.url += "/index.html";
        console.log("index.html found");
        sendFile(res, indexHtml);
        return;
    }

    let indexHtm = path.join(dirPath, 'index.htm');
    if (fileExists(indexHtm)) {
        req.url += "/index.htm";
        console.log("index.htm found");
        sendFile(res, indexHtm);
        return;
    }

    // not any index file
    res.status(404).send('404: Not Found');
}
  
webserver.get('*', (req, res) => {
    // Get the requested file path
    const localPath = path.join(__dirname, req.url);
    
    if (req.url.indexOf(".") == -1) {
        // if url is a dir, i.e. without any file extension
        getAnyIndexFileFromDir(req, res, localPath);
        return;
    }
    console.log("requesting file: " + localPath);
    // Check if the file exists
    if (fileExists(localPath) == false) {
      // If the file does not exist, send a 404 response
      res.status(404).send('404: Not Found');
      return;
    }
    sendFile(res, localPath);
});



// Start the server
webserver.listen(port, '0.0.0.0', () => {
  
    console.log(`Server running at http://0.0.0.0:${port}/`);
});

function sendFile(res, filePath) {
    // If the file exists, read and send it
    fs.readFile(filePath, 'utf8', (err, data) => {
      if (err) {
        // If there's an error reading the file, send a 500 response
        res.status(500).send('Internal Server Error');
      } else {
        // Send the file content with appropriate content type
        const contentType = getContentType(filePath);
        res.contentType(contentType).send(data);
      }
    });
}

// Function to determine the content type based on file extension
function getContentType(filePath) {
    const extname = path.extname(filePath).toLowerCase();
    switch (extname) {
      case '.html':
        return 'text/html';
      case '.css':
        return 'text/css';
      case '.js':
        return 'text/javascript';
      case '.json':
        return 'text/json';
      case '.txt':
        return 'text/plain';
      // Add more cases for other file types as needed
      default:
        return 'application/octet-stream'; // fallback content type
    }
  }

  function fileExists(filePath) {
    try {
      // Check if the file exists by attempting to read its stats
      fs.statSync(filePath);
      return true; // If successful, file exists
    } catch (err) {
      if (err.code === 'ENOENT') {
        return false; // File does not exist
      } else {
        throw err; // Other error occurred, propagate it
      }
    }
  }


// Handle WebSocket connections
wss.on('connection', function connection(ws) {
  console.log('server Client connected');
  
  ws.on('message', function incoming(message) {
    console.log('server Received: %s', message);
  });

  ws.send('{"connected":"Hello, Client from webserver!"}');
});

// Display a message when the server starts listening
wss.on('listening', () => {
  console.log('server WebSocket server is listening on port 8080');
});

console.log("injected javascript for autorefresh on file change");

// Establish WebSocket connection
const socket = new WebSocket('ws://localhost:8080');
// websocket for custom_nodemon
const socket2 = new WebSocket('ws://localhost:8081');

function onError(error) {
    console.error('WebSocket error:', error);
}

function onMessage(event) {
    console.log('Message from server:', event.data);
    const msg = JSON.parse(event.data);
    

    // You can handle the message here, for example, if the message indicates a page reload:
    if (msg.cmd === 'reload') {
        if (msg.timeout == undefined)
            location.reload();
        else
            setTimeout(function() {
                location.reload();
            },  parseInt(msg.timeout));
    }
}

// Log errors
socket.onerror = onError;
socket2.onerror = onError;

// Log messages from the server
socket.onmessage = onMessage;
socket2.onmessage = onMessage;

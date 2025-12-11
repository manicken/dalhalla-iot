const express = require('express');
const app = express();
const PORT = 8083;
const HOST = '0.0.0.0';  // Listen on all network interfaces

// For GET /update
app.get('/update', (req, res) => {
    const { api_key, field1, field2, field3, field4, field5, field6, field7, field8 } = req.query;

    if (!api_key) {
        return res.status(400).send('0'); // ThingSpeak returns 0 on failure
    }

    // OPTIONAL: validate API key
    if (api_key !== "0123456789ABCDEF") {
        return res.status(401).send('0');
    }

    // Simulate autoincrement entry ID
    const entryId = Math.floor(Math.random() * 50000) + 1;
    console.log(req.url);
    console.log("[" + new Date().toLocaleString() + "] Received TS update:", req.query);

    // ThingSpeak responds with the entry ID as a string
    res.send(entryId.toString());
});

// Optional: mock channel feeds endpoint
app.get('/channels/:id/feeds.json', (req, res) => {
    res.json({
        channel: { id: req.params.id, name: "Test Channel" },
        feeds: [
            { entry_id: 1, field1: "123", created_at: new Date().toISOString() }
        ]
    });
});

app.listen(PORT, HOST, () => {
    console.log(`Server running at http://${HOST}:${PORT}`);
});

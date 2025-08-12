const express = require('express');
const path = require('path');
const fs = require('fs');
const http = require('http');
const WebSocket = require('ws');

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

const DATA_FILE = path.join(__dirname, 'values.json');
let values = {};

// Load persisted values if file exists
if (fs.existsSync(DATA_FILE)) {
  try {
    values = JSON.parse(fs.readFileSync(DATA_FILE));
  } catch (err) {
    console.error('Failed to parse values.json:', err);
  }
}

app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));

// REST endpoints to persist and retrieve values
app.get('/api/values', (req, res) => {
  res.json(values);
});

app.post('/api/values', (req, res) => {
  const { key, value } = req.body;
  if (!key) {
    return res.status(400).json({ error: 'key is required' });
  }
  values[key] = value;
  fs.writeFile(DATA_FILE, JSON.stringify(values, null, 2), err => {
    if (err) {
      console.error('Failed to write values.json:', err);
      return res.status(500).json({ error: 'Failed to persist value' });
    }
    res.json({ status: 'ok' });
  });
});

// WebSocket connection for real-time graph data
wss.on('connection', ws => {
  console.log('Client connected');
  ws.on('close', () => console.log('Client disconnected'));
});

// Broadcast random data every second
setInterval(() => {
  const payload = JSON.stringify({ time: Date.now(), value: Math.random() * 100 });
  wss.clients.forEach(client => {
    if (client.readyState === WebSocket.OPEN) {
      client.send(payload);
    }
  });
}, 1000);

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
  console.log(`Server listening on http://localhost:${PORT}`);
});

# Real-Time Graph Demo

This project is a simple Node.js application demonstrating:

- An Express server serving an HTML5/JavaScript frontend
- Real-time charts powered by WebSockets
- Basic persistence of key/value pairs to a JSON file

## Running

```bash
npm install
npm start
```

Open <http://localhost:3000> in your browser to see the chart updating in real time. Use the form to save values, which are persisted to `values.json`.

## Testing

Syntax of `server.js` can be checked via:

```bash
npm test
```

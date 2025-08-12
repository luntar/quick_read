// Chart.js setup
const ctx = document.getElementById('chart').getContext('2d');
const chart = new Chart(ctx, {
  type: 'line',
  data: {
    labels: [],
    datasets: [{
      label: 'Random Data',
      data: [],
      fill: false,
      borderColor: 'rgb(75, 192, 192)',
      tension: 0.1
    }]
  },
  options: {
    responsive: true,
    maintainAspectRatio: false
  }
});

// WebSocket for real-time data
autoConnect();

function autoConnect() {
  const protocol = location.protocol === 'https:' ? 'wss' : 'ws';
  const ws = new WebSocket(`${protocol}://${location.host}`);
  ws.onmessage = event => {
    const { time, value } = JSON.parse(event.data);
    chart.data.labels.push(new Date(time).toLocaleTimeString());
    chart.data.datasets[0].data.push(value);
    chart.update('none');
  };
  ws.onclose = () => {
    // Attempt reconnection after a delay
    setTimeout(autoConnect, 1000);
  };
}

// Persist values via REST
const form = document.getElementById('value-form');
form.addEventListener('submit', async e => {
  e.preventDefault();
  const key = document.getElementById('key').value;
  const value = document.getElementById('value').value;
  await fetch('/api/values', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ key, value })
  });
  form.reset();
  loadValues();
});

async function loadValues() {
  const res = await fetch('/api/values');
  const data = await res.json();
  const list = document.getElementById('values-list');
  list.innerHTML = '';
  Object.entries(data).forEach(([k, v]) => {
    const li = document.createElement('li');
    li.textContent = `${k}: ${v}`;
    list.appendChild(li);
  });
}

loadValues();

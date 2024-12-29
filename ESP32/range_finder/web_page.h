#ifndef WEB_PAGE_H
#define WEB_PAGE_H

const char WEBPAGE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>ToF Distance Monitor</title>
    <style>
        :root {
            --primary-color: #00e5ff;
            --background-color: #0a0b1e;
            --panel-bg: rgba(20, 21, 44, 0.7);
            --text-color: #ffffff;
        }
        body {
            background: linear-gradient(135deg, var(--background-color), #1a1b3c);
            color: var(--text-color);
            font-family: 'Segoe UI', sans-serif;
            margin: 0;
            padding: 20px;
        }
        .dashboard {
            max-width: 1200px;
            margin: 0 auto;
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
        }
        .panel {
            background: var(--panel-bg);
            border-radius: 15px;
            padding: 20px;
            backdrop-filter: blur(10px);
            border: 1px solid rgba(0, 229, 255, 0.1);
        }
        .stat-box {
            background: rgba(10, 11, 30, 0.8);
            border-radius: 10px;
            padding: 15px;
            margin: 10px 0;
            border: 1px solid rgba(123, 47, 255, 0.2);
        }
        .stat-label {
            font-size: 0.9em;
            color: rgba(255,255,255,0.7);
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        .stat-value {
            font-size: 1.8em;
            color: var(--primary-color);
            text-shadow: 0 0 15px rgba(0, 229, 255, 0.5);
        }
        .history-graph {
            width: 100%;
            height: 200px;
            background: rgba(10, 11, 30, 0.8);
            border-radius: 10px;
            margin-top: 20px;
        }
        #distance.very-close { color: #ff4444; }
        #distance.near { color: #ffbb33; }
        #distance.normal { color: #00C851; }
        @media (max-width: 600px) {
            .dashboard { grid-template-columns: 1fr; }
        }
    </style>
</head>
<body>
    <div class="dashboard">
        <div class="panel">
            <div class="stat-box">
                <div class="stat-label">Distance</div>
                <div id="distance" class="stat-value">0 mm</div>
            </div>
            <div class="stat-box">
                <div class="stat-label">Status</div>
                <div id="status" class="stat-value">NO OBJECT</div>
            </div>
            <div class="stat-box">
                <canvas id="distanceHistory" class="history-graph"></canvas>
            </div>
        </div>
        <div class="panel">
            <div class="stat-box">
                <div class="stat-label">Network Name</div>
                <div class="stat-value">O_O</div>
            </div>
            <div class="stat-box">
                <div class="stat-label">IP Address</div>
                <div id="ipAddress" class="stat-value">192.168.1.101</div>
            </div>
            <div class="stat-box">
                <div class="stat-label">WebSocket Status</div>
                <div id="wsStatus" class="stat-value">Connecting...</div>
            </div>
            <div class="stat-box">
                <div class="stat-label">Update Rate</div>
                <div id="updateRate" class="stat-value">0 Hz</div>
            </div>
        </div>
    </div>
    <script>
        const ws = new WebSocket('ws://' + window.location.hostname + ':81/');
        const history = [];
        const MAX_HISTORY = 50;
        let lastUpdate = Date.now();
        let updateCounts = [];
        const canvas = document.getElementById('distanceHistory');
        const ctx = canvas.getContext('2d');
        function updateHistoryGraph() {
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            ctx.strokeStyle = 'rgba(0, 229, 255, 0.1)';
            for(let i = 0; i < canvas.width; i += 30) {
                ctx.beginPath();
                ctx.moveTo(i, 0);
                ctx.lineTo(i, canvas.height);
                ctx.stroke();
            }
            ctx.beginPath();
            ctx.strokeStyle = '#00e5ff';
            ctx.lineWidth = 2;
            history.forEach((dist, i) => {
                const x = (i / MAX_HISTORY) * canvas.width;
                const y = canvas.height - (dist / 2000) * canvas.height;
                if(i === 0) ctx.moveTo(x, y);
                else ctx.lineTo(x, y);
            });
            ctx.stroke();
        }
        ws.onopen = () => {
            document.getElementById('wsStatus').textContent = 'Connected';
        };
        ws.onclose = () => {
            document.getElementById('wsStatus').textContent = 'Disconnected';
        };
        ws.onmessage = function(event) {
            const data = JSON.parse(event.data);
            const distance = data.distance || 0;
            const distanceElement = document.getElementById('distance');
            distanceElement.textContent = distance + ' mm';
            const statusElement = document.getElementById('status');
            if(distance < 0 || distance > 2000) {
                statusElement.textContent = 'NO OBJECT';
                distanceElement.className = 'stat-value';
            } else if(distance < 200) {
                statusElement.textContent = 'VERY CLOSE';
                distanceElement.className = 'stat-value very-close';
            } else if(distance < 500) {
                statusElement.textContent = 'NEAR';
                distanceElement.className = 'stat-value near';
            } else {
                statusElement.textContent = 'DETECTED';
                distanceElement.className = 'stat-value normal';
            }
            history.push(distance);
            if(history.length > MAX_HISTORY) history.shift();
            updateHistoryGraph();
            const now = Date.now();
            updateCounts.push(now);
            updateCounts = updateCounts.filter(t => now - t < 1000);
            document.getElementById('updateRate').textContent = 
                updateCounts.length + ' Hz';
        };
        function resizeCanvas() {
            canvas.width = canvas.offsetWidth;
            canvas.height = canvas.offsetHeight;
        }
        window.addEventListener('resize', resizeCanvas);
        resizeCanvas();
    </script>
</body>
</html>
)=====";
#endif

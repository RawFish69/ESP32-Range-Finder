const char WEBPAGE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>Ultrasonic Radar System</title>
    <style>
        :root {
            --primary-color: #00e5ff;
            --secondary-color: #7b2fff;
            --background-color: #0a0b1e;
            --panel-bg: rgba(20, 21, 44, 0.7);
            --text-color: #ffffff;
            --grid-color: rgba(0, 229, 255, 0.1);
        }
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            min-height: 100vh;
            background: linear-gradient(135deg, var(--background-color), #1a1b3c);
            color: var(--text-color);
            font-family: 'Segoe UI', Arial, sans-serif;
            padding: 20px;
        }
        .dashboard {
            display: flex;
            flex-direction: column;
            gap: 20px;
            max-width: 1600px;
            margin: 0 auto;
        }
        .top-row {
            display: flex;
            gap: 20px;
            width: 100%;
            flex-wrap: wrap;
        }
        .radar-section, .stats-section {
            flex: 1;
            min-width: 300px;
            display: flex;
            flex-direction: column;
        }
        .visualization-container {
            background: var(--panel-bg);
            border-radius: 15px;
            padding: 20px;
            backdrop-filter: blur(10px);
            border: 1px solid rgba(0, 229, 255, 0.1);
            box-shadow: 0 4px 20px rgba(0, 229, 255, 0.1);
            display: flex;
            flex-direction: column;
            gap: 20px;
        }
        .radar-container {
            position: relative;
            width: 100%;
            max-width: 600px;
            aspect-ratio: 1 / 1;
            margin: 0 auto;
            overflow: hidden;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        #ultrasonicCanvas, #tofCanvas {
            position: absolute;
            top: 0; left: 0;
            width: 100%;
            height: 100%;
            border-radius: 50%;
        }
        .stats-panel {
            background: var(--panel-bg);
            border-radius: 15px;
            padding: 25px;
            backdrop-filter: blur(10px);
            border: 1px solid rgba(123, 47, 255, 0.1);
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
        }
        .stat-box {
            background: rgba(10, 11, 30, 0.8);
            border-radius: 15px;
            padding: 15px;
            border: 1px solid rgba(123, 47, 255, 0.2);
            transition: all 0.3s ease;
            display: flex;
            flex-direction: column;
            justify-content: center;
            position: relative;
            min-height: 100px;
        }
        .stat-box::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            height: 3px;
            background: linear-gradient(90deg, var(--primary-color), var(--secondary-color));
            opacity: 0.5;
        }
        .stat-box:hover {
            transform: translateY(-2px);
            border-color: var(--primary-color);
        }
        .stat-label {
            font-size: 1em;
            color: rgba(255,255,255,0.7);
            margin-bottom: 10px;
            text-transform: uppercase;
            letter-spacing: 2px;
        }
        .stat-value {
            font-size: 2em;
            font-weight: 600;
            color: var(--primary-color);
            text-shadow: 0 0 15px rgba(0, 229, 255, 0.5);
            transition: all 0.3s ease;
            word-wrap: break-word;
        }
        #detectionStatus {
            font-size: 1.6em;
            line-height: 1.2;
        }
        .stat-value.angle {
            color: #7b2fff;
            text-shadow: 0 0 15px rgba(123, 47, 255, 0.5);
        }
        .stat-value.range {
            font-family: 'Monaco', monospace;
        }
        .stat-value.tof {
            color: #ff6b6b;
            text-shadow: 0 0 15px rgba(255, 107, 107, 0.5);
        }
        .stat-value.packetLoss {
            color: #00C851;
            text-shadow: 0 0 15px rgba(0, 200, 81, 0.5);
        }
        .stat-box.status-very-close .stat-value {
            color: #ff4444;
            text-shadow: 0 0 15px rgba(255, 68, 68, 0.5);
            animation: pulse 1.5s infinite;
        }
        .stat-box.status-near .stat-value {
            color: #ffbb33;
            text-shadow: 0 0 15px rgba(255, 187, 51, 0.5);
        }
        .stat-box.status-detected .stat-value {
            color: #00C851;
            text-shadow: 0 0 15px rgba(0, 200, 81, 0.5);
        }
        @keyframes pulse {
            0%   { transform: scale(1); }
            50%  { transform: scale(1.05); }
            100% { transform: scale(1); }
        }
        .history-graph {
            width: 100%;
            height: 300px;
            background: rgba(10, 11, 30, 0.8);
            border-radius: 10px;
            box-shadow: 0 0 30px rgba(123, 47, 255, 0.1);
        }
        .panel-title {
            font-size: 1.2em;
            color: var(--text-color);
            margin-bottom: 10px;
            text-transform: uppercase;
            letter-spacing: 2px;
            text-align: center;
            text-shadow: 0 0 10px rgba(0, 229, 255, 0.3);
        }
        .bottom-row {
            width: 100%;
        }
        @media (max-width: 600px) {
            .stat-box {
                padding: 10px;
            }
            .stat-value {
                font-size: 1.2em;
            }
            .panel-title {
                font-size: 1em;
            }
            #detectionStatus {
                font-size: 1.4em;
            }
        }
    </style>
</head>
<body>
    <div class="dashboard">
        <div class="top-row">
            <div class="radar-section">
                <div class="visualization-container radar-container">
                    <canvas id="ultrasonicCanvas"></canvas>
                    <canvas id="tofCanvas"></canvas>
                </div>
            </div>
            <div class="stats-section">
                <div class="visualization-container">
                    <div class="stats-panel">
                        <div class="stat-box">
                            <div class="stat-label">Current Angle</div>
                            <div id="angleValue" class="stat-value angle">0°</div>
                        </div>
                        <div class="stat-box">
                            <div class="stat-label">Range (Ultrasonic)</div>
                            <div id="rangeValue" class="stat-value range">0 cm</div>
                        </div>
                        <div class="stat-box">
                            <div class="stat-label">Distance (ToF)</div>
                            <div id="tofDistance" class="stat-value tof">0 mm</div>
                        </div>
                        <div class="stat-box">
                            <div class="stat-label">Packet Loss</div>
                            <div id="packetLoss" class="stat-value packetLoss">0%</div>
                        </div>
                        <div class="stat-box">
                            <div class="stat-label">Detection Status</div>
                            <div id="detectionStatus" class="stat-value">NO OBJECT</div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
        <div class="bottom-row">
            <div class="visualization-container">
                <div class="panel-title">Distance History</div>
                <canvas id="historyGraph" class="history-graph"></canvas>
            </div>
        </div>
    </div>
    <script>
        const ws = new WebSocket('ws://' + window.location.hostname + ':81/');
        const ultrasonicCanvas = document.getElementById('ultrasonicCanvas');
        const tofCanvas = document.getElementById('tofCanvas');
        const ctxUltrasonic = ultrasonicCanvas.getContext('2d');
        const ctxToF = tofCanvas.getContext('2d');
        const historyCanvas = document.getElementById('historyGraph');
        const historyCtx = historyCanvas.getContext('2d');
        const history = [];
        const rangeHistory = [];
        const MAX_HISTORY_POINTS = 50;
        let totalPackets = 0;
        let lostPackets = 0;
        let lastAngle = -1;
        function resizeRadarCanvases() {
            const parentWidth = ultrasonicCanvas.parentElement.clientWidth;
            const parentHeight = ultrasonicCanvas.parentElement.clientHeight;
            ultrasonicCanvas.width = parentWidth;
            ultrasonicCanvas.height = parentHeight;
            tofCanvas.width = parentWidth;
            tofCanvas.height = parentHeight;
        }
        function resizeHistoryCanvas() {
            historyCanvas.width = historyCanvas.offsetWidth;
            historyCanvas.height = historyCanvas.offsetHeight;
        }
        window.addEventListener('resize', () => {
            resizeRadarCanvases();
            resizeHistoryCanvas();
        });
        resizeRadarCanvases();
        resizeHistoryCanvas();
        let centerX_Ultrasonic, centerY_Ultrasonic;
        let centerX_ToF, centerY_ToF;
        function updateCenters() {
            centerX_Ultrasonic = ultrasonicCanvas.width / 2;
            centerY_Ultrasonic = ultrasonicCanvas.height / 2;
            centerX_ToF = tofCanvas.width / 2;
            centerY_ToF = tofCanvas.height / 2;
        }
        updateCenters();
        function drawRadar(angleUltrasonic, distUltrasonic, distToF) {
            updateCenters();
            drawRadarGrid(ctxUltrasonic, centerX_Ultrasonic, centerY_Ultrasonic);
            fadeCanvas(ctxUltrasonic);
            if (distUltrasonic < 200 && distUltrasonic > 0) {
                drawDetection(
                    ctxUltrasonic,
                    centerX_Ultrasonic,
                    centerY_Ultrasonic,
                    angleUltrasonic,
                    distUltrasonic,
                    'rgba(0, 229, 255, 0.8)'
                );
            }
            drawRadarGrid(ctxToF, centerX_ToF, centerY_ToF);
            fadeCanvas(ctxToF);
            const tofCm = distToF / 10;
            if (tofCm < 200 && tofCm > 0) {
                drawDetection(
                    ctxToF,
                    centerX_ToF,
                    centerY_ToF,
                    90,
                    tofCm,
                    'rgba(255, 107, 107, 0.8)'
                );
            }
        }
        function drawRadarGrid(ctx, cx, cy) {
            ctx.clearRect(0, 0, ctx.canvas.width, ctx.canvas.height);
            const radius = Math.min(ctx.canvas.width, ctx.canvas.height) / 2;
            ctx.strokeStyle = 'rgba(0, 229, 255, 0.3)';
            for (let r = 1; r <= 4; r++) {
                ctx.beginPath();
                ctx.arc(cx, cy, (r * radius) / 5, Math.PI, 0);
                ctx.stroke();
                ctx.fillStyle = 'rgba(0, 229, 255, 0.6)';
                ctx.font = '14px Arial';
                ctx.fillText(`${r * 50}cm`, cx + 5, cy - ((r * radius) / 5) + 15);
            }
            for (let angle = 0; angle <= 180; angle += 30) {
                const rad = angle * Math.PI / 180;
                ctx.beginPath();
                ctx.moveTo(cx, cy);
                ctx.lineTo(cx + Math.cos(rad) * radius, cy - Math.sin(rad) * radius);
                ctx.stroke();
                ctx.fillText(
                    angle + '\u00B0',
                    cx + Math.cos(rad) * (radius * 0.85),
                    cy - Math.sin(rad) * (radius * 0.85)
                );
            }
        }
        function fadeCanvas(ctx) {
            ctx.fillStyle = 'rgba(0,0,0,0.05)';
            ctx.fillRect(0, 0, ctx.canvas.width, ctx.canvas.height);
        }
        function drawDetection(ctx, cx, cy, angleDeg, distanceCm, color) {
            const radius = Math.min(ctx.canvas.width, ctx.canvas.height) / 2;
            const maxRange = 200;
            let distPx = (distanceCm / maxRange) * radius;
            if (distPx > radius) distPx = radius;
            const rad = (angleDeg * Math.PI) / 180;
            const x2 = cx + Math.cos(rad) * distPx;
            const y2 = cy - Math.sin(rad) * distPx;
            ctx.strokeStyle = color;
            ctx.lineWidth = 3;
            ctx.beginPath();
            ctx.moveTo(cx, cy);
            ctx.lineTo(x2, y2);
            ctx.stroke();
            ctx.fillStyle = color;
            ctx.beginPath();
            ctx.arc(x2, y2, 5, 0, 2 * Math.PI);
            ctx.fill();
        }
        function updateHistory(angle, ultrasonicRange) {
            history.push({ angle, range: ultrasonicRange });
            if (history.length > MAX_HISTORY_POINTS) history.shift();
            rangeHistory.push(ultrasonicRange);
            if (rangeHistory.length > MAX_HISTORY_POINTS) rangeHistory.shift();
        }
        function drawHistoryGraph() {
            historyCtx.clearRect(0, 0, historyCanvas.width, historyCanvas.height);
            historyCtx.strokeStyle = 'rgba(0, 229, 255, 0.1)';
            for (let y = 0; y < historyCanvas.height; y += 25) {
                historyCtx.beginPath();
                historyCtx.moveTo(0, y);
                historyCtx.lineTo(historyCanvas.width, y);
                historyCtx.stroke();
            }
            historyCtx.beginPath();
            historyCtx.strokeStyle = 'rgba(0, 229, 255, 0.8)';
            rangeHistory.forEach((val, i) => {
                const x = (i / (rangeHistory.length - 1)) * historyCanvas.width;
                const y = historyCanvas.height - (val / 200) * historyCanvas.height;
                if (i === 0) {
                    historyCtx.moveTo(x, y);
                } else {
                    historyCtx.lineTo(x, y);
                }
            });
            historyCtx.stroke();
        }
        function updateStatistics(angle, range, tofDist) {
            document.getElementById('angleValue').textContent = angle + '\u00B0';
            document.getElementById('rangeValue').textContent = range + ' cm';
            document.getElementById('tofDistance').textContent = tofDist + ' mm';
            const statusBox = document.getElementById('detectionStatus').parentElement;
            statusBox.classList.remove('status-very-close', 'status-near', 'status-detected');
            const statusLabel = document.getElementById('detectionStatus');
            const effDist = Math.min(range, tofDist / 10);
            if (effDist < 50) {
                statusLabel.textContent = 'VERY CLOSE';
                statusBox.classList.add('status-very-close');
            } else if (effDist < 100) {
                statusLabel.textContent = 'NEAR';
                statusBox.classList.add('status-near');
            } else if (effDist < 200) {
                statusLabel.textContent = 'DETECTED';
                statusBox.classList.add('status-detected');
            } else {
                statusLabel.textContent = 'NO OBJECT';
            }
        }
        ws.onmessage = function(event) {
            const data = JSON.parse(event.data);
            const servoAngle = data.angle || 0;
            const usRange = data.range || 0;
            const tofDistance = data.tofDistance || 0;
            totalPackets++;
            if (lastAngle >= 0) {
                const diff = Math.abs(servoAngle - lastAngle);
                if (diff > 2) {
                    const missed = Math.round(diff / 2 - 1);
                    if (missed > 0) {
                        lostPackets += missed;
                    }
                }
            }
            lastAngle = servoAngle;
            const lossPercent = (lostPackets / totalPackets) * 100;
            document.getElementById('packetLoss').textContent = lossPercent.toFixed(1) + '%';
            drawRadar(servoAngle, usRange, tofDistance);
            updateStatistics(servoAngle, usRange, tofDistance);
            updateHistory(servoAngle, usRange);
            drawHistoryGraph();
        };
    </script>
</body>
</html>
)=====";
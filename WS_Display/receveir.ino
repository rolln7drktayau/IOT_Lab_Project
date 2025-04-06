
#include <RadioLib.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
#include <SPI.h>

const char *ssid = "RCT GS22 U";
const char *password = "xymp4417";

#define LORA_SCK 9
#define LORA_MISO 11
#define LORA_MOSI 10
#define LORA_SS 8
#define LORA_RST 12
#define LORA_DIO0 14
#define LORA_BUSY 13
#define LORA_FREQUENCY 868.0

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 18, 17, 21);

WebServer server(80);
SX1262 radio = new Module(LORA_SS, LORA_DIO0, LORA_RST, LORA_BUSY);

String lastRawData = "{}";
volatile bool rxFlag = false;
String ipAddress = "";

void setRxFlag() { rxFlag = true; }

void displayData()
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);

    u8g2.drawStr(0, 10, "LoRa Receiver");
    u8g2.drawStr(0, 25, ("IP: " + ipAddress).c_str());

    if (lastRawData != "{}")
    {
        u8g2.drawStr(0, 40, "Last Data:");
        u8g2.drawStr(0, 55, lastRawData.substring(0, 18).c_str());
    }
    else
    {
        u8g2.drawStr(0, 45, "En attente...");
    }
    u8g2.sendBuffer();
}

void setup()
{
    Serial.begin(115200);
    u8g2.begin();

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    ipAddress = WiFi.localIP().toString();
    Serial.println("Connected to WiFi, IP: " + ipAddress);

    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
    if (radio.begin(LORA_FREQUENCY) != RADIOLIB_ERR_NONE)
    {
        Serial.println("LoRa init failed!");
        while (true)
            ;
    }
    radio.setDio1Action(setRxFlag);
    radio.startReceive();

    server.on("/", HTTP_GET, []()
              { server.send(200, "text/html", R"rawliteral(
       <!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>TMG39931 - Monitoring LoRa</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css">
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        :root {
            --primary-color: #3498db;
            --secondary-color: #2ecc71;
            --text-color: #333;
            --bg-color: #f8f9fa;
            --card-bg: #ffffff;
            --shadow-color: rgba(0, 0, 0, 0.1);
            --border-color: #e0e0e0;
            --hover-color: #f1f1f1;
        }

        .dark-mode {
            --primary-color: #2980b9;
            --secondary-color: #27ae60;
            --text-color: #e0e0e0;
            --bg-color: #121212;
            --card-bg: #1e1e1e;
            --shadow-color: rgba(0, 0, 0, 0.5);
            --border-color: #333;
            --hover-color: #2a2a2a;
        }

        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            transition: background-color 0.3s, color 0.3s;
        }

        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: var(--bg-color);
            color: var(--text-color);
            line-height: 1.6;
        }

        header {
            background: linear-gradient(135deg, var(--primary-color), var(--secondary-color));
            color: white;
            padding: 1.5rem;
            text-align: center;
            box-shadow: 0 4px 12px var(--shadow-color);
            position: relative;
        }

        h1 {
            font-size: 2rem;
            margin-bottom: 0.5rem;
        }

        .subtitle {
            font-weight: 300;
            opacity: 0.9;
        }

        .container {
            max-width: 1200px;
            margin: 2rem auto;
            padding: 0 1rem;
        }

        .control-panel {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 2rem;
            padding: 1rem;
            background-color: var(--card-bg);
            border-radius: 10px;
            box-shadow: 0 4px 12px var(--shadow-color);
        }

        .toggle-container {
            display: flex;
            align-items: center;
            gap: 1rem;
        }

        .toggle-switch {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 30px;
        }

        .toggle-switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }

        .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            border-radius: 34px;
            transition: .4s;
        }

        .slider:before {
            position: absolute;
            content: "";
            height: 22px;
            width: 22px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            border-radius: 50%;
            transition: .4s;
        }

        input:checked + .slider {
            background-color: var(--primary-color);
        }

        input:checked + .slider:before {
            transform: translateX(30px);
        }

        .refresh-rate {
            display: flex;
            align-items: center;
            gap: 0.5rem;
        }

        select {
            padding: 0.5rem;
            border-radius: 5px;
            border: 1px solid var(--border-color);
            background-color: var(--card-bg);
            color: var(--text-color);
        }

        .dashboard {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
            gap: 1.5rem;
            margin-bottom: 2rem;
        }

        .card {
            background-color: var(--card-bg);
            border-radius: 10px;
            padding: 1.5rem;
            box-shadow: 0 4px 12px var(--shadow-color);
            transition: transform 0.3s;
            position: relative;
            overflow: hidden;
        }

        .card:hover {
            transform: translateY(-5px);
        }

        .card-icon {
            font-size: 2rem;
            margin-bottom: 1rem;
            display: inline-block;
        }

        .color-data .card-icon {
            color: #e74c3c;
        }

        .light-data .card-icon {
            color: #f39c12;
        }

        .proximity-data .card-icon {
            color: #9b59b6;
        }

        .card-title {
            font-size: 1.1rem;
            margin-bottom: 0.5rem;
            font-weight: 600;
        }

        .card-value {
            font-size: 2.5rem;
            font-weight: 700;
            margin-bottom: 0.5rem;
        }

        .card-unit {
            font-size: 0.9rem;
            opacity: 0.7;
        }

        .history-chart {
            background-color: var(--card-bg);
            border-radius: 10px;
            padding: 1.5rem;
            box-shadow: 0 4px 12px var(--shadow-color);
            margin-bottom: 2rem;
        }

        .chart-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 1rem;
        }

        .chart-title {
            font-size: 1.2rem;
            font-weight: 600;
        }

        .chart-controls {
            display: flex;
            gap: 1rem;
        }

        .chart-btn {
            background-color: transparent;
            border: 1px solid var(--border-color);
            border-radius: 20px;
            padding: 0.3rem 1rem;
            cursor: pointer;
            color: var(--text-color);
        }

        .chart-btn.active {
            background-color: var(--primary-color);
            color: white;
            border-color: var(--primary-color);
        }

        .chart-container {
            height: 300px;
            position: relative;
        }

        .raw-data {
            background-color: var(--card-bg);
            border-radius: 10px;
            padding: 1.5rem;
            box-shadow: 0 4px 12px var(--shadow-color);
        }

        .raw-data-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 1rem;
        }

        .raw-data-title {
            font-size: 1.2rem;
            font-weight: 600;
        }

        .raw-data-content {
            background-color: var(--hover-color);
            padding: 1rem;
            border-radius: 5px;
            font-family: 'Courier New', monospace;
            white-space: pre-wrap;
            max-height: 200px;
            overflow-y: auto;
        }

        .status {
            position: absolute;
            top: 1rem;
            right: 1rem;
            padding: 0.3rem 0.8rem;
            border-radius: 20px;
            font-size: 0.8rem;
            font-weight: 600;
            background-color: #2ecc71;
            color: white;
        }

        .offline {
            background-color: #e74c3c;
        }

        .blink {
            animation: blink 1s infinite;
        }

        @keyframes blink {
            0% {opacity: 1;}
            50% {opacity: 0.5;}
            100% {opacity: 1;}
        }

        .status-indicator {
            display: flex;
            align-items: center;
            gap: 0.5rem;
        }

        .indicator {
            width: 10px;
            height: 10px;
            border-radius: 50%;
            background-color: #2ecc71;
        }

        .connection-info {
            font-size: 0.9rem;
            opacity: 0.8;
        }

        footer {
            background-color: var(--card-bg);
            padding: 1.5rem;
            text-align: center;
            margin-top: 2rem;
            border-top: 1px solid var(--border-color);
        }

        @media (max-width: 768px) {
            .control-panel {
                flex-direction: column;
                gap: 1rem;
                align-items: flex-start;
            }
            
            .dashboard {
                grid-template-columns: 1fr;
            }
        }
    </style>
</head>
<body>
    <header>
        <h1>TMG39931 Monitoring</h1>
        <p class="subtitle">Capteur de couleur, lumière ambiante et proximité</p>
        <div class="status" id="connection-status">Connecté</div>
    </header>

    <div class="container">
        <div class="control-panel">
            <div class="toggle-container">
                <span><i class="fas fa-sun"></i></span>
                <label class="toggle-switch">
                    <input type="checkbox" id="theme-toggle">
                    <span class="slider"></span>
                </label>
                <span><i class="fas fa-moon"></i></span>
            </div>
            <div class="status-indicator">
                <div class="indicator" id="status-dot"></div>
                <span class="connection-info" id="connection-info">Dernière mise à jour: il y a quelques secondes</span>
            </div>
            <div class="refresh-rate">
                <label for="refresh-select">Taux de rafraîchissement:</label>
                <select id="refresh-select">
                    <option value="1000">1 seconde</option>
                    <option value="2000" selected>2 secondes</option>
                    <option value="5000">5 secondes</option>
                    <option value="10000">10 secondes</option>
                </select>
            </div>
        </div>

        <div class="dashboard">
            <div class="card color-data">
                <i class="fas fa-palette card-icon"></i>
                <h3 class="card-title">Couleur Rouge</h3>
                <div class="card-value" id="red-value">-</div>
                <div class="card-unit">Intensité</div>
            </div>
            <div class="card color-data">
                <i class="fas fa-palette card-icon"></i>
                <h3 class="card-title">Couleur Verte</h3>
                <div class="card-value" id="green-value">-</div>
                <div class="card-unit">Intensité</div>
            </div>
            <div class="card color-data">
                <i class="fas fa-palette card-icon"></i>
                <h3 class="card-title">Couleur Bleue</h3>
                <div class="card-value" id="blue-value">-</div>
                <div class="card-unit">Intensité</div>
            </div>
            <div class="card light-data">
                <i class="fas fa-lightbulb card-icon"></i>
                <h3 class="card-title">Lumière Ambiante</h3>
                <div class="card-value" id="clear-value">-</div>
                <div class="card-unit">Lux</div>
            </div>
            <div class="card proximity-data">
                <i class="fas fa-hand-paper card-icon"></i>
                <h3 class="card-title">Proximité</h3>
                <div class="card-value" id="proximity-value">-</div>
                <div class="card-unit">Distance relative</div>
            </div>
            <div class="card">
                <i class="fas fa-clock card-icon"></i>
                <h3 class="card-title">Dernière Transmission</h3>
                <div class="card-value" id="timestamp">-</div>
                <div class="card-unit">hh:mm:ss</div>
            </div>
        </div>

        <div class="history-chart">
            <div class="chart-header">
                <h3 class="chart-title">Historique des données</h3>
                <div class="chart-controls">
                    <button class="chart-btn active" data-chart="color">Couleurs</button>
                    <button class="chart-btn" data-chart="light">Lumière</button>
                    <button class="chart-btn" data-chart="proximity">Proximité</button>
                </div>
            </div>
            <div class="chart-container">
                <canvas id="history-chart"></canvas>
            </div>
        </div>

        <div class="raw-data">
            <div class="raw-data-header">
                <h3 class="raw-data-title">Données brutes</h3>
            </div>
            <div class="raw-data-content" id="raw-data">En attente de données...</div>
        </div>
    </div>

    <footer>
        <p>TMG39931 Monitoring Dashboard | Développé pour LoRa</p>
    </footer>

    <script>
        // Gestion du thème
        const themeToggle = document.getElementById('theme-toggle');
        
        // Vérifier les préférences sauvegardées
        if (localStorage.getItem('darkMode') === 'true') {
            document.body.classList.add('dark-mode');
            themeToggle.checked = true;
        }
        
        themeToggle.addEventListener('change', () => {
            if (themeToggle.checked) {
                document.body.classList.add('dark-mode');
                localStorage.setItem('darkMode', 'true');
            } else {
                document.body.classList.remove('dark-mode');
                localStorage.setItem('darkMode', 'false');
            }
            
            // Mettre à jour le graphique lorsque le thème change
            if (chartInstance) {
                chartInstance.update();
            }
        });
        
        // Gestion du taux de rafraîchissement
        const refreshSelect = document.getElementById('refresh-select');
        let refreshInterval = parseInt(refreshSelect.value);
        let refreshTimer;
        
        refreshSelect.addEventListener('change', () => {
            refreshInterval = parseInt(refreshSelect.value);
            clearInterval(refreshTimer);
            startRefreshTimer();
        });
        
        // Fonction pour formater l'heure
        function formatTime(date) {
            return date.toLocaleTimeString();
        }
        
        // Gestion de l'état de connexion
        let lastUpdateTime = new Date();
        const connectionStatus = document.getElementById('connection-status');
        const statusDot = document.getElementById('status-dot');
        const connectionInfo = document.getElementById('connection-info');
        
        function updateConnectionStatus() {
            const now = new Date();
            const timeDiff = Math.floor((now - lastUpdateTime) / 1000);
            
            if (timeDiff < 10) {
                connectionStatus.textContent = 'Connecté';
                connectionStatus.classList.remove('offline');
                statusDot.style.backgroundColor = '#2ecc71';
                connectionInfo.textContent = 'Dernière mise à jour: il y a ' + timeDiff + ' secondes';
            } else {
                connectionStatus.textContent = 'Déconnecté';
                connectionStatus.classList.add('offline');
                statusDot.style.backgroundColor = '#e74c3c';
                connectionInfo.textContent = 'Dernière mise à jour: il y a ' + timeDiff + ' secondes';
            }
        }
        
        setInterval(updateConnectionStatus, 1000);
        
        // Simulation de données historiques pour le graphique (sera remplacé par des données réelles)
        const chartButtons = document.querySelectorAll('.chart-btn');
        
        // Stockage pour l'historique des données
        let dataHistory = {
            timestamps: [],
            red: [],
            green: [],
            blue: [],
            clear: [],
            proximity: []
        };
        
        // Conserver un historique limité
        const MAX_HISTORY_POINTS = 30;
        
        // Instance du graphique
        let chartInstance = null;
        
        // Initialisation du graphique
        function initChart() {
            const ctx = document.getElementById('history-chart').getContext('2d');
            
            const chartConfig = {
                type: 'line',
                data: {
                    labels: dataHistory.timestamps,
                    datasets: []
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    scales: {
                        y: {
                            beginAtZero: true,
                            grid: {
                                color: function(context) {
                                    if (document.body.classList.contains('dark-mode')) {
                                        return context.tick.value === 0 ? 
                                            'rgba(255,255,255,0.25)' : 
                                            'rgba(255,255,255,0.05)';
                                    } else {
                                        return context.tick.value === 0 ? 
                                            'rgba(0,0,0,0.25)' : 
                                            'rgba(0,0,0,0.05)';
                                    }
                                }
                            },
                            ticks: {
                                color: getComputedStyle(document.body).getPropertyValue('--text-color')
                            }
                        },
                        x: {
                            grid: {
                                display: false
                            },
                            ticks: {
                                color: getComputedStyle(document.body).getPropertyValue('--text-color')
                            }
                        }
                    },
                    plugins: {
                        legend: {
                            labels: {
                                color: getComputedStyle(document.body).getPropertyValue('--text-color')
                            }
                        },
                        tooltip: {
                            mode: 'index',
                            intersect: false
                        }
                    },
                    interaction: {
                        mode: 'nearest',
                        axis: 'x',
                        intersect: false
                    }
                }
            };

            chartInstance = new Chart(ctx, chartConfig);
        }
        
        // Mise à jour du graphique
        function updateChart(type) {
            if (!chartInstance) return;

            // Supprimer les anciens datasets
            chartInstance.data.datasets = [];

            const colors = {
                red: '#e74c3c',
                green: '#2ecc71',
                blue: '#3498db',
                clear: '#f39c12',
                proximity: '#9b59b6'
            };

            switch(type) {
                case 'color':
                    chartInstance.data.datasets.push({
                        label: 'Rouge',
                        data: dataHistory.red,
                        borderColor: colors.red,
                        backgroundColor: colors.red + '20',
                        tension: 0.4,
                        fill: false,
                        borderWidth: 2
                    }, {
                        label: 'Vert',
                        data: dataHistory.green,
                        borderColor: colors.green,
                        backgroundColor: colors.green + '20',
                        tension: 0.4,
                        fill: false,
                        borderWidth: 2
                    }, {
                        label: 'Bleu',
                        data: dataHistory.blue,
                        borderColor: colors.blue,
                        backgroundColor: colors.blue + '20',
                        tension: 0.4,
                        fill: false,
                        borderWidth: 2
                    });
                    break;

                case 'light':
                    chartInstance.data.datasets.push({
                        label: 'Lumière',
                        data: dataHistory.clear,
                        borderColor: colors.clear,
                        backgroundColor: colors.clear + '20',
                        tension: 0.4,
                        fill: false,
                        borderWidth: 2
                    });
                    break;

                case 'proximity':
                    chartInstance.data.datasets.push({
                        label: 'Proximité',
                        data: dataHistory.proximity,
                        borderColor: colors.proximity,
                        backgroundColor: colors.proximity + '20',
                        tension: 0.4,
                        fill: false,
                        borderWidth: 2
                    });
                    break;
            }

            chartInstance.data.labels = dataHistory.timestamps;
            chartInstance.update();
        }
        
        // Gestion des boutons du graphique
        chartButtons.forEach(button => {
            button.addEventListener('click', () => {
                const chartType = button.dataset.chart;
                chartButtons.forEach(btn => btn.classList.remove('active'));
                button.classList.add('active');
                updateChart(chartType);
            });
        });
        
        // Fonction pour mettre à jour les données
        function updateData() {
            fetch('/data')
                .then(response => {
                    if (!response.ok) {
                        throw new Error('Erreur réseau');
                    }
                    return response.text();
                })
                .then(rawText => {
                    try {
                        // Essayer de parser le JSON
                        const data = JSON.parse(rawText);
                        
                        // Mise à jour de l'heure de la dernière connexion
                        lastUpdateTime = new Date();
                        
                        // Afficher les données brutes sans formatage pour débugger
                        document.getElementById('raw-data').textContent = rawText;
                        
                        // Mise à jour des valeurs - nous recherchons à la fois les noms courts et longs
                        // pour les propriétés du capteur
                        if (data.r !== undefined) {
                            document.getElementById('red-value').textContent = data.r;
                        } else if (data.red !== undefined) {
                            document.getElementById('red-value').textContent = data.red;
                        }
                        
                        if (data.g !== undefined) {
                            document.getElementById('green-value').textContent = data.g;
                        } else if (data.green !== undefined) {
                            document.getElementById('green-value').textContent = data.green;
                        }
                        
                        if (data.b !== undefined) {
                            document.getElementById('blue-value').textContent = data.b;
                        } else if (data.blue !== undefined) {
                            document.getElementById('blue-value').textContent = data.blue;
                        }
                        
                        if (data.c !== undefined) {
                            document.getElementById('clear-value').textContent = data.c;
                        } else if (data.clear !== undefined) {
                            document.getElementById('clear-value').textContent = data.clear;
                        }
                        
                        if (data.p !== undefined) {
                            document.getElementById('proximity-value').textContent = data.p;
                        } else if (data.proximity !== undefined) {
                            document.getElementById('proximity-value').textContent = data.proximity;
                        }
                        
                        // Parcourir toutes les propriétés et les afficher si elles ne sont pas encore prises en compte
                        for (const key in data) {
                            const elementId = key + '-value';
                            const element = document.getElementById(elementId);
                            if (element) {
                                element.textContent = data[key];
                            }
                        }
                        
                        document.getElementById('timestamp').textContent = formatTime(lastUpdateTime);
                        
                        // Ajouter à l'historique
                        dataHistory.timestamps.push(formatTime(lastUpdateTime));
                        dataHistory.red.push(data.r || data.red || 0);
                        dataHistory.green.push(data.g || data.green || 0);
                        dataHistory.blue.push(data.b || data.blue || 0);
                        dataHistory.clear.push(data.c || data.clear || 0);
                        dataHistory.proximity.push(data.p || data.proximity || 0);
                        
                        // Limiter la taille de l'historique
                        if (dataHistory.timestamps.length > MAX_HISTORY_POINTS) {
                            dataHistory.timestamps.shift();
                            dataHistory.red.shift();
                            dataHistory.green.shift();
                            dataHistory.blue.shift();
                            dataHistory.clear.shift();
                            dataHistory.proximity.shift();
                        }
                        
                        // Mettre à jour le graphique
                        if (chartInstance) {
                            const activeChart = document.querySelector('.chart-btn.active').dataset.chart;
                            updateChart(activeChart);
                        }
                    } catch (error) {
                        console.error('Erreur de parsing JSON:', error);
                        document.getElementById('raw-data').textContent = 'Erreur de parsing: ' + rawText;
                    }
                })
                .catch(error => {
                    console.error('Erreur lors de la récupération des données:', error);
                });
        }
        
        // Démarrer le timer de rafraîchissement
        function startRefreshTimer() {
            refreshTimer = setInterval(updateData, refreshInterval);
        }
        
        // Initialisation
        initChart();
        updateChart('color');
        updateData();
        startRefreshTimer();
    </script>
</body>
</html>
        )rawliteral"); });

    server.on("/data", HTTP_GET, []()
              { server.send(200, "application/json", lastRawData); });

    server.begin();
    Serial.println("Server started");
    displayData();
}

void loop()
{
    if (rxFlag)
    {
        rxFlag = false;
        String receivedData;
        if (radio.readData(receivedData) == RADIOLIB_ERR_NONE)
        {
            Serial.println("Paquet LoRa reçu : " + receivedData);
            lastRawData = receivedData;
        }
        else
        {
            Serial.println("Erreur réception LoRa");
        }
        radio.startReceive();
        displayData();
    }
    server.handleClient();
}

// Initialize panel
async function initPanel() {
    await loadConfig();
    await loadPinnedApps();
    await updateStatus();
    setInterval(updateStatus, 1000); // Update status every second
    setupSettings();
}

// Load configuration
async function loadConfig() {
    try {
        const response = await fetch('backend://get_config');
        const config = await response.json();
        document.documentElement.style.setProperty('--panel-bg', `rgba(30, 30, 30, ${config.settings.opacity || 0.9})`);
        document.getElementById('theme-stylesheet').href = `themes/${config.settings.theme || 'dark'}.css`;
        
        // Toggle icon visibility
        document.getElementById('wifi').classList.toggle('hidden', !config.settings.show_wifi);
        document.getElementById('bluetooth').classList.toggle('hidden', !config.settings.show_bt);
        document.getElementById('battery').classList.toggle('hidden', !config.settings.show_battery);
        document.getElementById('volume').classList.toggle('hidden', !config.settings.show_volume);
        document.getElementById('clock').classList.toggle('hidden', !config.settings.show_clock);
    } catch (error) {
        console.error('Error loading config:', error);
    }
}

// Update system status
async function updateStatus() {
    try {
        const response = await fetch('backend://get_status');
        const status = await response.json();
        
        // WiFi
        const wifi = document.getElementById('wifi');
        if (status.wifi.connected) {
            wifi.innerHTML = `<img src="icons/wifi.svg" alt="WiFi"> ${status.wifi.ssid} (${status.wifi.signal}%)`;
        } else {
            wifi.innerHTML = `<img src="icons/wifi-off.svg" alt="WiFi Off">`;
        }

        // Bluetooth
        document.getElementById('bluetooth').innerHTML = status.bluetooth.enabled
            ? `<img src="icons/bluetooth.svg" alt="Bluetooth">`
            : `<img src="icons/bluetooth-off.svg" alt="Bluetooth Off">`;

        // Battery
        document.getElementById('battery').innerHTML = status.battery.percentage > 0
            ? `<img src="icons/${status.battery.charging ? 'battery-charging' : 'battery'}.svg" alt="Battery"> ${status.battery.percentage}%`
            : `<img src="icons/battery.svg" alt="Battery"> N/A`;

        // Volume
        document.getElementById('volume').innerHTML = status.volume.level > 0
            ? `<img src="icons/volume.svg" alt="Volume"> ${status.volume.level}%`
            : `<img src="icons/volume-muted.svg" alt="Volume Muted">`;

        // Clock
        document.getElementById('clock').innerHTML = `<img src="icons/clock.svg" alt="Clock"> ${new Date(status.clock).toLocaleTimeString()}`;
    } catch (error) {
        console.error('Error updating status:', error);
    }
}

// Load pinned apps
async function loadPinnedApps() {
    try {
        const response = await fetch('backend://get_pinned_apps');
        const apps = await response.json();
        const launcher = document.getElementById('app-launcher');
        launcher.innerHTML = '';

        apps.forEach(app => {
            const icon = document.createElement('img');
            icon.src = `icons/app-icons/${app.icon}`;
            icon.className = 'app-icon';
            icon.title = app.name;
            icon.onclick = () => window.external.invoke(`launch_app:${app.exec}`);
            launcher.appendChild(icon);
        });

        // Enable drag-and-drop
        new Sortable(launcher, {
            animation: 150,
            ghostClass: 'dragging',
            onEnd: async (evt) => {
                const newOrder = Array.from(launcher.children).map(icon => ({
                    name: icon.title,
                    icon: icon.src.split('/').pop(),
                    exec: icon.onclick.toString().match(/launch_app:(\w+)/)[1]
                }));
                await fetch('backend://set_pinned_apps', {
                    method: 'POST',
                    body: JSON.stringify(newOrder)
                });
            }
        });
    } catch (error) {
        console.error('Error loading pinned apps:', error);
    }
}

// Setup settings panel
function setupSettings() {
    const settingsBtn = document.getElementById('settings-btn');
    const settingsPanel = document.getElementById('settings-panel');

    settingsBtn.onclick = async () => {
        if (settingsPanel.classList.contains('visible')) {
            settingsPanel.classList.remove('visible');
        } else {
            // Load settings.html dynamically
            const response = await fetch('settings.html');
            settingsPanel.innerHTML = await response.text();
            settingsPanel.classList.add('visible');
            initSettings();
        }
    };
}

// Initialize
initPanel();

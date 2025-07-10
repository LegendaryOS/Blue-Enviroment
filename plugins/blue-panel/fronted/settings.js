async function initSettings() {
    const config = await (await fetch('backend://get_config')).json();
    
    // Populate settings
    document.getElementById('theme-select').value = config.settings.theme || 'dark';
    document.getElementById('opacity-slider').value = config.settings.opacity || 0.9;
    document.getElementById('accent-color').value = config.settings.accent_color || '#6200ea';
    document.getElementById('show-wifi').checked = config.settings.show_wifi;
    document.getElementById('show-bt').checked = config.settings.show_bt;
    document.getElementById('show-battery').checked = config.settings.show_battery;
    document.getElementById('show-volume').checked = config.settings.show_volume;
    document.getElementById('show-clock').checked = config.settings.show_clock;

    // Populate app selector
    const apps = await (await fetch('backend://get_apps')).json();
    const appSelector = document.getElementById('app-selector');
    const pinnedApps = await (await fetch('backend://get_pinned_apps')).json();
    const pinnedExecs = pinnedApps.map(app => app.exec);

    apps.forEach(app => {
        const option = document.createElement('div');
        option.className = 'app-option';
        option.innerHTML = `<img src="icons/app-icons/${app.icon}" alt="${app.name}"> ${app.name}`;
        option.onclick = () => {
            if (pinnedExecs.includes(app.exec)) {
                pinnedApps.splice(pinnedExecs.indexOf(app.exec), 1);
                pinnedExecs.splice(pinnedExecs.indexOf(app.exec), 1);
                option.style.opacity = '0.5';
            } else {
                pinnedApps.push(app);
                pinnedExecs.push(app.exec);
                option.style.opacity = '1';
            }
        };
        option.style.opacity = pinnedExecs.includes(app.exec) ? '1' : '0.5';
        appSelector.appendChild(option);
    });

    // Save settings
    document.getElementById('save-settings').onclick = async () => {
        const newConfig = {
            pinned_apps: pinnedApps,
            settings: {
                theme: document.getElementById('theme-select').value,
                opacity: parseFloat(document.getElementById('opacity-slider').value),
                accent_color: document.getElementById('accent-color').value,
                show_wifi: document.getElementById('show-wifi').checked,
                show_bt: document.getElementById('show-bt').checked,
                show_battery: document.getElementById('show-battery').checked,
                show_volume: document.getElementById('show-volume').checked,
                show_clock: document.getElementById('show-clock').checked
            }
        };
        await fetch('backend://set_pinned_apps', {
            method: 'POST',
            body: JSON.stringify(newConfig.pinned_apps)
        });
        // Assume backend updates config.json with new settings
        await fetch('backend://set_config', {
            method: 'POST',
            body: JSON.stringify(newConfig)
        });
        location.reload(); // Reload to apply changes
    };

    // Close settings
    document.getElementById('close-settings').onclick = () => {
        document.getElementById('settings-panel').classList.remove('visible');
    };
}

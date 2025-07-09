import express from 'express';
import fs from 'fs/promises';
import path from 'path';
import ini from 'ini';

const app = express();
const port = 3000;
const historyFile = path.join(process.env.HOME, '.config', 'blue_launcher_history.json');

app.use(express.json());
app.use('/usr/share/icons', express.static('/usr/share/icons'));

// Funkcja parsowania plików .desktop
async function parseDesktopFiles() {
    const apps = [];
    const desktopPath = '/usr/share/applications';
    const iconSizes = ['64', '48', '32'];

    try {
        const files = await fs.readdir(desktopPath);
        for (const file of files) {
            if (!file.endsWith('.desktop')) continue;

            const filePath = path.join(desktopPath, file);
            const content = await fs.readFile(filePath, 'utf-8');
            const parsed = ini.parse(content);
            const entry = parsed['Desktop Entry'];

            if (!entry || !entry.Name || !entry.Exec || entry.NoDisplay === 'true') continue;

            let iconPath = '';
            if (entry.Icon) {
                for (const size of iconSizes) {
                    for (const ext of ['.png', '.svg']) {
                        const tryPath = `/usr/share/icons/breeze/apps/${size}/${entry.Icon}${ext}`;
                        if (await fs.access(tryPath).then(() => true).catch(() => false)) {
                            iconPath = `http://localhost:${port}${tryPath}`;
                            break;
                        }
                    }
                    if (iconPath) break;
                }
                if (!iconPath) iconPath = 'https://via.placeholder.com/72';
            }

            apps.push({
                name: entry.Name,
                icon: iconPath,
                exec: entry.Exec,
                is_system: true,
                launch_count: 0,
                is_favorite: false
            });
        }
    } catch (error) {
        console.error('Error reading desktop files:', error);
    }

    // Wczytaj historię
    try {
        const historyData = await fs.readFile(historyFile, 'utf-8');
        const history = JSON.parse(historyData);
        for (const app of apps) {
            if (history[app.name]) {
                app.launch_count = history[app.name].count || 0;
                app.is_favorite = history[app.name].favorite || false;
            }
        }
    } catch (error) {
        console.error('Error reading history:', error);
    }

    return apps.sort((a, b) => a.name.localeCompare(b.name));
}

// Funkcja zapisu historii
async function saveHistory(apps) {
    const history = {};
    for (const app of apps) {
        if (app.launch_count > 0 || app.is_favorite) {
            history[app.name] = {
                count: app.launch_count,
                favorite: app.is_favorite
            };
        }
    }
    try {
        await fs.writeFile(historyFile, JSON.stringify(history, null, 4));
    } catch (error) {
        console.error('Error saving history:', error);
    }
}

// Endpoint do pobierania aplikacji
app.get('/apps', async (req, res) => {
    const apps = await parseDesktopFiles();
    res.json(apps);
});

// Endpoint do uruchomienia aplikacji
app.post('/launch/:name', async (req, res) => {
    const apps = await parseDesktopFiles();
    const app = apps.find(a => a.name === decodeURIComponent(req.params.name));
    if (app) {
        app.launch_count++;
        await saveHistory(apps);
        // Symulacja uruchomienia (można dodać child_process.spawn)
        console.log(`Launching: ${app.exec}`);
    }
    res.sendStatus(200);
});

// Endpoint do przełączania ulubionych
app.post('/toggle-favorite/:name', async (req, res) => {
    const apps = await parseDesktopFiles();
    const app = apps.find(a => a.name === decodeURIComponent(req.params.name));
    if (app) {
        app.is_favorite = !app.is_favorite;
        await saveHistory(apps);
    }
    res.sendStatus(200);
});

app.listen(port, () => {
    console.log(`BlueLauncher backend running on http://localhost:${port}`);
});

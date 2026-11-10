import { readFileSync } from 'fs';
import { fileURLToPath } from 'url';
import path from 'path';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const cfgPath = path.join(__dirname, '..', 'config.json');

let config;
try {
  config = JSON.parse(readFileSync(cfgPath, 'utf8'));
} catch (e) {
  console.error(`\n[Quantix Bot] Could not read config.json at:\n  ${cfgPath}`);
  console.error('  Copy config.example.json to config.json and fill in your values.\n');
  process.exit(1);
}

if (!config.token || config.token === 'YOUR_DISCORD_BOT_TOKEN') {
  console.error('\n[Quantix Bot] Please set your bot token in bot/config.json before starting.\n');
  process.exit(1);
}

export default config;

// ── Import/Export tab: the whole setup (cfg.json + the scripts) as one zip ────────────────────────
// Export: /hal/cfg.json and every file of /scripts (list.txt too) as a zip, made in the browser (zip.js, as the
// Files tab). The fields that the firmware schema marks NoExport (passwords, api keys, ...) are left out of the
// cfg, so an export can be shared without its secrets. export.json in the zip says what was left out (names only).
//   zip: hal/cfg.json, scripts/<files>, export.json
// Import: a zip like that (also a zip of the Files tab with hal/ and scripts/ in it, or a cfg.json at the top).
// The NoExport fields that the imported cfg does not have are taken from the config on the device (the item with
// the same uid path), so importing a shared config keeps the own passwords. Then, in this order:
//   1. a backup of the current cfg.json and scripts is kept in the browser
//   2. the scripts are written to /scripts (other files there stay, list.txt decides which are active)
//   3. the cfg: written as cfg.new.json, the device checks and loads it (hal/config/reload), on success cfg.json
//   4. the device loads the active scripts again (hal/scripts/reload)
// When step 3 or 4 fails everything is put back from the backup (the scripts, and the old cfg is loaded again),
// so the device is never left with a config and scripts that do not fit together.

(() => {
  const FS = 'littlefs';
  const CFG_PATH = '/hal/cfg.json';
  const CFG_TEST = 'cfg.new.json';
  const SCRIPTS_DIR = '/scripts';
  const FLAG = 'NoExport';

  const fileUrl = (path) => `/fs/file?fs=${FS}&path=${encodeURIComponent(path)}`;
  const isObj = (v) => v !== null && typeof v === 'object' && !Array.isArray(v);
  const names = (list) => list.map((x) => `${x.path}.${x.key}`).join(', ');
  const fmtSize = (n) => n < 1024 ? `${n} B` : `${(n / 1024).toFixed(1)} kB`;

  let pending = null; // the chosen import: { name, cfg, cfgText, scripts: [{ name, data }], filled, missing }
  let busy = false;

  async function http(method, url, body) {
    const r = await fetch(url, { method, body, cache: 'no-store' });
    const text = await r.text();
    if (!r.ok) throw new Error(`${r.status} ${text}`);
    return text;
  }
  const put = (path, data) => http('PUT', fileUrl(path), new Blob([data], { type: 'application/octet-stream' }));

  function msg(text, kind) {
    const m = el('xp-msg');
    m.textContent = text;
    m.className = kind || '';
  }

  /** a line in the step log of the import */
  function step(text, kind) {
    el('xp-steps').append(newEl('div', { cls: kind || '', text }));
  }

  async function waitForWs() {
    // ws is a global "let" of main.js: visible here, but not as window.ws
    for (let i = 0; i < 100 && !(typeof ws !== 'undefined' && ws && ws.readyState === WebSocket.OPEN); i++) await new Promise((r) => setTimeout(r, 100));
  }

  async function ensureZip() {
    if (!window.Zip) await loadScript('zip.js');
  }

  /** the cfg text on the device as it is, null when there is none */
  async function readCfgText() {
    try { return await http('GET', fileUrl(CFG_PATH)); } catch (e) {
      if (String(e.message).startsWith('404')) return null;
      throw e;
    }
  }

  /** the cfg on the device, null when there is none */
  async function readCfg() {
    const text = await readCfgText();
    if (text === null) return null;
    const parsed = JSON.parse(text);
    if (!Array.isArray(parsed)) throw new Error('cfg.json is not a JSON array');
    return parsed;
  }

  /** every file of /scripts: [{ name, data }] */
  async function readScripts() {
    let items;
    try { items = JSON.parse(await http('GET', `/fs/list?fs=${FS}&path=${encodeURIComponent(SCRIPTS_DIR)}`)); } catch (e) { return []; }
    const out = [];
    for (const it of items) {
      if (it.dir || it.name.endsWith('.tmp')) continue;
      const r = await fetch(fileUrl(`${SCRIPTS_DIR}/${it.name}`), { cache: 'no-store' });
      if (!r.ok) throw new Error(`${SCRIPTS_DIR}/${it.name}: ${r.status}`);
      out.push({ name: it.name, data: new Uint8Array(await r.arrayBuffer()) });
    }
    return out;
  }

  // ═══════════════════════════════════════════════════════════════
  // EXPORT
  // ═══════════════════════════════════════════════════════════════
  async function showExportInfo() {
    const info = el('xp-export-info');
    info.className = '';
    try {
      const cfg = await readCfg();
      if (!cfg) { info.textContent = `${CFG_PATH} does not exist on the device`; return; }
      if (el('xp-ex-complete').checked) {
        info.textContent = 'complete backup: the passwords and keys are in the zip too, keep it for yourself, do not share it';
        info.className = 'warn';
        return;
      }
      const { removed } = CfgSchema.strip(cfg, FLAG);
      const def = removed.filter((r) => r.defaulted);
      const out = removed.filter((r) => !r.defaulted);
      info.textContent = !removed.length ? `nothing in the config is marked ${FLAG}`
        : [out.length && `left out (${FLAG}): ${names(out)}`, def.length && `default value (${FLAG}, required): ${names(def)}`].filter(Boolean).join('; ');
    } catch (e) { info.textContent = `could not read the config: ${e.message}`; }
  }

  /** the export zip: { blob, files, info }; complete: the NoExport fields too (a backup, not to be shared) */
  async function buildExport(withCfg, withScripts, complete = false) {
    await ensureZip();
    const files = [];
    const info = { format: 'dalhal-export', version: 1, date: new Date().toISOString(), complete, left_out: [] };
    if (withCfg) {
      msg('reading the config…');
      if (complete) {
        const text = await readCfgText();
        if (text !== null) files.push({ name: 'hal/cfg.json', data: new TextEncoder().encode(text) }); // as it is
      } else {
        const cfg = await readCfg();
        if (cfg) {
          const { cfg: out, removed } = CfgSchema.strip(cfg, FLAG);
          files.push({ name: 'hal/cfg.json', data: new TextEncoder().encode(CfgSchema.format(out)) });
          info.left_out = removed; // where and which field (defaulted: has the default value), never a value
        }
      }
    }
    if (withScripts) {
      msg('reading the scripts…');
      for (const s of await readScripts()) files.push({ name: `scripts/${s.name}`, data: s.data });
    }
    files.push({ name: 'export.json', data: new TextEncoder().encode(JSON.stringify(info, null, 2) + '\n') });
    return { blob: await Zip.create(files), files, info };
  }

  async function exportZip() {
    if (busy) return;
    busy = true;
    try {
      const withCfg = el('xp-ex-cfg').checked;
      const withScripts = el('xp-ex-scripts').checked;
      const complete = el('xp-ex-complete').checked;
      if (!withCfg && !withScripts) { msg('nothing chosen to export', 'err'); return; }
      const { blob, files, info } = await buildExport(withCfg, withScripts, complete);
      const zipName = `dalhal-${complete ? 'backup' : 'export'}-${info.date.slice(0, 10)}.zip`;
      const href = URL.createObjectURL(blob);
      downloadUrl(href, zipName);
      setTimeout(() => URL.revokeObjectURL(href), 10000);
      msg(`${zipName}: ${files.length - 1} files, ${fmtSize(blob.size)}` + (complete ? ', complete (with the secrets, do not share it)' : info.left_out.length ? `, without ${names(info.left_out)}` : ''), 'ok');
    } catch (e) {
      msg(`export failed: ${e.message}`, 'err');
    } finally { busy = false; }
  }

  // ═══════════════════════════════════════════════════════════════
  // IMPORT
  // ═══════════════════════════════════════════════════════════════
  /** a script file name from the zip, null when it is not one (folders below scripts/, odd names) */
  function scriptName(entry) {
    const m = /^(?:[^/]*\/)?scripts\/([^/]+)$/.exec(entry.replace(/\\/g, '/').replace(/^\/+/, ''));
    if (!m || m[1] === '..' || m[1] === '.' || m[1].endsWith('.tmp')) return null;
    return m[1];
  }
  const isCfgEntry = (entry) => /^(?:[^/]*\/)?(?:hal\/)?cfg\.json$/.test(entry.replace(/\\/g, '/').replace(/^\/+/, ''));

  async function chooseZip(file) {
    pending = null;
    el('xp-steps').innerHTML = '';
    el('xp-import-btn').disabled = true;
    el('xp-preview').innerHTML = '';
    try {
      await ensureZip();
      const buffer = await file.arrayBuffer();
      const entries = Zip.list(buffer).filter((e) => !e.dir);
      const cfgEntries = entries.filter((e) => isCfgEntry(e.name));
      // more than one cfg.json (e.g. a zip of the whole fs with a backup folder): take the shortest path, hal/cfg.json first
      cfgEntries.sort((a, b) => (b.name.endsWith('hal/cfg.json') - a.name.endsWith('hal/cfg.json')) || a.name.length - b.name.length);
      const p = { name: file.name, cfg: null, cfgText: null, complete: false, scripts: [], filled: [], missing: [] };
      // export.json: which NoExport fields got their default value (they are replaced by the values on the device)
      let info = null;
      const infoEntry = entries.find((e) => /(^|\/)export\.json$/.test(e.name));
      if (infoEntry) try { info = JSON.parse(new TextDecoder().decode(await Zip.extract(buffer, infoEntry))); } catch (e) { info = null; }
      if (cfgEntries.length) {
        const text = new TextDecoder().decode(await Zip.extract(buffer, cfgEntries[0]));
        const parsed = JSON.parse(text);
        if (!Array.isArray(parsed)) throw new Error(`${cfgEntries[0].name} is not a JSON array`);
        p.cfg = parsed;
        p.complete = !!(info && info.complete);
        if (p.complete) {
          p.cfgText = text; // a complete backup: as it is
        } else {
          const current = await readCfg();
          const defaulted = info && Array.isArray(info.left_out) ? info.left_out.filter((r) => r.defaulted) : [];
          Object.assign(p, CfgSchema.fillFrom(p.cfg, current, FLAG, defaulted)); // the own secrets are kept
          p.cfgText = CfgSchema.format(p.cfg);
        }
      }
      for (const e of entries) {
        const n = scriptName(e.name);
        if (n) p.scripts.push({ name: n, data: await Zip.extract(buffer, e) });
      }
      if (!p.cfg && !p.scripts.length) throw new Error('no hal/cfg.json and no scripts/ files in the zip');
      pending = p;
      renderPreview();
      el('xp-import-btn').disabled = false;
      msg(`${file.name}: ready to import, check what it contains below`);
    } catch (e) {
      msg(`${file.name}: ${e.message}`, 'err');
    }
  }

  function renderPreview() {
    const p = pending;
    const box = el('xp-preview');
    box.innerHTML = '';
    const row = (label, ...content) => box.append(newEl('div', { cls: 'xp-row' }, newEl('span', { cls: 'xp-label', text: label }), newEl('span', {}, ...content)));
    const chk = (id, text, on, disabled) => {
      const c = newEl('input', { type: 'checkbox', id });
      c.checked = on; c.disabled = disabled;
      return newEl('label', {}, c, ' ' + text);
    };
    row('import', chk('xp-im-cfg', 'config', !!p.cfg, !p.cfg), '   ', chk('xp-im-scripts', 'scripts', !!p.scripts.length, !p.scripts.length));
    if (p.cfg) {
      row('config', `${p.cfg.filter(isObj).length} root devices` + (p.complete ? ', a complete backup (with its own passwords and keys)' : ''));
      if (p.filled.length) row('kept', `from the config on the device: ${names(p.filled)}`);
      if (p.missing.length) box.append(newEl('div', { cls: 'xp-row warn', text: `not on this device, the default value or nothing is used: ${names(p.missing)}, set them in the Config tab afterwards` }));
    } else row('config', newEl('i', { text: 'none in the zip, the config on the device stays' }));
    if (p.scripts.length) {
      const list = p.scripts.find((s) => s.name === 'list.txt');
      const active = list ? ScriptTools.listNames(new TextDecoder().decode(list.data)) : null;
      row('scripts', p.scripts.map((s) => s.name + (active && active.includes(s.name) ? ' ✓' : '')).join(', ') + (active ? '   (✓ = active in list.txt)' : ''));
      if (!list) box.append(newEl('div', { cls: 'xp-row warn', text: 'no list.txt in the zip: the active scripts on the device stay as they are' }));
    } else row('scripts', newEl('i', { text: 'none in the zip, the scripts on the device stay' }));
  }

  /** the device checks the cfg text and loads it when it is valid (then the scripts too), the running one stays when not */
  async function reloadCfg(text) {
    await http('POST', `/fs/mkdir?fs=${FS}&path=%2Fhal`).catch(() => {}); // there already: fine
    await put(`/hal/${CFG_TEST}`, text);
    return JSON.parse(await wsRequest(`hal/config/reload/${CFG_TEST}`, 'reloadJSON', 30000));
  }

  /** puts the scripts back as they were: the old files written again, the new ones deleted */
  async function restoreScripts(old, written) {
    for (const s of old) await put(`${SCRIPTS_DIR}/${s.name}`, s.data);
    for (const n of written) if (!old.some((s) => s.name === n)) await http('DELETE', fileUrl(`${SCRIPTS_DIR}/${n}`)).catch(() => {});
  }

  async function importToDevice() {
    if (busy || !pending) return;
    const p = pending;
    const doCfg = !!p.cfg && el('xp-im-cfg').checked;
    const doScripts = !!p.scripts.length && el('xp-im-scripts').checked;
    if (!doCfg && !doScripts) { msg('nothing chosen to import', 'err'); return; }
    const what = [doCfg && 'the config', doScripts && `${p.scripts.length} script files`].filter(Boolean).join(' and ');
    if (!confirm(`Import ${what} from ${p.name} to the device?\n\nWhen the device does not accept it, everything is put back as it was.`)) return;
    busy = true;
    el('xp-import-btn').disabled = true;
    el('xp-steps').innerHTML = '';
    let oldCfg = null, oldScripts = [], written = [], cfgChanged = false;
    try {
      step('backup of the current config and scripts (in the browser)…');
      oldCfg = await readCfgText(); // as it is, put back byte for byte
      oldScripts = await readScripts();

      if (doScripts) {
        await http('POST', `/fs/mkdir?fs=${FS}&path=${encodeURIComponent(SCRIPTS_DIR)}`).catch(() => {}); // there already: fine
        for (const s of p.scripts) {
          await put(`${SCRIPTS_DIR}/${s.name}`, s.data);
          written.push(s.name);
        }
        step(`written to ${SCRIPTS_DIR}: ${written.join(', ')}`);
      }

      if (doCfg) {
        step('the device checks and loads the new config…');
        const answer = await reloadCfg(p.cfgText);
        if (answer.info !== 'OK') throw new Error('the device did not accept the config (see the Console log)');
        await put(CFG_PATH, p.cfgText);
        cfgChanged = true;
        step(`the device runs the new config, saved to ${CFG_PATH}`, 'ok');
      }

      step('the device loads the active scripts…');
      const s = JSON.parse(await wsRequest('hal/scripts/reload', 'hal/scripts/reload', 30000));
      if (s.error) throw new Error('the scripts are not valid with this config (see the Console log, or check them in the Scripts tab)');
      step('the scripts run', 'ok');
      await http('DELETE', `/fs/file?fs=${FS}&path=${encodeURIComponent('/hal/' + CFG_TEST)}`).catch(() => {});
      msg(`imported ${p.name}`, 'ok');
      pending = null;
      el('xp-preview').innerHTML = '';
    } catch (e) {
      step(e.message, 'err');
      try {
        step('putting everything back…');
        if (written.length) await restoreScripts(oldScripts, written);
        if (cfgChanged && oldCfg) {
          const answer = await reloadCfg(oldCfg); // loads the scripts again too
          await put(CFG_PATH, oldCfg);
          step(answer.info === 'OK' ? 'the old config runs again' : 'the old config could not be loaded again, see the Console log', answer.info === 'OK' ? 'ok' : 'err');
        } else if (written.length) {
          await wsRequest('hal/scripts/reload', 'hal/scripts/reload', 30000);
        }
        await http('DELETE', `/fs/file?fs=${FS}&path=${encodeURIComponent('/hal/' + CFG_TEST)}`).catch(() => {});
        step('the device is as before the import', 'ok');
        msg(`import of ${p.name} failed, nothing changed`, 'err');
      } catch (e2) {
        step(`putting back failed: ${e2.message}`, 'err');
        msg(`import of ${p.name} failed and could not be undone completely, check the Config and Scripts tabs`, 'err');
      }
      el('xp-import-btn').disabled = false;
    } finally { busy = false; }
  }

  // ═══════════════════════════════════════════════════════════════
  function wire() {
    el('xp-export-btn').onclick = exportZip;
    el('xp-ex-complete').onchange = showExportInfo;
    el('xp-choose').onclick = () => el('xp-file').click();
    el('xp-file').onchange = (e) => { const f = e.target.files[0]; e.target.value = ''; if (f) chooseZip(f); };
    el('xp-import-btn').onclick = importToDevice;
    const drop = el('xp-drop');
    drop.addEventListener('dragover', (e) => { e.preventDefault(); drop.classList.add('over'); });
    drop.addEventListener('dragleave', () => drop.classList.remove('over'));
    drop.addEventListener('drop', (e) => { e.preventDefault(); drop.classList.remove('over'); const f = e.dataTransfer.files[0]; if (f) chooseZip(f); });
  }

  async function init() {
    wire();
    msg('loading the schema from the firmware…');
    try {
      await waitForWs();
      await CfgSchema.load();
      msg('');
    } catch (e) {
      msg(`could not get the schema from the device, export/import cannot know the ${FLAG} fields: ${e.message}`, 'err');
      el('xp-export-btn').disabled = true;
      el('xp-choose').disabled = true;
      return;
    }
    showExportInfo();
  }

  window.TransferTab = {
    onShow: () => { if (CfgSchema.raw) showExportInfo(); },
    // for tests
    _build: buildExport,
    _choose: chooseZip,
    _pending: () => pending,
    _import: importToDevice,
  };
  init();
})();

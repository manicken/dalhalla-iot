// ── Files tab, FSBrowser style ───────────────────────────────────────────────
// Folder tree on the left (one root per mounted filesystem), editor (ACE) or preview on the right.
// The filesystems over HTTP, the same routes on every firmware (Arduino and ESP-IDF):
//   GET /fs/mounts   GET /fs/list?fs=&path=   GET/PUT/DELETE /fs/file?fs=&path=
//   POST /fs/mkdir?fs=&path=   POST /fs/rename?fs=&from=&to=
// A location in this file is "fs:/path", e.g. "littlefs:/cfg.json" or "sd:/logs/a.txt".
// A .zip is unpacked here in the browser (zip.js) and its files are sent one by one with PUT /fs/file;
// the other way, a folder is read file by file and packed into a .zip in the browser (download / backup).
// The editor (ACE or a textarea) comes from codeEditor.js.

(() => {
  const q = encodeURIComponent;

  const DEFAULT_FS = 'littlefs';
  const FS_LABELS = { littlefs: 'flash (littlefs)', sd: 'SD card' };
  const IMAGE_EXT = ['png', 'jpg', 'jpeg', 'gif', 'bmp', 'ico', 'webp', 'svg'];
  // extension -> ACE mode; files with other extensions are opened as text when they are valid UTF-8
  const MODES = {
    json: 'json', js: 'javascript', mjs: 'javascript', htm: 'html', html: 'html', css: 'css', xml: 'xml',
    svg: 'svg', md: 'markdown', ini: 'ini', cfg: 'ini', yaml: 'yaml', yml: 'yaml', csv: 'text', txt: 'text',
    log: 'text', py: 'python', sh: 'sh', c: 'c_cpp', h: 'c_cpp', cpp: 'c_cpp', hal: 'text',
  };
  const BINARY_EXT = ['bin', 'gz', 'pdf', 'mp3', 'wav', 'ttf', 'woff', 'woff2', 'dump'];
  const MAX_EDIT_BYTES = 512 * 1024;   // bigger files are shown as hex/download only
  const MAX_HEX_BYTES = 16 * 1024;     // hex view shows the start of a file

  const expanded = new Set([DEFAULT_FS + ':/']);
  let curDir = DEFAULT_FS + ':/'; // folder for upload / new file / new folder
  let open = null;         // { path: location, kind: 'text'|'image'|'binary' }
  let dirty = false;
  let editor = null;       // { get, set(text, mode), resize, focus }

  // ── locations "fs:/path" ──
  function split(loc) {
    const i = loc.indexOf(':');
    return { fs: loc.slice(0, i), path: loc.slice(i + 1) };
  }
  const loc = (fs, path) => fs + ':' + path;
  const rootOf = (l) => loc(split(l).fs, '/');
  const join = (dir, name) => (dir.endsWith('/') ? dir : dir + '/') + name;
  function parentOf(l) {
    const { fs, path } = split(l);
    const i = path.lastIndexOf('/');
    return loc(fs, i <= 0 ? '/' : path.slice(0, i));
  }
  const baseName = (l) => l.slice(l.lastIndexOf('/') + 1);
  const extOf = (name) => (baseName(name).includes('.') ? name.split('.').pop().toLowerCase() : '');
  /** fs=..&path=.. for a location */
  const params = (l, key = 'path') => { const { fs, path } = split(l); return `fs=${q(fs)}&${key}=${q(path)}`; };
  const fileUrl = (l) => `/fs/file?${params(l)}`;
  /** a name typed by the user: absolute path (in the same filesystem) or a name in dir */
  const resolve = (dir, name) => (name.startsWith('/') ? loc(split(dir).fs, name) : join(dir, name));

  function fmtSize(n) {
    if (n < 1024) return n + ' B';
    if (n < 1024 * 1024) return (n / 1024).toFixed(1) + ' kB';
    return (n / 1024 / 1024).toFixed(2) + ' MB';
  }

  function msg(text, isErr = false) {
    const m = el('fsb-msg');
    m.textContent = text;
    m.className = isErr ? 'err' : '';
  }

  /** one request, the answer text, throws with the device's error text on a non 2xx status */
  async function api(method, url, body) {
    const r = await fetch(url, { method, body, cache: 'no-store' });
    const text = await r.text();
    if (!r.ok) throw new Error(`${r.status} ${text}`);
    return text;
  }

  /** body as application/octet-stream, so no server tries to parse it as a form */
  const asBinary = (data) => new Blob([data], { type: 'application/octet-stream' });

  function onEdit() {
    if (dirty || !open) return;
    dirty = true;
    updateFileBar();
  }

  // ── tree ──
  function makeRow(node, label, isDir, extra) {
    const row = newEl('div', { cls: 'fsb-row' },
      newEl('span', { cls: 'fsb-icon', text: isDir ? '▸' : '' }),
      newEl('span', { cls: 'fsb-name', text: label }),
      extra && newEl('span', { cls: 'fsb-size', text: extra }));
    node.append(row);
    return row;
  }

  function makeDirNode(l, label, extra) {
    const node = newEl('div', { cls: 'fsb-node dir' + (split(l).path === '/' ? ' root' : ''), 'data-path': l });
    const row = makeRow(node, label, true, extra);
    node.append(newEl('div', { cls: 'fsb-children', hidden: true }));
    row.onclick = () => { setCurDir(l); node.classList.contains('open') ? collapse(node) : expand(node); };
    row.oncontextmenu = (e) => { e.preventDefault(); setCurDir(l); dirMenu(e, l); };
    addDropTarget(row, l);
    return node;
  }

  function makeFileNode(l, size) {
    const node = newEl('div', { cls: 'fsb-node file', 'data-path': l, 'data-size': size });
    const row = makeRow(node, baseName(l), false, fmtSize(size));
    row.onclick = () => openFile(l, size);
    row.oncontextmenu = (e) => { e.preventDefault(); fileMenu(e, l, size); };
    return node;
  }

  async function expand(node) {
    const l = node.dataset.path;
    const kids = node.querySelector(':scope > .fsb-children');
    let items;
    try {
      const { fs, path } = split(l);
      items = JSON.parse(await api('GET', `/fs/list?fs=${q(fs)}&path=${q(path)}`));
    } catch (e) {
      msg(`could not list ${l}: ${e.message}`, true);
      return;
    }
    items.sort((a, b) => (a.dir !== b.dir ? (a.dir ? -1 : 1) : a.name.localeCompare(b.name)));
    kids.innerHTML = '';
    for (const it of items) {
      const p = join(l, it.name);
      if (it.dir) {
        const child = makeDirNode(p, it.name);
        kids.append(child);
        if (expanded.has(p)) await expand(child);
      } else {
        kids.append(makeFileNode(p, it.size));
      }
    }
    if (!items.length) {
      kids.append(newEl('div', { cls: 'fsb-empty', text: '(empty)' }));
    }
    kids.hidden = false;
    node.classList.add('open');
    node.querySelector(':scope > .fsb-row .fsb-icon').textContent = '▾';
    expanded.add(l);
    markSelection();
  }

  function collapse(node) {
    node.querySelector(':scope > .fsb-children').hidden = true;
    node.classList.remove('open');
    node.querySelector(':scope > .fsb-row .fsb-icon').textContent = '▸';
    expanded.delete(node.dataset.path);
  }

  /** the mounted filesystems from the device, [] on error */
  async function getMounts() {
    try {
      return JSON.parse(await api('GET', '/fs/mounts'));
    } catch (e) {
      msg(`could not get the filesystems: ${e.message}`, true);
      return [];
    }
  }

  function showUsage(mounts) {
    el('fsb-usage').textContent = mounts.map((m) => {
      const pct = m.total ? Math.round((100 * m.used) / m.total) : 0;
      return `${m.fs}: ${fmtSize(m.used)} of ${fmtSize(m.total)} (${pct}%)`;
    }).join('  ·  ');
  }

  async function refreshStatus() {
    showUsage(await getMounts());
  }

  /** one root per mounted filesystem, the expanded folders are opened again */
  async function reloadTree() {
    const mounts = await getMounts();
    showUsage(mounts);
    const tree = el('fsb-tree');
    tree.innerHTML = '';
    for (const m of mounts) {
      const root = makeDirNode(loc(m.fs, '/'), FS_LABELS[m.fs] || m.fs, fmtSize(m.total));
      tree.appendChild(root);
      if (expanded.has(loc(m.fs, '/'))) await expand(root);
    }
    // the current folder or the open file may be on a filesystem that is gone (SD card removed)
    if (!mounts.some((m) => m.fs === split(curDir).fs)) setCurDir(loc(DEFAULT_FS, '/'));
    markSelection();
  }

  function markSelection() {
    document.querySelectorAll('#fsb-tree .fsb-row').forEach((r) => {
      const p = r.parentElement.dataset.path;
      r.classList.toggle('open-file', !!open && p === open.path);
      r.classList.toggle('cur-dir', r.parentElement.classList.contains('dir') && p === curDir);
    });
  }

  function setCurDir(l) {
    curDir = l;
    el('fsb-folder').textContent = 'folder: ' + l;
    markSelection();
  }

  // ── open / preview ──
  async function confirmDiscard() {
    return !dirty || confirm(`${open.path} has unsaved changes, discard them?`);
  }

  function showPane(kind) {
    el('fsb-editor').hidden = kind !== 'text';
    el('fsb-preview').hidden = kind === 'text';
    if (kind === 'text' && editor) editor.resize();
  }

  function updateFileBar() {
    el('fsb-path').textContent = open ? open.path + (dirty ? ' •' : '') : 'select a file in the tree';
    el('fsb-save').disabled = !open || open.kind !== 'text' || !dirty;
    el('fsb-discard').disabled = !open || open.kind !== 'text' || !dirty;
    el('fsb-download').disabled = !open;
    el('fsb-format').hidden = !open || open.kind !== 'text' || extOf(open.path) !== 'json';
  }

  function setPreview(...nodes) {
    const box = el('fsb-preview');
    box.innerHTML = '';
    box.append(...nodes);
  }

  function hexDump(bytes) {
    let out = '';
    for (let i = 0; i < bytes.length; i += 16) {
      const row = bytes.subarray(i, i + 16);
      const hex = Array.from(row, (b) => b.toString(16).padStart(2, '0')).join(' ');
      const asc = Array.from(row, (b) => (b >= 32 && b < 127 ? String.fromCharCode(b) : '.')).join('');
      out += i.toString(16).padStart(8, '0') + '  ' + hex.padEnd(47) + '  ' + asc + '\n';
    }
    return out;
  }

  /** size of a file as listed in the tree, 0 when it is not in the tree */
  function sizeOf(l) {
    const node = [...document.querySelectorAll('#fsb-tree .fsb-node.file')].find((n) => n.dataset.path === l);
    return node ? Number(node.dataset.size) : 0;
  }

  async function openFile(l, size = sizeOf(l)) {
    if (!(await confirmDiscard())) return;
    const ext = extOf(l);
    setCurDir(parentOf(l));

    if (ext === 'zip') {
      openZip(l, size);
      return;
    }

    if (IMAGE_EXT.includes(ext)) {
      open = { path: l, kind: 'image' };
      dirty = false;
      const img = newEl('img', { src: fileUrl(l) + '&t=' + Date.now(), alt: l });
      const info = newEl('div', { cls: 'fsb-info', text: `${baseName(l)}, ${fmtSize(size)}` });
      img.onload = () => {
        // small images (icons) are scaled up by a whole factor so they can be seen
        const scale = Math.min(16, Math.max(1, Math.floor(256 / Math.max(img.naturalWidth, img.naturalHeight, 1))));
        img.style.width = img.naturalWidth * scale + 'px';
        info.textContent += `, ${img.naturalWidth} × ${img.naturalHeight} px` + (scale > 1 ? ` (shown ${scale}×)` : '');
      };
      setPreview(img, info);
      showPane('image');
      updateFileBar();
      markSelection();
      return;
    }

    msg(`loading ${l}…`);
    let bytes;
    try {
      const r = await fetch(fileUrl(l), { cache: 'no-store' });
      if (!r.ok) throw new Error(`${r.status} ${await r.text()}`);
      bytes = new Uint8Array(await r.arrayBuffer());
    } catch (e) {
      msg(`could not open ${l}: ${e.message}`, true);
      return;
    }
    msg('');

    // text when it is valid UTF-8 without NUL bytes and not a known binary type
    let text = null;
    if (!BINARY_EXT.includes(ext) && bytes.length <= MAX_EDIT_BYTES && !bytes.includes(0)) {
      try { text = new TextDecoder('utf-8', { fatal: true }).decode(bytes); } catch (e) { text = null; }
    }

    if (text !== null) {
      open = { path: l, kind: 'text' };
      dirty = false;
      editor.set(text, MODES[ext] || 'text');
      showPane('text');
      editor.focus();
    } else {
      open = { path: l, kind: 'binary' };
      dirty = false;
      setPreview(
        newEl('div', { cls: 'fsb-info', text: `${baseName(l)}, ${fmtSize(bytes.length)}, binary` +
          (bytes.length > MAX_HEX_BYTES ? ` (first ${fmtSize(MAX_HEX_BYTES)} shown)` : '') }),
        newEl('pre', { cls: 'fsb-hex', text: hexDump(bytes.subarray(0, MAX_HEX_BYTES)) }));
      showPane('binary');
    }
    updateFileBar();
    markSelection();
  }

  function closeFile() {
    open = null;
    dirty = false;
    setPreview();
    showPane('none');
    updateFileBar();
    markSelection();
  }

  async function save() {
    if (!open || open.kind !== 'text') return;
    const text = editor.get();
    if (extOf(open.path) === 'json') {
      try { JSON.parse(text); } catch (e) {
        if (!confirm(`This is not valid JSON:\n${e.message}\n\nSave anyway?`)) return;
      }
    }
    try {
      await api('PUT', fileUrl(open.path), asBinary(text));
      dirty = false;
      updateFileBar();
      msg(`saved ${open.path}`);
      updateSize(open.path, new Blob([text]).size);
      refreshStatus();
    } catch (e) { msg(`save failed: ${e.message}`, true); }
  }

  function updateSize(l, size) {
    const node = [...document.querySelectorAll('#fsb-tree .fsb-node.file')].find((n) => n.dataset.path === l);
    if (node) { node.dataset.size = size; node.querySelector('.fsb-size').textContent = fmtSize(size); }
  }

  async function discard() {
    if (!open || !confirm(`Discard the changes to ${open.path}?`)) return;
    dirty = false;
    openFile(open.path);
  }

  function formatJson() {
    try {
      editor.set(JSON.stringify(JSON.parse(editor.get()), null, 2), 'json');
      dirty = true;
      updateFileBar();
    } catch (e) { msg(`not valid JSON: ${e.message}`, true); }
  }

  const download = (l) => downloadUrl(fileUrl(l), baseName(l));

  // ── zip: unpacked in the browser, the files are sent one by one ──
  async function ensureZip() {
    if (!window.Zip) await loadScript('zip.js');
  }

  /** a name from the zip as a safe relative path, null when it must be skipped */
  function safeZipName(name) {
    const rel = name.replace(/\\/g, '/').replace(/^\/+/, '');
    if (!rel || rel.split('/').some((s) => s === '..')) return null;
    if (rel.startsWith('__MACOSX/') || baseName('/' + rel.replace(/\/$/, '')) === '.DS_Store') return null;
    return rel;
  }

  /** unpacks the zip in buffer into the folder dir; label is the zip name for the messages */
  async function unzipInto(buffer, dir, label) {
    let entries;
    try {
      await ensureZip();
      entries = Zip.list(buffer);
    } catch (e) { msg(`${label}: ${e.message}`, true); return; }

    const files = [];
    const folders = new Set(); // relative, every folder entry and every parent of a file
    for (const e of entries) {
      const rel = safeZipName(e.name);
      if (rel === null) continue;
      const parts = rel.replace(/\/$/, '').split('/');
      const depth = e.dir ? parts.length : parts.length - 1;
      for (let k = 1; k <= depth; k++) folders.add(parts.slice(0, k).join('/'));
      if (!e.dir) files.push({ entry: e, rel });
    }
    if (!files.length) { msg(`${label} contains no files`, true); return; }
    const total = files.reduce((n, f) => n + f.entry.size, 0);
    if (!confirm(`Unzip ${files.length} files (${fmtSize(total)}) from ${label} into ${dir}?`)) return;

    // what is already there, to ask once about overwriting
    msg('checking existing files…');
    const existing = new Set();
    for (const f of [dir, ...[...folders].map((d) => join(dir, d))]) {
      try {
        JSON.parse(await api('GET', `/fs/list?${params(f)}`)).forEach((x) => existing.add(join(f, x.name)));
      } catch (e) { /* the folder does not exist yet */ }
    }
    const clashes = files.filter((f) => existing.has(join(dir, f.rel))).length;
    const overwrite = clashes === 0 ||
      confirm(`${clashes} of the ${files.length} files already exist.\n\nOK = overwrite them\nCancel = keep them (they are skipped)`);

    // folders first, parents before children
    for (const d of [...folders].sort((a, b) => a.split('/').length - b.split('/').length)) {
      const l = join(dir, d);
      if (existing.has(l)) continue;
      try {
        await api('POST', `/fs/mkdir?${params(l)}`);
      } catch (e) {
        if (!e.message.startsWith('409')) { msg(`could not create ${l}: ${e.message}`, true); reloadTree(); return; }
      }
    }

    let done = 0, skipped = 0, failed = 0;
    for (const f of files) {
      const l = join(dir, f.rel);
      if (!overwrite && existing.has(l)) { skipped++; continue; }
      msg(`unzipping ${done + skipped + failed + 1}/${files.length}: ${f.rel}`);
      try {
        const data = await Zip.extract(buffer, f.entry);
        await api('PUT', fileUrl(l), asBinary(data));
        done++;
      } catch (e) {
        failed++;
        if (!confirm(`${f.rel} failed:\n${e.message}\n\nContinue with the rest?`)) break;
      }
    }
    msg(`unzipped ${done} files into ${dir}` + (skipped ? `, ${skipped} kept` : '') + (failed ? `, ${failed} failed` : ''), failed > 0);
    expanded.add(dir);
    await reloadTree();
  }

  /** a .zip from this computer into dir */
  function pickUnzip(dir) {
    const input = el('fsb-upload');
    input.accept = '.zip,application/zip';
    input.multiple = false;
    input.onchange = async () => {
      const f = input.files[0];
      input.value = '';
      input.accept = '';
      input.multiple = true;
      if (f) unzipInto(await f.arrayBuffer(), dir, f.name);
    };
    input.click();
  }

  /** a .zip on the device: the browser reads it from the device and unpacks it into dir */
  async function unzipFromDevice(l, dir = parentOf(l)) {
    msg(`reading ${l}…`);
    try {
      const r = await fetch(fileUrl(l), { cache: 'no-store' });
      if (!r.ok) throw new Error(`${r.status} ${await r.text()}`);
      await unzipInto(await r.arrayBuffer(), dir, baseName(l));
    } catch (e) { msg(`could not read ${l}: ${e.message}`, true); }
  }

  /** preview of a .zip on the device: its content and a button to unpack it */
  async function openZip(l, size) {
    open = { path: l, kind: 'binary' };
    dirty = false;
    const info = newEl('div', { cls: 'fsb-info', text: `${baseName(l)}, ${fmtSize(size)}, zip` });
    const btn = newEl('button', { cls: 'cmd-btn', text: `⇪ unzip into ${parentOf(l)}`, onclick: () => unzipFromDevice(l) });
    const table = newEl('table', { cls: 'fsb-zip' });
    setPreview(info, btn, table);
    showPane('binary');
    updateFileBar();
    markSelection();
    try {
      await ensureZip();
      const r = await fetch(fileUrl(l), { cache: 'no-store' });
      if (!r.ok) throw new Error(`${r.status} ${await r.text()}`);
      const entries = Zip.list(await r.arrayBuffer());
      const files = entries.filter((e) => !e.dir);
      info.textContent += `, ${files.length} files, ${fmtSize(files.reduce((n, e) => n + e.size, 0))} unpacked`;
      for (const e of entries) {
        const skipped = safeZipName(e.name) === null;
        table.append(newEl('tr', { cls: skipped && 'skipped' },
          newEl('td', { text: e.name }),
          newEl('td', { cls: 'num', text: skipped ? 'skipped' : e.dir ? '' : fmtSize(e.size) })));
      }
    } catch (e) {
      info.textContent += `: ${e.message}`;
      btn.disabled = true;
    }
  }

  /** all files and folders below dir as [{ name: relative path, data }] for Zip.create, folders end with / */
  async function collect(dir, rel = '', out = [], progress = { n: 0 }) {
    const items = JSON.parse(await api('GET', `/fs/list?${params(dir)}`));
    if (rel && !items.length) out.push({ name: rel + '/' }); // keep empty folders
    for (const it of items) {
      const l = join(dir, it.name);
      const name = rel ? rel + '/' + it.name : it.name;
      if (it.dir) {
        await collect(l, name, out, progress);
      } else {
        msg(`reading ${++progress.n}: ${l}`);
        const r = await fetch(fileUrl(l), { cache: 'no-store' });
        if (!r.ok) throw new Error(`${l}: ${r.status} ${await r.text()}`);
        out.push({ name, data: new Uint8Array(await r.arrayBuffer()) });
      }
    }
    return out;
  }

  /** the folder dir with everything below it as a zip Blob */
  async function buildZip(dir) {
    await ensureZip();
    const files = await collect(dir);
    msg(`packing ${files.length} entries…`);
    return { blob: await Zip.create(files), count: files.filter((f) => !f.name.endsWith('/')).length };
  }

  /** download the folder dir as <name>.zip, the root as dalhal-<fs>-<date>.zip */
  async function downloadZip(dir) {
    const { fs, path } = split(dir);
    const day = new Date().toISOString().slice(0, 10);
    const zipName = path === '/' ? `dalhal-${fs}-${day}.zip` : `${baseName(path)}-${day}.zip`;
    try {
      const { blob, count } = await buildZip(dir);
      const href = URL.createObjectURL(blob);
      downloadUrl(href, zipName);
      setTimeout(() => URL.revokeObjectURL(href), 10000);
      msg(`${zipName}: ${count} files, ${fmtSize(blob.size)}`);
    } catch (e) { msg(`zip of ${dir} failed: ${e.message}`, true); }
  }

  // ── operations ──
  async function upload(files, dir) {
    for (const f of files) {
      const l = join(dir, f.name);
      msg(`uploading ${f.name} (${fmtSize(f.size)}) to ${dir}…`);
      try {
        await api('PUT', fileUrl(l), asBinary(f));
        msg(`uploaded ${l}`);
      } catch (e) { msg(`upload of ${f.name} failed: ${e.message}`, true); break; }
    }
    expanded.add(dir);
    reloadTree();
  }

  async function newFile(dir) {
    const name = prompt(`New file in ${dir}:`);
    if (!name) return;
    const l = resolve(dir, name);
    try {
      const parent = parentOf(l);
      const items = JSON.parse(await api('GET', `/fs/list?${params(parent)}`));
      if (items.some((it) => it.name === baseName(l))) { msg(`${l} already exists`, true); return; }
      await api('PUT', fileUrl(l), asBinary(''));
      expanded.add(parent);
      await reloadTree();
      openFile(l, 0);
    } catch (e) { msg(`create failed: ${e.message}`, true); }
  }

  async function mkdir(dir) {
    const name = prompt(`New folder in ${dir}:`);
    if (!name) return;
    const l = resolve(dir, name);
    try {
      await api('POST', `/fs/mkdir?${params(l)}`);
      expanded.add(parentOf(l));
      msg(`created ${l}`);
    } catch (e) { msg(`mkdir failed: ${e.message}`, true); }
    reloadTree();
  }

  async function rename(l) {
    const { fs, path } = split(l);
    const to = prompt(`New name, or full path within ${fs}:`, path);
    if (!to || to === path) return;
    const target = resolve(parentOf(l), to);
    try {
      await api('POST', `/fs/rename?fs=${q(fs)}&from=${q(path)}&to=${q(split(target).path)}`);
      if (open && open.path === l) { open.path = target; updateFileBar(); }
      if (expanded.delete(l)) expanded.add(target);
      msg(`renamed to ${target}`);
    } catch (e) { msg(`rename failed: ${e.message}`, true); }
    reloadTree();
  }

  async function remove(l, isDir) {
    if (!confirm(`Delete ${l}${isDir ? ' (folder, must be empty)' : ''}?`)) return;
    try {
      await api('DELETE', fileUrl(l));
      if (open && (open.path === l || open.path.startsWith(l + '/'))) closeFile();
      if (curDir === l) setCurDir(parentOf(l));
      msg(`deleted ${l}`);
    } catch (e) { msg(`delete failed: ${e.message}`, true); }
    reloadTree();
  }

  function pickUpload(dir) {
    const input = el('fsb-upload');
    input.onchange = () => { upload([...input.files], dir); input.value = ''; };
    input.click();
  }

  // ── context menu ──
  function showMenu(e, title, items) {
    const menu = el('fsb-menu');
    menu.innerHTML = '';
    menu.append(newEl('div', { cls: 'fsb-menu-title', text: title }),
      ...items.map((it) => newEl('div', { cls: 'fsb-menu-item' + (it.danger ? ' danger' : ''), text: it.label,
        onclick: () => { menu.hidden = true; it.fn(); } })));
    menu.hidden = false;
    const r = menu.getBoundingClientRect();
    menu.style.left = Math.min(e.clientX, window.innerWidth - r.width - 4) + 'px';
    menu.style.top = Math.min(e.clientY, window.innerHeight - r.height - 4) + 'px';
  }

  function dirMenu(e, l) {
    const items = [
      { label: '⇪ upload here', fn: () => pickUpload(l) },
      { label: '⇪ unzip a .zip here', fn: () => pickUnzip(l) },
      { label: '⬇ download as .zip', fn: () => downloadZip(l) },
      { label: '+ new file', fn: () => newFile(l) },
      { label: '+ new folder', fn: () => mkdir(l) },
    ];
    if (split(l).path !== '/') {
      items.push({ label: '✎ rename / move', fn: () => rename(l) });
      items.push({ label: '🗑 delete', fn: () => remove(l, true), danger: true });
    }
    showMenu(e, l, items);
  }

  function fileMenu(e, l, size) {
    const zip = extOf(l) === 'zip' ? [{ label: `⇪ unzip into ${parentOf(l)}`, fn: () => unzipFromDevice(l) }] : [];
    showMenu(e, l, [
      { label: 'open', fn: () => openFile(l, size) },
      ...zip,
      { label: '⬇ download', fn: () => download(l) },
      { label: '✎ rename / move', fn: () => rename(l) },
      { label: '🗑 delete', fn: () => remove(l, false), danger: true },
    ]);
  }

  document.addEventListener('click', () => { el('fsb-menu').hidden = true; });
  document.addEventListener('keydown', (e) => { if (e.key === 'Escape') el('fsb-menu').hidden = true; });

  // ── drag and drop upload: on a folder row, or anywhere in the tree (current folder) ──
  function addDropTarget(row, l) {
    row.addEventListener('dragover', (e) => { e.preventDefault(); e.stopPropagation(); row.classList.add('drag'); });
    row.addEventListener('dragleave', () => row.classList.remove('drag'));
    row.addEventListener('drop', (e) => {
      e.preventDefault();
      e.stopPropagation();
      row.classList.remove('drag');
      if (e.dataTransfer.files.length) upload([...e.dataTransfer.files], l);
    });
  }
  const tree = el('fsb-tree');
  tree.addEventListener('dragover', (e) => e.preventDefault());
  tree.addEventListener('drop', (e) => {
    e.preventDefault();
    if (e.dataTransfer.files.length) upload([...e.dataTransfer.files], curDir);
  });

  // ── wiring ──
  el('fsb-upload-btn').onclick = () => pickUpload(curDir);
  el('fsb-unzip-btn').onclick = () => pickUnzip(curDir);
  el('fsb-zip-btn').onclick = () => downloadZip(curDir);
  el('fsb-newfile').onclick = () => newFile(curDir);
  el('fsb-mkdir').onclick = () => mkdir(curDir);
  el('fsb-refresh').onclick = () => reloadTree();
  el('fsb-save').onclick = save;
  el('fsb-discard').onclick = discard;
  el('fsb-download').onclick = () => open && download(open.path);
  el('fsb-format').onclick = formatJson;
  window.addEventListener('beforeunload', (e) => { if (dirty) { e.preventDefault(); e.returnValue = ''; } });
  window.addEventListener('resize', () => editor && editor.resize());

  /** #files=/cfg.json (littlefs) or #files=sd:/logs/a.txt opens that file (bookmarkable),
      its folders are expanded in the tree */
  async function openFromHash() {
    const m = location.hash.match(/^#files=(.+)$/);
    if (!m) return;
    let l = decodeURIComponent(m[1]);
    if (l.startsWith('/')) l = loc(DEFAULT_FS, l);
    let added = false;
    for (let p = parentOf(l); ; p = parentOf(p)) {
      if (!expanded.has(p)) { expanded.add(p); added = true; }
      if (p === rootOf(l)) break;
    }
    if (added) await reloadTree();
    openFile(l);
  }

  (async () => {
    setCurDir(curDir);
    showPane('none');
    updateFileBar();
    editor = await CodeEditor.create(el('fsb-editor'), { onChange: onEdit, onSave: save }); // codeEditor.js
    el('fsb-editor-kind').textContent = editor.kind;
    await reloadTree();
    openFromHash();
  })();

  // later visits of the tab: the tree may be stale (SD card inserted?), the editor needs its size again
  window.FilesTab = { buildZip, onShow: async () => { if (editor) editor.resize(); if (!dirty) { await reloadTree(); openFromHash(); } } };
})();

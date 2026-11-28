// ── Scripts tab: the script editor ───────────────────────────────────────────
// The scripts are the .txt files in /scripts on LittleFS, the active ones are listed in /scripts/list.txt.
// Autocomplete and the live checks come from the running device:
//   /hal/cfg.json                 the devices and their uid paths (temps:a:b)
//   hal/meta/reg/functions        per device type: the values (uid, uid#name) and functions (uid#name())
//   hal/meta/reg/events           per device type: the events for triggers (on uid@Event then)
// The device checks a script text directly in RAM (hal/scripts/check/<name>/<text>: only parses, nothing is
// written to the flash, the running scripts are not touched); a saved file with hal/scripts/validate/<file>.
// The errors come with line numbers and are marked in the editor.
// "reload scripts" loads all active scripts (hal/scripts/reload); the firmware stops the scripts when one is not valid.
// The rule: an active script is always valid. Saving an active script is refused when the device does not accept it
// (it checks the text first, the file is only written when it is valid), a script is only activated when it is
// valid. An inactive script is a draft: it is saved, its errors are shown. The Config tab checks the other way round
// (a device that an active script uses can not be removed or disabled).

(() => {
  const DIR = '/scripts';
  const LIST_FILE = 'list.txt';
  const KEYWORDS = ScriptTools.KEYWORDS; // scriptTools.js
  const TEMPLATE = 'on eachloop then\n  \nendon\n';

  let files = [];        // script file names in /scripts (without list.txt)
  let active = [];       // the active file names: list.txt without its comments, in their order
  let listText = '';     // list.txt as it is, changed with care (its comments stay)
  let open = null;       // name of the open file
  let dirty = false;
  let editor = null;
  const devices = new Map(); // uid path -> { type, functions: [{ text, kind, help }], events: [names] }

  const q = encodeURIComponent;
  const fileUrl = (name) => `/fs/file?fs=littlefs&path=${q(DIR + '/' + name)}`;

  function msg(text, isErr = false) {
    el('scr-msg').textContent = text;
    el('scr-msg').className = isErr ? 'err' : '';
  }

  async function http(method, url, body) {
    const r = await fetch(url, { method, body, cache: 'no-store' });
    const text = await r.text();
    if (!r.ok) throw new Error(`${r.status} ${text}`);
    return text;
  }
  const put = (name, text) => http('PUT', fileUrl(name), new Blob([text], { type: 'application/octet-stream' }));

  // ═══════════════════════════════════════════════════════════════
  // DEVICES: uid paths of the cfg + what the registry says about their type
  // ═══════════════════════════════════════════════════════════════
  /** registry tree (hal/meta/reg/...) -> the entry for a cfg item, matched by type (or the only one) */
  function regEntry(list, item) {
    if (!list || !list.length) return null;
    if (typeof item.type === 'string') return list.find((r) => r.name.toUpperCase() === item.type.toUpperCase()) || null;
    return list.length === 1 ? list[0] : null; // items without type (e.g. SSD1306 elements): the one sub type
  }

  /** what can be written after a uid: the named values and functions of the type */
  function functionsOf(fEntry) {
    const out = [];
    const f = (fEntry && fEntry.functions) || {};
    for (const [cat, kind] of [['readValue', 'read'], ['writeValue', 'write'], ['exec', 'exec']]) {
      for (const x of f[cat] || []) {
        if (!x.name) continue; // the primary one is the uid itself
        const text = '#' + x.name + (kind === 'exec' ? '()' : '');
        const known = out.find((o) => o.text === text);
        if (known) { known.kind += '/' + kind; continue; }
        out.push({ text, kind, help: x.help || '' });
      }
    }
    return out;
  }

  function primaryHelp(fEntry) {
    const f = (fEntry && fEntry.functions) || {};
    const parts = [];
    if ((f.readValue || []).some((x) => x.primary)) parts.push('read');
    if ((f.writeValue || []).some((x) => x.primary)) parts.push('write');
    return parts.join('/');
  }

  function indexDevices(cfg, regFunctions, regEvents) {
    devices.clear();
    const walk = (arr, fList, eList, prefix, parentDisabled) => {
      for (const item of arr || []) {
        if (!item || typeof item !== 'object' || Array.isArray(item) || typeof item.uid !== 'string') continue;
        const path = prefix ? prefix + ':' + item.uid : item.uid;
        const fe = regEntry(fList, item), ee = regEntry(eList, item);
        devices.set(path, {
          type: item.type || (fe && fe.name) || 'item',
          // the registry does not describe every item (e.g. the devices of a 1-wire group): then nothing is
          // known about its #names and @events, and they are not checked
          known: !!fe,
          disabled: parentDisabled || item.disabled === true,
          functions: functionsOf(fe),
          primary: primaryHelp(fe),
          events: (ee && ee.events) || [],
        });
        // child items: every array of objects in the item (items, units ...)
        for (const v of Object.values(item)) {
          if (Array.isArray(v) && v.some((x) => x && typeof x === 'object' && !Array.isArray(x))) {
            walk(v, fe && fe.subitems && fe.subitems.regitems, ee && ee.subitems && ee.subitems.regitems, path, parentDisabled || item.disabled === true);
          }
        }
      }
    };
    walk(cfg, regFunctions.regitems, regEvents.regitems, '', false);
  }

  async function loadDevices() {
    try {
      const cfg = JSON.parse(await http('GET', `/fs/file?fs=littlefs&path=${q('/hal/cfg.json')}`));
      const fn = JSON.parse(await wsRequest('hal/meta/reg/functions', 'registry', 20000));
      const ev = JSON.parse(await wsRequest('hal/meta/reg/events', 'registry', 20000));
      indexDevices(cfg, fn, ev);
      el('scr-devcount').textContent = `${devices.size} devices for autocomplete`;
    } catch (e) {
      el('scr-devcount').textContent = 'no autocomplete: ' + e.message;
    }
  }

  // ═══════════════════════════════════════════════════════════════
  // ACE: syntax highlighting, autocomplete, live checks
  // ═══════════════════════════════════════════════════════════════
  function defineMode() {
    ace.define('ace/mode/dalhal_script_highlight_rules', ['require', 'exports', 'ace/lib/oop', 'ace/mode/text_highlight_rules'], (require, exports) => {
      const oop = require('ace/lib/oop');
      const { TextHighlightRules } = require('ace/mode/text_highlight_rules');
      function Rules() {
        this.$rules = {
          start: [
            { token: 'comment', regex: '//.*$' },
            { token: 'comment', regex: '/\\*', next: 'comment' },
            { token: 'keyword', regex: '\\b(?:on|then|do|endon|if|elseif|else|endif|eachloop)\\b', caseInsensitive: true },
            { token: 'keyword.operator', regex: '\\b(?:and|or)\\b', caseInsensitive: true },
            { token: 'constant.numeric', regex: '0[xX][0-9a-fA-F]+\\b' },
            { token: 'constant.numeric', regex: '\\d+(?:\\.\\d+)?\\b' },
            { token: 'entity.name.tag', regex: '@[A-Za-z_]\\w*' },        // @Event
            { token: 'support.function', regex: '#[A-Za-z_]\\w*' },       // #function / #value
            { token: 'variable', regex: '[A-Za-z_]\\w*(?::[A-Za-z_0-9]\\w*)*' }, // uid path
            { token: 'keyword.operator', regex: '<<=|>>=|[-+*/%&|^]=|==|!=|<=|>=|&&|\\|\\||<<|>>|[-+*/%<>=!&|^~]' },
            { token: 'paren.lparen', regex: '[(\\[]' },
            { token: 'paren.rparen', regex: '[)\\]]' },
          ],
          comment: [
            { token: 'comment', regex: '\\*/', next: 'start' },
            { defaultToken: 'comment' },
          ],
        };
      }
      oop.inherits(Rules, TextHighlightRules);
      exports.DalhalScriptHighlightRules = Rules;
    });
    ace.define('ace/mode/dalhal_script', ['require', 'exports', 'ace/lib/oop', 'ace/mode/text', 'ace/mode/dalhal_script_highlight_rules'], (require, exports) => {
      const oop = require('ace/lib/oop');
      const TextMode = require('ace/mode/text').Mode;
      const { DalhalScriptHighlightRules } = require('ace/mode/dalhal_script_highlight_rules');
      function Mode() { this.HighlightRules = DalhalScriptHighlightRules; }
      oop.inherits(Mode, TextMode);
      (function () {
        this.lineCommentStart = '//';
        this.blockComment = { start: '/*', end: '*/' };
        // one level deeper after "then"/"do"
        this.getNextLineIndent = function (state, line, tab) {
          const indent = line.match(/^\s*/)[0];
          return /\b(then|do)\s*(\/\/.*)?$/i.test(line) ? indent + tab : indent;
        };
        this.$id = 'ace/mode/dalhal_script';
      }).call(Mode.prototype);
      exports.Mode = Mode;
    });
  }

  /** completions for the word at the cursor; a word is a uid path with #name or @event */
  const completer = {
    identifierRegexps: [/[A-Za-z0-9_:#@]/],
    getCompletions(ed, session, pos, prefix, callback) {
      const before = session.getLine(pos.row).slice(0, pos.column - prefix.length);
      const out = [];
      const add = (value, meta, score, doc) => out.push({ value, caption: value, meta, score, docText: doc });
      const at = prefix.indexOf('@'), hash = prefix.indexOf('#');
      if (at >= 0) { // uid@Event
        const d = devices.get(prefix.slice(0, at));
        if (d) for (const e of d.events) add(prefix.slice(0, at) + '@' + e, 'event', 900);
      } else if (hash >= 0) { // uid#name
        const path = prefix.slice(0, hash);
        const d = devices.get(path);
        if (d) for (const f of d.functions) add(path + f.text, f.kind, 900, f.help);
      } else if (/\bon\s*$/i.test(before)) { // a trigger
        add('eachloop', 'trigger', 1000, 'runs every loop');
        for (const [path, d] of devices) for (const e of d.events) add(path + '@' + e, 'trigger', 800, `${d.type} event`);
      } else {
        for (const [path, d] of devices) {
          add(path, d.type + (d.disabled ? ' (disabled)' : ''), 700, d.primary ? `${d.type}: ${d.primary}` : d.type);
        }
        for (const k of KEYWORDS) add(k, 'keyword', 500);
        out.push({ caption: 'on … then … endon', snippet: 'on ${1:eachloop} then\n\t${2}\nendon', meta: 'snippet', score: 600 });
        out.push({ caption: 'if … then … endif', snippet: 'if ${1:condition} then\n\t${2}\nendif', meta: 'snippet', score: 600 });
        out.push({ caption: 'if … then … else … endif', snippet: 'if ${1:condition} then\n\t${2}\nelse\n\t${3}\nendif', meta: 'snippet', score: 600 });
      }
      callback(null, out);
    },
  };

  /** checks while typing (warnings): uid paths, #names and @events that the device does not know */
  function lintText(text) {
    if (!devices.size) return [];
    const found = [];
    for (const { row, column, path, suffix } of ScriptTools.refs(text)) {
      const d = devices.get(path);
      if (!d) { found.push({ row, column, text: `"${path}" is not a device in the config`, type: 'error' }); continue; }
      if (d.disabled) { found.push({ row, column, text: `"${path}" is disabled in the config, the firmware does not create it`, type: 'error' }); continue; }
      if (!d.known) continue;
      if (suffix.startsWith('@') && !d.events.includes(suffix.slice(1))) {
        found.push({ row, column, text: `${d.type} has no event ${suffix}` + (d.events.length ? ` (it has ${d.events.map((e) => '@' + e).join(', ')})` : ''), type: 'warning' });
      }
      if (suffix.startsWith('#')) {
        const name = suffix.replace('()', '');
        if (!d.functions.some((f) => f.text.replace('()', '') === name)) {
          found.push({ row, column, text: `${d.type} has no ${name}` + (d.functions.length ? ` (it has ${d.functions.map((f) => f.text).join(', ')})` : ''), type: 'warning' });
        }
      }
    }
    return found;
  }

  // ═══════════════════════════════════════════════════════════════
  // PROBLEMS: live warnings + the errors of the device
  // ═══════════════════════════════════════════════════════════════
  let deviceProblems = []; // from the last validation on the device
  let lintTimer = null;

  function showProblems() {
    const live = editor ? lintText(editor.get()) : [];
    const all = [...deviceProblems, ...live];
    if (editor && editor.ace) editor.ace.session.setAnnotations(all.filter((p) => p.row !== undefined));
    const box = el('scr-problems');
    box.replaceChildren(...all.map((p) => newEl('div', {
      cls: 'scr-prob ' + (p.type === 'error' ? 'err' : 'warn'),
      onclick: () => { if (p.row !== undefined && editor.ace) { editor.ace.gotoLine(p.row + 1, p.column || 0, true); editor.focus(); } },
    }, newEl('span', { cls: 'scr-prob-where', text: p.row !== undefined ? `line ${p.row + 1}` : 'device' }), ' ', p.text)));
    box.hidden = all.length === 0;
    el('scr-check').textContent = !open ? '' : deviceProblems.length ? `✗ ${deviceProblems.length} error(s) from the device`
      : open && el('scr-check').dataset.valid === '1' ? '✓ valid on the device' : '';
    el('scr-check').className = deviceProblems.length ? 'err' : 'ok';
  }

  /** the new log lines of the device (the log is a ring buffer) */
  function newLogLines(before, after) {
    const seen = new Map();
    for (const l of before) seen.set(l, (seen.get(l) || 0) + 1);
    return after.filter((l) => { const n = seen.get(l) || 0; if (n) { seen.set(l, n - 1); return false; } return true; });
  }
  const logLines = (t) => t.split(/\r?\n/).map((l) => l.trim()).filter(Boolean);
  const readLog = () => wsRequest('printLog', 'logs').then(logLines).catch(() => []);

  /** log lines -> problems; "(line 3, col 5): msg" gives a position */
  function logProblems(lines) {
    return lines.filter((l) => /\[ERR\]|\[WARN\]/.test(l)).map((l) => {
      const text = l.replace(/^\[[^\]]*\]\[(ERR|WARN)\]\s*/, '');
      const m = text.match(/\(line (\d+), col (\d+)\):\s*(.*)$/);
      const type = /\[ERR\]/.test(l) ? 'error' : 'warning';
      return m ? { row: Number(m[1]) - 1, column: Number(m[2]) - 1, text: m[3], type } : { text, type };
    });
  }

  /** the device checks the saved file name */
  const validateOnDevice = (name) => deviceCheck(`hal/scripts/validate/${name}`, 'hal/scripts/validate');
  /** the device checks a script text in RAM (nothing is written), e.g. the editor content before it is saved */
  const checkText = (name, text) => deviceCheck(`hal/scripts/check/${name}/${text}`, 'hal/scripts/check');

  async function deviceCheck(cmd, tag) {
    el('scr-check').dataset.valid = '';
    const before = await readLog();
    let answer;
    try { answer = JSON.parse(await wsRequest(cmd, tag)); }
    catch (e) { deviceProblems = [{ text: 'could not validate: ' + e.message, type: 'error' }]; showProblems(); return false; }
    const fresh = newLogLines(before, await readLog());
    deviceProblems = answer.info === 'ok' ? [] : logProblems(fresh);
    if (answer.info !== 'ok' && !deviceProblems.length) deviceProblems = [{ text: answer.error || 'not valid', type: 'error' }];
    el('scr-check').dataset.valid = answer.info === 'ok' ? '1' : '';
    showProblems();
    return answer.info === 'ok';
  }

  // ═══════════════════════════════════════════════════════════════
  // FILES: the list, list.txt, open/save/new/rename/delete
  // ═══════════════════════════════════════════════════════════════
  async function loadList() {
    try {
      const items = JSON.parse(await http('GET', `/fs/list?fs=littlefs&path=${q(DIR)}`));
      files = items.filter((i) => !i.dir && i.name !== LIST_FILE && !i.name.endsWith('.tmp')).map((i) => i.name).sort();
    } catch (e) {
      files = []; // no /scripts folder yet
    }
    try {
      listText = (await http('GET', fileUrl(LIST_FILE))).replace(/\r\n/g, '\n');
    } catch (e) {
      listText = '';
    }
    active = activeNames(listText);
    renderList();
  }

  function renderList() {
    const box = el('scr-files');
    box.replaceChildren(...files.map((name) => {
      const isActive = active.includes(name);
      const cb = newEl('input', { type: 'checkbox', title: 'active: listed in list.txt, loaded by "reload scripts"',
        onclick: (e) => e.stopPropagation(), onchange: () => setActive(name, cb.checked) });
      cb.checked = isActive;
      return newEl('div', { cls: 'scr-file' + (name === open ? ' open' : '') + (isActive ? '' : ' inactive'), onclick: () => openFile(name) },
        cb, newEl('span', { cls: 'scr-name', text: name }), name === open && dirty ? newEl('span', { cls: 'scr-dirty', text: '•' }) : null);
    }));
    if (!files.length) box.append(newEl('div', { cls: 'scr-empty', text: 'no scripts yet, create one with + new' }));
    const missing = active.filter((a) => !files.includes(a));
    el('scr-missing').textContent = missing.length ? `list.txt names missing files: ${missing.join(', ')}` : '';
  }

  /** the file names of list.txt: the firmware reads it with the script tokenizer, so line and block comments are allowed
      and the names are separated by white space */
  const activeNames = (text) => ScriptTools.listNames(text);

  /** list.txt with name switched on or off; the rest of the file (comments, order) stays as it is:
      off comments the name out ("//name"), on removes that comment again or adds the name at the end */
  function changeList(text, name, on) {
    const lines = text.split('\n');
    const code = (l) => l.replace(/\/\/.*$/, '');
    if (on) {
      if (activeNames(text).includes(name)) return text;
      const i = lines.findIndex((l) => /^\s*\/\/\s*/.test(l) && l.replace(/^\s*\/\/\s*/, '').trim() === name);
      if (i >= 0) lines[i] = name;
      else { while (lines.length && lines[lines.length - 1].trim() === '') lines.pop(); lines.push(name); }
      return lines.join('\n') + '\n';
    }
    return lines.map((l) => {
      const tokens = code(l).trim().split(/\s+/);
      if (!tokens.includes(name)) return l;
      if (l.trim() === name) return '//' + name;
      return l.replace(new RegExp('(^|\\s)' + name.replace(/[.*+?^${}()|[\]\\]/g, '\\$&') + '(?=\\s|$)'), '$1').replace(/\s+$/, '');
    }).join('\n');
  }

  async function setActive(name, on) {
    if (on) {
      if (name === open && dirty) { msg(`save ${name} first`, true); renderList(); return; }
      msg(`checking ${name} on the device before it is activated…`);
      if (name !== open) await openFile(name);
      if (!(await validateOnDevice(name))) {
        msg(`${name} is not activated: the device does not accept it (see below)`, true);
        renderList();
        return;
      }
    }
    listText = changeList(listText, name, on);
    active = activeNames(listText);
    try {
      await put(LIST_FILE, listText);
      msg(`${name} ${on ? 'is active' : 'is not active any more'}, use "reload scripts" to apply it on the device`);
    } catch (e) { msg('could not write list.txt: ' + e.message, true); }
    renderList();
  }

  async function openFile(name) {
    if (dirty && !confirm(`${open} has unsaved changes, discard them?`)) return;
    try {
      const text = await http('GET', fileUrl(name));
      open = name;
      dirty = false;
      deviceProblems = [];
      el('scr-check').dataset.valid = '';
      editor.set(text, editor.ace ? 'ace/mode/dalhal_script' : 'text');
      el('scr-editor').hidden = false;
      el('scr-empty-editor').hidden = true;
      updateBar();
      renderList();
      showProblems();
      editor.resize();
      editor.focus();
      if (location.hash.startsWith('#scripts')) window.history.replaceState(null, '', '#scripts=' + encodeURIComponent(name));
    } catch (e) { msg(`could not open ${name}: ${e.message}`, true); }
  }

  function updateBar() {
    el('scr-path').textContent = open ? DIR + '/' + open + (dirty ? ' •' : '') : 'select or create a script';
    el('scr-save').disabled = !open;
    el('scr-validate').disabled = !open;
  }

  function onChange() {
    if (!open) return;
    if (!dirty) { dirty = true; updateBar(); renderList(); }
    el('scr-check').dataset.valid = '';
    clearTimeout(lintTimer);
    lintTimer = setTimeout(showProblems, 400);
  }

  async function save() {
    if (!open) return;
    const text = editor.get();
    const isActive = active.includes(open);
    try {
      if (isActive) {
        const local = lintText(text).filter((p) => p.type === 'error');
        if (local.length) {
          deviceProblems = [];
          showProblems();
          msg(`not saved: ${open} is active and uses devices that the config does not have (see below). ` +
            'Fix it, or make it inactive to keep it as a draft.', true);
          return;
        }
        msg(`checking ${open} on the device before saving…`);
        if (!(await checkText(open, text))) {
          msg(`not saved: the device does not accept ${open}, it is active and the running version stays. ` +
            'Fix the errors, or make it inactive to keep it as a draft.', true);
          return;
        }
      }
      await put(open, text);
      dirty = false;
      updateBar();
      renderList();
      if (isActive) { msg(`saved ${DIR}/${open}, valid, use "reload scripts" to run the new version`); return; }
      msg(`saved ${DIR}/${open} (inactive draft), checking it on the device…`);
      const ok = await validateOnDevice(open);
      msg(ok ? `saved ${DIR}/${open}, valid (not active)` : `saved ${DIR}/${open} as an inactive draft, the device reports errors`, !ok);
    } catch (e) { msg('save failed: ' + e.message, true); }
  }

  async function validate() {
    if (!open) return;
    msg(`checking ${open} on the device…`);
    const ok = await checkText(open, editor.get()); // the text in the editor, saved or not
    msg(ok ? `${open} is valid` : `${open}: the device reports errors`, !ok);
  }

  async function newFile() {
    let name = prompt('New script file name:', 'script.txt');
    if (!name) return;
    name = name.trim();
    if (!name.includes('.')) name += '.txt';
    if (name.includes('/') || name === LIST_FILE) { msg('not a valid script file name', true); return; }
    if (files.includes(name)) { msg(`${name} already exists`, true); return; }
    try {
      await http('POST', `/fs/mkdir?fs=littlefs&path=${q(DIR)}`).catch(() => {}); // 409 when it is there
      await put(name, TEMPLATE);
      await loadList();
      openFile(name);
    } catch (e) { msg('could not create the file: ' + e.message, true); }
  }

  async function renameFile() {
    if (!open) return;
    let name = prompt('Rename to:', open);
    if (!name || name === open) return;
    if (!name.includes('.')) name += '.txt';
    try {
      await http('POST', `/fs/rename?fs=littlefs&from=${q(DIR + '/' + open)}&to=${q(DIR + '/' + name)}`);
      if (active.includes(open)) {
        listText = changeList(changeList(listText, open, false), name, true);
        active = activeNames(listText);
        await put(LIST_FILE, listText);
      }
      open = name;
      await loadList();
      updateBar();
      msg(`renamed to ${name}`);
    } catch (e) { msg('rename failed: ' + e.message, true); }
  }

  async function deleteFile() {
    if (!open || !confirm(`Delete ${DIR}/${open}?`)) return;
    try {
      await http('DELETE', fileUrl(open));
      if (active.includes(open)) {
        listText = changeList(listText, open, false);
        active = activeNames(listText);
        await put(LIST_FILE, listText);
      }
      msg(`deleted ${open}`);
      open = null;
      dirty = false;
      deviceProblems = [];
      el('scr-editor').hidden = true;
      el('scr-empty-editor').hidden = false;
      updateBar();
      showProblems();
      await loadList();
    } catch (e) { msg('delete failed: ' + e.message, true); }
  }

  async function reloadScripts() {
    if (dirty && !confirm(`${open} has unsaved changes, reload the saved versions anyway?`)) return;
    msg('loading the active scripts on the device…');
    const before = await readLog();
    let answer;
    try { answer = JSON.parse(await wsRequest('hal/scripts/reload', 'hal/scripts/reload', 30000)); }
    catch (e) { msg('reload failed: ' + e.message, true); return; }
    if (answer.info === 'ok') { msg(`the device runs the active scripts: ${active.join(', ') || '(none)'}`); return; }
    const errs = logProblems(newLogLines(before, await readLog()));
    deviceProblems = errs.map((p) => ({ ...p, row: undefined, text: p.row !== undefined ? `line ${p.row + 1}: ${p.text}` : p.text }));
    showProblems();
    msg('the device could not load the scripts, they are stopped now; fix the script and reload', true);
  }

  // ═══════════════════════════════════════════════════════════════
  // INIT
  // ═══════════════════════════════════════════════════════════════
  async function waitForWs() {
    for (let i = 0; i < 100 && !(typeof ws !== 'undefined' && ws && ws.readyState === WebSocket.OPEN); i++) await new Promise((r) => setTimeout(r, 100));
  }

  function openFromHash() {
    const m = location.hash.match(/^#scripts=(.+)$/);
    if (m && files.includes(decodeURIComponent(m[1])) && decodeURIComponent(m[1]) !== open) openFile(decodeURIComponent(m[1]));
  }

  el('scr-new').onclick = newFile;
  el('scr-rename').onclick = renameFile;
  el('scr-delete').onclick = deleteFile;
  el('scr-save').onclick = save;
  el('scr-validate').onclick = validate;
  el('scr-reload').onclick = reloadScripts;
  window.addEventListener('beforeunload', (e) => { if (dirty) { e.preventDefault(); e.returnValue = ''; } });
  window.addEventListener('resize', () => editor && editor.resize());

  (async () => {
    updateBar();
    editor = await CodeEditor.create(el('scr-editor'), { onChange, onSave: save });
    el('scr-editor-kind').textContent = editor.kind;
    if (editor.ace) {
      defineMode();
      if (await CodeEditor.loadExt('language_tools')) {
        editor.ace.setOptions({ enableBasicAutocompletion: true, enableLiveAutocompletion: true, enableSnippets: true });
        editor.ace.completers = [completer];
      }
    }
    el('scr-editor').hidden = true;
    await loadList();
    await waitForWs();
    await loadDevices();
    showProblems();
    openFromHash();
  })();

  window.ScriptsTab = {
    // a later visit: files may have changed elsewhere (Files tab, another browser), the editor needs its size
    onShow: async () => { if (editor) editor.resize(); if (!dirty) await loadList(); openFromHash(); },
    _lint: (text) => lintText(text),
    _changeList: changeList,
    _activeNames: activeNames,
    _devices: () => [...devices.keys()],
    _completions: (line, prefix) => new Promise((res) => completer.getCompletions(null,
      { getLine: () => line }, { row: 0, column: line.length }, prefix, (e, list) => res(list.map((x) => x.caption)))),
  };
})();

// ── Config tab: cfg.json editor ───────────────────────────────────────────────
// Based on the "cfg::editor" layout (device tree left, form right, JSON panel, status bar), driven by
// what the running firmware reports, so it always matches the firmware:
//   hal/meta/reg/cfgschema   the json schema of every device type (WebSocket, tag "schema")
//   hal/meta/gpio            the GPIOs of this chip and what each can do (tag "getAvailableGPIOs")
// The cfg is read and written with the /fs/... routes (/hal/cfg.json on LittleFS).
// Apply = write /hal/cfg.new.json, let the firmware validate and load it (hal/config/reload/cfg.new.json);
// only when that succeeds it is written to /hal/cfg.json. When it fails the running config stays and
// the new log entries of the firmware are shown.
//
// The rules follow the firmware (src/DALHAL/Core/JsonConfig):
//   modes: in a mode every conjunction with required must be there, every one without must NOT be there,
//          exactly one mode must match; an AllOfFieldsGroup counts as there only when all its fields are
//   UInt/Int maxValue 0 = no limit, uid max 8 chars, uid paths are uid:uid:..., a string item is a comment

(() => {
  const $ = (id) => document.getElementById(id);
  const CFG_DIR = '/hal';
  const CFG_FILE = 'cfg.json';
  const CFG_TEST_FILE = 'cfg.new.json';
  const UID_MAX = 8;
  const PIN_BITS = { OUT: 0x10, IN: 0x20, AIN: 0x40, AOUT: 0x80 };
  const PIN_BAD = 0x0001 | 0x0200 | 0x0400; // Reserved, UARTFLASH, JTAG
  const PIN_WARN = 0x0002;                  // SpecialAtBoot (strapping pins)

  // ── state ──
  let SCHEMA = null;     // { registers, objects, ByReference }
  let GPIO = null;       // { list: { "pin": maskHexString }, MCU }
  let cfg = [];          // the config being edited (root array)
  let savedText = '[]';  // the cfg as last loaded/saved, to know when there are unsaved changes
  let sel = null;        // { item, arr, ctx, parents: [{ item, ctx }] }
  let jsonOpen = false;
  let problems = [];     // [{ item, arr, ctx, parents, field, msg, level: 'err'|'warn' }]
  let formErr = {};      // field name -> element showing its error, for the selected item

  // ── small helpers ──
  const isObj = (v) => v !== null && typeof v === 'object' && !Array.isArray(v);
  const has = (o, k) => isObj(o) && Object.prototype.hasOwnProperty.call(o, k);
  const fsUrl = (name) => `/fs/file?fs=littlefs&path=${encodeURIComponent(CFG_DIR + '/' + name)}`;

  function status(text, kind = 'ok') {
    $('cfg-smsg').textContent = text;
    $('cfg-sdot').className = 'sdot' + (kind === 'err' ? ' err' : kind === 'warn' ? ' warn' : '');
  }

  async function http(method, url, body) {
    const r = await fetch(url, { method, body, cache: 'no-store' });
    const text = await r.text();
    if (!r.ok) throw new Error(`${r.status} ${text}`);
    return text;
  }

  // ═══════════════════════════════════════════════════════════════
  // SCHEMA
  // ═══════════════════════════════════════════════════════════════
  const byRef = {};
  const objects = {};

  function initSchema(raw) {
    SCHEMA = raw;
    for (const r of raw.ByReference || []) byRef[r.name] = r;
    for (const o of raw.objects || []) objects[o.name] = o;
  }

  /** fields with _byref_ FieldsGroups flattened; a referenced OneOfFieldsGroup stays one (group) field */
  function expand(fields, seen = new Set()) {
    const out = [];
    for (const f of fields || []) {
      if (f.type !== '_byref_') { out.push(f); continue; }
      const r = byRef[f.name];
      if (!r || seen.has(f.name)) continue;
      if (r.type === 'FieldsGroup') out.push(...expand(r.fields, new Set([...seen, f.name])));
      else out.push(r);
    }
    return out;
  }

  const defCache = new WeakMap();
  /** the expanded fields of a type definition (cached) */
  function fieldsOf(def) {
    if (!def) return [];
    if (!defCache.has(def)) defCache.set(def, expand(def.fields));
    return defCache.get(def);
  }

  function typeDef(regPath, type) {
    const group = SCHEMA.registers[regPath];
    if (!group || typeof type !== 'string') return null;
    if (group[type]) return group[type];
    const k = Object.keys(group).find((x) => x.toUpperCase() === type.toUpperCase());
    return k ? group[k] : null;
  }

  function objectDef(subtype) {
    if (typeof subtype === 'string') return objects[subtype] || null;
    return isObj(subtype) && subtype.fields ? subtype : null;
  }

  /** the definition of an item: a registry type (by its "type") or a fixed object type */
  function defOf(item, ctx) {
    if (!isObj(item)) return null;
    return ctx.kind === 'reg' ? typeDef(ctx.regPath, item.type) : ctx.def;
  }

  /** the fields that hold child items shown in the tree: registry arrays and object arrays (not inline ones) */
  function childArrays(def) {
    const out = [];
    for (const f of fieldsOf(def)) {
      if (f.type === 'ArrayOfRegistryItems') out.push({ f, ctx: { kind: 'reg', regPath: f.regPath } });
      else if (f.type === 'ArrayOfObjects' && !(f.gui || []).includes('UseInline')) {
        const d = objectDef(f.subtype);
        if (d) out.push({ f, ctx: { kind: 'obj', def: d } });
      }
    }
    return out;
  }

  /** a field (or group) of def by name */
  const fieldByName = (def, name) => fieldsOf(def).find((f) => f.name === name) || null;

  /** every json key a field (or group) uses */
  function keysOf(f) {
    if (f.type === 'OneOfFieldsGroup' || f.type === 'AllOfFieldsGroup') return f.fields.map((x) => x.name);
    return [f.name];
  }

  /** is the field there in the item, as the firmware sees it (ModeSelector::evaluate) */
  function present(item, f) {
    if (!f) return false;
    if (f.type === 'OneOfFieldsGroup') return f.fields.some((x) => has(item, x.name));
    if (f.type === 'AllOfFieldsGroup') return f.fields.length > 0 && f.fields.every((x) => has(item, x.name));
    return has(item, f.name);
  }

  // ── modes ──
  /** index of the matching mode, -1 none, -2 more than one (same as the firmware) */
  function evalMode(item, def) {
    let match = -1;
    def.modes.forEach((mode, i) => {
      const ok = mode.conjunctions.every((c) => !!c.required === present(item, fieldByName(def, c.name)));
      if (!ok) return;
      match = match === -1 ? i : -2;
    });
    return match;
  }

  /** a field that the selected mode does not allow is not shown */
  function hiddenByMode(item, def, f) {
    if (!def.modes || !def.modes.length) return false;
    const m = evalMode(item, def);
    if (m < 0) return false; // no clear mode: show everything so it can be fixed
    const c = def.modes[m].conjunctions.find((x) => x.name === f.name);
    return !!c && !c.required;
  }

  /** the field is a required part of the current mode */
  function requiredByMode(item, def, f) {
    if (!def.modes || !def.modes.length) return false;
    const m = evalMode(item, def);
    if (m < 0) return false;
    const c = def.modes[m].conjunctions.find((x) => x.name === f.name);
    return !!c && !!c.required;
  }

  function applyMode(item, def, mode) {
    for (const c of mode.conjunctions) {
      const f = fieldByName(def, c.name);
      if (!f) continue;
      if (c.required && !present(item, f)) addField(item, f);
      if (!c.required) for (const k of keysOf(f)) delete item[k];
    }
  }

  // ── defaults ──
  function defaultValue(f) {
    if (f.default !== undefined && f.default !== null) return f.default;
    switch (f.type) {
      case 'Bool': return false;
      case 'UInt': case 'Int': case 'Float': case 'Number': return f.minValue || 0;
      case 'HardwarePin': return null; // must be chosen
      case 'StringAnyOfByFuncConstrained': return (f.allowedValues || [''])[0];
      case 'StringHexBytes': return Array(f.byteCount || 1).fill('00').join(':');
      case 'Object': return {};
      case 'ArrayOfObjects': case 'ArrayOfRegistryItems': case 'ArrayOfPrimitives': return [];
      default: return '';
    }
  }

  /** puts a field (or group) into the item with default values */
  function addField(item, f) {
    if (f.type === 'OneOfFieldsGroup') {
      const pick = f.fields.find((x) => x.name === 'refreshtimesec') || f.fields[0];
      item[pick.name] = defaultValue(pick);
    } else if (f.type === 'AllOfFieldsGroup') {
      for (const x of f.fields) if (!has(item, x.name)) item[x.name] = defaultValue(x);
    } else {
      item[f.name] = defaultValue(f);
    }
  }

  /** a uid that is not used in arr: base1, base2 ... (max 8 chars) */
  function freeUid(arr, base) {
    base = (base || 'item').toLowerCase().replace(/[^a-z0-9_]/g, '').slice(0, UID_MAX - 2) || 'item';
    const used = new Set(arr.filter(isObj).map((x) => x.uid));
    for (let n = 1; ; n++) if (!used.has(base + n)) return base + n;
  }

  /** a new item of a registry type (typeKey) or of a fixed object type, with its required fields */
  function newItem(arr, ctx, typeKey) {
    const def = ctx.kind === 'reg' ? typeDef(ctx.regPath, typeKey) : ctx.def;
    const item = {};
    if (ctx.kind === 'reg') item.type = typeKey;
    item.uid = freeUid(arr, ctx.kind === 'reg' ? typeKey : def && def.name);
    for (const f of fieldsOf(def)) {
      if (['disabled', 'uid', 'note', 'type'].includes(f.name)) continue;
      if (f.required) addField(item, f);
    }
    if (def && def.modes && def.modes.length) applyMode(item, def, def.modes[0]);
    return item;
  }

  // ═══════════════════════════════════════════════════════════════
  // WALK + VALIDATION
  // ═══════════════════════════════════════════════════════════════
  /** calls fn(item, arr, ctx, parents) for every object item in the tree (depth first) */
  function walk(arr, ctx, parents, fn) {
    for (const item of arr) {
      if (!isObj(item)) continue;
      fn(item, arr, ctx, parents);
      const def = defOf(item, ctx);
      for (const { f, ctx: cctx } of childArrays(def)) {
        if (Array.isArray(item[f.name])) walk(item[f.name], cctx, [...parents, { item, ctx }], fn);
      }
    }
  }

  /** every uid path in the config, e.g. temps, temps:a, temps:a:b (for StringUID_Path) */
  function allUidPaths() {
    const out = [];
    walk(cfg, { kind: 'reg', regPath: 'ROOT' }, [], (item, arr, ctx, parents) => {
      if (typeof item.uid !== 'string' || !item.uid) return;
      out.push([...parents.map((p) => p.item.uid), item.uid].join(':'));
    });
    return out;
  }

  /** GPIO number -> [{ item, field }] of all enabled items, for pin conflict checks */
  function pinUsers() {
    const map = new Map();
    const add = (pin, item, name) => {
      if (typeof pin !== 'number' || pin < 0) return;
      if (!map.has(pin)) map.set(pin, []);
      map.get(pin).push({ item, name });
    };
    const scan = (obj, def, owner, disabled) => {
      for (const f of fieldsOf(def)) {
        for (const sub of f.type === 'AllOfFieldsGroup' ? f.fields : [f]) {
          if (sub.type === 'HardwarePin' && !disabled) add(obj[sub.name], owner, sub.name);
          if (sub.type === 'Object' && isObj(obj[sub.name])) scan(obj[sub.name], objectDef(sub.subtype), owner, disabled);
        }
      }
    };
    walk(cfg, { kind: 'reg', regPath: 'ROOT' }, [], (item, arr, ctx, parents) => {
      const disabled = item.disabled === true || parents.some((p) => p.item.disabled === true);
      scan(item, defOf(item, ctx), item, disabled);
    });
    return map;
  }

  function pinMask(pin) {
    if (!GPIO || !GPIO.list || GPIO.list[pin] === undefined) return null;
    return parseInt(GPIO.list[pin], 16);
  }

  /** the GPIO capabilities a HardwarePin field needs: "OUT|IN" needs both */
  const pinNeeds = (f) => (f.mode || '').split('|').reduce((m, x) => m | (PIN_BITS[x.trim()] || 0), 0);

  /** problems of one value; returns [msg, level] or null */
  function checkValue(f, v, ctx) {
    const isNum = typeof v === 'number' && isFinite(v);
    switch (f.type) {
      case 'Bool':
        return typeof v === 'boolean' ? null : ['must be true or false', 'err'];
      case 'UInt': case 'Int': {
        if (!isNum || !Number.isInteger(v)) return ['must be a whole number', 'err'];
        if (f.type === 'UInt' && v < 0) return ['must not be negative', 'err'];
        if (f.minValue && v < f.minValue) return [`must be at least ${f.minValue}`, 'err'];
        if (f.maxValue && v > f.maxValue) return [`must be at most ${f.maxValue}`, 'err'];
        return null;
      }
      case 'Float': {
        if (!isNum) return ['must be a number', 'err'];
        if (f.minValue !== null && f.minValue !== undefined && v < f.minValue) return [`must be at least ${f.minValue}`, 'err'];
        if (f.maxValue !== null && f.maxValue !== undefined && f.maxValue !== 0 && v > f.maxValue) return [`must be at most ${f.maxValue}`, 'err'];
        return null;
      }
      case 'Number': {
        const t = f.allowedTypes || {};
        if (typeof v === 'boolean') return t.bool ? null : ['must be a number', 'err'];
        if (!isNum) return ['must be a number', 'err'];
        if (!Number.isInteger(v) && t.float === false) return ['must be a whole number', 'err'];
        return null;
      }
      case 'StringUID':
        if (typeof v !== 'string' || !v) return ['uid is required', 'err'];
        if (v.length > UID_MAX) return [`uid is max ${UID_MAX} characters`, 'err'];
        if (/[:#@\s]/.test(v)) return ['uid can not contain : # @ or spaces', 'err'];
        return null;
      case 'StringUID_Path':
        if (typeof v !== 'string') return ['must be a text', 'err'];
        if (v && ctx && ctx.uidPaths && !ctx.uidPaths.has(v.split('#')[0])) return [`"${v}" is not a uid in this config`, 'warn'];
        return null;
      case 'StringSizeConstrained':
        if (typeof v !== 'string') return ['must be a text', 'err'];
        if (f.minLength && v.length < f.minLength) return [f.minLength === f.maxLength ? `must be ${f.minLength} characters` : `at least ${f.minLength} characters`, 'err'];
        if (f.maxLength && v.length > f.maxLength) return [`at most ${f.maxLength} characters`, 'err'];
        return null;
      case 'StringHexBytes': {
        if (typeof v !== 'string') return ['must be a text', 'err'];
        const hex = v.replace(/[:\s-]/g, '');
        if (!/^[0-9a-fA-F]*$/.test(hex) || hex.length !== 2 * (f.byteCount || 1)) return [`must be ${f.byteCount || 1} hex byte${f.byteCount > 1 ? 's' : ''}`, 'err'];
        return null;
      }
      case 'StringAnyOfByFuncConstrained':
        if (typeof v !== 'string') return ['must be a text', 'err'];
        if (f.allowedValues && !f.allowedValues.some((a) => a.toUpperCase() === v.toUpperCase())) return [`must be one of ${f.allowedValues.join(', ')}`, 'err'];
        return null;
      case 'HardwarePin': {
        if (v === null || v === '' || v === undefined) return ['choose a pin', 'err'];
        if (!Number.isInteger(v) || v < -128 || v > 127) return ['must be a GPIO number', 'err'];
        if (v < 0) return null; // a negative pin is a disabled pin use, the firmware does not check it
        if (!GPIO) return null;
        const mask = pinMask(v);
        if (mask === null) return [`GPIO ${v} is not available on this ${GPIO.MCU || 'chip'}`, 'err'];
        if (mask & PIN_BAD) return [`GPIO ${v} is reserved (flash/UART/JTAG)`, 'err'];
        const need = pinNeeds(f);
        if ((mask & need) !== need) return [`GPIO ${v} can not be used as ${f.mode}`, 'err'];
        if (mask & PIN_WARN) return [`GPIO ${v} is a boot strapping pin, check the boot level`, 'warn'];
        return null;
      }
      case 'ArrayOfPrimitives':
        return Array.isArray(v) && v.every((x) => typeof x !== 'object') ? null : ['must be an array of values', 'err'];
      case 'Object':
        return isObj(v) ? null : ['must be an object', 'err'];
      case 'ArrayOfObjects': case 'ArrayOfRegistryItems':
        return Array.isArray(v) ? null : ['must be an array', 'err'];
      default:
        return typeof v === 'string' ? null : ['must be a text', 'err'];
    }
  }

  /** problems of one object (item or Object field) against its definition: [{ field, msg, level }] */
  function checkObject(obj, def, vctx, prefix = '') {
    const out = [];
    const add = (field, msg, level = 'err') => out.push({ field: prefix + field, msg, level });
    const known = new Set();
    for (const f of fieldsOf(def)) {
      keysOf(f).forEach((k) => known.add(k));
      if (hiddenByMode(obj, def, f)) continue;
      if (f.type === 'OneOfFieldsGroup') {
        const there = f.fields.filter((x) => has(obj, x.name));
        if (there.length > 1) add(f.name, `only one of ${f.fields.map((x) => x.name).join(', ')}`);
        else if (!there.length && f.required) add(f.name, `${f.name} is required`);
        for (const x of there) { const p = checkValue(x, obj[x.name], vctx); if (p) add(x.name, p[0], p[1]); }
        continue;
      }
      if (f.type === 'AllOfFieldsGroup') {
        const there = f.fields.filter((x) => has(obj, x.name));
        if (there.length && there.length < f.fields.length) add(f.name, `${f.name} needs all of ${f.fields.map((x) => x.name).join(', ')}`);
        for (const x of there) { const p = checkValue(x, obj[x.name], vctx); if (p) add(x.name, p[0], p[1]); }
        continue;
      }
      if (!has(obj, f.name)) {
        if (f.required) add(f.name, `${f.name} is required`);
        continue;
      }
      const p = checkValue(f, obj[f.name], vctx);
      if (p) { add(f.name, p[0], p[1]); continue; }
      if (f.type === 'Object') {
        const sub = objectDef(f.subtype);
        if (sub) out.push(...checkObject(obj[f.name], sub, vctx, prefix + f.name + '.'));
      }
      if ((f.type === 'ArrayOfObjects' || f.type === 'ArrayOfRegistryItems') && f.required && obj[f.name].length === 0) {
        add(f.name, `${f.name} is empty`, 'warn');
      }
    }
    for (const k of Object.keys(obj)) {
      if (!known.has(k)) add(k, `unknown field "${k}"`, def.unknownPolicy === 'Error' ? 'err' : 'warn');
    }
    if (def.modes && def.modes.length) {
      const m = evalMode(obj, def);
      if (m === -1) add('_mode', 'the fields do not match any mode, choose a mode');
      if (m === -2) add('_mode', 'the fields match more than one mode');
    }
    for (const c of def.constraints || []) {
      const fa = fieldByName(def, c.fieldA), fb = fieldByName(def, c.fieldB);
      if (!fa || !fb || hiddenByMode(obj, def, fa) || hiddenByMode(obj, def, fb)) continue;
      const a = has(obj, c.fieldA) ? obj[c.fieldA] : fa.default;
      const b = has(obj, c.fieldB) ? obj[c.fieldB] : fb.default;
      if (typeof a !== 'number' || typeof b !== 'number') continue;
      const ok = c.type === '<' ? a < b : c.type === '<=' ? a <= b : true;
      if (!ok) add(c.fieldA, `${c.fieldA} (${a}) must be ${c.type} ${c.fieldB} (${b})`);
    }
    return out;
  }

  /** validates the whole config into problems[] */
  function validate() {
    problems = [];
    const vctx = { uidPaths: new Set(allUidPaths()) };
    const users = pinUsers();
    const root = { kind: 'reg', regPath: 'ROOT' };
    if (!cfg.some((x) => isObj(x) && x.disabled !== true)) {
      problems.push({ item: null, field: '', msg: 'the config has no enabled devices, the firmware will not load it', level: 'err' });
    }
    walk(cfg, root, [], (item, arr, ctx, parents) => {
      const push = (field, msg, level) => problems.push({ item, arr, ctx, parents, field, msg, level });
      const disabled = item.disabled === true || parents.some((p) => p.item.disabled === true);
      const def = defOf(item, ctx);
      if (!def) {
        push('type', ctx.kind === 'reg' ? `unknown type "${item.type}" for this firmware` : 'unknown item', disabled ? 'warn' : 'err');
        return;
      }
      for (const p of checkObject(item, def, vctx)) push(p.field, p.msg, disabled && p.level === 'err' ? 'warn' : p.level);
      if (typeof item.uid === 'string' && arr.filter((x) => isObj(x) && x.uid === item.uid).length > 1) {
        push('uid', `uid "${item.uid}" is used more than once here`, disabled ? 'warn' : 'err');
      }
      if (!disabled) {
        for (const f of fieldsOf(def)) {
          for (const sub of f.type === 'AllOfFieldsGroup' ? f.fields : [f]) {
            if (sub.type !== 'HardwarePin' || typeof item[sub.name] !== 'number' || item[sub.name] < 0) continue;
            const others = (users.get(item[sub.name]) || []).filter((u) => u.item !== item || u.name !== sub.name);
            if (others.length) push(sub.name, `GPIO ${item[sub.name]} is also used by ${others.map((u) => `${u.item.uid}.${u.name}`).join(', ')}`, 'err');
          }
        }
      }
    });
    renderProblems();
    renderTreeBadges();
    renderFormErrors();
  }

  // ═══════════════════════════════════════════════════════════════
  // TREE (sidebar)
  // ═══════════════════════════════════════════════════════════════
  const treeRows = new Map(); // item -> row element

  function renderTree() {
    const list = $('cfg-tree');
    list.innerHTML = '';
    treeRows.clear();
    renderTreeLevel(list, cfg, { kind: 'reg', regPath: 'ROOT' }, [], 0);
    $('cfg-dev-cnt').textContent = cfg.filter(isObj).length;
    renderTreeBadges();
  }

  function renderTreeLevel(container, arr, ctx, parents, depth) {
    arr.forEach((item, idx) => {
      if (typeof item === 'string') { // a comment item
        const row = newEl('div', { cls: 'di comment', style: `padding-left:${7 + depth * 14}px`, title: 'comment (a text item, ignored by the firmware)' },
          newEl('span', { cls: 'du', text: '// ' + item }));
        row.addEventListener('click', () => {
          const t = prompt('Comment:', item);
          if (t === null) return;
          arr[idx] = t;
          changed();
        });
        container.append(row);
        return;
      }
      if (!isObj(item)) return;
      const def = defOf(item, ctx);
      const kids = childArrays(def);
      const childCount = kids.reduce((n, k) => n + (Array.isArray(item[k.f.name]) ? item[k.f.name].filter(isObj).length : 0), 0);
      const isSel = sel && sel.item === item;
      const row = newEl('div', {
        cls: `di${isSel ? ' active' : ''}${item.disabled === true ? ' dis' : ''}`,
        style: `padding-left:${7 + depth * 14}px`,
        onclick: () => select(item, arr, ctx, parents),
      },
      newEl('span', { cls: 'du', text: item.uid || '(no uid)' }),
      newEl('span', { cls: 'tb', text: ctx.kind === 'reg' ? (item.type || '?') : (ctx.def.name || 'item') }),
      childCount ? newEl('span', { cls: 'ch', text: `▾${childCount}` }) : null,
      newEl('span', { cls: 'pb' }));
      treeRows.set(item, row);
      container.append(row);
      for (const k of kids) {
        if (Array.isArray(item[k.f.name])) renderTreeLevel(container, item[k.f.name], k.ctx, [...parents, { item, ctx }], depth + 1);
      }
    });
  }

  /** red/yellow dot on the rows with problems (also on their parents) */
  function renderTreeBadges() {
    const worst = new Map();
    const mark = (item, level) => { if (worst.get(item) !== 'err') worst.set(item, level); };
    for (const p of problems) {
      if (!p.item) continue;
      mark(p.item, p.level);
      for (const parent of p.parents || []) mark(parent.item, p.level === 'err' ? 'err' : 'warn');
    }
    for (const [item, row] of treeRows) {
      const b = row.querySelector('.pb');
      const lvl = worst.get(item);
      b.className = 'pb' + (lvl ? ' ' + lvl : '');
      b.title = lvl ? problems.filter((p) => p.item === item).map((p) => p.msg).join('\n') : '';
    }
  }

  function select(item, arr, ctx, parents) {
    sel = item ? { item, arr, ctx, parents } : null;
    // the address follows the selection (bookmarkable), replaceState does not fire hashchange;
    // window.history: autocomplete.js has a global "history" (the command history)
    const path = sel ? [...parents.map((p) => p.item.uid), item.uid].join(':') : '';
    if (location.hash.startsWith('#config')) window.history.replaceState(null, '', path ? '#config=' + encodeURIComponent(path).replace(/%3A/gi, ':') : '#config');
    for (const [it, row] of treeRows) row.classList.toggle('active', it === item);
    renderForm();
    syncJson();
  }

  /** the add bar adds root devices */
  function fillAddSelect() {
    const s = $('cfg-add-type');
    s.innerHTML = '';
    for (const [k, def] of Object.entries(SCHEMA.registers.ROOT || {})) {
      s.append(newEl('option', { value: k, text: `${k} – ${def.name}` }));
    }
  }

  // ═══════════════════════════════════════════════════════════════
  // FORM
  // ═══════════════════════════════════════════════════════════════
  function renderForm() {
    const fa = $('cfg-form');
    fa.innerHTML = '';
    formErr = {};
    renderPath();
    $('cfg-del').disabled = !sel;
    $('cfg-dup').disabled = !sel;
    $('cfg-up').disabled = !sel || sel.arr.indexOf(sel.item) <= 0;
    $('cfg-down').disabled = !sel || sel.arr.indexOf(sel.item) >= sel.arr.length - 1;
    if (!sel) {
      fa.append(newEl('div', { cls: 'empty' }, newEl('div', { cls: 'em-icon', text: '⬡' }),
        newEl('div', { cls: 'em-txt', text: SCHEMA ? 'Select a device or add one' : 'Loading…' })));
      return;
    }
    const { item, ctx } = sel;
    const def = defOf(item, ctx);

    const dis = newEl('input', { type: 'checkbox', id: 'cfg-f-disabled' });
    dis.checked = item.disabled === true;
    dis.addEventListener('change', () => { if (dis.checked) item.disabled = true; else delete item.disabled; changed({ tree: true }); });
    fa.append(newEl('div', { cls: 'dev-hdr' },
      newEl('span', { cls: 'dname', text: item.uid || '(no uid)' }),
      newEl('span', { cls: 'dtype', text: (ctx.kind === 'reg' ? (item.type || '?') + ' — ' : '') + (def ? def.name : 'unknown') }),
      newEl('div', { cls: 'dis-wrap' }, dis, newEl('label', { for: 'cfg-f-disabled', text: 'disabled' }))));

    if (!def) {
      fa.append(newEl('div', { cls: 'warn-box', text: ctx.kind === 'reg'
        ? `The type "${item.type}" is not known by this firmware. Its fields are shown as they are.`
        : 'Unknown item, its fields are shown as they are.' }));
      renderExtraFields(fa, item, new Set());
      return;
    }

    if (def.modes && def.modes.length) fa.append(renderModeBox(item, def));
    renderFields(fa, item, def, '');
    renderFormErrors();
  }

  /** root › parent › item in the toolbar, every part opens that item */
  function renderPath() {
    const box = $('cfg-path');
    box.innerHTML = '';
    box.append(newEl('span', { cls: 'bc-seg', text: 'root', onclick: () => select(null) }));
    if (!sel) return;
    const chain = [...sel.parents, { item: sel.item, ctx: sel.ctx }];
    chain.forEach((p, i) => {
      box.append(newEl('span', { cls: 'bc-arr', text: '›' }));
      box.append(newEl('span', { cls: 'bc-seg' + (i === chain.length - 1 ? ' bc-cur' : ''), text: p.item.uid || '?', onclick: () => {
        const parents = chain.slice(0, i);
        const owner = parents[parents.length - 1];
        const arr = owner
          ? owner.item[childArrays(defOf(owner.item, owner.ctx)).find((k) => (owner.item[k.f.name] || []).includes(p.item)).f.name]
          : cfg;
        select(p.item, arr, p.ctx, parents);
      } }));
    });
  }

  function renderModeBox(item, def) {
    const m = evalMode(item, def);
    const s = newEl('select');
    if (m < 0) s.append(newEl('option', { value: '', text: m === -1 ? '(no mode matches, choose one)' : '(more than one mode matches)' }));
    def.modes.forEach((mode, i) => s.append(newEl('option', { value: i, text: mode.name })));
    s.value = m >= 0 ? m : '';
    s.addEventListener('change', () => {
      if (s.value === '') return;
      applyMode(item, def, def.modes[s.value]);
      changed({ form: true, tree: true });
    });
    const errEl = newEl('div', { cls: 'err' });
    formErr._mode = errEl;
    return newEl('div', { cls: 'mode-box' }, newEl('label', { text: 'mode' }), s, errEl);
  }

  /** all fields of def for obj (an item or an Object field value); prefix is for nested error names */
  function renderFields(container, obj, def, prefix) {
    const shown = new Set();
    const fields = fieldsOf(def);
    // uid, note first (they are in every device), disabled is in the header
    const order = [...fields.filter((f) => f.name === 'uid'), ...fields.filter((f) => f.name === 'note'),
      ...fields.filter((f) => !['uid', 'note', 'disabled', 'type'].includes(f.name))];
    for (const f of fields) if (['disabled', 'type'].includes(f.name) && prefix === '') keysOf(f).forEach((k) => shown.add(k));
    for (const f of order) {
      keysOf(f).forEach((k) => shown.add(k));
      if (hiddenByMode(obj, def, f)) continue;
      switch (f.type) {
        case 'OneOfFieldsGroup': container.append(renderOneOf(obj, f, prefix)); break;
        case 'AllOfFieldsGroup': container.append(renderAllOf(obj, f, prefix, requiredByMode(obj, def, f))); break;
        case 'ArrayOfRegistryItems': container.append(renderChildArray(obj, f, { kind: 'reg', regPath: f.regPath })); break;
        case 'ArrayOfObjects': {
          const sub = objectDef(f.subtype);
          if ((f.gui || []).includes('RenderAllAllowedValues') && f.renderAllAllowedValuesFromStringConstraint && sub) container.append(renderChips(obj, f, sub));
          else if (sub) container.append(renderChildArray(obj, f, { kind: 'obj', def: sub }));
          else container.append(renderJsonField(obj, f, prefix));
          break;
        }
        case 'Object': {
          const sub = objectDef(f.subtype);
          container.append(sub ? renderObject(obj, f, sub, prefix) : renderJsonField(obj, f, prefix));
          break;
        }
        case 'ArrayOfPrimitives': container.append(renderJsonField(obj, f, prefix)); break;
        default: container.append(renderField(obj, f, prefix));
      }
    }
    renderExtraFields(container, obj, shown);
  }

  // ── one value ──
  function label(f, extraHint) {
    return newEl('label', {}, f.name, f.required ? newEl('span', { cls: 'req', text: ' *' }) : null,
      newEl('span', { cls: 'type-hint', text: extraHint || f.type }));
  }

  function hintOf(f) {
    const parts = [];
    if (['UInt', 'Int', 'Float'].includes(f.type)) {
      const lo = f.minValue || f.minValue === 0 ? f.minValue : null;
      const hi = f.maxValue ? f.maxValue : null;
      if (lo !== null && hi !== null) parts.push(`${lo} – ${hi}`);
      else if (lo) parts.push(`≥ ${lo}`);
      else if (hi !== null) parts.push(`≤ ${hi}`);
    }
    if (f.type === 'StringSizeConstrained') parts.push(f.minLength === f.maxLength ? `${f.minLength} characters` : `${f.minLength || 0}–${f.maxLength || '∞'} characters`);
    if (f.type === 'StringHexBytes') parts.push(`${f.byteCount || 1} hex byte${f.byteCount > 1 ? 's, e.g. ' + Array(f.byteCount).fill('00').join(':') : ', e.g. 3C'}`);
    if (f.type === 'StringUID') parts.push(`max ${UID_MAX} characters`);
    if (f.type === 'StringUID_Path') parts.push('uid or uid:uid:… of a device in this config');
    if (f.type === 'HardwarePin') parts.push(`GPIO, needs ${f.mode || '-'} · a negative number disables the pin use`);
    if (f.default !== undefined && f.default !== null && f.default !== '' && !f.required) parts.push(`default ${JSON.stringify(f.default)}`);
    return parts.join(' · ');
  }

  /** sets obj[name] (or removes it when the optional field is cleared) and validates */
  function setVal(obj, f, v, opts) {
    if (v === undefined) delete obj[f.name];
    else obj[f.name] = v;
    changed(opts);
  }

  function clearButton(obj, f, onClear) {
    if (f.required) return null;
    return newEl('button', { cls: 'clr', title: 'remove this field (use the default)', text: '×', onclick: () => { delete obj[f.name]; onClear(); changed(); } });
  }

  function renderField(obj, f, prefix) {
    const row = newEl('div', { cls: 'fr' }, label(f));
    const there = has(obj, f.name);
    const val = obj[f.name];
    let input;

    if (f.type === 'Bool') {
      input = newEl('input', { type: 'checkbox' });
      input.checked = there ? val === true : f.default === true;
      input.addEventListener('change', () => setVal(obj, f, input.checked));
      row.append(newEl('div', { cls: 'fr-check' }, input, newEl('span', { cls: 'fn', text: there ? '' : '(default)' })));
    } else if (f.type === 'StringAnyOfByFuncConstrained') {
      input = newEl('select');
      if (!f.required) input.append(newEl('option', { value: '', text: f.default ? `(default: ${f.default})` : '(not set)' }));
      for (const v of f.allowedValues || []) input.append(newEl('option', { value: v, text: v }));
      if (there && !(f.allowedValues || []).some((a) => a.toUpperCase() === String(val).toUpperCase())) input.append(newEl('option', { value: val, text: `${val} (not allowed)` }));
      input.value = there ? (f.allowedValues || []).find((a) => a.toUpperCase() === String(val).toUpperCase()) || val : '';
      input.addEventListener('change', () => setVal(obj, f, input.value === '' ? undefined : input.value));
      row.append(input);
    } else if (f.type === 'HardwarePin') {
      input = renderPinSelect(obj, f);
      row.append(input);
    } else if (['UInt', 'Int', 'Float', 'Number'].includes(f.type)) {
      input = newEl('input', { type: 'number', step: f.type === 'UInt' || f.type === 'Int' ? '1' : 'any' });
      if (f.minValue || f.minValue === 0) input.min = f.minValue;
      if (f.maxValue) input.max = f.maxValue;
      input.value = there && typeof val === 'number' ? val : '';
      if (!there && f.default !== undefined && f.default !== null) input.placeholder = `default ${f.default}`;
      if (there && typeof val !== 'number') input.placeholder = `${JSON.stringify(val)} (not a number)`;
      input.addEventListener('input', () => {
        if (input.value === '') { setVal(obj, f, f.required ? null : undefined); return; }
        const n = Number(input.value);
        setVal(obj, f, isNaN(n) ? input.value : n);
      });
      row.append(newEl('div', { cls: 'fr-line' }, input, clearButton(obj, f, () => { input.value = ''; })));
    } else {
      const secret = /^(pass|password|passwd|pwd|secret|key|apikey|token)$/i.test(f.name);
      input = newEl('input', { type: secret ? 'password' : 'text', spellcheck: 'false', autocomplete: secret ? 'new-password' : 'off' });
      if (f.type === 'StringUID') input.maxLength = UID_MAX;
      if (f.type === 'StringSizeConstrained' && f.maxLength) input.maxLength = f.maxLength;
      if (f.type === 'StringUID_Path') input.setAttribute('list', 'cfg-uidpaths');
      input.value = there ? (typeof val === 'string' ? val : JSON.stringify(val)) : '';
      if (!there && f.default) input.placeholder = `default ${f.default}`;
      input.addEventListener('input', () => {
        const v = input.value;
        if (v === '' && !f.required && f.type !== 'StringUID') { setVal(obj, f, undefined, { tree: f.name === 'uid' }); return; }
        setVal(obj, f, v, { tree: f.name === 'uid', header: f.name === 'uid' });
      });
      const show = secret ? newEl('button', { cls: 'clr', title: 'show / hide', text: '👁', onclick: () => { input.type = input.type === 'password' ? 'text' : 'password'; } }) : null;
      row.append(newEl('div', { cls: 'fr-line' }, input, show, clearButton(obj, f, () => { input.value = ''; })));
    }
    const hint = hintOf(f);
    if (hint) row.append(newEl('div', { cls: 'fn', text: hint }));
    const err = newEl('div', { cls: 'err' });
    formErr[prefix + f.name] = err;
    row.append(err);
    return row;
  }

  function renderPinSelect(obj, f) {
    const s = newEl('select');
    const val = obj[f.name];
    s.append(newEl('option', { value: '', text: f.required ? '— choose a GPIO —' : '— not used —' }));
    const users = pinUsers();
    const need = pinNeeds(f);
    const pins = GPIO ? Object.keys(GPIO.list).map(Number).sort((a, b) => a - b) : [];
    for (const p of pins) {
      const mask = pinMask(p);
      const caps = Object.entries(PIN_BITS).filter(([, b]) => mask & b).map(([n]) => n).join('/');
      const others = (users.get(p) || []).filter((u) => !(u.item === sel.item && u.name === f.name));
      let txt = `GPIO ${p}  ${caps}`;
      if (mask & PIN_WARN) txt += '  ⚠ boot';
      if (others.length) txt += `  ← ${others.map((u) => u.item.uid).join(', ')}`;
      const unusable = (mask & PIN_BAD) || (mask & need) !== need;
      s.append(newEl('option', { value: p, text: txt, disabled: unusable && p !== val }));
    }
    if (typeof val === 'number' && val < 0) s.append(newEl('option', { value: val, text: `GPIO ${-val} disabled (${val})` }));
    else if (typeof val === 'number' && !pins.includes(val)) s.append(newEl('option', { value: val, text: `GPIO ${val} (not on this chip)` }));
    s.value = typeof val === 'number' ? String(val) : '';
    s.addEventListener('change', () => setVal(obj, f, s.value === '' ? (f.required ? null : undefined) : Number(s.value)));
    return s;
  }

  // ── groups ──
  function renderOneOf(obj, f, prefix) {
    const row = newEl('div', { cls: 'fr' }, label(f, 'one of'));
    const cur = f.fields.find((x) => has(obj, x.name));
    const sel_ = newEl('select');
    if (!f.required || !cur) sel_.append(newEl('option', { value: '', text: f.required ? '(choose)' : '(not set)' }));
    for (const x of f.fields) sel_.append(newEl('option', { value: x.name, text: x.name.replace(/^refreshtime/, '') || x.name }));
    sel_.value = cur ? cur.name : '';
    const inp = newEl('input', { type: 'number', step: 'any' });
    inp.value = cur && typeof obj[cur.name] === 'number' ? obj[cur.name] : '';
    inp.disabled = !cur;
    const write = () => {
      for (const x of f.fields) delete obj[x.name];
      const x = f.fields.find((y) => y.name === sel_.value);
      if (x) obj[x.name] = inp.value === '' ? defaultValue(x) : Number(inp.value);
      changed();
    };
    sel_.addEventListener('change', () => { inp.disabled = !sel_.value; if (sel_.value && inp.value === '') inp.value = 1; write(); });
    inp.addEventListener('input', write);
    row.append(newEl('div', { cls: 'rfg' }, sel_, inp));
    row.append(newEl('div', { cls: 'fn', text: f.fields.map((x) => x.name).join(' / ') }));
    const err = newEl('div', { cls: 'err' });
    formErr[prefix + f.name] = err;
    f.fields.forEach((x) => { formErr[prefix + x.name] = err; });
    row.append(err);
    return row;
  }

  function renderAllOf(obj, f, prefix, byMode) {
    const box = newEl('div', { cls: 'grp-box' });
    const there = f.fields.some((x) => has(obj, x.name));
    const title = newEl('div', { cls: 'grp-title' }, f.name, newEl('span', { cls: 'pill pill-pur', text: byMode ? 'needed by the mode' : 'all or none' }));
    if (!byMode) {
      const toggle = newEl('input', { type: 'checkbox', title: 'use this group' });
      toggle.checked = there;
      toggle.addEventListener('change', () => {
        if (toggle.checked) addField(obj, f);
        else for (const x of f.fields) delete obj[x.name];
        changed({ form: true });
      });
      title.prepend(toggle);
    }
    box.append(title);
    const err = newEl('div', { cls: 'err' });
    formErr[prefix + f.name] = err;
    box.append(err);
    if (there) for (const x of f.fields) box.append(renderField(obj, x, prefix));
    return box;
  }

  function renderObject(obj, f, sub, prefix) {
    const box = newEl('div', { cls: 'grp-box' });
    const there = isObj(obj[f.name]);
    const title = newEl('div', { cls: 'grp-title' }, f.name, newEl('span', { cls: 'pill pill-cya', text: sub.name || 'object' }));
    if (!f.required) {
      const toggle = newEl('input', { type: 'checkbox', title: 'use this object' });
      toggle.checked = there;
      toggle.addEventListener('change', () => {
        if (toggle.checked) {
          const o = {};
          for (const x of fieldsOf(sub)) if (x.required) addField(o, x);
          obj[f.name] = o;
        } else delete obj[f.name];
        changed({ form: true });
      });
      title.prepend(toggle);
    }
    box.append(title);
    const err = newEl('div', { cls: 'err' });
    formErr[prefix + f.name] = err;
    box.append(err);
    if (there) renderFields(box, obj[f.name], sub, prefix + f.name + '.');
    else if (f.required) box.append(newEl('button', { cls: 'btn sml', text: '+ create', onclick: () => { obj[f.name] = {}; changed({ form: true }); } }));
    return box;
  }

  /** free JSON (Object without a known subtype, arrays of values) */
  function renderJsonField(obj, f, prefix) {
    const row = newEl('div', { cls: 'fr' }, label(f, f.type === 'ArrayOfPrimitives' ? 'array of values' : 'JSON'));
    const ta = newEl('textarea', { spellcheck: 'false', placeholder: f.type === 'ArrayOfPrimitives' ? '[1, 2, 3]' : '{ }' });
    ta.value = has(obj, f.name) ? JSON.stringify(obj[f.name], null, f.type === 'ArrayOfPrimitives' ? 0 : 2) : '';
    ta.addEventListener('input', () => {
      if (ta.value.trim() === '' && !f.required) { ta.classList.remove('bad'); setVal(obj, f, undefined); return; }
      try { const v = JSON.parse(ta.value); ta.classList.remove('bad'); setVal(obj, f, v); }
      catch (e) { ta.classList.add('bad'); }
    });
    row.append(ta);
    const err = newEl('div', { cls: 'err' });
    formErr[prefix + f.name] = err;
    row.append(err);
    return row;
  }

  // ── child items ──
  function renderChildArray(obj, f, ctx) {
    const wrap = newEl('div');
    const there = Array.isArray(obj[f.name]);
    const arr = there ? obj[f.name] : [];
    wrap.append(newEl('div', { cls: 'sec-title' }, f.name, f.required ? newEl('span', { cls: 'req', text: '*' }) : null,
      newEl('span', { cls: 'pill pill-grn', text: ctx.kind === 'reg' ? `types: ${ctx.regPath}` : (ctx.def.name || 'objects') })));
    const box = newEl('div', { cls: 'items-box' });
    box.append(newEl('div', { cls: 'items-hdr' }, 'items', newEl('span', { cls: 'cnt ic', text: arr.filter(isObj).length })));
    arr.forEach((child) => {
      if (!isObj(child)) {
        if (typeof child === 'string') box.append(newEl('div', { cls: 'ni comment' }, newEl('span', { cls: 'ni-uid', text: '// ' + child })));
        return;
      }
      const parents = [...sel.parents, { item: sel.item, ctx: sel.ctx }];
      const open = () => select(child, arr, ctx, parents);
      box.append(newEl('div', { cls: `ni${child.disabled === true ? ' dis' : ''}`, onclick: open },
        newEl('span', { cls: 'ni-uid', text: child.uid || '(no uid)' }),
        child.type ? newEl('span', { cls: 'ni-type', text: child.type }) : null,
        newEl('button', { cls: 'ni-btn', text: 'edit ›', onclick: (e) => { e.stopPropagation(); open(); } }),
        newEl('button', { cls: 'ni-btn ni-del', text: '✕', title: 'delete', onclick: (e) => {
          e.stopPropagation();
          if (!confirm(`Delete "${child.uid || '?'}"?`)) return;
          arr.splice(arr.indexOf(child), 1);
          changed({ form: true, tree: true });
        } })));
    });
    const add = newEl('div', { cls: 'items-add' });
    let typeSel = null;
    if (ctx.kind === 'reg') {
      typeSel = newEl('select');
      for (const [k, d] of Object.entries(SCHEMA.registers[ctx.regPath] || {})) typeSel.append(newEl('option', { value: k, text: `${k} – ${d.name}` }));
      add.append(typeSel);
    }
    add.append(newEl('button', { cls: 'btn pri sml', text: '+ add', onclick: () => {
      if (!Array.isArray(obj[f.name])) obj[f.name] = [];
      const n = newItem(obj[f.name], ctx, typeSel && typeSel.value);
      obj[f.name].push(n);
      changed({ tree: true });
      select(n, obj[f.name], ctx, [...sel.parents, { item: sel.item, ctx: sel.ctx }]);
    } }));
    box.append(add);
    wrap.append(box);
    const err = newEl('div', { cls: 'err' });
    formErr[f.name] = err;
    wrap.append(err);
    return wrap;
  }

  /** REGO600 style: every allowed value as a toggle chip */
  function renderChips(obj, f, sub) {
    const key = f.renderAllAllowedValuesFromStringConstraint;
    const vf = fieldsOf(sub).find((x) => x.name === key);
    const wrap = newEl('div');
    wrap.append(newEl('div', { cls: 'sec-title' }, f.name, newEl('span', { cls: 'pill pill-grn', text: `${key}: click to toggle` })));
    const grid = newEl('div', { cls: 'chips' });
    const arr = () => (Array.isArray(obj[f.name]) ? obj[f.name] : (obj[f.name] = []));
    for (const v of (vf && vf.allowedValues) || []) {
      const on = Array.isArray(obj[f.name]) && obj[f.name].some((r) => isObj(r) && String(r[key]).toUpperCase() === v.toUpperCase());
      grid.append(newEl('div', { cls: 'chip' + (on ? ' on' : ''), text: v, onclick: () => {
        const a = arr();
        const i = a.findIndex((r) => isObj(r) && String(r[key]).toUpperCase() === v.toUpperCase());
        if (i >= 0) a.splice(i, 1);
        else {
          const uid = v.toLowerCase().slice(0, UID_MAX); // the register name as uid, as in the examples
          a.push({ uid: a.some((r) => isObj(r) && r.uid === uid) ? freeUid(a, v) : uid, [key]: v });
        }
        changed({ form: true });
      } }));
    }
    wrap.append(grid);
    const err = newEl('div', { cls: 'err' });
    formErr[f.name] = err;
    wrap.append(err);
    return wrap;
  }

  /** keys the definition does not know */
  function renderExtraFields(container, obj, known) {
    const extra = Object.keys(obj).filter((k) => !known.has(k));
    if (!extra.length) return;
    container.append(newEl('div', { cls: 'sec-title' }, 'unknown fields', newEl('span', { cls: 'pill pill-yel', text: 'not in the schema' })));
    for (const k of extra) {
      const row = newEl('div', { cls: 'fr' }, newEl('label', { cls: 'unknown', text: k }));
      const ta = newEl('textarea', { spellcheck: 'false', rows: 1 });
      ta.value = JSON.stringify(obj[k]);
      ta.addEventListener('input', () => {
        try { obj[k] = JSON.parse(ta.value); ta.classList.remove('bad'); changed(); } catch (e) { ta.classList.add('bad'); }
      });
      row.append(newEl('div', { cls: 'fr-line' }, ta, newEl('button', { cls: 'clr', title: 'remove', text: '×', onclick: () => { delete obj[k]; changed({ form: true }); } })));
      const err = newEl('div', { cls: 'err' });
      formErr[k] = err;
      row.append(err);
      container.append(row);
    }
  }

  /** the validation messages of the selected item into its form */
  function renderFormErrors() {
    const msgs = new Map(); // element -> [problems]
    if (sel) {
      for (const p of problems) {
        if (p.item !== sel.item) continue;
        const e = formErr[p.field] || formErr[p.field.split('.')[0]];
        if (!e) continue;
        if (!msgs.has(e)) msgs.set(e, []);
        msgs.get(e).push(p);
      }
    }
    for (const e of new Set(Object.values(formErr))) {
      const list = msgs.get(e) || [];
      e.textContent = list.map((p) => p.msg).join(' · ');
      e.classList.toggle('warn', list.length > 0 && list.every((p) => p.level === 'warn'));
    }
  }

  // ═══════════════════════════════════════════════════════════════
  // PROBLEMS, JSON PANEL, CHANGES
  // ═══════════════════════════════════════════════════════════════
  function renderProblems() {
    const errs = problems.filter((p) => p.level === 'err').length;
    const warns = problems.length - errs;
    const b = $('cfg-problems');
    b.textContent = errs || warns ? `${errs} error${errs === 1 ? '' : 's'}, ${warns} warning${warns === 1 ? '' : 's'}` : '✓ no problems';
    b.className = 'btn sml ' + (errs ? 'dan' : warns ? 'wrn' : 'ok');
    const list = $('cfg-problem-list');
    list.innerHTML = '';
    for (const p of problems) {
      const where = p.item ? [...(p.parents || []).map((x) => x.item.uid), p.item.uid].join(':') : 'config';
      list.append(newEl('div', { cls: 'prob ' + p.level, onclick: () => { if (p.item) select(p.item, p.arr, p.ctx, p.parents); list.hidden = true; } },
        newEl('span', { cls: 'prob-where', text: where + (p.field && !p.field.startsWith('_') ? '.' + p.field : '') }), ' ', p.msg));
    }
  }

  function syncJson() {
    if (!jsonOpen) return;
    const ta = $('cfg-json');
    if (document.activeElement === ta) return; // do not overwrite while typing there
    ta.value = JSON.stringify(sel ? sel.item : cfg, null, 2);
    ta.classList.remove('bad');
    $('cfg-json-what').textContent = sel ? `JSON of ${sel.item.uid || 'item'}` : 'JSON of the whole config';
  }

  function refreshDirty() {
    const dirty = JSON.stringify(cfg) !== savedText;
    $('cfg-dirty').hidden = !dirty;
    return dirty;
  }

  /** after every change: validate, refresh what depends on it */
  function changed(opts = {}) {
    if (opts.tree) renderTree();
    if (opts.form) renderForm();
    else if (opts.header && sel) {
      const n = document.querySelector('#cfg-form .dname');
      if (n) n.textContent = sel.item.uid || '(no uid)';
      renderPath();
    }
    validate();
    syncJson();
    refreshDirty();
    const list = $('cfg-uidpaths');
    list.innerHTML = '';
    for (const p of allUidPaths()) list.append(newEl('option', { value: p }));
  }

  // ═══════════════════════════════════════════════════════════════
  // LOAD / SAVE / APPLY
  // ═══════════════════════════════════════════════════════════════
  /** one root device per line: readable, and not much bigger than compact JSON (the firmware
      reserves about twice the file size in RAM to parse it) */
  function cfgText() {
    return '[\n' + cfg.map((d) => '  ' + JSON.stringify(d)).join(',\n') + '\n]\n';
  }

  async function loadFromDevice(force = false) {
    if (!force && refreshDirty() && !confirm('Discard the unsaved changes and load the config from the device?')) return;
    status('loading the config from the device…', 'warn');
    try {
      const text = await http('GET', fsUrl(CFG_FILE));
      const parsed = JSON.parse(text);
      if (!Array.isArray(parsed)) throw new Error('cfg.json is not a JSON array');
      setCfg(parsed);
      status(`loaded ${CFG_DIR}/${CFG_FILE}: ${cfg.filter(isObj).length} root devices`);
    } catch (e) {
      if (String(e.message).startsWith('404')) { setCfg([]); status(`${CFG_DIR}/${CFG_FILE} does not exist yet, start with an empty config`, 'warn'); }
      else status(`could not load the config: ${e.message}`, 'err');
    }
  }

  function setCfg(arr) {
    cfg = arr;
    savedText = JSON.stringify(cfg);
    sel = null;
    renderTree();
    renderForm();
    changed();
  }

  async function writeFile(name, text) {
    await http('PUT', fsUrl(name), new Blob([text], { type: 'application/octet-stream' }));
  }

  async function saveOnly() {
    const errs = problems.filter((p) => p.level === 'err').length;
    if (errs && !confirm(`The config has ${errs} error(s), the firmware will not load it at the next start.\n\nSave it anyway?`)) return;
    try {
      await writeFile(CFG_FILE, cfgText());
      savedText = JSON.stringify(cfg);
      refreshDirty();
      status(`saved ${CFG_DIR}/${CFG_FILE} (used at the next start, or with apply)`);
    } catch (e) { status(`save failed: ${e.message}`, 'err'); }
  }

  /** log lines of the firmware that are new since before (the log is a ring buffer) */
  function newLogLines(before, after) {
    const seen = new Map();
    for (const l of before) seen.set(l, (seen.get(l) || 0) + 1);
    return after.filter((l) => { const n = seen.get(l) || 0; if (n) { seen.set(l, n - 1); return false; } return true; });
  }
  const logLines = (t) => t.split(/\r?\n/).map((l) => l.trim()).filter(Boolean);

  async function apply() {
    const errs = problems.filter((p) => p.level === 'err').length;
    if (errs && !confirm(`The editor found ${errs} error(s).\n\nSend it to the device anyway? The device checks it again and keeps the running config if it is not valid.`)) return;
    const text = cfgText();
    showDeviceLog(null);
    try {
      status('writing the new config…', 'warn');
      await writeFile(CFG_TEST_FILE, text);
      const before = logLines(await wsRequest('printLog', 'logs').catch(() => ''));
      status('the device checks and loads the new config…', 'warn');
      const answer = JSON.parse(await wsRequest(`hal/config/reload/${CFG_TEST_FILE}`, 'reloadJSON', 30000));
      const after = logLines(await wsRequest('printLog', 'logs').catch(() => ''));
      const fresh = newLogLines(before, after);
      if (answer.info === 'OK') {
        await writeFile(CFG_FILE, text);
        await http('DELETE', fsUrl(CFG_TEST_FILE)).catch(() => {});
        savedText = JSON.stringify(cfg);
        refreshDirty();
        status(`applied: the device runs the new config, saved to ${CFG_DIR}/${CFG_FILE}`);
        showDeviceLog(fresh, false);
      } else {
        await http('DELETE', fsUrl(CFG_TEST_FILE)).catch(() => {});
        status('the device did not accept the config, it keeps running the old one (see the device log)', 'err');
        showDeviceLog(fresh, true);
      }
    } catch (e) {
      status(`apply failed: ${e.message}`, 'err');
    }
  }

  function showDeviceLog(lines, failed) {
    const box = $('cfg-devlog');
    if (!lines) { box.hidden = true; return; }
    box.hidden = false;
    box.className = failed ? 'failed' : '';
    box.innerHTML = '';
    box.append(newEl('div', { cls: 'devlog-title' }, failed ? 'device log: why the config was not accepted' : 'device log', newEl('button', { cls: 'clr', text: '×', onclick: () => { box.hidden = true; } })));
    const shown = lines.filter((l) => !/raw (u)?int value converted/.test(l)); // conversion info is noise here
    for (const l of shown.length ? shown : ['(no new log entries)']) {
      box.append(newEl('div', { cls: /\[ERR\]/.test(l) ? 'l-err' : /\[WARN\]/.test(l) ? 'l-warn' : '', text: l }));
    }
  }

  // ═══════════════════════════════════════════════════════════════
  // TOOLBAR, SIDEBAR ACTIONS
  // ═══════════════════════════════════════════════════════════════
  function wire() {
    $('cfg-reload').onclick = loadFromDevice;
    $('cfg-save').onclick = saveOnly;
    $('cfg-apply').onclick = apply;
    $('cfg-problems').onclick = () => { $('cfg-problem-list').hidden = !$('cfg-problem-list').hidden || !problems.length; };
    $('cfg-export').onclick = () => {
      const a = newEl('a', { href: URL.createObjectURL(new Blob([cfgText()], { type: 'application/json' })), download: 'cfg.json' });
      document.body.append(a); a.click(); a.remove();
    };
    $('cfg-import').onclick = () => $('cfg-import-file').click();
    $('cfg-import-file').onchange = async (e) => {
      const f = e.target.files[0];
      e.target.value = '';
      if (!f) return;
      try {
        const parsed = JSON.parse(await f.text());
        if (!Array.isArray(parsed)) throw new Error('not a JSON array');
        if (refreshDirty() && !confirm('Replace the config in the editor (not saved yet)?')) return;
        cfg = parsed;
        sel = null;
        renderTree(); renderForm(); changed();
        status(`imported ${f.name} into the editor, not saved on the device yet`, 'warn');
      } catch (err) { status(`import failed: ${err.message}`, 'err'); }
    };
    $('cfg-json-btn').onclick = () => {
      jsonOpen = !jsonOpen;
      $('cfg-json-panel').hidden = !jsonOpen;
      $('cfg-json-btn').classList.toggle('on', jsonOpen);
      syncJson();
    };
    $('cfg-json').addEventListener('input', (e) => {
      const ta = e.target;
      try {
        const v = JSON.parse(ta.value);
        if (sel) {
          if (!isObj(v)) throw new Error('an item must be an object');
          for (const k of Object.keys(sel.item)) delete sel.item[k]; // keep the object, the tree refers to it
          Object.assign(sel.item, v);
        } else {
          if (!Array.isArray(v)) throw new Error('the config must be an array');
          cfg.splice(0, cfg.length, ...v);
        }
        ta.classList.remove('bad');
        changed({ tree: true, form: true });
      } catch (err) { ta.classList.add('bad'); }
    });
    $('cfg-add').onclick = () => {
      const t = $('cfg-add-type').value;
      const root = { kind: 'reg', regPath: 'ROOT' };
      const n = newItem(cfg, root, t);
      cfg.push(n);
      changed({ tree: true });
      select(n, cfg, root, []);
    };
    $('cfg-add-comment').onclick = () => {
      const t = prompt('Comment (a text item, ignored by the firmware):');
      if (!t) return;
      const i = sel && sel.arr === cfg ? cfg.indexOf(sel.item) + 1 : cfg.length;
      cfg.splice(i, 0, t);
      changed({ tree: true });
    };
    $('cfg-del').onclick = () => {
      if (!sel || !confirm(`Delete "${sel.item.uid || '?'}"${childArrays(defOf(sel.item, sel.ctx)).length ? ' with everything in it' : ''}?`)) return;
      sel.arr.splice(sel.arr.indexOf(sel.item), 1);
      const p = sel.parents[sel.parents.length - 1];
      sel = null;
      changed({ tree: true, form: true });
      status('deleted');
      if (p) treeRows.get(p.item)?.click();
    };
    $('cfg-dup').onclick = () => {
      if (!sel) return;
      const copy = JSON.parse(JSON.stringify(sel.item));
      copy.uid = freeUid(sel.arr, String(sel.item.uid || 'copy').replace(/\d+$/, ''));
      sel.arr.splice(sel.arr.indexOf(sel.item) + 1, 0, copy);
      changed({ tree: true });
      select(copy, sel.arr, sel.ctx, sel.parents);
    };
    const move = (d) => {
      if (!sel) return;
      const i = sel.arr.indexOf(sel.item), j = i + d;
      if (j < 0 || j >= sel.arr.length) return;
      [sel.arr[i], sel.arr[j]] = [sel.arr[j], sel.arr[i]];
      changed({ tree: true, form: true });
    };
    $('cfg-up').onclick = () => move(-1);
    $('cfg-down').onclick = () => move(1);
    window.addEventListener('beforeunload', (e) => { if (cfg && refreshDirty()) { e.preventDefault(); e.returnValue = ''; } });
    document.addEventListener('click', (e) => {
      if (!e.target.closest('#cfg-problem-list') && e.target.id !== 'cfg-problems') $('cfg-problem-list').hidden = true;
    });
  }

  // ═══════════════════════════════════════════════════════════════
  // INIT
  // ═══════════════════════════════════════════════════════════════
  async function waitForWs() {
    // ws is a global "let" of main.js: visible here, but not as window.ws
    for (let i = 0; i < 100 && !(typeof ws !== 'undefined' && ws && ws.readyState === WebSocket.OPEN); i++) await new Promise((r) => setTimeout(r, 100));
  }

  async function init() {
    wire();
    renderForm();
    status('loading the schema from the firmware…', 'warn');
    try {
      await waitForWs();
      initSchema(JSON.parse(await wsRequest('hal/meta/reg/cfgschema', 'schema', 30000)));
      try { GPIO = JSON.parse(await wsRequest('hal/meta/gpio', 'getAvailableGPIOs')); } catch (e) { GPIO = null; }
    } catch (e) {
      status(`could not get the schema from the device: ${e.message}`, 'err');
      return;
    }
    fillAddSelect();
    await loadFromDevice(true);
    openFromHash();
  }

  /** #config=uid:uid opens that item (bookmarkable, also when only the hash changes) */
  function openFromHash() {
    if (!SCHEMA || !location.hash.startsWith('#config=')) return;
    const want = decodeURIComponent(location.hash.slice(8));
    walk(cfg, { kind: 'reg', regPath: 'ROOT' }, [], (item, arr, ctx, parents) => {
      if ([...parents.map((p) => p.item.uid), item.uid].join(':') === want && (!sel || sel.item !== item)) {
        select(item, arr, ctx, parents);
        treeRows.get(item)?.scrollIntoView({ block: 'nearest' });
      }
    });
  }

  window.ConfigTab = {
    onShow: openFromHash,
    // for tests: the editor state and actions
    _state: () => ({ cfg, problems: problems.map((p) => ({ where: p.item ? [...(p.parents || []).map((x) => x.item.uid), p.item.uid].join(':') : '', field: p.field, msg: p.msg, level: p.level })) }),
    _apply: apply,
    _text: () => cfgText(),
    _select: (path) => { window.history.replaceState(null, '', '#config=' + path); openFromHash(); },
    _sel: () => sel && sel.item,
    _changed: () => changed({ tree: true, form: true }),
    _debug: () => { const d = defOf(sel.item, sel.ctx); return { mode: d.modes ? evalMode(sel.item, d) : null, conj: d.modes ? d.modes.map((m) => m.conjunctions.map((c) => c.name + (c.required ? '*' : '') + '=' + present(sel.item, fieldByName(d, c.name)))) : null, check: checkObject(sel.item, d, { uidPaths: new Set() }) }; },
  };
  init();
})();

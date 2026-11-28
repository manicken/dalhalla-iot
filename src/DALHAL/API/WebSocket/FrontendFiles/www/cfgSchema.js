// ── cfgSchema: the firmware cfg schema, shared by the Config and the Import/Export tab ────────────
// The schema comes from the running firmware (hal/meta/reg/cfgschema, tag "schema"), so it always matches it:
//   { registers: { regPath: { TYPE: def } }, objects: [def], ByReference: [group] }
// A field can have gui flags ("gui": [...], DALHAL_JSON_Schema_FieldGuiFlags), used here:
//   Secret    the value is masked in the editor (a password, an api key)
//   NoExport  the field is left out of an export, so a config can be shared without its secrets
//
//   CfgSchema.load()                    -> Promise<raw>, asks the device once, later calls get the same
//   CfgSchema.init(raw)                 the schema to use (load() calls it)
//   CfgSchema.fieldsOf(def), typeDef(regPath, type), objectDef(subtype), defOf(item, ctx)
//   CfgSchema.hasGui(f, flag)           has the field (or the group it is in) that gui flag
//   CfgSchema.walk(cfg, visit)          visit(item, def, path) for every item, path = 'uid:uid' (a list index when no uid)
//   CfgSchema.flagged(cfg, flag)        [{ path, key }] of the fields with that flag that are in the cfg
//   CfgSchema.strip(cfg, flag)          a copy of cfg without the fields with that flag, + the list of what was removed;
//                                       a required one gets its default value instead (the cfg stays valid), defaulted: true
//   CfgSchema.fillFrom(cfg, old, flag, replace)  puts the flagged fields that cfg does not have from old (same item path)
//                                       into cfg; the ones in replace ([{ path, key }], defaulted by strip) also when cfg has them
//   CfgSchema.format(cfg)               cfg.json text: one root item per line, child item arrays one item per line

window.CfgSchema = (() => {
  const isObj = (v) => v !== null && typeof v === 'object' && !Array.isArray(v);
  const has = (o, k) => isObj(o) && Object.prototype.hasOwnProperty.call(o, k);
  const ROOT = { kind: 'reg', regPath: 'ROOT' };

  let SCHEMA = null;
  let loading = null;
  const byRef = {};
  const objects = {};
  const defCache = new WeakMap();

  function init(raw) {
    SCHEMA = raw;
    for (const r of raw.ByReference || []) byRef[r.name] = r;
    for (const o of raw.objects || []) objects[o.name] = o;
    return raw;
  }

  function load() {
    if (!loading) {
      loading = wsRequest('hal/meta/reg/cfgschema', 'schema', 30000).then((t) => init(JSON.parse(t)));
      loading.catch(() => { loading = null; }); // try again next time
    }
    return loading;
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

  /** the expanded fields of a type definition (cached) */
  function fieldsOf(def) {
    if (!def) return [];
    if (!defCache.has(def)) defCache.set(def, expand(def.fields));
    return defCache.get(def);
  }

  function typeDef(regPath, type) {
    const group = SCHEMA && SCHEMA.registers[regPath];
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

  const hasGui = (f, flag, group) => (f.gui || []).includes(flag) || (!!group && (group.gui || []).includes(flag));

  /** the plain fields of a def with the group they are in (OneOf/AllOf groups opened) */
  function leafFields(def) {
    const out = [];
    for (const f of fieldsOf(def)) {
      if (f.type === 'OneOfFieldsGroup' || f.type === 'AllOfFieldsGroup') for (const x of f.fields) out.push({ f: x, group: f });
      else out.push({ f, group: null });
    }
    return out;
  }

  /** visit(item, def, path) for every item of the cfg and below it (registry items, object arrays, objects) */
  function walk(cfg, visit) {
    const items = (arr, ctx, base) => {
      if (!Array.isArray(arr)) return;
      arr.forEach((item, i) => one(item, ctx, base, i));
    };
    const one = (item, ctx, base, i) => {
      if (!isObj(item)) return; // a string item is a comment
      const def = defOf(item, ctx);
      const name = typeof item.uid === 'string' && item.uid ? item.uid : `#${i}`;
      const path = base ? `${base}:${name}` : name;
      visit(item, def, path);
      if (!def) return;
      for (const { f } of leafFields(def)) {
        const v = item[f.name];
        if (f.type === 'ArrayOfRegistryItems') items(v, { kind: 'reg', regPath: f.regPath }, path);
        else if (f.type === 'ArrayOfObjects' && objectDef(f.subtype)) items(v, { kind: 'obj', def: objectDef(f.subtype) }, path);
        else if (f.type === 'Object' && objectDef(f.subtype) && isObj(v)) one(v, { kind: 'obj', def: objectDef(f.subtype) }, path, f.name);
      }
    };
    items(cfg, ROOT, '');
  }

  /** [{ path, key }] of the fields with the flag that are in the cfg */
  function flagged(cfg, flag) {
    const out = [];
    walk(cfg, (item, def, path) => {
      for (const { f, group } of leafFields(def)) if (hasGui(f, flag, group) && has(item, f.name)) out.push({ path, key: f.name });
    });
    return out;
  }

  /** the default of a field: the schema default, else an empty value of its type */
  function defaultOf(f) {
    if (f.default !== undefined && f.default !== null) return f.default;
    if (f.type === 'Bool') return false;
    if (['UInt', 'Int', 'Float', 'Number'].includes(f.type)) return f.minValue || 0;
    return '';
  }

  /** a deep copy of cfg without the values of the fields that have the flag; removed = [{ path, key, defaulted }]:
   *  a required field gets its default value (so the cfg stays valid, it is changed after the import), others are left out */
  function strip(cfg, flag) {
    const copy = JSON.parse(JSON.stringify(cfg));
    const removed = [];
    walk(copy, (item, def, path) => {
      for (const { f, group } of leafFields(def)) {
        if (!hasGui(f, flag, group) || !has(item, f.name)) continue;
        if (f.required) { item[f.name] = defaultOf(f); removed.push({ path, key: f.name, defaulted: true }); }
        else { delete item[f.name]; removed.push({ path, key: f.name }); }
      }
    });
    return { cfg: copy, removed };
  }

  /** the flagged fields that cfg does not have are taken from old (the item with the same path);
   *  filled = [{ path, key }] taken over, missing = [{ path, key }] flagged fields old does not have either */
  function fillFrom(cfg, old, flag, replace = []) {
    const oldItems = new Map();
    walk(old || [], (item, def, path) => oldItems.set(path, item));
    const again = new Set(replace.map((r) => `${r.path}.${r.key}`));
    const filled = [];
    const missing = [];
    walk(cfg, (item, def, path) => {
      const o = oldItems.get(path);
      for (const { f, group } of leafFields(def)) {
        if (!hasGui(f, flag, group) || (has(item, f.name) && !again.has(`${path}.${f.name}`))) continue;
        if (o && has(o, f.name) && (!o.type || o.type === item.type)) { item[f.name] = o[f.name]; filled.push({ path, key: f.name }); }
        else if (f.required || group) missing.push({ path, key: f.name });
      }
    });
    return { filled, missing };
  }

  /** cfg.json text: one root item per line, arrays of child items one item per line (easier to read and diff) */
  function format(cfg) {
    const isItemArray = (v) => Array.isArray(v) && v.some(isObj);
    const fmt = (v, indent) => {
      if (!isObj(v)) return JSON.stringify(v);
      const parts = Object.entries(v).map(([k, x]) => JSON.stringify(k) + ':' + (isItemArray(x)
        ? '[\n' + x.map((c) => ' '.repeat(indent + 2) + fmt(c, indent + 2)).join(',\n') + '\n' + ' '.repeat(indent) + ']'
        : JSON.stringify(x)));
      return '{' + parts.join(',') + '}';
    };
    return '[\n' + cfg.map((d) => '  ' + fmt(d, 2)).join(',\n') + '\n]\n';
  }

  return {
    load, init, get raw() { return SCHEMA; },
    expand, fieldsOf, typeDef, objectDef, defOf, hasGui, walk, flagged, strip, fillFrom, format,
  };
})();

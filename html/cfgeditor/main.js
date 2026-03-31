let basicTypes = ["String","UID","HardwarePin","Number","UInt","Float","Bool", "HexBytes"];

// ================= STATE =================
let devices = [];
let expanded = {}; // key: index → bool
let editIndex = -1; // -1 = add mode, otherwise editing

let currentEditingDevice = null; // the device object currently in the modal
let currentEditingSchema = null; // the schema for that device type
let currentMode = null;           // currently selected mode in modal

// ================= UTIL =================
function createInput(field, obj) {
  let input;

  if (field.allowedValues) {
    input = document.createElement("select");
    field.allowedValues.forEach(v => {
      const o = document.createElement("option");
      o.value = v;
      o.textContent = v;
      input.appendChild(o);
    });
    input.value = obj[field.name] ?? field.default ?? "";
    input.onchange = e => obj[field.name] = e.target.value;
  }
  else if (field.type === "Bool") {
    input = document.createElement("input");
    input.type = "checkbox";
    input.checked = obj[field.name] ?? field.default ?? false;
    input.onchange = e => obj[field.name] = e.target.checked;
  }
  else if (["Number","UInt","Float"].includes(field.type)) {
    input = document.createElement("input");
    input.type = "number";
    input.value = obj[field.name] ?? field.default ?? 0;
    input.oninput = e => obj[field.name] = Number(e.target.value);
  }
  else {
    input = document.createElement("input");
    input.type = "text";
    input.value = obj[field.name] ?? field.default ?? "";
    input.oninput = e => obj[field.name] = e.target.value;
  }

  return input;
}

// Render OneOfGroup
function renderOneOfGroup(field, parentObj, container, path = "") {
    const wrapper = document.createElement("div");

    // ensure parentObj exists
    if (!parentObj[field.name] || typeof parentObj[field.name] !== "object") {
        parentObj[field.name] = {};
    }

    const select = document.createElement("select");
    field.fields.forEach((f, idx) => {
        const opt = document.createElement("option");
        opt.value = idx;
        opt.textContent = f.name;
        select.appendChild(opt);
    });
    wrapper.appendChild(select);

    // render the variant container
    const variantContainer = document.createElement("div");
    wrapper.appendChild(variantContainer);

    function renderVariant(idx) {
        variantContainer.innerHTML = ""; // clear previous
        const variantField = field.fields[idx];
        if (!parentObj[field.name][variantField.name]) {
            parentObj[field.name][variantField.name] = {};
        }
        renderField(variantField, parentObj[field.name][variantField.name], variantContainer, path + field.name + ".");
    }

    // initial render
    renderVariant(0);

    select.addEventListener("change", () => {
        renderVariant(parseInt(select.value));
    });

    container.appendChild(wrapper);
}

// ================= RECURSIVE RENDER =================
function renderField(field, parentObj, container, path = "") {
    
    if (parentObj === null) {
        console.error("Found null item @ path: " + path);
        return;
    }
    // If parentObj is a primitive, we assume it's a raw JSON comment
    // (only for humans editing the JSON), so we skip rendering it in the GUI.
    if (typeof parentObj !== "object") {
        console.log("Skipping comment item @ path " + path + ": " + parentObj);
        return;
    }
    
    const wrapper = document.createElement("div");
    wrapper.style.marginLeft = "10px";

    const label = document.createElement("label");
    label.textContent = field.name;
    wrapper.appendChild(label);

    // ===== BASIC TYPES =====
    if (!field.type || basicTypes.includes(field.type)) {
        const input = createInput(field, parentObj);
        input.id = "field_" + path + field.name;
        wrapper.appendChild(input);
    }
    // ===== OBJECT =====
    else if (field.type === "Object" || field.type === "object") {
        if (!parentObj[field.name]) parentObj[field.name] = {};
        const sub = document.createElement("div");
        sub.style.borderLeft = "2px solid #ccc";
        field.subtype.fields.forEach(f => {
        renderField(f, parentObj[field.name], sub, path + field.name + ".");
        });
        wrapper.appendChild(sub);
    }
    // ===== ARRAY =====
    else if (field.type === "Array") {
        if (!Array.isArray(parentObj[field.name])) parentObj[field.name] = [];
        const list = document.createElement("div");

        parentObj[field.name].forEach((item, idx) => {
        const itemDiv = document.createElement("div");
        itemDiv.style.border = "1px solid #aaa";
        itemDiv.style.margin = "5px";
        field.subtype.fields.forEach(f => renderField(f, item, itemDiv, path + field.name + idx + "."));
        list.appendChild(itemDiv);
        });
        const btn = document.createElement("button");
        btn.textContent = "Add Item";
        btn.onclick = () => {
        const item = {};
        parentObj[field.name].push(item);
        renderForm();
        };
        wrapper.appendChild(btn);
        wrapper.appendChild(list);
    }
    // ===== REGISTRY ARRAY =====
    else if (field.type === "RegistryArray") {
        if (!parentObj[field.name]) parentObj[field.name] = [];
        const registry = schema[field.regPath];
        const list = document.createElement("div");
        const select = document.createElement("select");
        Object.keys(registry).forEach(k => {
        const o = document.createElement("option");
        o.value = k;
        o.textContent = k;
        select.appendChild(o);
        });
        const btn = document.createElement("button");
        btn.textContent = "Add";
        btn.onclick = () => {
        const type = select.value;
        const def = registry[type];
        const item = { __type: type };
        parentObj[field.name].push(item);
        const itemDiv = document.createElement("div");
        itemDiv.style.border = "1px solid #888";
        def.fields.forEach(f => renderField(f, item, itemDiv, path + field.name + "."));
        list.appendChild(itemDiv);
        };
        wrapper.appendChild(select);
        wrapper.appendChild(btn);
        wrapper.appendChild(list);
    }
    // ===== ANY OF GROUP =====
    else if (field.type === "OneOfGroup") {
        renderOneOfGroup(field, parentObj, container, path);
        /*
        if (!parentObj[field.name]) {
            parentObj[field.name] = {};
        }

        const select = document.createElement("select");
        const sub = document.createElement("div");

        // build options from field.fields
        field.fields.forEach((f, idx) => {
            const o = document.createElement("option");
            o.value = idx;
            o.textContent = f.name;
            select.appendChild(o);
        });

        // determine initial selection
        let selectedIndex = 0;
        const existingKeys = Object.keys(parentObj[field.name]);
        if (existingKeys.length > 0) {
            const match = field.fields.findIndex(f => existingKeys.includes(f.name));
            if (match >= 0) selectedIndex = match;
        }
        select.value = selectedIndex;

        // initial render for that variant
        const initField = field.fields[selectedIndex];
        // if no object exists yet for that variant, create it
        if (!parentObj[field.name][initField.name]) {
            parentObj[field.name][initField.name] = {};
        }
        renderField(initField, parentObj[field.name][initField.name], sub, path + field.name + ".");

        // on change, re-render
        select.onchange = () => {
            sub.innerHTML = "";
            const idx = select.value;
            const f = field.fields[idx];

            // reset to only that variant
            parentObj[field.name] = { [f.name]: {} };

            renderField(f, parentObj[field.name][f.name], sub, path + field.name + ".");
        };

        wrapper.appendChild(select);
        wrapper.appendChild(sub);
        */
    }
    // ===== ALL OF GROUP =====
    else if (field.type === "AllOfGroup") {
        if (!parentObj[field.name]) parentObj[field.name] = {};
        const sub = document.createElement("div");
        field.fields.forEach(f => renderField(f, parentObj[field.name], sub, path + field.name + "."));
        wrapper.appendChild(sub);
    }

    container.appendChild(wrapper);
}

// ================= MODAL =================
function openAddModal() {
  editIndex = -1;
  const sel = document.getElementById("typeSelect");
  if (sel.options.length === 0) initTypes();
  const type = sel.value || Object.keys(schema["ROOT"])[0];
  currentEditingSchema = schema["ROOT"][type];
  currentEditingDevice = { __type: type, type: type };
  setupModeUI();
  document.getElementById("modal").style.display = "block";
  renderForm();
}

function closeModal() {
  document.getElementById("modal").style.display = "none";
}

function renderForm() {
  const formArea = document.getElementById("formArea");
  formArea.innerHTML = "";
  currentEditingSchema.fields.forEach(f => {
    if (!fieldVisibleInMode(f.name, currentEditingSchema)) return;
    renderField(f, currentEditingDevice, formArea);
  });
}

function confirmAdd() {
  if (editIndex >= 0) {
    devices[editIndex] = currentEditingDevice;
  } else {
    devices.push(currentEditingDevice);
  }
  renderTable();
  closeModal();
}

// ================= TABLE =================
function renderTable() {
  const tbody = document.getElementById("deviceTable");
  tbody.innerHTML = "";
  devices.forEach((dev, i) => renderRow(tbody, dev, i, 0));
}

function isComplex(val) {
  return typeof val === "object" && val !== null;
}

function toggleExpand(key) {
  expanded[key] = !expanded[key];
  renderTable();
}

function renderRow(tbody, dev, index, level, parentKey = "") {
  const key = parentKey + index;
  const tr = document.createElement("tr");
  const hasChildren = Object.values(dev).some(v => isComplex(v));
  let expandBtn = "";
  if (hasChildren) expandBtn = `<button onclick="toggleExpand('${key}')">${expanded[key] ? "▼" : "▶"}</button>`;
  const type = dev.__type || "";
  const uid = dev.uid || "";
  const note = dev.note || "";
  let data = "";
  Object.keys(dev).forEach(k => {
    if (["__type","uid","note","type"].includes(k)) return;
    const v = dev[k];
    data += isComplex(v) ? (Array.isArray(v) ? `${k}:[...] ` : `${k}:{...} `) : `${k}:${v} `;
  });
  const enabled = dev.disabled ? "" : "✓";
  tr.innerHTML = `
    <td>${enabled}</td>
    <td style="padding-left:${level*20}px">${expandBtn} ${type}</td>
    <td>${uid}</td>
    <td>${data}</td>
    <td>${note}</td>
    <td>
      <button onclick="editDevice(${index})">Edit</button>
      <button onclick="removeDevice(${index})">Remove</button>
    </td>
  `;
  tbody.appendChild(tr);
  if (expanded[key]) {
    Object.keys(dev).forEach(k => {
      const v = dev[k];
      if (Array.isArray(v)) v.forEach((child, idx) => isComplex(child) && renderRow(tbody, child, idx, level+1, key + "." + k));
      else if (isComplex(v)) renderRow(tbody, v, 0, level+1, key + "." + k);
    });
  }
}

function removeDevice(i) {
  devices.splice(i,1);
  renderTable();
}

// ================= MODES =================
function initModes(schema) {
  const sel = document.getElementById("modeSelect");
  sel.innerHTML = "";
  schema.modes?.forEach(m => {
    const o = document.createElement("option");
    o.value = m.name;
    o.textContent = m.name;
    sel.appendChild(o);
  });
  currentMode = schema.modes?.[0]?.name || null;
}

function changeMode() {
  currentMode = document.getElementById("modeSelect").value;
  renderForm();
}

function fieldVisibleInMode(fieldName, schema) {
  if (!currentMode) return true;
  const mode = schema.modes?.find(m => m.name === currentMode);
  if (!mode) return true;
  const conj = mode.conjunctions?.find(c => c.name === fieldName);
  return conj ? conj.required : true;
}

// ================= TYPE =================
function changeType() {
  const sel = document.getElementById("typeSelect");
  const type = sel.value;
  currentEditingSchema = schema["ROOT"][type];
  currentEditingDevice = { __type: type, type: type };
  currentMode = null;
  setupModeUI();
  renderForm();
}

// ================= EDIT =================
function editDevice(index) {
  editIndex = index;
  const dev = devices[index];
  currentEditingDevice = JSON.parse(JSON.stringify(dev));
  const sel = document.getElementById("typeSelect");
  sel.value = dev.__type;
  currentEditingSchema = schema["ROOT"][dev.__type];
  setupModeUI();
  document.getElementById("modal").style.display = "block";
  renderForm();
}

function setupModeUI() {
  const container = document.getElementById("modeContainer");
  if (currentEditingSchema.modes?.length) {
    container.style.display = "block";
    initModes(currentEditingSchema);
  } else {
    container.style.display = "none";
    currentMode = null;
  }
}

// ================= LOAD / SAVE =================
function loadJSON() {
  const txt = document.getElementById("loadBox").value;
  try {
    const json = JSON.parse(txt);
    if (!Array.isArray(json)) throw new Error("Root must be an array");
    devices = json.map(obj => {
      if (!obj.type) return null;
      return { ...obj, __type: obj.type };
    }).filter(Boolean);
    renderTable();
  } catch(e) { console.error(e); alert("Invalid JSON"); }
}

function saveJSON() {
  const out = devices.map(d => {
    const copy = {...d};
    copy.type = d.__type;
    delete copy.__type;
    return copy;
  });
  document.getElementById("saveBox").value = JSON.stringify(out,null,2);
}

// ================= INIT =================
function initTypes() {
  const sel = document.getElementById("typeSelect");
  sel.innerHTML = "";
  Object.keys(schema["ROOT"]).forEach(k => {
    const o = document.createElement("option");
    o.value = k;
    o.textContent = k;
    sel.appendChild(o);
  });
}

document.addEventListener("DOMContentLoaded", () => {
  initTypes();
  renderTable();
  if (typeof examplecfg !== "undefined") {
    document.getElementById("loadBox").value = JSON.stringify(examplecfg);
    loadJSON();
  }
});

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
    input.onchange = e => obj[field.name] = e.target.value;
  }
  else if (field.type === "Bool") {
    input = document.createElement("input");
    input.type = "checkbox";
    input.onchange = e => obj[field.name] = e.target.checked;
  }
  else if (["Number","UInt","Float"].includes(field.type)) {
    input = document.createElement("input");
    input.type = "number";
    input.oninput = e => obj[field.name] = Number(e.target.value);
  }
  else {
    input = document.createElement("input");
    input.type = "text";
    input.oninput = e => obj[field.name] = e.target.value;
  }

  return input;
}

// ================= RECURSIVE RENDER =================

function renderField(field, parentObj, container, path = "") {

  const wrapper = document.createElement("div");
  wrapper.style.marginLeft = "10px";

  const label = document.createElement("label");
  label.textContent = field.name;
  wrapper.appendChild(label);

  // ===== BASIC TYPES =====
  if (!field.type || ["String","UID","HardwarePin","Number","UInt","Float","Bool"].includes(field.type)) {
    const input = createInput(field, parentObj);
    wrapper.appendChild(input);
  }

  // ===== OBJECT =====
  else if (field.type === "Object" || field.type === "object") {
    if (!parentObj[field.name]) parentObj[field.name] = {};

    const sub = document.createElement("div");
    sub.style.borderLeft = "2px solid #ccc";

    field.subtype.fields.forEach(f => {
      renderField(f, parentObj[field.name], sub, path + "." + field.name);
    });

    wrapper.appendChild(sub);
  }

  // ===== ARRAY =====
  else if (field.type === "Array") {
    if (!parentObj[field.name]) parentObj[field.name] = [];

    const list = document.createElement("div");

    // render existing items
    parentObj[field.name].forEach(item => {
      const itemDiv = document.createElement("div");
      itemDiv.style.border = "1px solid #aaa";
      itemDiv.style.margin = "5px";

      field.subtype.fields.forEach(f => {
        renderField(f, item, itemDiv, path + "." + field.name);
      });

      list.appendChild(itemDiv);
    });

    const btn = document.createElement("button");
    btn.textContent = "Add Item";
    btn.onclick = () => {
      const item = {};
      parentObj[field.name].push(item);

      renderForm(); // simpler than manual DOM patching
    };

    wrapper.appendChild(btn);
    wrapper.appendChild(list);
  }
  else if (field.type === "AllOfGroup") {
    if (!parentObj[field.name]) parentObj[field.name] = {};

    const sub = document.createElement("div");

    field.fields.forEach(f => {
      renderField(f, parentObj[field.name], sub, path);
    });

    wrapper.appendChild(sub);
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

      def.fields.forEach(f => {
        renderField(f, item, itemDiv, path + "." + field.name);
      });

      list.appendChild(itemDiv);
    };

    wrapper.appendChild(select);
    wrapper.appendChild(btn);
    wrapper.appendChild(list);
  }

  container.appendChild(wrapper);
}

// ================= MODAL =================

function openAddModal() {
  editIndex = -1;

  const sel = document.getElementById("typeSelect");

  // Ensure types are initialized (safety)
  if (sel.options.length === 0) {
    initTypes();
  }

  const type = sel.value || Object.keys(schema["ROOT"])[0];

  // Set schema
  currentEditingSchema = schema["ROOT"][type];

  // Create fresh device
  currentEditingDevice = {
    __type: type,
    type: type
  };

  // Setup mode UI
  setupModeUI();

  // Open modal
  document.getElementById("modal").style.display = "block";

  // Render form
  renderForm();
}

function closeModal(){
  document.getElementById("modal").style.display="none";
}
/*
function renderForm() {
  const schema = currentEditingSchema; // currently editing device type schema
  const formArea = document.getElementById("formArea");
  formArea.innerHTML = "";

  schema.fields.forEach(f => {
    if (!fieldVisibleInMode(f.name, schema)) return;

    const val = currentEditingDevice[f.name] ?? f.default ?? "";
    let inputHtml = "";

    switch(f.type) {
      case "Bool":
        inputHtml = `<input type="checkbox" id="field_${f.name}" ${val ? "checked" : ""}>`;
        break;
      case "String":
      case "UID":
      case "UInt":
      case "Float":
      case "Pin":
      case "Number":
      case "Int":
        inputHtml = `<input type="text" id="field_${f.name}" value="${val}">`;
        break;
      case "AllOfGroup":
        inputHtml = `<div id="group_${f.name}" style="border:1px solid #ccc;padding:5px;margin:5px">
          ${f.fields.map(sf => `<label>${sf.name}</label>
            <input type="text" id="field_${f.name}_${sf.name}" value="${currentEditingDevice[f.name]?.[sf.name]??''}">`
          ).join("<br>")}
        </div>`;
        break;
      case "Object":
        inputHtml = `<pre style="background:#eee;padding:5px">${JSON.stringify(val,null,2)}</pre>`;
        break;
      default:
        inputHtml = `<pre>${JSON.stringify(val)}</pre>`;
    }

    const fieldDiv = document.createElement("div");
    fieldDiv.innerHTML = `<label>${f.name}</label>: ${inputHtml}`;
    formArea.appendChild(fieldDiv);
  });
}
*/
function renderForm() {
  const formArea = document.getElementById("formArea");
  formArea.innerHTML = "";

  currentEditingSchema.fields.forEach(f => {
    if (!fieldVisibleInMode(f.name, currentEditingSchema)) return;

    renderField(f, currentEditingDevice, formArea);
  });
}
function confirmAdd() {
  const schema = currentEditingSchema;
  schema.fields.forEach(f => {
    if (!fieldVisibleInMode(f.name, schema)) return;

    switch(f.type) {
      case "Bool":
        currentEditingDevice[f.name] = document.getElementById(`field_${f.name}`).checked;
        break;
      case "String":
      case "UID":
      case "UInt":
      case "Float":
      case "Pin":
      case "Number":
      case "Int":
        currentEditingDevice[f.name] = document.getElementById(`field_${f.name}`).value;
        break;
      case "AllOfGroup":
        currentEditingDevice[f.name] = {};
        f.fields.forEach(sf => {
          currentEditingDevice[f.name][sf.name] = document.getElementById(`field_${f.name}_${sf.name}`).value;
        });
        break;
    }
  });

  // if adding new:
  if (!devices.includes(currentEditingDevice)) devices.push(currentEditingDevice);

  renderTable();
  closeModal();
}

// ================= TABLE =================

function renderTable() {
  const tbody = document.getElementById("deviceTable");
  tbody.innerHTML = "";

  devices.forEach((dev, i) => {
    renderRow(tbody, dev, i, 0);
  });
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

  // ==== Expand button ====
  let expandBtn = "";
  if (hasChildren) {
    expandBtn = `<button onclick="toggleExpand('${key}')">
      ${expanded[key] ? "▼" : "▶"}
    </button>`;
  }

  // ==== Basic columns ====
  const type = dev.__type || "";
  const uid = dev.uid || "";
  const note = dev.note || "";

  // ==== Extra fields ====
  let data = "";

  Object.keys(dev).forEach(k => {
    if (k === "__type" || k === "uid" || k === "note" || k === "type") return;

    const v = dev[k];

    if (isComplex(v)) {
      if (Array.isArray(v)) {
        data += `${k}:[...] `;
      } else {
        data += `${k}:{...} `;
      }
    } else {
      data += `${k}:${v} `;
    }
  });

  let enabled = dev.disabled?"":"✓"; // (dev.disabled == undefined) || (dev.disabled == false)

  tr.innerHTML = `
    <td>${enabled}</td>
    <td style="padding-left:${level * 20}px">
      ${expandBtn} ${type}
    </td>
    
    <td>${uid}</td>
    <td>${data}</td>
    <td>${note}</td>
    <td>
      <button onclick="editDevice(${index})">Edit</button>
      <button onclick="removeDevice(${index})">Remove</button>
    </td>
  `;

  tbody.appendChild(tr);

  // ==== Render children ====
  if (expanded[key]) {
    Object.keys(dev).forEach(k => {
      const v = dev[k];

      if (Array.isArray(v)) {
        v.forEach((child, idx) => {
          if (typeof child === "object") {
            renderRow(tbody, child, idx, level + 1, key + "." + k);
          }
        });
      }
      else if (typeof v === "object" && v !== null) {
        renderRow(tbody, v, 0, level + 1, key + "." + k);
      }
    });
  }
}

function removeDevice(i){
  devices.splice(i,1);
  renderTable();
}

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
  renderForm(); // re-render form for the selected mode
}

function fieldVisibleInMode(fieldName, schema) {
  if (!currentMode) return true; // fallback: show all
  const mode = schema.modes.find(m => m.name === currentMode);
  if (!mode) return true;

  // check if field is explicitly required in this mode
  const conj = mode.conjunctions.find(c => c.name === fieldName);
  if (conj) return conj.required;

  // field not listed → always show
  return true;
}

function changeType() {
  const sel = document.getElementById("typeSelect");
  const type = sel.value;

  // update schema
  currentEditingSchema = schema["ROOT"][type];

  // IMPORTANT: reset device completely
  currentEditingDevice = {
    __type: type,
    type: type
  };

  // reset mode
  currentMode = null;

  // rebuild mode UI (if exists)
  setupModeUI();

  // re-render form
  renderForm();
}

// ================= ADD =================

function editDevice(index) {
  editIndex = index;

  const dev = devices[index];

  // deep clone
  currentEditingDevice = JSON.parse(JSON.stringify(dev));
  console.log(currentEditingDevice);

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

function populateForm(obj, container = document.getElementById("formArea")) {
  const inputs = container.querySelectorAll("input, select");

  inputs.forEach(input => {
    const name = input.parentElement.firstChild.textContent;

    if (obj[name] !== undefined) {
      if (input.type === "checkbox") {
        input.checked = obj[name];
      } else {
        input.value = obj[name];
      }
    } else {
		console.log("obj[name] == undefined" + name, obj);
	}
  });
}

// ================= LOAD =================

function loadJSON(){
  const txt = document.getElementById("loadBox").value;

  try {
    const json = JSON.parse(txt);

    if (!Array.isArray(json)) {
      throw new Error("Root must be an array");
    }

    devices = json.map(obj => {
      // Ensure type exists
      if (!obj.type) {
        console.warn("Missing type in object:", obj);
        return null;
      }

      return {
        ...obj,
        __type: obj.type   // internal mapping
      };
    }).filter(Boolean);

    renderTable();

  } catch(e) {
    console.log(e);
    alert("Invalid JSON");
  }
}

// ================= SAVE =================

function saveJSON(){
  const out = devices.map(d => {
    const copy = {...d};

    // Ensure type is preserved
    copy.type = d.__type;

    delete copy.__type;
    return copy;
  });

  document.getElementById("saveBox").value =
    JSON.stringify(out, null, 2);
}

// ================= INIT =================

function initTypes(){
  const sel = document.getElementById("typeSelect");
  sel.innerHTML = ""; // clear
  Object.keys(schema["ROOT"]).forEach(k=>{
    const o = document.createElement("option");
    o.value = k;
    o.textContent = k;
    sel.appendChild(o);
  });
}

document.addEventListener("DOMContentLoaded", () => {
  initTypes();
  renderTable();
  if (examplecfg != undefined) {
	  document.getElementById("loadBox").value = JSON.stringify(examplecfg);
    loadJSON();
  }
  
});
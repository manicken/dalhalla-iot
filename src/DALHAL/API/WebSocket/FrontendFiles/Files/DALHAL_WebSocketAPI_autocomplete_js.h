/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2026 Jannik Svensson

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or 
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include "stdint.h"
#include <pgmspace.h>

namespace DALHAL {

static const uint8_t WS_API_AUTOCOMPLETE_JS[] PROGMEM = R"rawliteral(
// ── Autocomplete terminal ─────────────────────────────────────────────────────
// ── Command tree ──────────────────────────────────────────────────────────────
var helpTree = {};

const input   = document.getElementById('cmd-input');
const sugBox  = document.getElementById('suggestions');
const termOut = document.getElementById('autocomplete-output');
let activeIdx = -1, currentSugs = [], history = [], histIdx = -1;

function termPrintLine(text, cls) {
  const div = document.createElement('div');
  div.className = cls;
  div.textContent = text;
  termOut.appendChild(div);
  termOut.scrollTop = termOut.scrollHeight;
}

function splitCmd(val) {
  const si = val.lastIndexOf('/');
  if (si === -1) return { prefix: '', last: val, parts: val ? [] : [] };
  return {
    prefix: val.slice(0, si + 1),
    last:   val.slice(si + 1),
    parts:  val.slice(0, si).split('/').filter(Boolean)
  };
}

function getNode(parts) {
  let node = helpTree.root;
  for (const p of parts) {
    if (!node.children) return null;
    const found = node.children.find(c => c.name === p);
    if (!found) return null;
    node = found;
  }
  return node;
}

function getSuggestions(val) {
  const { parts, last } = splitCmd(val);
  // If no slash yet, search root children
  const parentParts = val.includes('/') ? parts : [];
  const parentNode = getNode(parentParts);
  if (!parentNode || !parentNode.children) return [];
  const token = val.includes('/') ? last : val;
  return parentNode.children
    .filter(c => c.name.toLowerCase().startsWith(token.toLowerCase()))
    .map(c => ({ name: c.name, help: c.help, hasChildren: !!(c.children && c.children.length) }));
}

function getCommonPrefix(strs) {
  if (!strs.length) return '';
  let p = strs[0];
  for (let i = 1; i < strs.length; i++) while (!strs[i].toLowerCase().startsWith(p.toLowerCase())) p = p.slice(0,-1);
  return p;
}

function renderSuggestions(sugs) {
  currentSugs = sugs; activeIdx = -1;
  if (!sugs.length) { sugBox.style.display = 'none'; return; }
  sugBox.style.display = 'block';
  sugBox.innerHTML = sugs.map((s,i) => `<div class="sug" data-idx="${i}" role="option" aria-selected="false">
    <span class="sug-name${s.hasChildren?' has-children':''}">${s.name}</span>
    <span class="sug-help">${s.help}</span>
  </div>`).join('');
  sugBox.querySelectorAll('.sug').forEach(el =>
    el.addEventListener('mousedown', e => { e.preventDefault(); pickSuggestion(+el.dataset.idx); })
  );
}

function hideSuggestions() { sugBox.style.display='none'; currentSugs=[]; activeIdx=-1; }

function setActive(idx) {
  const items = sugBox.querySelectorAll('.sug');
  items.forEach((el,i) => { el.classList.toggle('active', i===idx); el.setAttribute('aria-selected', i===idx); });
  if (idx>=0 && items[idx]) items[idx].scrollIntoView({block:'nearest'});
  activeIdx = idx;
}

function pickSuggestion(idx) {
  if (idx < 0 || idx >= currentSugs.length) return;
  const sug = currentSugs[idx];
  const { prefix } = splitCmd(input.value);
  input.value = (input.value.includes('/') ? prefix : '') + sug.name + (sug.hasChildren ? '/' : '');
  hideSuggestions();
  input.focus();
  updateSuggestions();
}

function updateSuggestions() {
  if (!input.value.trim()) { hideSuggestions(); return; }
  renderSuggestions(getSuggestions(input.value));
}

input.addEventListener('input', updateSuggestions);

input.addEventListener('keydown', e => {
  const visible = sugBox.style.display !== 'none';

  if (e.key === 'Tab') {
    e.preventDefault();
    const sugs = getSuggestions(input.value);
    if (!sugs.length) return;
    if (sugs.length === 1) { pickSuggestion(0); return; }
    const { prefix, last } = splitCmd(input.value);
    const common = getCommonPrefix(sugs.map(s=>s.name));
    const token = input.value.includes('/') ? last : input.value;
    if (common.length > token.length) {
      input.value = (input.value.includes('/') ? prefix : '') + common;
      updateSuggestions();
    } else {
      renderSuggestions(sugs);
    }
    return;
  }

  if (e.key === 'ArrowDown') {
    e.preventDefault();
    if (!visible) { updateSuggestions(); return; }
    setActive(Math.min(activeIdx+1, currentSugs.length-1));
    return;
  }

  if (e.key === 'ArrowUp') {
    e.preventDefault();
    if (visible && activeIdx > 0) { setActive(activeIdx-1); return; }
    if (!visible && histIdx < history.length-1) { histIdx++; input.value=history[histIdx]; hideSuggestions(); }
    return;
  }

  if (e.key === 'Escape') { hideSuggestions(); return; }

  if (e.key === 'Enter') {
    if (visible && activeIdx >= 0) { pickSuggestion(activeIdx); return; }
    hideSuggestions();
    const cmd = input.value.trim();
    if (cmd) {
      history.unshift(cmd); histIdx = -1;
      wsSend(cmd);
    }
    input.value = '';
    return;
  }
});

input.focus();
)rawliteral";

}
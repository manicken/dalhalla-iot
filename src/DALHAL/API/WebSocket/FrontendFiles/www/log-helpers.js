// ── Log helpers ───────────────────────────────────────────────────────────────
const log = document.getElementById('log');

function logText(text) {
  log.append(newEl('div', { text, styles: { whiteSpace: 'pre-wrap' } }));
  log.scrollTop = log.scrollHeight;
}

function logCollapsible(jsonData, titleStr) {
  const oneLine = JSON.stringify(jsonData);
  const pre = newEl('pre', { text: JSON.stringify(jsonData, null, 2), onmousedown: (e) => {
    if (e.ctrlKey == false) { return; }
    e.preventDefault(); // stops native selection behavior: Ctrl+click selects the whole JSON
    const range = document.createRange();
    range.selectNodeContents(pre);
    const sel = window.getSelection();
    sel.removeAllRanges();
    sel.addRange(range);
  } });
  log.append(newEl('details', {},
    newEl('summary', { text: `${titleStr ?? ""} ` + (oneLine.length > 150 ? oneLine.slice(0, 150 - 3) + "…" : oneLine) }),
    pre));
  log.scrollTop = log.scrollHeight;
}

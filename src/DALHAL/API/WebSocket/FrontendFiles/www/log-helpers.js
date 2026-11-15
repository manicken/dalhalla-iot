// ── Log helpers ───────────────────────────────────────────────────────────────
const log = document.getElementById('log');

function logText(text) {
  const div = document.createElement('div');
  div.style.whiteSpace = 'pre-wrap';
  div.textContent = text;
  log.appendChild(div);
  log.scrollTop = log.scrollHeight;
}

function logCollapsible(jsonData, titleStr) {
  const details = document.createElement("details");

  const summary = document.createElement("summary");
  const oneLine = JSON.stringify(jsonData);

  summary.textContent =
    `${titleStr ?? ""} ` +
    (oneLine.length > 150 ? oneLine.slice(0, 150-3) + "…" : oneLine);

  const pre = document.createElement("pre");
  pre.textContent = JSON.stringify(jsonData, null, 2);

  details.appendChild(summary);
  details.appendChild(pre);

  pre.addEventListener("mousedown", (e) => {
    if (e.ctrlKey == false) { return;}
    e.preventDefault(); // stops native selection behavior
    const range = document.createRange();
    range.selectNodeContents(pre);
    const sel = window.getSelection();
    sel.removeAllRanges();
    sel.addRange(range);
  });

  log.appendChild(details);
  log.scrollTop = log.scrollHeight;
}

// DOM helpers for all frontend scripts (loaded first)

function setStyles(el, styles) {
  if (typeof styles === "object") {
    for (const [name, value] of Object.entries(styles)) {
      el.style[name] = value;
    }
  }
}

// newEl('div', { cls, text, title, onclick, styles: {..}, any-attribute }, ...children)
// null/false/undefined attributes and children are left out
function newEl(tag, attrs = {}, ...children) {
  const e = document.createElement(tag);
  for (const [k, v] of Object.entries(attrs)) {
    if (v === undefined || v === null || v === false) continue;
    if (k === 'text') e.textContent = v;
    else if (k === 'cls') e.className = v;
    else if (k === 'styles') setStyles(e, v);
    else if (k.startsWith('on')) e.addEventListener(k.slice(2), v);
    else e.setAttribute(k, v === true ? '' : v);
  }
  for (const c of children) if (c) e.append(c);
  return e;
}

// browser download of a url (also blob:) as fileName
function downloadUrl(href, fileName) {
  const a = newEl('a', { href, download: fileName });
  document.body.append(a);
  a.click();
  a.remove();
}

// ── Tabs ──────────────────────────────────────────────────────────────────────
// One page per tab (<div id="tab-NAME" class="tab-page">), selected by the url hash (#console, #files),
// so a tab can be bookmarked and the back button works. The WebSocket (main.js) is shared by all tabs.
// A tab with a script gets it loaded the first time it is shown, later it gets onShow() called.
// #NAME=ARG passes ARG to the tab, e.g. #files=/cfg.json opens that file.

const TABS = {
  console: { script: null },
  files:   { script: 'files.js', api: () => window.FilesTab },
  config:  { script: 'config.js', api: () => window.ConfigTab },
};
const tabLoaded = {};

function showTab(hash) {
  let name = hash.split('=')[0];
  if (!(name in TABS)) name = 'console';
  document.querySelectorAll('.tab-page').forEach(p => { p.hidden = p.id !== 'tab-' + name; });
  document.querySelectorAll('#tabs a').forEach(a => a.classList.toggle('active', a.dataset.tab === name));

  const tab = TABS[name];
  if (tab.script && !tabLoaded[name]) {
    tabLoaded[name] = true;
    document.body.append(newEl('script', { src: tab.script })); // calls its own onShow when loaded
  } else if (tab.api && tab.api()) {
    tab.api().onShow();
  }
}

window.addEventListener('hashchange', () => showTab(location.hash.slice(1)));
showTab(location.hash.slice(1));

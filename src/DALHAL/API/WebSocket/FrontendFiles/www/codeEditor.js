// Code editor for the Files and Scripts tabs: ACE, or a plain textarea when ACE can not be loaded.
// ACE is not in the firmware: it is loaded from cdnjs, else from LittleFS /edit/ace.js (as the old FSBrowser),
// else the textarea is used (e.g. failsafe AP without internet).
//   const ed = await CodeEditor.create(hostElement, { onChange, onSave });
//   ed.set(text, 'json') / ed.get() / ed.resize() / ed.focus() / ed.kind ('ACE (cdnjs)' ...) / ed.ace (null without ACE)

const CodeEditor = (() => {
  const ACE_CDN = 'https://cdnjs.cloudflare.com/ajax/libs/ace/1.36.2/';
  const ACE_LOCAL = '/edit/';
  let base = null;
  let loading = null;

  /** loads ACE once, true when it is there */
  function loadAce() {
    if (!loading) {
      loading = (async () => {
        for (const b of [ACE_CDN, ACE_LOCAL]) {
          try {
            await loadScript(b + 'ace.js');
            ace.config.set('basePath', b);
            base = b;
            return true;
          } catch (e) { /* next */ }
        }
        return false;
      })();
    }
    return loading;
  }

  /** an ACE extension (e.g. 'language_tools'), false when it can not be loaded */
  async function loadExt(name) {
    if (!(await loadAce())) return false;
    try { await loadScript(base + 'ext-' + name + '.js'); return true; } catch (e) { return false; }
  }

  async function create(host, { onChange = () => {}, onSave = () => {} } = {}) {
    if (await loadAce()) {
      const ed = ace.edit(host);
      ed.setOptions({ fontSize: '13px', showPrintMargin: false, tabSize: 2, useSoftTabs: true });
      const applyTheme = () => ed.setTheme(document.body.classList.contains('dark') ? 'ace/theme/tomorrow_night' : 'ace/theme/chrome');
      applyTheme();
      new MutationObserver(applyTheme).observe(document.body, { attributes: true, attributeFilter: ['class'] });
      ed.commands.addCommand({ name: 'save', bindKey: { win: 'Ctrl-S', mac: 'Command-S' }, exec: () => onSave() });
      let loadingText = false; // set() fires change events, they are not user changes
      ed.on('change', () => { if (!loadingText) onChange(); });
      return {
        ace: ed,
        kind: 'ACE ' + (base === ACE_CDN ? '(cdnjs)' : '(LittleFS /edit/)'),
        get: () => ed.getValue(),
        set: (text, mode) => {
          loadingText = true;
          ed.session.setMode(mode.includes('/') ? mode : 'ace/mode/' + mode);
          ed.setValue(text, -1);
          ed.session.getUndoManager().reset();
          loadingText = false;
        },
        resize: () => ed.resize(),
        focus: () => ed.focus(),
      };
    }
    const ta = newEl('textarea', { cls: 'code-textarea', spellcheck: 'false', oninput: () => onChange(), onkeydown: (e) => {
      if ((e.ctrlKey || e.metaKey) && e.key === 's') { e.preventDefault(); onSave(); }
      if (e.key === 'Tab') {
        e.preventDefault();
        ta.setRangeText('  ', ta.selectionStart, ta.selectionEnd, 'end');
        onChange();
      }
    } });
    host.append(ta);
    return {
      ace: null,
      kind: 'plain editor (ACE could not be loaded)',
      get: () => ta.value,
      set: (text) => { ta.value = text; },
      resize: () => {},
      focus: () => ta.focus(),
    };
  }

  return { create, loadAce, loadExt };
})();

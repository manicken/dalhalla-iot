// Script language helpers, shared by the Scripts tab (live checks) and the Config tab (do the scripts still
// find their devices?). Same comment rules as the firmware tokenizer: line comments and block comments.

const ScriptTools = (() => {
  const KEYWORDS = ['on', 'then', 'do', 'endon', 'if', 'elseif', 'else', 'endif', 'and', 'or'];
  const TRIGGER_KEYWORDS = ['eachloop'];

  /** the lines of a script without their comments (a comment becomes spaces, so columns stay right) */
  function codeLines(text) {
    let inBlock = false;
    return text.split('\n').map((line) => {
      let code = '';
      for (let i = 0; i < line.length; i++) {
        if (inBlock) {
          if (line.startsWith('*/', i)) { inBlock = false; code += '  '; i++; } else code += ' ';
          continue;
        }
        if (line.startsWith('//', i)) break;
        if (line.startsWith('/*', i)) { inBlock = true; code += '  '; i++; continue; }
        code += line[i];
      }
      return code;
    });
  }

  /** the device references of a script: [{ row, column, path, suffix }], 0 based row/column,
      suffix is '#name', '#name()' or '@Event' (or '') */
  function refs(text) {
    const out = [];
    codeLines(text).forEach((code, row) => {
      const re = /(^|[^\w:#@.])([A-Za-z_]\w*(?::[A-Za-z_0-9]\w*)*)(#[A-Za-z_]\w*(?:\(\))?|@[A-Za-z_]\w*)?/g;
      let m;
      while ((m = re.exec(code))) {
        const path = m[2];
        const lower = path.toLowerCase();
        if (KEYWORDS.includes(lower) || TRIGGER_KEYWORDS.includes(lower)) continue;
        out.push({ row, column: m.index + m[1].length, path, suffix: m[3] || '' });
      }
    });
    return out;
  }

  /** the file names of list.txt (read by the firmware with the script tokenizer: comments allowed,
      names separated by white space) */
  function listNames(text) {
    return codeLines(text.replace(/\r\n/g, '\n')).join(' ').split(/\s+/).filter(Boolean);
  }

  return { KEYWORDS, TRIGGER_KEYWORDS, codeLines, refs, listNames };
})();

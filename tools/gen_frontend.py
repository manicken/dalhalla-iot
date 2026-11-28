"""
Generates the embedded frontend files for the DALHAL WebSocket API.

Source:  src/DALHAL/API/WebSocket/FrontendFiles/www/**      (plain html/js/css/...)
Output:  src/DALHAL/API/WebSocket/FrontendFiles/Generated/DALHAL_FrontendFiles.h

The path of a file relative to www/ is used as its virtual path (e.g. css/main.css).
JS, CSS and HTML are minified first: comments, indentation and empty lines out (see minify()),
DALHAL_FRONTEND_MINIFY=0 keeps the files as they are.
Files are gzip compressed when that makes them smaller, and are then served
with "Content-Encoding: gzip" so the browser decompresses them transparently.

The output is only rewritten when its content changes, to avoid needless rebuilds.

Usage:
  - PlatformIO: extra_scripts = pre:tools/gen_frontend.py
  - standalone: python tools/gen_frontend.py
"""

import gzip
import os
import re

try:
    Import("env")  # noqa: F821 - provided by PlatformIO/SCons
    PROJECT_DIR = env.subst("$PROJECT_DIR")  # noqa: F821
except NameError:
    PROJECT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# targets/esp-idf has its own platformio.ini, the sources are two levels up from there
if not os.path.isdir(os.path.join(PROJECT_DIR, "src", "DALHAL")) and os.path.isdir(os.path.join(PROJECT_DIR, "..", "..", "src", "DALHAL")):
    PROJECT_DIR = os.path.normpath(os.path.join(PROJECT_DIR, "..", ".."))

FRONTEND_DIR = os.path.join(PROJECT_DIR, "src", "DALHAL", "API", "WebSocket", "FrontendFiles")
SOURCE_DIR = os.path.join(FRONTEND_DIR, "www")
OUTPUT_FILE = os.path.join(FRONTEND_DIR, "Generated", "DALHAL_FrontendFiles.h")

# extension -> MIME_* constant defined in DALHAL_VirtualFile.h
MIME_TYPES = {
    ".html": "MIME_HTML",
    ".htm":  "MIME_HTML",
    ".js":   "MIME_JS",
    ".css":  "MIME_CSS",
    ".txt":  "MIME_TEXT",
    ".json": "MIME_JSON",
    ".xml":  "MIME_XML",
    ".png":  "MIME_PNG",
    ".jpg":  "MIME_JPEG",
    ".jpeg": "MIME_JPEG",
    ".gif":  "MIME_GIF",
    ".svg":  "MIME_SVG",
    ".ico":  "MIME_ICON",
    ".webp": "MIME_WEBP",
}
TEXT_EXTENSIONS = {".html", ".htm", ".js", ".css", ".txt", ".json", ".xml", ".svg"}
# already compressed formats, gzip would only waste build time
NO_GZIP_EXTENSIONS = {".png", ".jpg", ".jpeg", ".gif", ".webp", ".gz", ".zip", ".woff", ".woff2"}

BYTES_PER_LINE = 20


def symbol_name(vpath):
    return "WS_API_" + re.sub(r"[^A-Za-z0-9]", "_", vpath).upper()


def collect_files():
    files = []
    for root, dirs, names in os.walk(SOURCE_DIR):
        dirs.sort()
        for name in sorted(names):
            full = os.path.join(root, name)
            vpath = os.path.relpath(full, SOURCE_DIR).replace(os.sep, "/")
            files.append((vpath, full))
    return files


# ---------------- minify: comments and indentation out, nothing else ----------------
# Not a full minifier on purpose (no renaming): small, no dependencies, and the code in the browser stays
# readable. Every line stays on its line number: a comment line or an empty line becomes an empty line,
# so an error at line N in the browser dev tools is at line N in the source file (only the columns move
# by the removed indentation); runs of empty lines cost next to nothing after gzip.
# Strings, template literals (with nested ${...}) and regex literals are copied exactly, so "//" or "/*"
# inside them is never taken as a comment.
# DALHAL_FRONTEND_MINIFY=0 turns it off (e.g. to debug against the original files).

MINIFY = os.environ.get("DALHAL_FRONTEND_MINIFY", "1") != "0"

# after these a "/" starts a regex literal, otherwise it is a division
_REGEX_AFTER_CHARS = set("(,=:[!&|?{};+-*%<>~^")
_REGEX_AFTER_WORDS = {"return", "typeof", "case", "do", "else", "in", "of", "new", "delete", "void",
                      "throw", "instanceof", "yield", "await"}


class _Out:
    """collects the output: no indentation, no trailing spaces, every line break kept (same line numbers)"""

    def __init__(self):
        self.parts = []
        self.line_start = True

    def code(self, ch):
        if ch == "\n":
            while self.parts and self.parts[-1] in (" ", "\t"):
                self.parts.pop()
            self.parts.append("\n")
            self.line_start = True
        elif ch in " \t\r":
            if not self.line_start:
                self.parts.append(ch)
        else:
            self.parts.append(ch)
            self.line_start = False

    def raw(self, text):  # strings, templates, regex: exactly as they are
        if text:
            self.parts.append(text)
            self.line_start = False

    def text(self):
        return "".join(self.parts)


def minify_js(src):
    out = _Out()
    n = len(src)
    last = ""  # last significant char or word, for the regex/division decision

    def copy_quoted(i, q):  # 'string' or "string", i at the opening quote
        j = i + 1
        while j < n and src[j] != q:
            j += 2 if src[j] == "\\" else 1
        return j + 1

    def copy_regex(i):  # /regex/flags, i at the opening /
        j, in_class = i + 1, False
        while j < n:
            c = src[j]
            if c == "\\":
                j += 2
                continue
            if c == "[":
                in_class = True
            elif c == "]":
                in_class = False
            elif c == "/" and not in_class:
                break
            elif c == "\n":
                raise ValueError("unterminated regex literal")
            j += 1
        j += 1
        while j < n and (src[j].isalnum() or src[j] == "_"):
            j += 1
        return j

    def code(i, until_brace):
        """code from i; with until_brace it stops at the } that closes a template ${ (returns its index)"""
        nonlocal last
        depth = 0
        while i < n:
            c = src[i]
            nxt = src[i + 1] if i + 1 < n else ""
            if c in "'\"":
                j = copy_quoted(i, c)
                out.raw(src[i:j]); last = c; i = j
            elif c == "`":
                i = template(i)
                last = "`"
            elif c == "/" and nxt == "/":
                while i < n and src[i] != "\n":
                    i += 1
            elif c == "/" and nxt == "*":
                end = src.find("*/", i + 2)
                end = n if end < 0 else end + 2
                lines = src.count("\n", i, end)
                for _ in range(lines):
                    out.code("\n")
                if not lines:
                    out.code(" ")
                i = end
            elif c == "/" and (last == "" or last in _REGEX_AFTER_CHARS or last in _REGEX_AFTER_WORDS):
                j = copy_regex(i)
                out.raw(src[i:j]); last = "/regex/"; i = j
            elif c.isalnum() or c in "_$":
                j = i
                while j < n and (src[j].isalnum() or src[j] in "_$"):
                    j += 1
                out.raw(src[i:j]); last = src[i:j]; i = j
            else:
                if until_brace:
                    if c == "{":
                        depth += 1
                    elif c == "}":
                        if depth == 0:
                            return i
                        depth -= 1
                out.code(c)
                if c not in " \t\r\n":
                    last = c
                i += 1
        return i

    def template(i):  # `...${code}...`, i at the opening backtick, returns the index after the closing one
        j = i + 1
        start = i
        while j < n:
            c = src[j]
            if c == "\\":
                j += 2
            elif c == "`":
                out.raw(src[start:j + 1])
                return j + 1
            elif c == "$" and j + 1 < n and src[j + 1] == "{":
                out.raw(src[start:j + 2])
                j = code(j + 2, True)
                start = j  # the closing } of ${ is copied with the next template part
                j += 1
            else:
                j += 1
        raise ValueError("unterminated template literal")

    code(0, False)
    return out.text()


def minify_css(src):
    out = _Out()
    i, n = 0, len(src)
    while i < n:
        c = src[i]
        if c in "'\"":
            j = i + 1
            while j < n and src[j] != c:
                j += 2 if src[j] == "\\" else 1
            out.raw(src[i:j + 1]); i = j + 1
        elif c == "/" and i + 1 < n and src[i + 1] == "*":
            end = src.find("*/", i + 2)
            end = n if end < 0 else end + 2
            for _ in range(src.count("\n", i, end)):
                out.code("\n")
            i = end
        else:
            out.code(c); i += 1
    return out.text()


def minify_html(src):
    """<!-- comments --> and indentation out (same line numbers), pre/textarea/script/style stay as they are"""
    src = re.sub(r"<!--(?!\[).*?-->", lambda m: "\n" * m.group(0).count("\n"), src, flags=re.S)
    keep = re.compile(r"(<(pre|textarea|script|style)\b.*?</\2\s*>)", re.S | re.I)
    pieces = keep.split(src)
    result = []
    for k, piece in enumerate(pieces):
        if k % 3 == 1:
            result.append(piece)          # a kept block, as it is
        elif k % 3 == 0:
            # line by line: indentation and trailing spaces out, the line breaks themselves stay
            result.append("\n".join(l.strip() for l in piece.split("\n")))
        # k % 3 == 2 is the tag name group of the split
    return "".join(result)


def minify(ext, data):
    if not MINIFY:
        return data
    text = data.decode("utf-8")
    if ext == ".js":
        text = minify_js(text)
    elif ext == ".css":
        text = minify_css(text)
    elif ext in (".html", ".htm"):
        text = minify_html(text)
    else:
        return data
    return text.encode("utf-8")


def load_file(vpath, full):
    ext = os.path.splitext(vpath)[1].lower()
    with open(full, "rb") as f:
        data = f.read()
    src_size = len(data)
    if ext in TEXT_EXTENSIONS:
        # make output independent of git autocrlf
        data = data.replace(b"\r\n", b"\n")
        data = minify(ext, data)
    raw_size = len(data)
    gzipped = False
    if ext not in NO_GZIP_EXTENSIONS and data:
        compressed = gzip.compress(data, compresslevel=9, mtime=0)
        if len(compressed) < len(data):
            data = compressed
            gzipped = True
    return {
        "vpath": vpath,
        "symbol": symbol_name(vpath),
        "mime": MIME_TYPES.get(ext, "MIME_BIN"),
        "data": data,
        "src_size": src_size,
        "raw_size": raw_size,
        "gzip": gzipped,
    }


def format_bytes(data):
    lines = []
    for i in range(0, len(data), BYTES_PER_LINE):
        chunk = data[i:i + BYTES_PER_LINE]
        lines.append("    " + ",".join("0x%02x" % b for b in chunk) + ",")
    return "\n".join(lines)


def totals(entries):
    """one line: number of files, source size, minified size and what is stored in flash (the file data)"""
    return "%d files: %d bytes in www/, %d minified, %d bytes in flash (file data)" % (
        len(entries), sum(e["src_size"] for e in entries), sum(e["raw_size"] for e in entries),
        sum(len(e["data"]) for e in entries))


def generate(entries):
    out = []
    out.append("// AUTO-GENERATED by tools/gen_frontend.py from FrontendFiles/www/ - DO NOT EDIT")
    out.append("// " + totals(entries))
    out.append("")
    out.append("#pragma once")
    out.append("")
    out.append("#include <stdint.h>")
    out.append("#include <pgmspace.h>")
    out.append('#include "../DALHAL_VirtualFile.h"')
    out.append("")
    out.append("namespace DALHAL {")
    out.append("")
    for e in entries:
        info = "gzip %d -> %d bytes" % (e["raw_size"], len(e["data"])) if e["gzip"] else "%d bytes" % e["raw_size"]
        out.append("    // %s (%s)" % (e["vpath"], info))
        if e["data"]:
            out.append("    static const uint8_t %s[] PROGMEM = {" % e["symbol"])
            out.append(format_bytes(e["data"]))
            out.append("    };")
        else:
            # zero sized arrays are not allowed, size is taken from the entry instead
            out.append("    static const uint8_t %s[1] PROGMEM = { 0 };" % e["symbol"])
        out.append("")
    out.append("    // " + totals(entries))
    out.append("    constexpr ApiVirtualFile VirtualFiles[] = {")
    for e in entries:
        out.append('        { CE_MATCH_EMIT_STR("%s"), %s, %d, %s, %s },' % (
            e["vpath"], e["symbol"], len(e["data"]), e["mime"], "true" if e["gzip"] else "false"))
    out.append("    };")
    out.append("    constexpr size_t VirtualFiles_Size = sizeof(VirtualFiles) / sizeof(VirtualFiles[0]);")
    out.append("")
    out.append("}")
    out.append("")
    return "\n".join(out)


def write_if_changed(path, content):
    content = content.encode("utf-8")
    if os.path.exists(path):
        with open(path, "rb") as f:
            if f.read() == content:
                return False
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "wb") as f:
        f.write(content)
    return True


def main():
    entries = [load_file(vpath, full) for vpath, full in collect_files()]
    if not entries:
        raise SystemExit("gen_frontend: no files found in " + SOURCE_DIR)
    changed = write_if_changed(OUTPUT_FILE, generate(entries))
    print("gen_frontend: %s%s" % (totals(entries), "" if changed else " (unchanged)"))


main()

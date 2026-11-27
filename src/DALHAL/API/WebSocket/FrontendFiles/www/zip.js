// ── Minimal ZIP reader and writer ─────────────────────────────────────────────
// No library: the zip structures (local headers, central directory = the table of content at the end)
// are read and written here, compression is done by the browser's own CompressionStream /
// DecompressionStream('deflate-raw'), so it also works without internet.
// Supports "stored" (0) and "deflate" (8), which is what zip tools write; no ZIP64 (> 4 GB / > 65535
// entries) and no encryption. Names are written as UTF-8 (flag bit 11).
//   Zip.list(arrayBuffer)            -> [{ name, dir, size, ... }]
//   Zip.extract(arrayBuffer, entry)  -> Promise<Uint8Array>
//   Zip.create([{ name, data }])     -> Promise<Blob>   (a name ending with / is a folder, data is ignored)

window.Zip = (() => {
  const SIG_EOCD = 0x06054b50;    // end of central directory
  const SIG_CENTRAL = 0x02014b50; // central directory entry
  const SIG_LOCAL = 0x04034b50;   // local file header
  const FLAG_UTF8 = 0x800;

  // ── reading ──

  /** utf-8 when flagged or valid, else windows-1252 (old zip tools on Windows) */
  function decodeName(bytes, utf8Flag) {
    if (utf8Flag) return new TextDecoder('utf-8').decode(bytes);
    try { return new TextDecoder('utf-8', { fatal: true }).decode(bytes); } catch (e) { /* not utf-8 */ }
    return new TextDecoder('windows-1252').decode(bytes);
  }

  function findEndOfCentralDirectory(dv) {
    const min = Math.max(0, dv.byteLength - 22 - 0xffff); // the record may be followed by a comment
    for (let i = dv.byteLength - 22; i >= min; i--) {
      if (dv.getUint32(i, true) === SIG_EOCD) return i;
    }
    throw new Error('not a zip file');
  }

  function list(buffer) {
    const bytes = new Uint8Array(buffer);
    const dv = new DataView(buffer);
    const eocd = findEndOfCentralDirectory(dv);
    const count = dv.getUint16(eocd + 10, true);
    let p = dv.getUint32(eocd + 16, true);
    if (count === 0xffff || p === 0xffffffff) throw new Error('ZIP64 is not supported');

    const entries = [];
    for (let i = 0; i < count; i++) {
      if (p + 46 > dv.byteLength || dv.getUint32(p, true) !== SIG_CENTRAL) throw new Error('broken zip (central directory)');
      const flags = dv.getUint16(p + 8, true);
      const nameLen = dv.getUint16(p + 28, true);
      const extraLen = dv.getUint16(p + 30, true);
      const commentLen = dv.getUint16(p + 32, true);
      const name = decodeName(bytes.subarray(p + 46, p + 46 + nameLen), (flags & FLAG_UTF8) !== 0);
      entries.push({
        name,
        dir: name.endsWith('/'),
        encrypted: (flags & 1) !== 0,
        method: dv.getUint16(p + 10, true),
        compressedSize: dv.getUint32(p + 20, true),
        size: dv.getUint32(p + 24, true),
        localOffset: dv.getUint32(p + 42, true),
      });
      p += 46 + nameLen + extraLen + commentLen;
    }
    return entries;
  }

  async function extract(buffer, entry) {
    if (entry.encrypted) throw new Error('encrypted zip entries are not supported');
    const dv = new DataView(buffer);
    const h = entry.localOffset;
    if (dv.getUint32(h, true) !== SIG_LOCAL) throw new Error('broken zip (local header)');
    // the sizes in the local header may be 0 (data descriptor), the central directory ones are used
    const start = h + 30 + dv.getUint16(h + 26, true) + dv.getUint16(h + 28, true);
    const data = new Uint8Array(buffer, start, entry.compressedSize);
    if (entry.method === 0) return data;
    if (entry.method !== 8) throw new Error(`compression method ${entry.method} is not supported`);
    if (typeof DecompressionStream === 'undefined') throw new Error('this browser can not unzip (no DecompressionStream)');
    const stream = new Blob([data]).stream().pipeThrough(new DecompressionStream('deflate-raw'));
    return new Uint8Array(await new Response(stream).arrayBuffer());
  }

  // ── writing ──

  const CRC_TABLE = (() => {
    const t = new Uint32Array(256);
    for (let n = 0; n < 256; n++) {
      let c = n;
      for (let k = 0; k < 8; k++) c = c & 1 ? 0xedb88320 ^ (c >>> 1) : c >>> 1;
      t[n] = c >>> 0;
    }
    return t;
  })();

  function crc32(data) {
    let c = 0xffffffff;
    for (let i = 0; i < data.length; i++) c = CRC_TABLE[(c ^ data[i]) & 0xff] ^ (c >>> 8);
    return (c ^ 0xffffffff) >>> 0;
  }

  async function deflate(data) {
    const stream = new Blob([data]).stream().pipeThrough(new CompressionStream('deflate-raw'));
    return new Uint8Array(await new Response(stream).arrayBuffer());
  }

  /** MS-DOS time and date as stored in zip files */
  function dosDateTime(d) {
    return {
      time: (d.getHours() << 11) | (d.getMinutes() << 5) | (d.getSeconds() >> 1),
      date: ((d.getFullYear() - 1980) << 9) | ((d.getMonth() + 1) << 5) | d.getDate(),
    };
  }

  async function create(files) {
    if (files.length > 0xffff) throw new Error('more than 65535 entries (ZIP64 is not supported)');
    const enc = new TextEncoder();
    const { time, date } = dosDateTime(new Date());
    const canDeflate = typeof CompressionStream !== 'undefined';
    const parts = [];
    const central = [];
    let offset = 0;

    for (const f of files) {
      const name = enc.encode(f.name);
      const isDir = f.name.endsWith('/');
      const data = isDir ? new Uint8Array(0) : f.data;
      const crc = crc32(data);
      let method = 0;
      let stored = data;
      if (canDeflate && data.length > 64) { // deflate only when it makes the file smaller
        const packed = await deflate(data);
        if (packed.length < data.length) { method = 8; stored = packed; }
      }

      const local = new DataView(new ArrayBuffer(30));
      local.setUint32(0, SIG_LOCAL, true);
      local.setUint16(4, 20, true);           // version needed: 2.0
      local.setUint16(6, FLAG_UTF8, true);
      local.setUint16(8, method, true);
      local.setUint16(10, time, true);
      local.setUint16(12, date, true);
      local.setUint32(14, crc, true);
      local.setUint32(18, stored.length, true);
      local.setUint32(22, data.length, true);
      local.setUint16(26, name.length, true);
      local.setUint16(28, 0, true);           // extra field length
      parts.push(local.buffer, name, stored);

      const cd = new DataView(new ArrayBuffer(46));
      cd.setUint32(0, SIG_CENTRAL, true);
      cd.setUint16(4, 20, true);              // version made by: 2.0, MS-DOS attributes
      cd.setUint16(6, 20, true);              // version needed
      cd.setUint16(8, FLAG_UTF8, true);
      cd.setUint16(10, method, true);
      cd.setUint16(12, time, true);
      cd.setUint16(14, date, true);
      cd.setUint32(16, crc, true);
      cd.setUint32(20, stored.length, true);
      cd.setUint32(24, data.length, true);
      cd.setUint16(28, name.length, true);
      // 30 extra length, 32 comment length, 34 disk, 36 internal attributes: 0
      cd.setUint32(38, isDir ? 0x10 : 0, true); // external attributes: MS-DOS directory bit
      cd.setUint32(42, offset, true);
      central.push(cd.buffer, name);

      offset += 30 + name.length + stored.length;
      if (offset > 0xffffffff) throw new Error('the zip would be larger than 4 GB (ZIP64 is not supported)');
    }

    const centralSize = central.reduce((n, b) => n + b.byteLength, 0);
    const end = new DataView(new ArrayBuffer(22));
    end.setUint32(0, SIG_EOCD, true);
    end.setUint16(8, files.length, true);    // entries on this disk
    end.setUint16(10, files.length, true);   // entries total
    end.setUint32(12, centralSize, true);
    end.setUint32(16, offset, true);         // start of the central directory
    return new Blob([...parts, ...central, end.buffer], { type: 'application/zip' });
  }

  return { list, extract, create };
})();

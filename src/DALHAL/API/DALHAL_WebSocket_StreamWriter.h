class StreamWriter {
public:
    StreamWriter(CommandCallback cb, CmdCbType type)
        : _cb(cb), _type(type), _pos(0) {}

    void write(const char* data, size_t len) {
        while (len > 0) {

            size_t space = sizeof(_buf) - _pos;
            if (space == 0) {
                flush();
                space = sizeof(_buf);
            }

            size_t toCopy = (len < space) ? len : space;

            memcpy(_buf + _pos, data, toCopy);
            _pos += toCopy;

            data += toCopy;
            len -= toCopy;
        }
    }

    void write(char c) {
        if (_pos >= sizeof(_buf)) {
            flush();
        }

        _buf[_pos++] = c;
    }

    void flush() {
        if (_pos == 0) return;

        _cb(_buf, _type);   // send raw chunk
        _pos = 0;
    }

    ~StreamWriter() {
        flush();
    }

private:
    CommandCallback _cb;
    CmdCbType _type;

    char _buf[512];
    size_t _pos;
};
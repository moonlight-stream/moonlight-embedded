#/usr/bin/env python
import sys

def ver2appver(version):
    try:
        versions = map(int, version.split('.'))
        return '%02d.%02d' % tuple(versions[:2])
    except ValueError:
        return '00.00'

if len(sys.argv) < 2:
    version = raw_input("new release version? ")
else:
    version = sys.argv[1]

versions = map(int, version.split('.'))
if len(versions) != 3:
    print("version format must be A.B.C");
    raise SystemExit(1)

# update livearea
with open('sce_sys/livearea/contents/template.xml.format') as f:
    template = f.read()
    with open('sce_sys/livearea/contents/template.xml', 'w') as w:
        w.write(template.format(version=version))

# update cmake version information
lines = []
with open('CMakeLists.txt') as f:
    for line in f.readlines():
        if 'set(VERSION_MAJOR' in line:
            line = 'set(VERSION_MAJOR "%d")\n' % versions[0]
        elif 'set(VERSION_MINOR' in line:
            line = 'set(VERSION_MINOR "%d")\n' % versions[1]
        elif 'set(VERSION_PATCH' in line:
            line = 'set(VERSION_PATCH "%d")\n' % versions[2]
        lines.append(line)
with open('CMakeLists.txt', 'w') as w:
    w.write(''.join(lines))

# update release note
lines = []
groups = []
with open('CHANGELOG.md') as f:
    group = dict(version=None)
    for line in f.readlines():
        if '## Unreleased' in line:
            lines.append('## Unreleased\n\n')
            line = '## %s\n' % version
        lines.append(line)
        if '## ' in line:
            app_ver = ver2appver(line.lstrip('## ').strip())
            if group['version'] != app_ver:
                group = dict(version=app_ver, item=[])
                groups.append(group)
        group['item'].append(line.strip())
with open('CHANGELOG.md', 'w') as w:
    w.write(''.join(lines))

with open('resources/changeinfo.xml', 'w') as w:
    w.write('<?xml version="1.0" encoding="UTF-8"?>\n')
    w.write('<changeinfo>\n')
    pos = w.tell() + 14 # endtag length
    for group in groups:
        entry = ['<changes app_ver="%s"><![CDATA[\n' % group['version'],
                 '<br>\n'.join(group['item']),
                 ']]></changes>\n']
        entry = ''.join(entry)
        if pos + len(entry.encode('utf8')) > 65536: # prevent over 64k
            break
        w.write(entry)
    w.write('</changeinfo>\n')
